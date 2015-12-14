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
#include "platform/image-decoders/wbmp/WBMPImageReader.h"


namespace blink {

WBMPImageReader::WBMPImageReader(ImageDecoder* parent, int width, int height, size_t imgDataOffset)
    : m_parent(parent)
    , m_width(width)
    , m_height(height)
    , m_buffer(0)
    , m_imgDataOffset(imgDataOffset)
{
}

bool WBMPImageReader::decodeWBMP()
{
    // Initialize the framebuffer if needed.
    ASSERT(m_buffer);  // Parent should set this before asking us to decode!
    if (m_buffer->status() == ImageFrame::FrameEmpty) {
        if (!m_buffer->setSize(m_width, m_height))
            return m_parent->setFailed(); // Unable to allocate.
        m_buffer->setStatus(ImageFrame::FramePartial);
        // setSize() calls eraseARGB(), which resets the alpha flag, so we force
        // it back to false here.  We'll set it true below in all cases where
        // these 0s could actually show through.
        m_buffer->setHasAlpha(false);

        // For WBMPs, the frame always fills the entire image.
        m_buffer->setOriginalFrameRect(IntRect(IntPoint(), m_parent->size()));
    }

    // Decode the data
    const ProcessingResult result = processData();
    if (result != Success)
        return (result == Failure) ? m_parent->setFailed() : false;

    // Done!
    m_buffer->setStatus(ImageFrame::FrameComplete);
    return true;
}

WBMPImageReader::ProcessingResult WBMPImageReader::processData()
{
    long widthInBytesIn = (m_width + 7) >> 3;  // byte padded
    long numBytes = m_height * widthInBytesIn;

    if ((m_data->size() - m_imgDataOffset) < (size_t)numBytes) {
        return InsufficientData;
    }

    #define setWHITE() setRGBA(0xff, 0xff, 0xff, 0xff /*alpha*/)
    #define setBLACK() setRGBA(0x00, 0x00, 0x00, 0xff /*alpha*/)

    int cursor = m_imgDataOffset;

    for (int y = 0; y < m_parent->size().height(); y++) {
        char pixels = 0;
        int width = m_width;

        while (width >= 8) {
            pixels = m_data->data()[cursor++];
            for (unsigned char bitPosition = 0x80; bitPosition; bitPosition >>= 1) {
                if (pixels & bitPosition) {
                    setWHITE();
                } else {
                    setBLACK();
                }
            }
            width -= 8;
        }

        if (width)
            pixels = m_data->data()[cursor++];

        for (unsigned char bitPosition = 0x80; width && bitPosition; bitPosition >>= 1) {
            if (pixels & bitPosition) {
                setWHITE();
            } else {
                setBLACK();
            }
            width--;
        }
        moveBufferToNextRow();
    }

    // Finished decoding whole image.
    return Success;
}

void WBMPImageReader::moveBufferToNextRow()
{
    m_coord.move(-m_coord.x(), 1);
}
} // namespace blink
