# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from measurements import smoothness
import page_sets
from telemetry import benchmark


class SmoothnessSwePages(benchmark.Benchmark):
  """Measures rendering statistics for mobile sites page set for SWE browser """
  test = smoothness.Smoothness
  page_set = page_sets.SwePageSet

  @classmethod
  def Name(cls):
    return 'smoothness.swe_smoothness_pageset'
