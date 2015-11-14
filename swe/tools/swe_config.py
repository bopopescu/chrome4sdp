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

def subst_template(contents, values):
  for key, val in values.iteritems():
    try:
      contents = contents.replace('@' + key + '@', val)
    except TypeError:
      print repr(key), repr(val)
  return contents

def subst_file(file_name, values):
  template = open(file_name, 'r').read()
  return subst_template(template, values);

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

def main():

  parser = argparse.ArgumentParser()
  parser.add_argument('-i', '--input', default=None,
                      help='Read base version FILE.')
  parser.add_argument('-c', '--config_value',
                      default=None, help='Config name')
  parser.add_argument('-ti', '--template_input',
                      default=None, help='Template input')
  parser.add_argument('-to', '--template_output',
                      default=None, help='Template output_patch')
  options = parser.parse_args()

  path = os.path.dirname(os.path.abspath(__file__))

  values = dict([])


  #fetch KEY=VALUE pairs
  if options.input is not None:
    fetch_values_from_file(values, options.input)
    contents = """PACKAGE_NAME=%(PACKAGE_NAME)s""" % values


  if options.config_value:
    if "PACKAGE_NAME" in options.config_value:
      print """%(PACKAGE_NAME)s""" % values
      return

  elif options.template_input and options.template_output:
    contents = subst_file(options.template_input, values)
    write_if_changed(options.template_output, contents)

if __name__ == '__main__':
  sys.exit(main())
