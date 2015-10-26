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

#ifndef Canvas2DLayerAsync_h
#define Canvas2DLayerAsync_h

#include "SkImage.h"
#include "SkCanvasPlayback.h"
#include "base/synchronization/waitable_event.h"
#include "base/message_loop/message_loop_task_runner.h"
#include "platform/geometry/IntSize.h"
#include "public/platform/WebGraphicsContext3D.h"
#include "public/platform/WebGraphicsContext3DProvider.h"
#include "wtf/DoublyLinkedList.h"
#include "wtf/OwnPtr.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/RefPtr.h"

namespace cc {
class TextureLayer;
}

namespace base {
class SingleThreadTaskRunner;
}

namespace blink {
class Canvas2DLayerBridge;
class AsyncCanvasPlayback;

class CommitCallback : public ThreadSafeRefCounted<CommitCallback>
{
public:
    static PassRefPtr<CommitCallback> create(AsyncCanvasPlayback* p)
    {
        return adoptRef(new CommitCallback(p));
    }

    void cancel();
    void run();
private:
    CommitCallback(AsyncCanvasPlayback* p) : m_playback(p) {}
    base::Lock m_lock;
    AsyncCanvasPlayback* m_playback;
};

class AsyncCanvasPlayback
{
public:
    static AsyncCanvasPlayback* create(Canvas2DLayerBridge*, const IntSize&, int);

    ~AsyncCanvasPlayback();

    void threadSafeDestroy();
    SkImage* acquirePlaybackImageSnapshot();
    void releasePlaybackImageSnapshot(unsigned, PassRefPtr<SkImage>);
    bool prepareForImmediateDraw();
    void scheduleCommit();

private:
    AsyncCanvasPlayback(Canvas2DLayerBridge*, IntSize, int msaaSampleCount);
    void createPlaybackBuffers();
    void createPlaybackBuffersOnRenderThread(base::WaitableEvent*);
    void deletePlaybackBuffers();
    void deletePlaybackBuffersOnRenderThread(base::WaitableEvent*);
    void createPlaybackImageSnapshot();
    void releasePlaybackImageSnapshotOnRenderThread(unsigned, PassRefPtr<SkImage>, base::WaitableEvent*);
    void createPlayback();
    void createPlaybackOnRenderThread(base::WaitableEvent*);
    void schedulePlayback();
    void playbackOnRenderThread(base::WaitableEvent* completion);
    void waitForPlaybackTaskComplete();
    blink::WebGraphicsContext3D* context();
    void copyTexture(unsigned, unsigned);

    Canvas2DLayerBridge* m_bridge;
    OwnPtr<blink::WebGraphicsContext3DProvider> m_contextProvider;
    IntSize m_size;
    int m_msaaSampleCount;

    bool m_inImmediateFlushMode;
    bool m_newPlaybackTextureAvailable;
    bool m_inPrepareForImmediateDraw;
    bool m_playbackScheduled;

    RefPtr<SkPicture> m_playbackPicture;

    SkCanvasPlayback* m_playbackFactory;

    unsigned m_playbackFrameCount;
    unsigned m_compositorFrameCount;
    double m_frameRateTime;

    // Synchronization between main thread and Canvas render thread
    base::Lock m_lock;
    base::WaitableEvent m_playbackCompletion;
    base::WaitableEvent m_releaseMailboxCompletion;

    // Number of direct pixel reads/writes on current frame
    unsigned m_directPixelAccessCount;

    // To schedule delayed commit requests
    unsigned m_scheduleCommitDelay;
    scoped_refptr<base::SingleThreadTaskRunner> m_mainThread;

    RefPtr<CommitCallback> m_commitCallback;

    friend class Canvas2DLayerBridge;
};
} //namespace blink
#endif //Canvas2DLayerAsync_h

