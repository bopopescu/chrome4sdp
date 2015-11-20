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

package org.chromium.chrome.browser.preferences.website;

import android.app.Activity;
import android.content.res.ColorStateList;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;

import org.chromium.base.ApplicationStatus;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ContentSettingsType;
import org.chromium.chrome.browser.UrlUtilities;
import org.chromium.chrome.browser.document.BrandColorUtils;
import org.chromium.chrome.browser.favicon.FaviconHelper;
import org.chromium.chrome.browser.preferences.PrefServiceBridge;
import org.chromium.chrome.browser.ssl.ConnectionSecurityLevel;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.widget.TintedImageView;
import org.chromium.content.browser.WebDefender;
import org.chromium.content.browser.WebRefiner;

import java.io.ByteArrayOutputStream;
import java.util.EnumMap;
import java.util.Formatter;
import java.util.Map;

public class BrowserSingleWebsitePreferences extends SingleWebsitePreferences {

    public static final String EXTRA_SECURITY_CERT_LEVEL = "org.chromium.chrome.preferences." +
            "website_security_cert_level";
    public static final String EXTRA_FAVICON = "org.chromium.chrome.preferences.favicon";
    public static final String EXTRA_WEB_REFINER_ADS_INFO = "website_refiner_ads_info";
    public static final String EXTRA_WEB_REFINER_TRACKER_INFO = "website_refiner_tracker_info";
    public static final String EXTRA_WEB_REFINER_MALWARE_INFO = "website_refiner_malware_info";
    public static final String EXTRA_INCOGNITO = "website_incognito";

    private int mSecurityLevel = -1;
    private int mSiteColor = -1;
    private String mWebRefinerMessages;
    private boolean mIsIncognito;

    private SiteSecurityViewFactory mSecurityViews;
    private Preference mSecurityInfoPrefs;
    private Switch mWebRefinerSwitch;
    private Switch mWebDefenderSwitch;
    private WebDefender.ProtectionStatus mWebDefenderStatus;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        mSecurityViews = new SiteSecurityViewFactory();
        Bundle arguments = getArguments();
        if (arguments != null) {
            setupWebRefinerInformation(arguments);
            mSecurityLevel = arguments.getInt(EXTRA_SECURITY_CERT_LEVEL);
            mIsIncognito = arguments.getBoolean(EXTRA_INCOGNITO);
            updateSecurityInfo();
            WebDefenderPreferenceHandler.StatusParcel parcel = arguments.getParcelable(
                    WebDefenderDetailsPreferences.EXTRA_WEBDEFENDER_PARCEL);
            if (parcel != null)
                mWebDefenderStatus = parcel.getStatus();
        }
        super.onActivityCreated(savedInstanceState);
    }

    private void updateSecurityInfo() {
        switch (mSecurityLevel) {
            case ConnectionSecurityLevel.NONE:
                break;
            case ConnectionSecurityLevel.SECURITY_WARNING:
                mSecurityViews.appendText(SiteSecurityViewFactory.ViewType.WARNING,
                        getResources().getString(R.string.ssl_warning));
                break;
            case ConnectionSecurityLevel.SECURITY_ERROR:
                mSecurityViews.appendText(SiteSecurityViewFactory.ViewType.ERROR,
                        getResources().getString(R.string.ssl_error));
                break;
            case ConnectionSecurityLevel.SECURE:
            case ConnectionSecurityLevel.EV_SECURE:
                mSecurityViews.appendText(SiteSecurityViewFactory.ViewType.INFO,
                        getResources().getString(R.string.ssl_secure));
                break;
            default:
                break;
        }
    }

    @Override
    protected Drawable getEnabledIcon(int contentType) {
        if (mSiteColor == -1) return super.getEnabledIcon(contentType);
        Drawable icon = getResources().getDrawable(ContentSettingsResources.getIcon(contentType));

        if (contentType == ContentSettingsType.CONTENT_SETTINGS_TYPE_WEBDEFENDER ||
                contentType == ContentSettingsType.CONTENT_SETTINGS_TYPE_WEBREFINER)
            return icon;

        icon.mutate();
        icon.setColorFilter(mSiteColor, PorterDuff.Mode.SRC_IN);
        return icon;
    }

    @Override
    protected void setUpListPreference(Preference preference, ContentSetting value) {
        if (mIsIncognito) {
            getPreferenceScreen().removePreference(preference);
        } else {
            super.setUpListPreference(preference, value);
        }
    }

    @Override
    protected void displayExtraSitePermissions(Preference preference) {
        super.displayExtraSitePermissions(preference);

        if (PREF_WEBREFINER_PERMISSION.equals(preference.getKey())) {
            if (!WebRefinerPreferenceHandler.isInitialized()) {
                getPreferenceScreen().removePreference(preference);
                Preference category = findPreference("webrefiner_title");
                getPreferenceScreen().removePreference(category);
                return;
            }

            preference.setOnPreferenceClickListener(this);
            setTextForPreference(preference,
                    getWebRefinerPermission() ? ContentSetting.ALLOW : ContentSetting.BLOCK);

            preference.setIcon(
                    getEnabledIcon(ContentSettingsType.CONTENT_SETTINGS_TYPE_WEBREFINER));
        } else if (PREF_WEBDEFENDER_PERMISSION.equals(preference.getKey())) {
            if (!WebDefenderPreferenceHandler.isInitialized()) {
                getPreferenceScreen().removePreference(preference);
                Preference category = findPreference("webdefender_title");
                getPreferenceScreen().removePreference(category);
                return;
            }

            preference.setOnPreferenceClickListener(this);
            setTextForPreference(preference,
                    getWebDefenderPermission() ? ContentSetting.ALLOW : ContentSetting.BLOCK);

            preference.setIcon(
                    getEnabledIcon(ContentSettingsType.CONTENT_SETTINGS_TYPE_WEBDEFENDER));
        } else if (preference.getKey().equals("webdefender_details")) {
            preference.setOnPreferenceClickListener(this);
        }
    }

    @Override
    protected int getContentSettingsTypeFromPreferenceKey(String preferenceKey) {
        int type = super.getContentSettingsTypeFromPreferenceKey(preferenceKey);
        if (type != 0)
            return type;

        switch (preferenceKey) {
            case PREF_WEBREFINER_PERMISSION:
                return ContentSettingsType.CONTENT_SETTINGS_TYPE_WEBREFINER;
            case PREF_WEBDEFENDER_PERMISSION:
                return ContentSettingsType.CONTENT_SETTINGS_TYPE_WEBDEFENDER;
            default:
                return 0;
        }
    }

    private void setTextForPreference(Preference preference, ContentSetting value) {
        if (PREF_WEBREFINER_PERMISSION.equals(preference.getKey())) {
            preference.setTitle(value == ContentSetting.ALLOW
                    ? (mWebRefinerMessages != null) ?
                    mWebRefinerMessages
                    : getResources().getString(R.string.website_settings_webrefiner_enabled)
                    : getResources().getString(R.string.website_settings_webrefiner_disabled));
        } else if (PREF_WEBDEFENDER_PERMISSION.equals(preference.getKey())) {
            int count = 0;
            if (mWebDefenderStatus != null) {
                count = mWebDefenderStatus.mTrackerDomains.length;
            }

            preference.setTitle(value == ContentSetting.ALLOW
                    ? (count > 0) ?
                    WebDefenderDetailsPreferences.getOverviewMessage(getResources(), count)
                    : getResources().getString(R.string.website_settings_webdefender_enabled)
                    : getResources().getString(R.string.website_settings_webdefender_disabled));
            }
        }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (!PREF_WEBREFINER_PERMISSION.equals(preference.getKey()) &&
                !PREF_WEBDEFENDER_PERMISSION.equals(preference.getKey())) {
            ContentSetting permission = ContentSetting.fromString((String) newValue);
            createPermissionInfo(preference, permission);
            requestReloadForOrigin();
            preference.setSummary("%s");
            super.onPreferenceChange(preference, newValue);
            updateSecurityPreferenceVisibility();
        }
        return true;
    }

    @Override
    protected void resetSite() {
        requestReloadForOrigin();
        if (mIsIncognito) {
            WebRefinerPreferenceHandler.clearIncognitoOrigin(mSite.getAddress().getOrigin());
            WebDefenderPreferenceHandler.clearIncognitoOrigin(mSite.getAddress().getOrigin());
            getActivity().finish();
            return;
        }

        mSite.setWebRefinerPermission(null);
        mSite.setWebDefenderPermission(null);
        PreferenceScreen screen = getPreferenceScreen();
        Preference pref = findPreference("webrefiner_title");
        if (pref != null) screen.removePreference(pref);
        pref = findPreference("webdefender_title");
        if (pref != null) screen.removePreference(pref);

        super.resetSite();
    }

    @Override
    protected void requestReloadForOrigin() {
        String origin = (mSiteAddress != null)
                ? mSiteAddress.getOrigin() : mSite.getAddress().getOrigin();
        PrefServiceBridge.getInstance().addOriginForReload(origin);
    }

    @Override
    protected void updateSecurityPreferenceVisibility() {
        //Now update any warnings we want to show to the user
        String securityWarnings = "";

        if (ContentSetting.ALLOW.equals(mSite.getGeolocationPermission())) {
            securityWarnings += (securityWarnings.isEmpty()) ? getResources().getString(
                    ContentSettingsResources.getTitle(ContentSettingsType.
                            CONTENT_SETTINGS_TYPE_GEOLOCATION))
                    : "";
        }

        if (!securityWarnings.isEmpty()) {
            securityWarnings += " ";
            securityWarnings += getResources().getString(R.string.page_info_permission_allowed);
            mSecurityViews.setText(SiteSecurityViewFactory.ViewType.WARNING, securityWarnings);
        } else {
            mSecurityViews.clearText(SiteSecurityViewFactory.ViewType.WARNING);
        }


        if (mSecurityViews.mbEmpty) {
            PreferenceScreen screen = getPreferenceScreen();

            if (mSecurityInfoPrefs == null) {
                mSecurityInfoPrefs = findPreference("site_security_info_title");
            }

            if (mSecurityInfoPrefs != null && screen != null) {
                screen.removePreference(mSecurityInfoPrefs);
            }
        } else {
            PreferenceScreen screen = getPreferenceScreen();

            Preference pref = findPreference("site_security_info_title");
            if (pref == null && mSecurityInfoPrefs != null) {
                screen.addPreference(mSecurityInfoPrefs);
            }
        }
    }

    @Override
    protected boolean hasUsagePreferences() {
        if (mIsIncognito) {
            Preference preference = findPreference(PREF_CLEAR_DATA);
            if (preference != null) {
                getPreferenceScreen().removePreference(preference);
            }
        }
        return super.hasUsagePreferences();
    }


    @Override
    protected void updateSummary(Preference preference, int contentType, ContentSetting value){
        if (ContentSettingsResources.getDefaultEnabledValue(contentType).equals(ContentSetting.ASK))
        {
            // MEDIA_STREAM permissions like Mic and Camera only have an "ASK" and "BLOCK" option.
            if (preference.getKey().equals(PREF_CAMERA_CAPTURE_PERMISSION) ||
                    preference.getKey().equals(PREF_MIC_CAPTURE_PERMISSION)) {
                CharSequence[] entries = ((ListPreference)preference).getEntries();
                entries[0] = getResources().getString(R.string.website_settings_category_ask);
                ((ListPreference)preference).setEntries(entries);
            }

            if (value == ContentSetting.ASK)
                preference.setSummary(R.string.website_settings_category_ask);
        }
    }

    private static class SiteSecurityViewFactory {
        private class SiteSecurityView {
            private TextView mTextView;
            private String mDisplayText;

            public SiteSecurityView(View parent, int resId, String text) {
                mTextView = (TextView) parent.findViewById(resId);
                mTextView.setText(text);
                mDisplayText = text;
                updateVisibility();
            }

            private void updateVisibility() {
                if (TextUtils.isEmpty(mDisplayText)) {
                    mTextView.setVisibility(View.GONE);
                } else {
                    mTextView.setVisibility(View.VISIBLE);
                }
            }

            public void setText(String text) {
                mDisplayText = text;
                mTextView.setText(mDisplayText);
                updateVisibility();
            }

            public void clearText() {
                mDisplayText = null;
                updateVisibility();
            }
        }

        public enum ViewType{
            ERROR,
            WARNING,
            INFO
        };

        private Map<ViewType, SiteSecurityView> mViews =
                new EnumMap<ViewType, SiteSecurityView>(ViewType.class);
        private Map<ViewType, String> mTexts = new EnumMap<ViewType, String>(ViewType.class);

        private boolean mbEmpty = true;

        public void setText(ViewType type, String text) {
            mTexts.put(type, text);

            SiteSecurityView view = mViews.get(type);
            if (view != null) {
                view.setText(text);
            }

            mbEmpty = false;
        }

        public void appendText(ViewType type, String text) {
            String newText = mTexts.get(type);
            if (newText != null)
                newText += " " + text;
            else
                newText = text;

            mTexts.put(type, newText);

            SiteSecurityView view = mViews.get(type);
            if (view != null) {
                view.setText(newText);
            }

            mbEmpty = false;
        }

        public void clearText(ViewType type) {
            mTexts.remove(type);

            SiteSecurityView view = mViews.get(type);
            if (view != null) {
                view.clearText();
            }

            boolean empty = true;
            for (Map.Entry<ViewType, String> entry: mTexts.entrySet()) {
                if (!entry.getValue().isEmpty()) {
                    empty = false;
                }
            }
            mbEmpty = empty;
        }

        public void setResource(ViewType type, View parent, int resId) {
            String text = mTexts.get(type);
            mViews.remove(type);
            mViews.put(type, new SiteSecurityView(parent, resId, text));
        }
    }

    /**
     * Get the global setting value for the given prefernece.
     * @param preference The ListPreference to be checked.
     */
    @Override
    protected ContentSetting getGlobalDefaultPermission(Preference preference) {
        String preferenceKey = preference.getKey();
        int contentType = getContentSettingsTypeFromPreferenceKey(preferenceKey);
        ContentSetting defaultValue;
        boolean isEnabled;

        if (PREF_CAMERA_CAPTURE_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isCameraEnabled();
        } else if (PREF_COOKIES_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isAcceptCookiesEnabled();
        } else if (PREF_FULLSCREEN_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isFullscreenAllowed();
        } else if (PREF_JAVASCRIPT_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().javaScriptEnabled();
        } else if (PREF_LOCATION_ACCESS.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isAllowLocationEnabled();
        } else if (PREF_MIC_CAPTURE_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isMicEnabled();
        } else if (PREF_POPUP_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().popupsEnabled();
        } else if (PREF_PROTECTED_MEDIA_IDENTIFIER_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isProtectedMediaIdentifierEnabled();
        } else if (PREF_PUSH_NOTIFICATIONS_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isPushNotificationsEnabled();
        } else if (PREF_WEBREFINER_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isWebRefinerEnabled();
        } else if (PREF_WEBDEFENDER_PERMISSION.equals(preferenceKey)) {
            isEnabled = PrefServiceBridge.getInstance().isWebDefenderEnabled();
        } else {
            return null;
        }

        if (isEnabled) {
            defaultValue = ContentSettingsResources.getDefaultEnabledValue(contentType);
        } else {
            defaultValue = ContentSettingsResources.getDefaultDisabledValue(contentType);
        }

        return defaultValue;
    }

    /**
     * Create Info object for the given preference if it does not exist.
     * @param preference The ListPreference to initialize.
     * @param permission The ContentSetting to initialize it to.
     */
    private void createPermissionInfo(Preference preference, ContentSetting permission) {
        String preferenceKey = preference.getKey();
        int contentType = getContentSettingsTypeFromPreferenceKey(preferenceKey);

        if (PREF_CAMERA_CAPTURE_PERMISSION.equals(preferenceKey) && mSite.getCameraInfo() == null) {
            mSite.setCameraInfo(new CameraInfo(mSite.getAddress().getOrigin(),
                    null,
                    mIsIncognito));
        } else if (PREF_COOKIES_PERMISSION.equals(preferenceKey) && mSite.getCookieInfo() == null) {
            mSite.setCookieInfo(new CookieInfo(mSite.getAddress().getOrigin(),
                    null,
                    mIsIncognito));
        } else if (PREF_FULLSCREEN_PERMISSION.equals(preferenceKey)
                && mSite.getFullscreenInfo() == null) {
            mSite.setFullscreenInfo(new FullscreenInfo(mSite.getAddress().getOrigin(),
                    null,
                    mIsIncognito));
        } else if (PREF_JAVASCRIPT_PERMISSION.equals(preferenceKey)
                && mSite.getJavaScriptPermission() == null) {
            mSite.setJavaScriptException(new ContentSettingException(contentType,
                    mSite.getAddress().getOrigin(),
                    permission.toString(),
                    "policy"));
        } else if (PREF_LOCATION_ACCESS.equals(preferenceKey)
                && mSite.getGeolocationInfo() == null) {
            mSite.setGeolocationInfo(new GeolocationInfo(mSite.getAddress().getOrigin(),
                    null,
                    mIsIncognito));
        } else if (PREF_MIC_CAPTURE_PERMISSION.equals(preferenceKey)
                && mSite.getMicrophoneInfo() == null) {
            mSite.setMicrophoneInfo(new MicrophoneInfo(mSite.getAddress().getOrigin(),
                    null,
                    mIsIncognito));
        } else if (PREF_POPUP_PERMISSION.equals(preferenceKey)
                && mSite.getPopupException() == null) {
            mSite.setPopupException(new ContentSettingException(contentType,
                    mSite.getAddress().getOrigin(),
                    permission.toString(),
                    "policy"));
        } else if (PREF_PROTECTED_MEDIA_IDENTIFIER_PERMISSION.equals(preferenceKey)
                && mSite.getProtectedMediaIdentifierInfo() == null) {
            mSite.setProtectedMediaIdentifierInfo(
                    new ProtectedMediaIdentifierInfo(mSite.getAddress().getOrigin(),
                            mSite.getAddress().getOrigin(),
                            mIsIncognito));
        } else if (PREF_PUSH_NOTIFICATIONS_PERMISSION.equals(preferenceKey)
                && mSite.getPushNotificationInfo() == null) {
            mSite.setPushNotificationInfo(
                    new PushNotificationInfo(mSite.getAddress().getOrigin(),
                            null,
                            mIsIncognito));
        } else if (PREF_WEBREFINER_PERMISSION.equals(preferenceKey)
                && mSite.getWebRefinerInfo() == null) {
            mSite.setWebRefinerInfo(
                    new WebRefinerInfo(mSite.getAddress().getOrigin(), null, mIsIncognito));
        } else if (PREF_WEBDEFENDER_PERMISSION.equals(preferenceKey)
                && mSite.getWebDefenderInfo() == null) {
            mSite.setWebDefenderInfo(
                    new WebDefenderInfo(mSiteAddress.getOrigin(), null, mIsIncognito));
        }
    }

    private boolean getWebRefinerPermission() {
        ContentSetting setting = null;
        if (mIsIncognito) {
            setting = WebRefinerPreferenceHandler.getSettingForIncognitoOrigin(
                    mSite.getAddress().getOrigin());
        } else {
            WebRefinerInfo info = mSite.getWebRefinerInfo();
            if (info != null) {
                setting = info.getContentSetting();
            }
        }

        if (setting != null) {
            String permission = setting.toString();
            if (permission.equalsIgnoreCase(ContentSetting.ALLOW.toString()))
                return true;

            return false;
        }

        return PrefServiceBridge.getInstance().isWebRefinerEnabled();
    }

    private void setWebRefinerPermission(boolean value) {
        Preference preference = findPreference(PREF_WEBREFINER_PERMISSION);
        ContentSetting permission = (value) ? ContentSetting.ALLOW : ContentSetting.BLOCK;
        requestReloadForOrigin();
        setTextForPreference(preference, permission);
        createPermissionInfo(preference, permission);
        if (mIsIncognito) {
            WebRefinerPreferenceHandler.addIncognitoOrigin(mSite.getAddress().getOrigin(),
                    permission);
        } else {
            mSite.setWebRefinerPermission(permission);
        }
    }

    private boolean getWebDefenderPermission() {
        ContentSetting setting = null;
        if (mIsIncognito) {
            setting = WebDefenderPreferenceHandler.getSettingForIncognitoOrigin(
                    mSite.getAddress().getOrigin());
        } else {
            WebDefenderInfo info = mSite.getWebDefenderInfo();
            if (info != null) {
                setting = info.getContentSetting();
            }
        }

        if (setting != null) {
            String permission = setting.toString();
            if (permission.equalsIgnoreCase(ContentSetting.ALLOW.toString()))
                return true;

            return false;
        }

        return PrefServiceBridge.getInstance().isWebDefenderEnabled();
    }

    private void setWebDefenderPermission(boolean value) {
        Preference preference = findPreference(PREF_WEBDEFENDER_PERMISSION);
        ContentSetting permission = (value) ? ContentSetting.ALLOW : ContentSetting.BLOCK;
        setTextForPreference(preference, permission);
        createPermissionInfo(preference, permission);
        if (mIsIncognito) {
            WebDefenderPreferenceHandler.addIncognitoOrigin(mSite.getAddress().getOrigin(),
                    permission);
        } else {
            mSite.setWebDefenderPermission(permission);
        }
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        String preferenceKey = preference.getKey();

        if (PREF_WEBREFINER_PERMISSION.equals(preferenceKey)) {
            boolean currentPermission = getWebRefinerPermission();
            setWebRefinerPermission(!currentPermission);
            mWebRefinerSwitch.setChecked(!currentPermission);
            return true;
        } else if (PREF_WEBDEFENDER_PERMISSION.equals(preferenceKey)) {
            boolean currentPermission = getWebDefenderPermission();
            setWebDefenderPermission(!currentPermission);
            mWebDefenderSwitch.setChecked(!currentPermission);
            return true;
        } else if (preferenceKey.equals("webdefender_details")) {
            if (preference.getFragment() != null &&
                    getActivity() instanceof OnPreferenceStartFragmentCallback) {
                Bundle args = getArguments();
                if (args != null) {
                    Bundle extra = preference.getExtras();
                    extra.putAll(args);
                }

                return ((OnPreferenceStartFragmentCallback)getActivity()).onPreferenceStartFragment(
                        this, preference);
            }
            return false;
        }

        return super.onPreferenceClick(preference);
    }


    @Override
    public void onChildViewAddedToHierarchy(View parent, View child) {
        if (child.getId() == R.id.site_security_info) {
            mSecurityViews.setResource(SiteSecurityViewFactory.ViewType.ERROR,
                    child, R.id.site_security_error);
            mSecurityViews.setResource(SiteSecurityViewFactory.ViewType.WARNING,
                    child, R.id.site_security_warning);
            mSecurityViews.setResource(SiteSecurityViewFactory.ViewType.INFO,
                    child, R.id.site_security_verbose);
        }

        TextView view = (TextView) child.findViewById(android.R.id.title);

        if (view != null && view.getText().equals(
               getResources().getText(R.string.website_settings_webdefender_privacy_meter_title))) {
            View meter = child.findViewById(R.id.webdefender_privacy_meter);
            if (meter != null) {
                WebDefenderDetailsPreferences.setupPrivacyMeterDisplay(meter, mWebDefenderStatus);
            }
        }

        Switch switchBtn = null;
        if (child.getId() == R.id.browser_pref_cat_switch) {
            switchBtn = (Switch) child.findViewById(R.id.browser_pref_cat_switch_btn);
            if (switchBtn != null) {
                if (view.getText().equals(
                        getResources().getText(R.string.website_settings_webrefiner_title))) {
                    boolean currentSetting = getWebRefinerPermission();
                    mWebRefinerSwitch = switchBtn;
                    switchBtn.setChecked(currentSetting);
                    switchBtn.setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            setWebRefinerPermission(mWebRefinerSwitch.isChecked());
                        }
                    });
                } else if (view.getText().equals(
                        getResources().getText(R.string.website_settings_webdefender_title))) {
                    boolean currentSetting = getWebDefenderPermission();
                    switchBtn.setChecked(currentSetting);
                    mWebDefenderSwitch = switchBtn;
                    switchBtn.setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            setWebDefenderPermission(mWebDefenderSwitch.isChecked());
                        }
                    });
                }
            }
        }

        if (mSiteColor != -1) {
            if (child.getId() == R.id.browser_pref_cat
                    || child.getId() == R.id.browser_pref_cat_first
                    || child.getId() == R.id.browser_pref_cat_switch) {
                if (view != null) {
                    view.setTextColor(mSiteColor);
                }
            }

            if (switchBtn != null) {
                int[][] states = new int[][] {
                        new int[] {android.R.attr.state_checked},  // checked
                        new int[] {-android.R.attr.state_checked}, // unchecked
                };

                int[] colors = new int[] {
                        mSiteColor,
                        Color.GRAY
                };

                ColorStateList myList = new ColorStateList(states, colors);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    switchBtn.setThumbTintList(myList);
                }
            }

            Button btn = (Button) child.findViewById(R.id.button_preference);
            if (btn != null) {
                btn.setBackgroundColor(mSiteColor);
            }
            ImageView imageView = (ImageView) child.findViewById(R.id.clear_site_data);
            if (imageView != null && imageView instanceof TintedImageView) {
                ColorStateList colorList = ColorStateList.valueOf(mSiteColor);
                ((TintedImageView) imageView).setTint(colorList);
            }
        }
    }

    private void appendActionBarDisplayOptions(ActionBar bar, int extraOptions) {
        int options = bar.getDisplayOptions();
        options |= extraOptions;
        bar.setDisplayOptions(options);
    }

    private void setStatusBarColor(int color) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            final Activity activity = ApplicationStatus.getLastTrackedFocusedActivity();
            if (!PrefServiceBridge.getInstance().getPowersaveModeEnabled()) {
                activity.getWindow().addFlags(
                        WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            }

            mSiteColor = BrandColorUtils.computeStatusBarColor(color);
            activity.getWindow().setStatusBarColor(mSiteColor);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (getActivity() instanceof AppCompatActivity) {
            AppCompatActivity activity = (AppCompatActivity) getActivity();
            ActionBar bar = activity.getSupportActionBar();
            Bundle args = getArguments();
            if (bar != null && args != null) {
                byte[] data = args.getByteArray(EXTRA_FAVICON);
                if (data != null) {
                    Bitmap bm = BitmapFactory.decodeByteArray(data, 0, data.length);
                    if (bm != null) {
                        Bitmap bitmap = Bitmap.createScaledBitmap(bm, 150, 150, true);
                        int color = FaviconHelper.getDominantColorForBitmap(bitmap);
                        appendActionBarDisplayOptions(bar,
                                ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_TITLE);
                        bar.setHomeButtonEnabled(true);
                        bar.setIcon(new BitmapDrawable(getResources(), bitmap));
                        bar.setBackgroundDrawable(new ColorDrawable(
                                BrandColorUtils.computeActionBarColor(color)
                        ));
                        setStatusBarColor(color);
                    }
                }
            }
        }
    }

    private void setupWebRefinerInformation(Bundle args) {
        int ads = args.getInt(EXTRA_WEB_REFINER_ADS_INFO, 0);
        String[] strings = new String[3];
        int index = 0;

        if (ads > 0) {
            strings[index++] = ads + " " + getResources().getString((ads > 1) ?
                    R.string.webrefiner_ads_plural :
                    R.string.webrefiner_ads);
        }

        int trackers = args.getInt(EXTRA_WEB_REFINER_TRACKER_INFO, 0);
        if (trackers > 0) {
            strings[index++] = trackers + " " + getResources().getString((trackers > 1) ?
                    R.string.webrefiner_trackers_plural :
                    R.string.webrefiner_trackers);

        }

        int malware = args.getInt(EXTRA_WEB_REFINER_MALWARE_INFO, 0);
        if (malware > 0) {
            strings[index++] = malware + " " + getResources().getString(
                    R.string.webrefiner_malware);
        }
        if (index > 0) {
            String[] formats = new String[3];
            formats[0] = getResources().getString(R.string.webrefiner_one_message);
            formats[1] = getResources().getString(R.string.webrefiner_two_message);
            formats[2] = getResources().getString(R.string.
                    webrefiner_three_message);

            Formatter formatter = new Formatter();
            formatter.format(formats[index - 1], strings[0], strings[1], strings[2]);
            mWebRefinerMessages = formatter.toString();
        }
    }

    /**
     * Creates a Bundle with the correct arguments for opening this fragment for
     * the website with the given url and icon.
     *
     * @param url The URL to open the fragment with. This is a complete url including scheme,
     *            domain, port,  path, etc.
     * @param icon The favicon for the URL
     *
     * @param tab The tab for the url
     * @return The bundle to attach to the preferences intent.
     */
    public static Bundle createFragmentArgsForSite(String url, Bitmap icon, Tab tab) {
        Bundle fragmentArgs = new Bundle();
        // TODO(mvanouwerkerk): Define a pure getOrigin method in UrlUtilities that is the
        // equivalent of the call below, because this is perfectly fine for non-display purposes.
        String origin = UrlUtilities.getOriginForDisplay(Uri.parse(url), true /*  showScheme */);
        fragmentArgs.putString(SingleWebsitePreferences.EXTRA_ORIGIN, origin);
        fragmentArgs.putBoolean(EXTRA_INCOGNITO, tab.isIncognito());

        if (icon != null) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            icon.compress(Bitmap.CompressFormat.PNG, 100, baos);
            fragmentArgs.putByteArray(EXTRA_FAVICON, baos.toByteArray());
        }

        // Add webrefiner related messages
        int ads, trackers, malware;
        ads = trackers = malware = 0;
        if (tab != null) {
            fragmentArgs.putInt(BrowserSingleWebsitePreferences.EXTRA_SECURITY_CERT_LEVEL,
                    tab.getSecurityLevel());
            if (tab.getContentViewCore() != null) {
                WebRefiner.PageInfo pageInfo =
                        WebRefinerPreferenceHandler.getPageInfo(tab.getContentViewCore());
                if (pageInfo != null) {
                    for (WebRefiner.MatchedURLInfo urlInfo : pageInfo.mMatchedURLInfoList) {
                        if (urlInfo.mActionTaken == WebRefiner.MatchedURLInfo.ACTION_BLOCKED) {
                            switch (urlInfo.mMatchedFilterCategory) {
                                case WebRefiner.RuleSet.CATEGORY_ADS:
                                    ads++;
                                    break;
                                case WebRefiner.RuleSet.CATEGORY_TRACKERS:
                                    trackers++;
                                    break;
                                case WebRefiner.RuleSet.CATEGORY_MALWARE_DOMAINS:
                                    malware++;
                                    break;
                            }
                        }
                    }
                }
                fragmentArgs.putInt(EXTRA_WEB_REFINER_ADS_INFO, ads);
                fragmentArgs.putInt(EXTRA_WEB_REFINER_TRACKER_INFO, trackers);
                fragmentArgs.putInt(EXTRA_WEB_REFINER_MALWARE_INFO, malware);
            }
        }

        if (WebDefenderPreferenceHandler.isInitialized()) {
            WebDefenderPreferenceHandler.StatusParcel parcel =
                    WebDefenderPreferenceHandler.getStatus(tab.getContentViewCore());

            fragmentArgs.putParcelable(
                    WebDefenderDetailsPreferences.EXTRA_WEBDEFENDER_PARCEL, parcel);
        }

        return fragmentArgs;
    }

    //Because we expose all settings to the user always, we want to show the warning about
    //Android's permission management to explain why some settings are disabled.
    protected boolean showWarningFor(int type) {
        if (mIsIncognito) return false;
        switch (type) {
            case ContentSettingsType.CONTENT_SETTINGS_TYPE_GEOLOCATION:
                break;
            case ContentSettingsType.CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA:
                break;
            case ContentSettingsType.CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC:
                break;
            default:
                return false;
        }
        SiteSettingsCategory category = SiteSettingsCategory.fromContentSettingsType(type);
        return category.showPermissionBlockedMessage(getActivity());
    }

}
