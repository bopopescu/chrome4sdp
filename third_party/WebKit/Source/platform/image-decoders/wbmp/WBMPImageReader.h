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

#ifndef WBMPImageReader_h
#define WBMPImageReader_h

#include <stdint.h>
#include "platform/image-decoders/ImageDecoder.h"
#include "wtf/CPU.h"

namespace blink {

    // This class decodes a WBMP image
    class WBMPImageReader {
        WTF_MAKE_FAST_ALLOCATED(WBMPImageReader);
    public:
        WBMPImageReader(ImageDecoder* parent, int width, int height, size_t imgDataOffset);

        void setBuffer(ImageFrame* buffer) { m_buffer = buffer; }
        void setData(SharedBuffer* data) { m_data = data; }

        // Does the actual decoding.
        bool decodeWBMP();

    private:
        enum ProcessingResult {
            Success,
            Failure,
            InsufficientData,
        };

        // Resets the relevant local variables to start drawing at the left edge
        // of the "next" row, where "next" is above or below the current row
        // depending on the value of |m_isTopDown|.
        void moveBufferToNextRow();

        // The decoder that owns us.
        ImageDecoder* m_parent;

        int m_width;
        int m_height;

        // Processes the image data.  Expects |m_coord| to point at the beginning
        // of the next row to be decoded.  Returns InsufficientData if there wasn't
        // enough data to decode the whole image.
        //
        // This function returns a ProcessingResult instead of a bool so that it
        // can avoid calling m_parent->setFailed(), which could lead to memory
        // corruption since that will delete |this| but some callers still want
        // to access member variables after this returns.
        ProcessingResult processData();

        inline void setRGBA(unsigned red, unsigned green, unsigned blue, unsigned alpha)
        {
            m_buffer->setRGBA(m_coord.x(), m_coord.y(), red, green, blue, alpha);
            m_coord.move(1, 0);
        }

        // The destination for the pixel data.
        ImageFrame* m_buffer;

        // The file to decode.
        RefPtr<SharedBuffer> m_data;

        // The file offset at which the actual image bits start.
        size_t m_imgDataOffset;

        // The coordinate to which we've decoded the image.
        IntPoint m_coord;
    };

} // namespace blink

#endif
