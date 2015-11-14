// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIEW_MANAGER_SURFACES_SURFACES_STATE_H_
#define COMPONENTS_VIEW_MANAGER_SURFACES_SURFACES_STATE_H_

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "cc/surfaces/surface_manager.h"
#include "components/view_manager/surfaces/surfaces_scheduler.h"

namespace cc {
class SurfaceManager;
}  // namespace cc

namespace surfaces {

// The SurfacesState object is an object global to the View Manager app that
// holds the SurfaceManager, SurfacesScheduler and allocates new Surfaces
// namespaces. This object lives on the main thread of the View Manager.
// TODO(rjkroege, fsamuel): This object will need to change to support multiple
// displays.
class SurfacesState : public base::RefCounted<SurfacesState> {
 public:
  SurfacesState();

  uint32_t next_id_namespace() { return next_id_namespace_++; }

  cc::SurfaceManager* manager() { return &manager_; }

  SurfacesScheduler* scheduler() { return &scheduler_; }

 private:
  friend class base::RefCounted<SurfacesState>;
  ~SurfacesState();

  // A Surface ID is an unsigned 64-bit int where the high 32-bits are generated
  // by the Surfaces service, and the low 32-bits are generated by the process
  // that requested the Surface.
  uint32_t next_id_namespace_;
  cc::SurfaceManager manager_;
  SurfacesScheduler scheduler_;

  DISALLOW_COPY_AND_ASSIGN(SurfacesState);
};

}  // namespace surfaces

#endif  //  COMPONENTS_VIEW_MANAGER_SURFACES_SURFACES_STATE_H_