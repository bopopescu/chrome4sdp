// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_REFINER_EXPORT_H_
#define WEB_REFINER_EXPORT_H_

#if !defined(COMPILE_CONTENT_STATICALLY)
#if defined(WIN32)

#if defined(CONTENT_IMPLEMENTATION)
#define WEB_REFINER_EXPORT __declspec(dllexport)
#else
#define WEB_REFINER_EXPORT __declspec(dllimport)
#endif  // defined(CONTENT_IMPLEMENTATION)

#else // defined(WIN32)
#if defined(CONTENT_IMPLEMENTATION)
#define WEB_REFINER_EXPORT __attribute__((visibility("default")))
#else
#define WEB_REFINER_EXPORT
#endif
#endif

#else // !defined(COMPILE_CONTENT_STATICALLY)
#define WEB_REFINER_EXPORT
#endif

#endif  // WEB_REFINER_EXPORT_H_
