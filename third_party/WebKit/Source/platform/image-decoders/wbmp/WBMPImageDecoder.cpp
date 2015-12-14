/*
 * Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
 * Copyright (c) 2008, 2009, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"
#include "platform/image-decoders/wbmp/WBMPImageDecoder.h"
#include "platform/image-decoders/wbmp/WBMPImageReader.h"
#include "wtf/PassOwnPtr.h"

namespace blink {

WBMPImageDecoder::WBMPImageDecoder(
    AlphaOption alphaOption,
    GammaAndColorProfileOption gammaAndColorProfileOption,
    size_t maxDecodedBytes)
    : ImageDecoder(alphaOption, gammaAndColorProfileOption, maxDecodedBytes)
    , m_width(0)
    , m_height(0)
{
}

void WBMPImageDecoder::onSetData(SharedBuffer* data)
{
    if (failed())
        return;

    if (m_reader)
        m_reader->setData(data);
}

bool WBMPImageDecoder::setFailed()
{
    m_reader.clear();
    return ImageDecoder::setFailed();
}

void WBMPImageDecoder::decode(bool onlySize)
{
    if (failed())
        return;

    size_t imgDataOffset = 0;
    if (!processFileHeader(imgDataOffset)) {
        if (isAllDataReceived()) {
            setFailed();
        }
        return;
    }

    if (onlySize)
        return;

    if (!m_reader) {
        m_reader = adoptPtr(new WBMPImageReader(this, m_width, m_height, imgDataOffset));
        m_reader->setData(m_data.get());
    }

    if (!m_frameBufferCache.isEmpty())
        m_reader->setBuffer(&m_frameBufferCache.first());

    if (!m_reader->decodeWBMP()) {
        if (isAllDataReceived()) {
            setFailed();
        }
        return;
    }

    if (!failed()) {
        // If we're done decoding the image, we don't need the WBMPImageReader
        // anymore.  (If we failed, |m_reader| has already been cleared.)
        if (!m_frameBufferCache.isEmpty() && (m_frameBufferCache.first().status() == ImageFrame::FrameComplete)) {
            m_reader.clear();
        }
    }
}

long readVarInt(SharedBuffer * buffer, size_t &cursor)
{
    ASSERT(cursor >= 0);
    ASSERT(cursor < buffer->size());

    int quantity = 0;

    do {
        if (quantity >= (1<<23)) {
            // overflow (something is wrong)
            quantity = -1;
            break;
        }
        quantity <<= 7;
        quantity += (buffer->data()[cursor] & 0x7f);
    } while (buffer->data()[cursor++] & 0x80);

    return quantity;
}

bool WBMPImageDecoder::processFileHeader(size_t& imgDataOffset)
{
    size_t decodedOffset = 0;
    long type;
    long fixedHeader;

    type = readVarInt(m_data.get(), decodedOffset);

    // We only support type 0 for now.  We've never seen anything else.
    if (type != 0) {
        return setFailed();
    }

    fixedHeader = m_data->data()[decodedOffset++];

    // The spec says that bits 6:5 can legally be any value but that
    // the rest of the bits *must* be 0 for a type 0 WBMP image.
    if (fixedHeader & ~0x60) {
        return setFailed();
    }

    m_width = readVarInt(m_data.get(), decodedOffset);
    if ((m_width < 0) || (m_width >= (1 << 16)))
        return setFailed();

    m_height = readVarInt(m_data.get(), decodedOffset);
    if ((m_height < 0) || (m_height >= (1 << 16)))
        return setFailed();

    if (!setSize(m_width, m_height))
        return setFailed();

    imgDataOffset = decodedOffset;

    return true;
}

} // namespace blink
