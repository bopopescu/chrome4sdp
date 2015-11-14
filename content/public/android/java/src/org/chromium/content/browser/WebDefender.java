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

package org.chromium.content.browser;

import org.chromium.base.ThreadUtils;

public class WebDefender {

    public static class TrackerDomain {
        /**
         * Protective action on this domain.
         */
        public static final int PROTECTIVE_ACTION_UNBLOCK                 = 0; // GREEN
        public static final int PROTECTIVE_ACTION_BLOCK_COOKIES           = 1; // YELLOW
        public static final int PROTECTIVE_ACTION_BLOCK_URL               = 2; // RED

        /**
         * Tracking methods employed by this domain.
         */
        public static final int TRACKING_METHOD_NONE                        = 0;
        public static final int TRACKING_METHOD_HTTP_COOKIES                = 1;
        public static final int TRACKING_METHOD_HTML5_LOCAL_STORAGE         = 1 << 1;
        public static final int TRACKING_METHOD_CANVAS_FINGERPRINT          = 1 << 2;
        /* Note: More methods to follow */


        public final String mName;
        public final int mProtectiveAction;
        public final int mTrackingMethods;
        public final boolean mPotentialTracker;

        /**
         * @param name Domain name.
         * @param action Protective action on this domain.
         * @param trackingMethods One or more tracking methods employed by this domain.
         * @param potentialTracker Whether this domain is a potential tracker.
         */
        public TrackerDomain(String name, int action, int trackingMethods, boolean potentialTracker) {
            mName = name;
            mProtectiveAction = action;
            mTrackingMethods = trackingMethods;
            mPotentialTracker = potentialTracker;
        }

        public TrackerDomain(String name, int action) {
            this(name, action, TRACKING_METHOD_NONE, false);
        }

        public TrackerDomain(String name) {
            this(name, PROTECTIVE_ACTION_UNBLOCK, TRACKING_METHOD_NONE, false);
        }
    }

    public static class ProtectionStatus {
        /**
         * List of tracker domains identified on a given web page.
         */
        public final TrackerDomain[] mTrackerDomains;
        /**
         * Flag to indicate whether the tracking protection is enabled on a given web page.
         */
        public final boolean mTrackingProtectionEnabled;

        public ProtectionStatus(TrackerDomain[] list, boolean flag) {
            mTrackerDomains = list;
            mTrackingProtectionEnabled = flag;
        }
    }

    private static WebDefender sInstance = null;

    /**
     * Called from WebDefender internal implementation on WebRefiner's initialization.
     * @param instance WebDefender instance available to the application.
     */
    public static void setInstance(WebDefender instance) {
        ThreadUtils.assertOnUiThread();
        sInstance = instance;
    }

    /**
     * A convenient method to check whether the WebDefender is initialized and set successfully
     * from its internal implementation.
     * @return Whether the WebDefender has been initialized.
     */
    public static boolean isInitialized() {
        ThreadUtils.assertOnUiThread();
        return sInstance != null;
    }

    /**
     * @return The singleton WebDefender object. Could be null if WebRefiner initialization is failed.
     */
    public static WebDefender getInstance() {
        ThreadUtils.assertOnUiThread();
        return sInstance;
    }

    /**
     * Generates ProtectionStatus from the statistics collected by WebDefender for the given ContentViewCore session.
     * @param cvc The ContentViewCore.
     * @return Protection status info collected by WebDefender on the ContentViewCore's current page session.
     */
    public ProtectionStatus getProtectionStatus(ContentViewCore cvc) { return null; }

    /**
     * Enables or disables tracking protection by default on all the websites.
     * Can be overridden by {@link #setPermissionForOrigins(String[], boolean)}.
     * @param allow True to enable and False to disable tracking protection on all the websites.
     */
    public void setDefaultPermission(boolean allow) {}

    /**
     * Enables or disables tracking protection for the given origins.
     * Overrides {@link #setDefaultPermission(boolean)}.
     * @param origins List of origins .
     * @param allow True to enable and False to disable tracking protection for the given origins.
     */
    public void setPermissionForOrigins(String[] origins, boolean allow) {}

    /**
     * Overrides the permission set by {@link #setPermissionForOrigins(String[] origins, boolean allow)}
     * with the default permission set by {@link #setDefaultPermission(boolean)} for the given origins.
     * @param origins List of origins .
     */
    public void useDefaultPermissionForOrigins(String[] origins) {}

    /**
     * Overrides WebDefenders default protective action on the given tracker domains.
     * This override applies globally
     * @param trackerDomains List of tracker domains with their corresponding protective actions.
     */
    public void overrideProtectiveActionsForTrackerDomains(TrackerDomain[] trackerDomains) {}

    /**
     * Resets user overridden protective actions on the given tracker domains to WebDefender's
     * default actions computed internally.
     * This reset applies globally
     * @param trackerDomains List of tracker domains.
     */
    public void resetProtectiveActionsForTrackerDomains(TrackerDomain[] trackerDomains) {}

}
