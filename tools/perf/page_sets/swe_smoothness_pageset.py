# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from telemetry.page import page as page_module
from telemetry.page import shared_page_state
from telemetry import story


class SwePage(page_module.Page):

  def RunNavigateSteps(self, action_runner):
    super(SwePage, self).RunNavigateSteps(action_runner)
    action_runner.Wait(5)


class SweScrollPage(SwePage):

  def RunPageInteractions(self, action_runner):
    # Make the scroll longer to reduce noise.
    with action_runner.CreateGestureInteraction('ScrollAction'):
      action_runner.ScrollPage(
        direction='down', speed_in_pixels_per_second=300)


class SwePageSet(story.StorySet):

  """ SWE smoothness measurement mobile sites """

  def __init__(self):
    super(SwePageSet, self).__init__()

    scroll_page_list = [
      # Why: Scrolls moderately complex pages (up to 60 layers)
      'http://engadget.com',
      'http://www.mobile01.com/newsdetail.php?id=13217',
      #'http://qq.com',
      'http://news.baidu.com',
      'http://www.cnn.com',
      'http://www.amazon.com',
      'http://www.nytimes.com/?nytmobile=0',
      'http://www.quackit.com/html/codes/html_marquee_code.cfm'
    ]

    for url in scroll_page_list:
      self.AddStory(SweScrollPage(url, self))

