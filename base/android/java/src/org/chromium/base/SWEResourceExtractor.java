/*Copyright (c) 2015, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

package org.chromium.base;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Handles extracting the necessary resources bundled in an APK and moving them to a location on
 * the file system accessible from the native code.
 */

public class SWEResourceExtractor {

    private static final String LOGTAG = "SWEResourceExtractor";
    private static final int BUFFER_SIZE = 16 * 1024;

    public static void ExtractLibnetxtResources(Context context) {
        ExtractSWEResources(context, new String[] {"swe_sta.rl"}, "libsta");
        ExtractSWEResources(context, new String[] {"wl.json"}, "libnetxt");
    }

    public static void ExtractSWEResources(Context context, String [] resources, String destDir) {
        try {
            File dataDir = new File(PathUtils.getDataDirectory(context));
            File outputDir = new File(dataDir, destDir);
            if (!outputDir.exists() && !outputDir.mkdirs()) {
                Log.d(LOGTAG, "Unable to create" + destDir + "resources directory!");
                return;
            }

            for (String file : resources) {
                File output = new File(outputDir, file);
                if (output.exists()) {
                    continue;
                }

                InputStream is = null;
                OutputStream os = null;
                try {
                    is = context.getResources().getAssets().open(file);
                    os = new FileOutputStream(output);
                    Log.d(LOGTAG, "Extracting resource " + file);
                    byte[] buffer = new byte[BUFFER_SIZE];
                    int readBytes = 0;
                    while ((readBytes = is.read(buffer, 0, BUFFER_SIZE)) != -1) {
                        os.write(buffer, 0, readBytes);
                    }
                    os.flush();
                } finally {
                    try {
                        if (is != null) {
                            is.close();
                        }
                    } finally {
                        if (os != null) {
                            os.close();
                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.d(LOGTAG, "Skipping unpacking libnetxt resources: " + e.getMessage());
        }
    }
}
