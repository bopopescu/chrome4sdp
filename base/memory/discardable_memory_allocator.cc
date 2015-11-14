// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/discardable_memory_allocator.h"

#include "base/logging.h"

#include "skia/ext/discardable_memory_allocator_holder.h"

namespace base {
namespace {

DiscardableMemoryAllocator* g_allocator = nullptr;

}  // namespace

// static
void DiscardableMemoryAllocator::SetInstance(
    DiscardableMemoryAllocator* allocator) {
  DCHECK(allocator);

  // Make sure this function is only called once before the first call
  // to GetInstance().
  DCHECK(!g_allocator);

  g_allocator = allocator;

  // Use skia to store the discardable memory allocator instance. This is
  // because skia and content end up having their own version of the global
  // variable when using the shared skia library. This is problematic as the
  // skia global variable is never set and causes crash because calling skia
  // code doesn't check if the return value is non zero and immediately tries to
  // deref to call a member function.
  DiscardableMemoryAllocatorHolder::SetDiscardableMemoryAllocator(allocator);
}

// static
DiscardableMemoryAllocator* DiscardableMemoryAllocator::GetInstance() {
  return g_allocator;
}

}  // namespace base
