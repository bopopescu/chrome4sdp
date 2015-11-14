#!/usr/bin/env python
#
# Copyright (c) 2015, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided
#      with the distribution.
#    * Neither the name of The Linux Foundation nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
import argparse
import os.path
import sys

def main():
  DoMain(sys.argv[1:])
  return 0

def _is_dir(dir_name):
  return os.path.isdir(dir_name)

def list_dirs(channels):
  return channels.split(',')

def channel_component_name(channels):
  """ First dir is considered to be either channel namd or channel path
  """
  return channels.split(',')[0]

def channel_path(channels):
  """ First dir is considered to be branding path
  """
  path = channel_component_name(channels)
  if os.path.isabs(path) == True:
    return path + "branding/"
  else:
    internal_folder = INTERNAL_CHANNELS_FOLDER + path + "/branding/"
    external_folder = EXTERNAL_CHANNELS_FOLDER + path + "/branding/"

    #First check if internal folder has branding file, else try external folder
    if _is_dir(internal_folder):
      return internal_folder

    if _is_dir(external_folder):
      return external_folder
    else:
      print "Error missing branding folder"
      return ""


def channel_res_folders(channels):
  res_folders = []
  channels_list = channels.split(',')
  for channel in channels_list:
    if os.path.isabs(channel) == True:
      res_folders.append(channel + '/res')
    else:
      internal_folder = INTERNAL_CHANNELS_FOLDER + channel + "/res"
      external_folder = EXTERNAL_CHANNELS_FOLDER + channel + "/res"
      if _is_dir(internal_folder):
        res_folders.append(internal_folder)
      if _is_dir(external_folder):
        res_folders.append(external_folder)

  return ' '.join(["'%s'" % x for x in res_folders])

def DoMain(argv):
  global DEPTH_DIR
  global EXTERNAL_CHANNELS_FOLDER
  global INTERNAL_CHANNELS_FOLDER

  usage = 'usage: %prog [options]'
  parser = argparse.ArgumentParser()
  parser.add_argument('--swe-channels', dest='swe_channels',
                         help='Comman seperated channel list')
  parser.add_argument('-d', '--depth', default=None,
                      help='Depth folder.')
  parser.add_argument('--branding-name', help='Only print branding component name.',
                    action='store_true')
  parser.add_argument('--branding-folder', help='Print branding path.',
                    action='store_true')
  parser.add_argument('--channel-res-folder', help='List channels res folders.',
                    action='store_true')

  args = parser.parse_args(argv)

  DEPTH_DIR = args.depth
  EXTERNAL_CHANNELS_FOLDER = DEPTH_DIR + "/swe/channels/"
  INTERNAL_CHANNELS_FOLDER = DEPTH_DIR + "/swe/channels/internal/"

  if not args.swe_channels:
    parser.error('Please specify "--swe_channels".\n')

  if not DEPTH_DIR:
    parser.error('Please specify "-d".\n')

  if args.branding_name:
    return channel_component_name(args.swe_channels)

  if args.branding_folder:
    return channel_path(args.swe_channels)

  if args.channel_res_folder:
    return channel_res_folders(args.swe_channels)

if __name__ == '__main__':
  sys.exit(main())
