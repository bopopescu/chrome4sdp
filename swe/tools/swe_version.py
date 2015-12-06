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
import os
import sys
import subprocess


def fetch_values_from_file(values_dict, file_name):
  """
  Fetches KEYWORD=VALUE settings from the specified file.
  """
  for line in open(file_name, 'r').readlines():
    key, val = line.rstrip('\r\n').split('=', 1)
    values_dict[key] = val

def write_if_changed(file_name, contents):
  try:
    old_contents = open(file_name, 'r').read()
  except EnvironmentError:
    pass
  else:
    if contents == old_contents:
      return
    os.unlink(file_name)
  open(file_name, 'w').write(contents)


def RunGitCommand(directory, command):

  command = ['git'] + command
  if sys.platform == 'cygwin':
    command = ['sh', '-c', ' '.join(command)]
  try:
    proc = subprocess.Popen(command,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            cwd=directory,
                            shell=(sys.platform=='win32'))
    return proc
  except OSError:
    return None


def getData(directory, gitCommand):
  data = ''
  proc = RunGitCommand(directory, gitCommand)
  if proc:
    output = proc.communicate()[0].strip()
    if proc.returncode == 0 and output:
      data = output
  return data

def run_script(script, stdin=None):
    """Returns (stdout, stderr)"""
    import subprocess
    proc = subprocess.Popen(['bash', '-c', script],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        stdin=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    if proc.returncode:
        raise ScriptException(proc.returncode, stdout, stderr, script)
    return stdout, stderr

#Get buildid # (number of patches on top of base commit)
def get_build_id(directory):

  #hsh = getData(directory, ['rev-parse', '--short', 'HEAD'])
  hsh = getData(directory, ['rev-parse', 'HEAD'])
  if hsh is None:
    return None

  #merge_base = getData(directory, ['merge-base', 'remotes/origin/master', '%s' %(hsh)])
  #if merge_base is None:
  #  return None

  patch_count = getData(directory, ['rev-list', '%s' %(hsh), '--count'])
  if patch_count is None:
    return None

  return patch_count

#about string for about.xml
def about_string():
  return "<?xml version=\"1.0\" encoding=\"utf-8\"?> \n \
<resources xmlns:xliff=\"urn:oasis:names:tc:xliff:document:1.2\"> \n \
<!-- Text to display in about dialog --> \n\
<string name=\"about_text\" formatted=\"false\">\n\
%(SWE_CONTACT)s\n\
Version: %(SWE_VERSION)s\\n\n\
Built: %(SWE_BUILD_DATE)s\\n\n\
Host: %(HOST_NAME)s\\n\n\
User: %(USER_NAME)s\\n\n\
Hash: %(SWE_BUILD_HASH)s\\n\n\
</string>\n\
</resources>\n"

#feedback string
def feedback_string():
  feedback = "Please help us make your experience better by contacting the\
team at <a href=\"mailto:%(SWE_APK_SUPPORT)s\">%(SWE_APK_SUPPORT)s</a>\\n"
  feedback_email = os.environ.get('SWE_APK_SUPPORT')
  if feedback_email:
    return feedback % dict([('SWE_APK_SUPPORT', '%s' %(feedback_email))])
  else:
    return ""

def main():

  parser = argparse.ArgumentParser()
  parser.add_argument('-i', '--input', default=None,
                      help='Read base version FILE.')
  parser.add_argument('-o', '--output', default=None,
                      help='Write SWE strings to FILE.')
  parser.add_argument('--version-code-only', action='store_true',
                      help='Write versionCode information.')
  parser.add_argument('--version-string-only', action='store_true',
                      help='Write versionString information.')
  parser.add_argument('-about', '--write-about-string', default=None,
                      help='Write about page string information.')
  options = parser.parse_args()

  path = os.path.dirname(os.path.abspath(__file__))

  buildid = get_build_id(path)
  values = dict([('BUILDID', '%s' %(buildid))])

  last_swe_change = getData(path, ['log', '-1', '--pretty=%H'] )
  values.update({'LASTSWECHANGE' :'%s' %(last_swe_change)})

  #fetch KEY=VALUE pairs
  if options.input is not None:
    fetch_values_from_file(values, options.input)

    contents = """MAJOR=%(MAJOR)s
MINOR=%(MINOR)s
BUILD=%(BUILD)s
PATCH=%(PATCH)s
SWE_MAJOR=%(MAJOR)s
SWE_MINOR=%(MINOR)s
SWE_BUILD=%(BUILD)s
SWE_PATCH=%(BUILDID)s
LASTSWECHANGE=%(LASTSWECHANGE)s
""" % values

  version_code = "%(BUILD)s" %values+buildid.zfill(4)
  hash_string = getData(path, ['rev-parse', '--short', 'HEAD'])
  version_string = """%(MAJOR)s.%(MINOR)s.%(BUILD)s.%(BUILDID)s""" % values
  build_hash = hash_string + " (%(BUILD)s)" %values


  values.update({'SWE_VERSION' :'%s' %(version_string)})
  values.update({'SWE_BUILD_DATE' :run_script('date')[0].strip('\n') })
  values.update({'HOST_NAME' :run_script('hostname')[0].strip('\n') })
  values.update({'USER_NAME' :run_script('whoami')[0].strip('\n') })
  values.update({'SWE_BUILD_HASH' :'%s' %(build_hash)})
  values.update({'SWE_CONTACT' :'%s' %(feedback_string())})
  about_string_xml = about_string() % values

  if options.version_code_only:
    print version_code
    return

  if options.version_string_only:
    print version_string + ' (%s)' %(hash_string)
    return

  if options.write_about_string:
    write_if_changed(options.write_about_string, about_string_xml)
    return

  if options.output is not None:
    write_if_changed(options.output, contents)
  else:
    print contents

if __name__ == '__main__':
  sys.exit(main())
