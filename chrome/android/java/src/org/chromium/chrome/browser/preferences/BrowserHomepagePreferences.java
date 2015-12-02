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
package org.chromium.chrome.browser.preferences;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceManager;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.TextView;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.UrlUtilities;
import org.chromium.chrome.browser.partnercustomizations.HomepageManager;

import java.util.ArrayList;

/**
 * Fragment that allows the user to configure homepage related preferences.
 */
public class BrowserHomepagePreferences extends BrowserPreferenceFragment {
    public static final String CURRENT_URL = "CURRENT_URL";
    private final String PREF_HOMEPAGE_EDIT = "homepage_edit";
    private final String PREF_HOMEPAGE_SWITCH = "homepage_switch";
    private final String PREF_HOMEPAGE_CUSTOM_URI = "homepage_custom_uri";
    private final String HOMEPAGE = "HOMEPAGE";
    private final int HOMEPAGE_REQUEST = -1;
    private final String BLANK_URL = UrlUtilities.fixupUrl("about:blank");

    private String DEFAULT_PAGE;
    private String MOST_VISITED_PAGE;
    private String CUSTOM_PAGE;
    private String BLANK_PAGE;
    private String CURRENT_PAGE;
    private String DEFAULT_URL = "";


    private String mCurrentUrl;

    private HomepageManager mHomepageManager;
    private ChromeSwitchPreference mHomepageSwitch;
    private ChromeBaseListPreference mHomepageEdit;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mHomepageManager = HomepageManager.getInstance(getActivity());
        getActivity().setTitle(R.string.options_homepage_title);
        addPreferencesFromResource(R.xml.browser_homepage_preferences);
        mCurrentUrl = UrlUtilities.fixupUrl(getArguments().getString(CURRENT_URL));
        mHomepageSwitch = (ChromeSwitchPreference) findPreference(PREF_HOMEPAGE_SWITCH);
        boolean isHomepageEnabled = mHomepageManager.getPrefHomepageEnabled();
        mHomepageSwitch.setChecked(isHomepageEnabled);
        mHomepageSwitch.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                mHomepageManager.setPrefHomepageEnabled((boolean) newValue);
                setSummary();
                return true;
            }
        });

        mHomepageEdit = (ChromeBaseListPreference)findPreference(PREF_HOMEPAGE_EDIT);
        mHomepageEdit.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                if (CUSTOM_PAGE.equals(newValue)) {
                    promptForHomepage();
                    return false;
                } else if (MOST_VISITED_PAGE.equals(newValue)) {
                    mHomepageManager.setPrefHomepageCustomUri("");
                } else if (CURRENT_PAGE.equals(newValue) && !TextUtils.isEmpty(mCurrentUrl)) {
                    mHomepageManager.setPrefHomepageCustomUri(mCurrentUrl);
                } else {
                    mHomepageManager.setPrefHomepageCustomUri(UrlUtilities
                            .fixupUrl((String) newValue));
                }
                setSummary();
                return true;
            }
        });
        setupListPreference();
    }

    private void setupListPreference() {
        //init the text fields
        DEFAULT_PAGE = getResources().getString(R.string.default_homepage);
        MOST_VISITED_PAGE =
                getResources().getString(R.string.most_visited_sites_homepage);
        CUSTOM_PAGE = getResources().getString(R.string.other_homepage);
        BLANK_PAGE = getResources().getString(R.string.blank_homepage);
        CURRENT_PAGE = getResources().getString(R.string.current_page_homepage);

        ArrayList<CharSequence> keys = new ArrayList<>();
        ArrayList<CharSequence> descriptions = new ArrayList<>();
        descriptions.add(0, CURRENT_PAGE);
        descriptions.add(1, DEFAULT_PAGE);
        descriptions.add(2, BLANK_PAGE);
        descriptions.add(3, MOST_VISITED_PAGE);
        descriptions.add(4, CUSTOM_PAGE);
        keys.add(0, CURRENT_PAGE);
        keys.add(1, DEFAULT_URL =
                UrlUtilities.fixupUrl(getResources().getString(R.string.default_homepage_url)));
        keys.add(2, BLANK_URL);
        keys.add(3, MOST_VISITED_PAGE);
        keys.add(4, CUSTOM_PAGE);
        if (TextUtils.isEmpty(mCurrentUrl)) {
            keys.remove(CURRENT_PAGE);
            descriptions.remove(CURRENT_PAGE);
        }
        mHomepageEdit.setEntryValues(keys.toArray(new CharSequence[keys.size()]));
        mHomepageEdit.setEntries(descriptions.toArray(new CharSequence[descriptions.size()]));
        setSummary();
    }

    private void setSummary() {
        String currentHomePage = mHomepageManager.getPrefHomepageCustomUri();
        SharedPreferences preferences = PreferenceManager.
                getDefaultSharedPreferences(getActivity());
        if (!preferences.contains(PREF_HOMEPAGE_CUSTOM_URI)) { //first time
            if (mHomepageManager.getPrefHomepageEnabled()) {
                mHomepageManager.setPrefHomepageCustomUri(DEFAULT_URL);
                mHomepageEdit.setValueIndex(mHomepageEdit.findIndexOfValue(DEFAULT_URL));
                mHomepageEdit.setSummary(DEFAULT_URL);
            } else {
                mHomepageEdit.setSummary(getResources().getString(R.string.touch_to_select));
            }
            return;
        } else if (TextUtils.isEmpty(currentHomePage)) {
            mHomepageEdit.setSummary(MOST_VISITED_PAGE);
            return;
        } else if (!DEFAULT_URL.equals(currentHomePage) && !BLANK_URL.equals(currentHomePage)){
            if (!TextUtils.isEmpty(mCurrentUrl) && mCurrentUrl.equals(currentHomePage)) {
                mHomepageEdit.setValueIndex(mHomepageEdit.findIndexOfValue(CURRENT_PAGE));
            } else {
                mHomepageEdit.setValueIndex(mHomepageEdit.findIndexOfValue(CUSTOM_PAGE));
            }
        }
        mHomepageEdit.setSummary(currentHomePage);
        return;
    }

    @Override
    public void onResume() {
        super.onResume();
        setupListPreference();
    }

    void promptForHomepage() {
        BrowserAlertDialogFragment fragment = new BrowserAlertDialogFragment();
        fragment.setTargetFragment(this, HOMEPAGE_REQUEST);
        fragment.show(getActivity().getFragmentManager(), "setHomepage dialog");
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == HOMEPAGE_REQUEST) {
            String homepage = data.getStringExtra(HOMEPAGE);
            if (!TextUtils.isEmpty(homepage)) {
                mHomepageEdit.setValueIndex(mHomepageEdit.findIndexOfValue(CUSTOM_PAGE));
                mHomepageEdit.setValue(homepage);
                mHomepageManager.setPrefHomepageCustomUri(UrlUtilities
                        .fixupUrl(mHomepageEdit.getValue()));
            }
            setSummary();
        }
    }

    /*
     Add this class to manage AlertDialog lifecycle.
   */
    public static class BrowserAlertDialogFragment extends DialogFragment {
        private final String HOMEPAGE = "HOMEPAGE";
        private EditText editText = null;

        public static BrowserAlertDialogFragment newInstance() {
            return new BrowserAlertDialogFragment();
        }

        private void sendResult(String homepage) {
            Intent intent = new Intent();
            intent.putExtra(HOMEPAGE, homepage);
            getTargetFragment().onActivityResult(
                    getTargetRequestCode(), -1, intent);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            editText = new EditText(getActivity());
            String homePage = savedInstanceState != null
                    ? savedInstanceState.getString(HOMEPAGE)
                    : HomepageManager.getInstance(getActivity()).getPrefHomepageCustomUri();
            editText.setInputType(InputType.TYPE_CLASS_TEXT
                    | InputType.TYPE_TEXT_VARIATION_URI);
            editText.setText(homePage);
            editText.setSelectAllOnFocus(true);
            editText.setSingleLine(true);
            editText.setHint(R.string.default_homepage_url);
            editText.setImeActionLabel(null, EditorInfo.IME_ACTION_DONE);
            final AlertDialog dialog = new AlertDialog.Builder(getActivity())
                    .setView(editText)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            String homepage = editText.getText().toString().trim();
                            homepage = UrlUtilities.fixupUrl(homepage);
                            Fragment fragment = getTargetFragment();
                            if (fragment == null
                                    || !(fragment instanceof BrowserHomepagePreferences)) {
                                Log.e("BrowserAlertDialog", "get target fragment error!");
                                return;
                            }
                            sendResult(homepage);
                        }
                    })
                    .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            sendResult("");
                            dialog.cancel();
                        }
                    })
                    .setTitle(R.string.options_homepage_edit_label)
                    .create();

            editText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if (actionId == EditorInfo.IME_ACTION_DONE) {
                        dialog.getButton(AlertDialog.BUTTON_POSITIVE).performClick();
                        return true;
                    }
                    return false;
                }
            });

            dialog.getWindow().setSoftInputMode(
                    WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
            return dialog;
        }
    }
}

