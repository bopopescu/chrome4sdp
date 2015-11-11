// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/permissions/permission_infobar_delegate.h"

#include "chrome/browser/permissions/permission_context_uma_util.h"
#include "chrome/browser/permissions/permission_queue_controller.h"
#include "chrome/grit/generated_resources.h"
#include "components/infobars/core/infobar.h"
#include "ui/base/l10n/l10n_util.h"

PermissionInfobarDelegate::~PermissionInfobarDelegate() {
  if (!action_taken_)
    PermissionContextUmaUtil::PermissionIgnored(type_, requesting_origin_);
}

PermissionInfobarDelegate::PermissionInfobarDelegate(
    PermissionQueueController* controller,
    const PermissionRequestID& id,
    const GURL& requesting_origin,
    ContentSettingsType type)
    : controller_(controller), id_(id), requesting_origin_(requesting_origin),
      action_taken_(false),
      type_(type) {
}

infobars::InfoBarDelegate::Type
PermissionInfobarDelegate::GetInfoBarType() const {
  return PAGE_ACTION_TYPE;
}

void PermissionInfobarDelegate::InfoBarDismissed() {
  SetPermission(false, CONTENT_SETTING_BLOCK);
}

PermissionInfobarDelegate*
PermissionInfobarDelegate::AsPermissionInfobarDelegate() {
  return this;
}

base::string16 PermissionInfobarDelegate::GetButtonLabel(
    InfoBarButton button) const {
  return l10n_util::GetStringUTF16((button == BUTTON_OK) ?
      IDS_PERMISSION_ALLOW : IDS_PERMISSION_DENY);
}

bool PermissionInfobarDelegate::Accept() {
  SetPermission(true, CONTENT_SETTING_ALLOW);
  return true;
}

bool PermissionInfobarDelegate::Accept(ContentSetting action, const std::string& action_value) {
  switch(action) {
    case CONTENT_SETTING_BLOCK:
      SetPermission(true, action);
      return true;
    case CONTENT_SETTING_ALLOW:
      SetPermission(true, action);
      return true;
    case CONTENT_SETTING_ALLOW_24H:
      SetPermission(true, action);
      return true;
    case CONTENT_SETTING_SESSION_ONLY:
      SetPermission(false, action);
      return true;
    default:
      return false;
  }
}

bool PermissionInfobarDelegate::Cancel() {
  SetPermission(true, CONTENT_SETTING_BLOCK);
  return true;
}

void PermissionInfobarDelegate::SetPermission(bool update_content_setting,
                                              ContentSetting content_setting) {
  action_taken_ = true;
  controller_->OnPermissionSet(
      id_, requesting_origin_,
      InfoBarService::WebContentsFromInfoBar(
          infobar())->GetLastCommittedURL().GetOrigin(),
      update_content_setting, content_setting);
}
