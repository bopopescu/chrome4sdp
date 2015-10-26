/*
 *  Copyright (c) 2013, 2015, The Linux Foundation. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *      * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "config.h"

#include "platform/graphics/Canvas2DLayerAsync.h"
#include "platform/graphics/ImageBuffer.h"

#include "GrContext.h"
#include "SkCanvasPlayback.h"
#include "SkDevice.h"
#include "SkSurface.h"
#include "SkPicture.h"
#include "platform/graphics/GraphicsLayer.h"
#include "platform/graphics/GraphicsTypes3D.h"
#include "platform/graphics/Canvas2DLayerBridge.h"
#include "public/platform/Platform.h"
#include "public/platform/WebCompositorSupport.h"
#include "public/platform/WebGraphicsContext3D.h"
#include "public/platform/WebGraphicsContext3DProvider.h"
#include "public/platform/WebThread.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "wtf/ArrayBufferContents.h"
#include "wtf/CurrentTime.h"
#include "wtf/Functional.h"

#undef LOG
#include "base/callback.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "base/threading/thread.h"
#include "cc/base/switches.h"
#include "cc/layers/texture_layer.h"
#include "cc/trees/layer_tree_host.h"
#include "cc/trees/proxy.h"
#if OS(ANDROID)
#include <android/log.h>
#endif

//#define ENABLE_C2DL_DEBUG
#if defined(ENABLE_C2DL_DEBUG) && OS(ANDROID)
#define C2DL_TRACE()        TraceLog tLog(m_bridge, __FUNCTION__)
#define C2DL_DEBUG(...)     __android_log_print(ANDROID_LOG_INFO, "chromium", __VA_ARGS__);
#define C2DL_VERBOSE(...)   __android_log_print(ANDROID_LOG_INFO, "chromium", __VA_ARGS__);
#else
#define C2DL_TRACE()
#define C2DL_DEBUG(...)
#define C2DL_VERBOSE(...)
#endif

#if OS(ANDROID)
#define C2DL_WARNING(...)   __android_log_print(ANDROID_LOG_WARN, "chromium", __VA_ARGS__);
#else
#define C2DL_WARNING(...)
#endif

//In milli seconds
#define MAX_SCHEDULE_COMMIT_DELAY           80
#define MIN_SCHEDULE_COMMIT_DELAY           2

#ifdef ENABLE_C2DL_DEBUG
class TraceLog {
public:
    TraceLog(const void* ptr, const char* funcName)
        : funcName_(funcName)
        , ptr_(ptr)
    {
        C2DL_DEBUG("%p: AsyncCanvasPlayback::%s ENTER", ptr_, funcName_);
    }
    ~TraceLog()
    {
        C2DL_DEBUG("%p: AsyncCanvasPlayback::%s EXIT", ptr_, funcName_);
    }
private:
    const char* funcName_;
    const void* ptr_;
};
#endif

#include <android/log.h>
#define LOG_TEST(...) __android_log_print(ANDROID_LOG_ERROR, "chromium",__VA_ARGS__)

namespace blink {

void CommitCallback::cancel()
{
    base::AutoLock lock(m_lock);
    m_playback = 0;
}

void CommitCallback::run()
{
    base::AutoLock lock(m_lock);
    if (m_playback && refCount() > 1)
        m_playback->scheduleCommit();
    deref();
}

class CanvasRenderThread : public base::Thread
{
public:
    CanvasRenderThread() :base::Thread("canvasRenderThread") { }
    virtual ~CanvasRenderThread() { Stop(); }

    void PostTask(const tracked_objects::Location& from_here, const base::Closure& task)
    {
        message_loop()->PostTask(from_here, task);
    }
};

static CanvasRenderThread* s_renderThread = 0;
CanvasRenderThread* canvasRenderThread()
{
    return s_renderThread;
}

static void initializeRenderThreadIfNeeded()
{
    if (!s_renderThread) {
        s_renderThread = new CanvasRenderThread();
        s_renderThread->Start();
    }
}

AsyncCanvasPlayback* AsyncCanvasPlayback::create(Canvas2DLayerBridge* bridge, const IntSize& size, int msaaSampleCount)
{
    AsyncCanvasPlayback* asyncPlayback = 0;
    if (!size.isEmpty()) {
        asyncPlayback = new AsyncCanvasPlayback(bridge, size, msaaSampleCount);
        if (asyncPlayback->m_playbackFactory) {
            initializeRenderThreadIfNeeded();
            asyncPlayback->createPlaybackBuffers();
            if (!(asyncPlayback->m_playbackFactory->hasPlaybackSurface())) {
                asyncPlayback->threadSafeDestroy();
                asyncPlayback = 0;
            }
        } else {
           asyncPlayback = 0;
        }
    }
    return asyncPlayback;
}

AsyncCanvasPlayback::AsyncCanvasPlayback(Canvas2DLayerBridge* bridge, IntSize size, int msaaSampleCount)
    : m_bridge(bridge)
    , m_size(size)
    , m_msaaSampleCount(msaaSampleCount)
    , m_inImmediateFlushMode(false)
    , m_newPlaybackTextureAvailable(false)
    , m_inPrepareForImmediateDraw(false)
    , m_playbackScheduled(false)
    , m_playbackFrameCount(0)
    , m_compositorFrameCount(0)
    , m_playbackCompletion(true, true)
    , m_releaseMailboxCompletion(true, true)
    , m_directPixelAccessCount(0)
    , m_scheduleCommitDelay(MIN_SCHEDULE_COMMIT_DELAY)
    , m_mainThread(base::ThreadTaskRunnerHandle::Get())
{
    C2DL_DEBUG("AsyncCanvasPlayback::AsyncCanvasPlayback size of canvas width: %d height: %d", m_size.width(), m_size.height());
    m_playbackFactory = SkCanvasPlayback::create(size.width(), size.height(), msaaSampleCount);
}

AsyncCanvasPlayback::~AsyncCanvasPlayback()
{
}

void AsyncCanvasPlayback::threadSafeDestroy()
{
    C2DL_TRACE();
    waitForPlaybackTaskComplete();
    m_releaseMailboxCompletion.Wait();
    deletePlaybackBuffers();
    delete this;
}


void AsyncCanvasPlayback::createPlaybackBuffers()
{
    C2DL_TRACE();
    ASSERT(canvasRenderThread());
    m_contextProvider = adoptPtr(blink::Platform::current()->createCanvasOffscreenGraphicsContext3DProvider());
    if (!m_contextProvider.get())
        return;
    m_playbackCompletion.Reset();
    canvasRenderThread()->PostTask(FROM_HERE,
            base::Bind(&AsyncCanvasPlayback::createPlaybackBuffersOnRenderThread,
                        base::Unretained(this), &(m_playbackCompletion)));
    m_playbackCompletion.Wait();
}

void AsyncCanvasPlayback::createPlaybackBuffersOnRenderThread(base::WaitableEvent* completion)
{
    C2DL_TRACE();
    TRACE_EVENT0("cc", "AsyncCanvasPlayback::createPlaybackBuffersOnRenderThread: contextProvider");
    if (!m_contextProvider->bindToCurrentThread())
        return;

    GrContext* gr = m_contextProvider->grContext();

    if (m_playbackFactory->createPlaybackSurface(gr))
        m_contextProvider->grContext()->resetContext(kTextureBinding_GrGLBackendState);

    m_frameRateTime = WTF::currentTime();
    m_playbackCompletion.Signal();
}

void AsyncCanvasPlayback::deletePlaybackBuffers()
{
    C2DL_TRACE();
    ASSERT(canvasRenderThread());
    m_playbackCompletion.Reset();
    canvasRenderThread()->PostTask(FROM_HERE,
            base::Bind(&AsyncCanvasPlayback::deletePlaybackBuffersOnRenderThread,
                        base::Unretained(this), &(m_playbackCompletion)));
    m_playbackCompletion.Wait();
    if (m_commitCallback.get())
        m_commitCallback->cancel();
    m_commitCallback.clear();
}

void AsyncCanvasPlayback::deletePlaybackBuffersOnRenderThread(base::WaitableEvent* completion)
{
    C2DL_TRACE();
    m_playbackFactory->deletePlaybackSurface();
    completion->Signal();
}

blink::WebGraphicsContext3D* AsyncCanvasPlayback::context()
{
    return m_contextProvider->context3d();
}

void AsyncCanvasPlayback::scheduleCommit()
{
    C2DL_TRACE();
    m_bridge->layer()->invalidate();
}

// Called from Canvas render thread
void AsyncCanvasPlayback::createPlaybackImageSnapshot()
{
    C2DL_TRACE();
    base::AutoLock lock(m_lock);
    m_playbackFactory->createPlaybackImageSnapshot();

    // Because we will be changing the texture binding without going through skia,
    // we must dirty the context.
    m_contextProvider->grContext()->resetContext(kTextureBinding_GrGLBackendState);

    m_playbackFrameCount++;

    // Schedule a delayed commit callback that can be cancelled if a new frame request arrives
    // before the callback is actually fired.
    if (m_commitCallback.get())
        m_commitCallback->cancel();
    m_commitCallback = CommitCallback::create(this);
    m_commitCallback->ref(); // Corresponding deref() is in CommitCallback::run()
    m_mainThread->PostDelayedTask(FROM_HERE,
        base::Bind(&CommitCallback::run, base::Unretained(m_commitCallback.get())),
        base::TimeDelta::FromMilliseconds(m_scheduleCommitDelay));
}

// Called from main thread
SkImage* AsyncCanvasPlayback::acquirePlaybackImageSnapshot()
{
    C2DL_TRACE();
    base::AutoLock lock(m_lock);
    if (m_commitCallback.get())
        m_commitCallback->cancel();
    m_compositorFrameCount++;
    if (m_playbackFrameCount > 15) {
        double time = WTF::currentTime();
        double duration = (time - m_frameRateTime);
        if (duration) {
            // Average time taken to process a frame in compositor
            double avgFrameTime = 1000 * (duration/m_compositorFrameCount);
            if(avgFrameTime > MAX_SCHEDULE_COMMIT_DELAY)
                avgFrameTime = MAX_SCHEDULE_COMMIT_DELAY;
            // Factor in the frame time variations.
            m_scheduleCommitDelay = avgFrameTime * 1.25;
        }
        m_frameRateTime = time;
        m_compositorFrameCount = 0;
        m_playbackFrameCount = 0;
        C2DL_DEBUG("%p: AsyncCanvasPlayback::acquirePlaybackImageSnapshot: m_scheduleCommitDelay = %d", m_bridge, m_scheduleCommitDelay);
    }
    SkImage* playbackImage = m_playbackFactory->acquirePlaybackImageSnapshot();
    m_playbackFactory->releasePlaybackImageSnapshot();
    C2DL_DEBUG("%p: AsyncCanvasPlayback::acquirePlaybackImageSnapshot: textureId = %d", m_bridge, (playbackImage ? playbackImage->getTexture()->getTextureHandle() : 0));
    return playbackImage;
}

void AsyncCanvasPlayback::releasePlaybackImageSnapshot(unsigned syncPoint, PassRefPtr<SkImage> image)
{
    //Post the release task to render thread.
    m_releaseMailboxCompletion.Reset();
    canvasRenderThread()->PostTask(FROM_HERE,
                base::Bind(&AsyncCanvasPlayback::releasePlaybackImageSnapshotOnRenderThread,
                    base::Unretained(this), syncPoint, image, &(m_releaseMailboxCompletion)));
}

void AsyncCanvasPlayback::releasePlaybackImageSnapshotOnRenderThread(unsigned syncPoint, PassRefPtr<SkImage> image, base::WaitableEvent* completion)
{
    C2DL_TRACE();
    RefPtr<SkImage> img(image);
    if (syncPoint)
        context()->waitSyncPoint(syncPoint);

    if (img.get()) {
        C2DL_DEBUG("%p: AsyncCanvasPlayback::releasePlaybackImageSnapshotOnRenderThread: textureId: %d", m_bridge, img->getTexture()->getTextureHandle());
        img->getTexture()->textureParamsModified();
        img.clear();
    }
    completion->Signal();
}

void AsyncCanvasPlayback::copyTexture(unsigned dstTexture, unsigned srcTexture)
{
    blink::WebGraphicsContext3D* webContext = m_bridge->context();
    webContext->copyTextureCHROMIUM(GL_TEXTURE_2D, srcTexture, dstTexture, GL_RGBA, GL_UNSIGNED_BYTE, GL_FALSE, GL_FALSE, GL_FALSE);
    webContext->flush();
}

bool AsyncCanvasPlayback::prepareForImmediateDraw()
{
    if (m_inPrepareForImmediateDraw)
        return false;
    C2DL_TRACE();
    bool didTextureCopy = false;
    m_inPrepareForImmediateDraw = true;
    m_directPixelAccessCount++;
    m_inImmediateFlushMode = true;
    if (m_newPlaybackTextureAvailable) {
        C2DL_DEBUG("AsyncCanvasPlayback:: prepareForImmediateDraw waitForPlaybackTaskComplete");
        waitForPlaybackTaskComplete();
        m_newPlaybackTextureAvailable = false;
        GrRenderTarget* src = m_playbackFactory->getPlaybackTarget();
        m_playbackFactory->flush();
        GrRenderTarget* dst = m_bridge->canvas()->getTopDevice()->accessRenderTarget();
        m_bridge->canvas()->flush();
        C2DL_DEBUG("AsyncCanvasPlayback:: prepareForImmediateDraw waitForPlaybackTaskComplete complete src: %p dst: %p", src, dst);
        if (src && dst) {
            unsigned playbackTexture = src->asTexture()->getTextureHandle();
            unsigned immediateTexture = dst->asTexture()->getTextureHandle();
            C2DL_DEBUG("AsyncCanvasPlayback:: copyTexture using playback texturehandle %u %u", playbackTexture, immediateTexture);
            copyTexture(immediateTexture, playbackTexture);
            didTextureCopy = true;
        }
    }
    //Cancel any commits that were scheduled by the async thread
    if (m_commitCallback.get())
        m_commitCallback->cancel();
    m_commitCallback.clear();
    m_inPrepareForImmediateDraw = false;
    C2DL_DEBUG("%p: AsyncCanvasPlayback::prepareForImmediateDraw: directDrawCount = %d, didTextureCopy = %d", m_bridge, m_directPixelAccessCount, didTextureCopy);
    return didTextureCopy;
}

void AsyncCanvasPlayback::waitForPlaybackTaskComplete()
{
    C2DL_TRACE();
    m_playbackCompletion.Wait();
}

void AsyncCanvasPlayback::schedulePlayback()
{
    C2DL_TRACE();
    TRACE_EVENT0("cc", "AsyncCanvasPlayback::schedulePlayback");
    ASSERT(canvasRenderThread());
    if (m_bridge->m_haveRecordedDrawCommands && m_playbackFactory->hasPlaybackSurface()) {

        // Make sure canvas render thread is finished with the previous frame.
        waitForPlaybackTaskComplete();

        m_playbackPicture.release();
        RefPtr<SkPicture> picture(m_bridge->recording());
        m_playbackPicture = picture.release();

        m_newPlaybackTextureAvailable = true;
        m_playbackScheduled = true;

        // Do the manual reset here
        m_playbackCompletion.Reset();

        //Post the flush task to render thread.
        canvasRenderThread()->PostTask(FROM_HERE,
            base::Bind(&AsyncCanvasPlayback::playbackOnRenderThread, base::Unretained(this), &(m_playbackCompletion)));

    }
}

void AsyncCanvasPlayback::playbackOnRenderThread(base::WaitableEvent* completion)
{
    C2DL_TRACE();
    TRACE_EVENT0("cc", "AsyncCanvasPlayback::playbackOnRenderThread");
    blink::WebGraphicsContext3D* canvasContext = context();
    m_playbackFactory->playback(m_playbackPicture.get());
    canvasContext->flush();
    createPlaybackImageSnapshot();
    completion->Signal();
}

}

