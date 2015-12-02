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

import android.content.Context;
import android.util.Log;

import java.lang.reflect.Constructor;

import org.chromium.base.ThreadUtils;

public class WebRefiner {

    private static final String LOGTAG = "WebRefiner";

    /**
     * WebRefiner status flags.
     */
    public static final int STATUS_OK          = 0;
    public static final int STATUS_LOADING     = 1;
    public static final int STATUS_FAILED      = 2;

    /**
     * WebRefiner site/origin-specific permissions.
     */
    public static final int PERMISSION_USE_DEFAULT      = 0;
    public static final int PERMISSION_ENABLE           = 1;
    public static final int PERMISSION_DISABLE          = 2;

    public static class RuleSet {
        /**
         * WebRefiner Ruleset categories.
         */
        public static final int CATEGORY_MDM                      = 1;
        public static final int CATEGORY_ADS                      = 1 << 1;
        public static final int CATEGORY_TRACKERS                 = 1 << 2;
        public static final int CATEGORY_MALWARE_DOMAINS          = 1 << 3;
        /**
         * Add aditional categories below and make sure that the name to id mapping in
         * {@link #GetCategoryIdFromName(String)} is updated with additional
         * names from web_refiner.conf
         * public static final int CATEGORY_USER_01          = 1 << 4;
         * public static final int CATEGORY_USER_02          = 1 << 5;
         */
        public static final int CATEGORY_ALL                      = 0x7FFFFFFF;

        public final String mPath;
        public final int mCategory;
        public final int mPriority;

        /**
         * @param path Absolute file system path where the rule set file is stored.
         * @param category One of the rule set categories listed above (excluding CATEGORY_ALL).
         * @param priority Order of priority that determines the RuleSet precedence.
         *                 Valid values 1-99, 1 - Highest priority, 99 - Lowest priority
         */
        public RuleSet(String path, int category, int priority) {
            mPath = path;
            mCategory = category;
            mPriority = priority;
        }
    }

    public static class MatchedURLInfo {
        /**
         * WebRefiner actions on URLs.
         */
        public static final int ACTION_BLOCKED         = 0;
        public static final int ACTION_WHITELISTED     = 1;

        public final String mUrl;
        /**
         * Resource type of the URL (e.g. Image, Script, Stylesheet, Font, etc...)
         */
        public final String mType;
        /**
         * Raw filter string that matches the URL
         */
        public final String mMatchedFilter;
        /**
         * Ruleset name where the filter belongs
         */
        public final String mMatchedFilterSource;
        /**
         * Ruleset category where the filter belongs (one of the Ruleset categories listed below)
         */
        public final int mMatchedFilterCategory;
        /**
         * Action (listed above) taken on this URL
         */
        public final int mActionTaken;

        public MatchedURLInfo(String url, String type, String filter, String filterSource, int filterSourceCategory, int actionTaken) {
            mUrl = url;
            mType = type;
            mMatchedFilter = filter;
            mMatchedFilterSource = filterSource;
            mMatchedFilterCategory = filterSourceCategory;
            mActionTaken = actionTaken;
        }
    }

    public static class PageInfo {
        public final MatchedURLInfo[] mMatchedURLInfoList;
        /**
         * Total number of URLs (requests) in the page session.
         */
        public final int mTotalUrls;
        public final int mBlockedUrls;
        public final int mWhiteListedUrls;

        public PageInfo(MatchedURLInfo[] list, int totalUrls, int blockedUrls, int whiteListedUrls) {
            mMatchedURLInfoList = list;
            mTotalUrls = totalUrls;
            mBlockedUrls = blockedUrls;
            mWhiteListedUrls = whiteListedUrls;
        }
    }

    public static class FeatureName {
        /**
         * Common product name/label under which this feature exists.
         */
        public final String mProductName;
        /**
         * Actual name/label of the feature.
         */
        public final String mLocalName;
        public FeatureName(String product, String local) {
            mProductName = product;
            mLocalName = local;
        }
    }

    private static WebRefiner sInstance = null;

    /**
     * Should be called only once after the engine initialization is completed.
     * There should be a valid JSON configuration file in your applications
     * assets/web_refiner/web_refiner.conf
     * @return Initialization success/failure.
     */
    static boolean Initialize(Context ctx) {
        try {
            Constructor<?> constructor =
                Class.forName("com.qualcomm.qti.webrefiner.WebRefinerImpl").getConstructor(Context.class);
            sInstance = (WebRefiner)constructor.newInstance(ctx);
        } catch (Exception e) {
            Log.e(LOGTAG, "Initialization failed: " + e.toString());
        }
        return (sInstance != null);
    }

    static public int GetCategoryIdFromName(String category) {
        // TODO: Find a better alternative for hard coded strings.
        if (0 == category.compareTo("MALWARE_DOMAINS")) {
            return  WebRefiner.RuleSet.CATEGORY_MALWARE_DOMAINS;
        } else if (0 == category.compareTo("TRACKERS")) {
            return  WebRefiner.RuleSet.CATEGORY_TRACKERS;
        } else if (0 == category.compareTo("ADS")) {
            return  WebRefiner.RuleSet.CATEGORY_ADS;
        } /*else if (0 == category.compareTo("USER_01")) {
            return  WebRefiner.RuleSet.CATEGORY_USER_01;
        } else if (0 == category.compareTo("USER_02")) {
            return  WebRefiner.RuleSet.CATEGORY_USER_02;
        }*/

        return WebRefiner.RuleSet.CATEGORY_ADS; //Default category;
    }

    /**
     * @return Whether the WebRefiner has been initialized.
     */
    public static boolean isInitialized() {
        ThreadUtils.assertOnUiThread();
        return sInstance != null;
    }

    /**
     * @return The singleton WebRefiner object.
     */
    public static WebRefiner getInstance() {
        ThreadUtils.assertOnUiThread();
        return sInstance;
    }

    /**
     * Returns the WebRefiner's initialization status.
     * @return Status.
     */
    public int getInitializationStatus() { return STATUS_FAILED; }

    /**
     * Master switch to turn on/off WebRefiner.
     * Overrides all the other settings.
     * @param flag True to enable and False to disable WebRefiner.
     */
    public void setEnabled(boolean flag)    {}

    /**
     * Returns the total number URL requests on the ContentViewCore's current page session.
     * @param cvc The ContentViewCore.
     * @return Total number URL requests.
     */
    public int getTotalURLCount(ContentViewCore cvc) { return 0; }

    /**
     * Returns the number URLs blocked on the ContentViewCore's current page session.
     * @param cvc The ContentViewCore.
     * @return Number of URLs blocked.
     */
    public int getBlockedURLCount(ContentViewCore cvc) { return 0; }

    /**
     * Generates PageInfo from the statistics collected by WebRefiner for the given ContentViewCore session.
     * Note: This could be an expensive call.
     * @param cvc The ContentViewCore.
     * @return Detailed info collected by WebRefiner on the ContentViewCore's current page session.
     */
    public PageInfo getPageInfo(ContentViewCore cvc) { return null; }

    /**
     * Adds a new RuleSet.
     * @param ruleSet The new RuleSet to be added.
     * @return True if parameters are correct the rule file exists, false otherwise.
     */
    public boolean addRuleSet(RuleSet ruleSet) { return false; }

    /**
     * Updates an existing RuleSet.
     * @param ruleSet The RuleSet to be updated.
     * @return True if parameters are correct and the rule file exists, false otherwise.
     */
    public boolean updateRuleSet(RuleSet ruleSet) { return false; }

    /**
     * Removes an existing RuleSet.
     * @param ruleSet The RuleSet to be removed.
     * @return True if parameters are correct and the rule file exists, false otherwise.
     */
    public boolean removeRuleSet(RuleSet ruleSet) { return false; }

    /**
     * Enables or disables all the rules by default on all the websites.
     * Can be overridden by {@link #setPermissionForOrigins(String[], boolean)}.
     * @param allow True to enable rules and False to disable rules on all the websites.
     */
    public void setDefaultPermission(boolean allow)    {}

    /**
     * Enables or disables all the rules for the given origins.
     * Overrides {@link #setDefaultPermission(boolean)}.
     * @param origins List of origins .
     * @param allow True to enable and False to disable all the rules for the given origins.
     */
    public void setPermissionForOrigins(String[] origins, boolean allow) {}

    /**
     * Uses the permission set by {@link #setDefaultPermission(boolean)} for the given origins.
     * @param origins List of origins .
     */
    public void useDefaultPermissionForOrigins(String[] origins) {}

    /**
     * Returns the unified and local name (en-us strings) for WebRefiner.
     * @return A valid FeatureName object or null.
     */
    public FeatureName getFeatureName() { return null; }
}
