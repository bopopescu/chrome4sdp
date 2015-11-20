// Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/http_network_layer.h"

#include "base/logging.h"
#include "base/power_monitor/power_monitor.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "net/http/http_network_session.h"
#include "net/http/http_network_transaction.h"
#include "net/http/http_getzip_factory.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/http_stream_factory_impl_job.h"
#include "net/spdy/spdy_framer.h"
#include "net/spdy/spdy_session.h"
#include "net/spdy/spdy_session_pool.h"
#include "net/libsta/instance_creation.h"

namespace net {

HttpNetworkLayer::HttpNetworkLayer(HttpNetworkSession* session)
    : session_(session),
      suspended_(false) {
  DCHECK(session_.get());
#if defined(OS_WIN)
 base::PowerMonitor* power_monitor = base::PowerMonitor::Get();
 if (power_monitor)
   power_monitor->AddObserver(this);
#endif
  HttpGetZipFactory::InitGETZipManager();
}

HttpNetworkLayer::~HttpNetworkLayer() {
#if defined(OS_WIN)
  base::PowerMonitor* power_monitor = base::PowerMonitor::Get();
  if (power_monitor)
    power_monitor->RemoveObserver(this);
#endif
}

// static
HttpTransactionFactory* HttpNetworkLayer::CreateFactory(
    HttpNetworkSession* session) {
  DCHECK(session);

  return new HttpNetworkLayer(session);
}

int HttpNetworkLayer::CreateTransaction(RequestPriority priority,
                                        scoped_ptr<HttpTransaction>* trans) {
  if (suspended_)
    return ERR_NETWORK_IO_SUSPENDED;

  HttpTransaction* pTransact;
  std::string errString;
  CreateInstance_TATransaction(&pTransact, priority, GetSession(), &errString);
  trans->reset(pTransact);
  return OK;
}

HttpCache* HttpNetworkLayer::GetCache() {
  return NULL;
}

HttpNetworkSession* HttpNetworkLayer::GetSession() { return session_.get(); }

void HttpNetworkLayer::OnSuspend() {
  suspended_ = true;

  if (session_.get())
    session_->CloseIdleConnections();
}

void HttpNetworkLayer::OnResume() {
  suspended_ = false;
}

}  // namespace net
