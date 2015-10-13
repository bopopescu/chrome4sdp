/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.chromium.base.resourceFetcher;

import android.util.Log;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.MalformedURLException;
import android.content.Context;
import org.chromium.base.ResourceFetcher;


public class FetcherThread implements Runnable {

    public FetcherThread (String url, Notifiable cb) {
        this.cb = cb;
        this.url = url;
    }

    public void run() {
        try {
            URL url = new URL(this.url);
            HttpURLConnection connection=(HttpURLConnection)url.openConnection();
            connection.setUseCaches(true);
            InputStream is = connection.getInputStream();
            File outDir = new File(ResourceFetcher.getLocalDataDir(), "fetched");
            if (!outDir.exists() && !outDir.mkdirs()){
                Log.d("libnetxt", "ResourceFetcher - FetcherThread - unable to create " + outDir.getAbsolutePath());
                return;
            }
            String filename = url.getFile().substring(url.getFile().lastIndexOf("/") + 1);
            File outFile = new File(outDir, filename);
            if(outFile.exists()) {
                outFile.delete();
            }
            outFile.createNewFile();
            if(!outFile.exists()) {
                Log.d("libnetxt", "ResourceFetcher - FetcherThread - unable to create file: " + outFile.getAbsolutePath());
                return;
            }
            OutputStream os = new FileOutputStream(outFile);
            byte[] buff=new byte[1024];
            int len;
            while((len=is.read(buff))>0) {
                os.write(buff,0,len);
            }
            os.close();
            is.close();
            cb.notify(outFile.getAbsolutePath());
        }
        catch (MalformedURLException e) {
            Log.d("libnetxt", "ResourceFetcher - FetcherThread - Malformed URL Exception");
        }
        catch (IOException e) {
            Log.d("libnetxt", "ResourceFetcher - FetcherThread - IO exception", e);
        }
    }

    private String url;
    private Notifiable cb;
}
