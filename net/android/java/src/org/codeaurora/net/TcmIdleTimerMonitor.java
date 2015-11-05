/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

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

package org.codeaurora.net;
import dalvik.system.PathClassLoader;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import android.content.Context;
import android.util.Log;
import org.codeaurora.net.NetworkServices;
import com.quicinc.tcmiface.DpmTcmIface;

/**
 * @hide
 */
public class TcmIdleTimerMonitor implements DpmTcmIface {
  private static Object tcmClient = null;
  private static Method mTcmRegisterMethod = null;
  private static Object lockObj = new Object();
  Object result = null;
  public static String TAG= "libnetxt";

  /** @hide */
  public TcmIdleTimerMonitor(Context ctx) {
  synchronized(lockObj) {
    try {
      if (mTcmRegisterMethod == null || tcmClient == null) {
        //load tcm if not already loaded
        PathClassLoader tcmClassLoader =
          new PathClassLoader("/system/framework/tcmclient.jar",
              ClassLoader.getSystemClassLoader());
        Class<?> tcmClass = tcmClassLoader.loadClass("com.qti.tcmclient.DpmTcmClient");
        Method mGetTcmMethod = tcmClass.getDeclaredMethod("getInstance");
        tcmClient = mGetTcmMethod.invoke(null);
        mTcmRegisterMethod = tcmClass.getDeclaredMethod("registerTcmMonitor", DpmTcmIface.class);
      }
      if (mTcmRegisterMethod != null && tcmClient != null) {
        result = mTcmRegisterMethod.invoke(tcmClient, this);
      }
    } catch (ClassNotFoundException e) {
      //Ignore ClassNotFound Exception
      Log.d(TAG, "DpmTcmClient: Class not found. Continue without it");
    } catch (Exception e) {
      Log.w(TAG,"TCM tcmclient load failed: " + e);
    }

    if(mTcmRegisterMethod!=null)
      Log.i(TAG, "TCM class loaded: TRUE");
    else
      Log.i(TAG, "TCM class loaded: FALSE");
    }// sync
  }// TcmIdleTimerMonitor ctor


  /** @override */
  public void OnCloseIdleConn(){
    Log.i(TAG, "TCM OnCloseIdleConn");
    NetworkServices.OnCloseIdleConnections();
  }
} // class TcmIdleTimerMonitor
