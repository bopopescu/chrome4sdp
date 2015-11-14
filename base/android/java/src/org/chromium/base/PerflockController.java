// Copyright (c) 2015, The Linux Foundation. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of The Linux Foundation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package org.chromium.base;

import org.chromium.base.CommandLine;

import java.lang.Class;
import java.lang.reflect.Method;
import android.os.Build;
import android.util.Log;

public class PerflockController {

    // Static Constants
    static private final String PERF_MGMT_PLUGIN_NAME = "/system/framework/org.codeaurora.Performance.jar";
    static private final String PERF_MGMT_CLASS_NAME  = "org.codeaurora.Performance";
    // Use different plugin and class values on android M
    static private final String PERF_MGMT_PLUGIN_NAME_M = "/system/framework/QPerformance.jar";
    static private final String PERF_MGMT_CLASS_NAME_M  = "com.qualcomm.qti.Performance";

    static private final String PERF_DISABLED = "disable-perflock";
    static private final String TAG = "PerflockController";
    // Resource constants are defined in Performance.java class in framework/base
    static private final int[] mDefaultResources = { 0x3DFF, 0x2704, 0x2B5F, 0x2C07, 0x2F5A };

    // Static Members
    static private Class<?> mPerfMgmtClassType = null;
    static private Method mAcquireLock = null;
    static private Method mReleaseLock = null;
    static boolean mPerfMgmtLoaded = false;

    // Instance Members
    private int[] mResources = null;
    private int mDurationOfPerflock = 0; //milliseconds
    private Object mPerfMgmtInst = null;

    public PerflockController(int duration) {
        this (duration, mDefaultResources);
    }

    public PerflockController(int duration, int[] resources) {
        LoadPerf();
        mResources = resources;
        mDurationOfPerflock = duration;
        if (mPerfMgmtClassType != null) {
            try {
                mPerfMgmtInst = mPerfMgmtClassType.newInstance();
            } catch (Throwable e) {
                Log.e(TAG, "Object initialization failed: " + e);
            }
        }
    }

    // Dynamically load Performance.jar
    static void LoadPerf() {
        // Only attempt to load Performance.jar once
        if (mPerfMgmtLoaded) {
            return;
        }
        mPerfMgmtLoaded = true;
        if (!getPerfLockEnabled()) {
            return;
        }

        // Specify the correct plugin/class to load
        String perfPluginToLoad;
        String perfClassToLoad;
        // build code for android M is 23
        if (Build.VERSION.SDK_INT >= 23) {
            perfPluginToLoad = PERF_MGMT_PLUGIN_NAME_M;
            perfClassToLoad = PERF_MGMT_CLASS_NAME_M;
        } else {
            perfPluginToLoad = PERF_MGMT_PLUGIN_NAME;
            perfClassToLoad = PERF_MGMT_CLASS_NAME;
        }

        try {
            dalvik.system.PathClassLoader pluginClassLoader =
                new dalvik.system.PathClassLoader(perfPluginToLoad,
                        ClassLoader.getSystemClassLoader());
            mPerfMgmtClassType = pluginClassLoader.loadClass(perfClassToLoad);

            Class[] AcqArgsTypes = new Class[2];
            AcqArgsTypes[0] = int.class;
            AcqArgsTypes[1] = int[].class;
            mAcquireLock = mPerfMgmtClassType.getMethod("perfLockAcquire", AcqArgsTypes);
            if (mAcquireLock == null) {
                Log.e(TAG, "perfLockAcquire method not found");
            }

            Class[] mRelArgsTypes = new Class[0];
            mReleaseLock = mPerfMgmtClassType.getMethod("perfLockRelease", mRelArgsTypes);
            if (mReleaseLock == null) {
                Log.e(TAG, "perfLockRelease method not found");
            }
            Log.e(TAG, "Class initialization succeeded");
        } catch (Throwable e) {
            Log.e(TAG, "Class initialization failed: " + e);
            mPerfMgmtClassType = null;
            mAcquireLock = null;
            mReleaseLock = null;
        }
    }

    public void acquirePerfLock(int duration) {
        if (mPerfMgmtInst != null && mAcquireLock != null) {
            try {
                Object ret = mAcquireLock.invoke(mPerfMgmtInst, duration, mResources);
            } catch (Exception e) {
                Log.e(TAG, "perfLockAcquire failed: " + e);
            }
        }
    }

    public void acquirePerfLock() {
        acquirePerfLock(mDurationOfPerflock);
    }

    public void releasePerfLock() {
        if (mPerfMgmtInst != null && mReleaseLock != null) {
            try {
                Object ret = mReleaseLock.invoke(mPerfMgmtInst);
            } catch (Exception e) {
                Log.e(TAG, "perfLockRelease failed: " + e);
            }
        }
    }

    private static boolean getPerfLockEnabled() {
        // Return true to enable perf lock.
        return !CommandLine.getInstance().hasSwitch(PERF_DISABLED);
    }
}
