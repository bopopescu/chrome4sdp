#!/usr/bin/env python
# Copyright (c) 2015, The Linux Foundation. All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.

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

import os, subprocess, sys, argparse, stat, shutil

description = """
              This script will create breakpad symbols for all libs within
              the out/Release dir(default).
              When --output is specified, components/crash/dmp2minidump.py
              and out/*/minidump_stackwalk will be copied into the output
              directory. This creates a standlone directory that can be used
              to symbolize all dumps from the same build.
              """

#Files that will be copied if a output dir is specified
dmp2minidump = "components/crash/tools/dmp2minidump.py"
minidump_stackwalk = "minidump_stackwalk"

# =======================================================================================
def parse_args(argv):
    parser = argparse.ArgumentParser(description=description)

    parser.add_argument("--debug", action='store_true',
                        help="Generate symbols for out/Debug/lib instead\n")
    parser.add_argument("--src", action='store',
                        help="Path to src. Not needed if calling from src\n")
    parser.add_argument("--x64", action='store_true',
                        help="Generate symbols for a 64-bit build\n")
    parser.add_argument("--output", action='store', default=None,
                        help="Specify where the breakpad_symbols directory should be\n")
    (options, args) = parser.parse_known_args(argv)
    return options

# =======================================================================================
def copy(src, dest):
    if not os.path.exists(dest):
        print os.path.dirname(dest)
        os.makedirs(os.path.dirname(dest))
    shutil.copy2(src, dest)

# =======================================================================================
if __name__ == '__main__':
    options = parse_args(sys.argv[1:])
    #look in ../../../out/*/lib
    build = "Release"
    clear = True
    src = os.getcwd() # Assume Src dir
    generator = "components/crash/tools/generate_breakpad_symbols.py"
    if options.debug:
        build = "Debug"
    if options.x64:
        build = build + "_x64"
    if options.src:
        src = os.path.abspath(options.src)
        generator = os.path.join(src, generator)
    build_out = os.path.join(src, "out/", build)
    outdir = os.path.join(build_out, "breakpad_symbols")
    if options.output:
        outdir = os.path.abspath(os.path.join(options.output, "breakpad_symbols"))
        minidump_stackwalk = os.path.relpath(os.path.join(build_out, minidump_stackwalk), src)
    libpath = os.path.join(build_out, "lib")
    if not os.path.exists(generator):
        print "Can't find %s" % (os.path.abspath(os.path.join(src, generator)))
        print "Run from src/ or specify --src"
        sys.exit()

    #for each lib, run componenets/crash/tools/generate_breakpad_symbols.py
    for f in os.listdir(libpath):
        if f.endswith(".so"):
            fpath = os.path.join(libpath, f)
            #give execute if lib doesn't have it
            if not os.access(fpath, os.X_OK):
                fstat = os.stat(fpath)
                #chmod +x
                os.chmod(fpath, fstat.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
            #construct the command
            cmd = ["python", generator,"--build-dir=" + build_out, "--symbols-dir=" + outdir]
            cmd.append("--binary=" + fpath)
            if clear: # Clear the directory just the first time
                cmd.append("--clear")
                clear = False
            try:
                sys.stdout.write("\x1b[KWorking on %s\r" % f)
                sys.stdout.flush()
                subprocess.check_output(cmd);
            except:
                sys.exit("ERROR: Coudln't run the following command - " + " ".join(cmd))

    if options.output:
        #Copy the final symbolizing dependencies - See description
        print "Copying %s and %s into %s" % (dmp2minidump, minidump_stackwalk, outdir)
        copy(os.path.join(src, dmp2minidump), os.path.join(outdir, dmp2minidump))
        copy(os.path.join(src, minidump_stackwalk), os.path.join(outdir, minidump_stackwalk))
    exit(0)
