# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': ['../skia/sweskia.gypi'],
  'variables': {
    'skia_opts_ext%': '<!(python <(DEPTH)/build/dir_exists.py ../third_party/skia/src/opts/ext/)',
    'skia_version': '<!(echo $SKIA_VERSION)',
    'conditions': [
      ['clang==1', {
        # Do not use clang's integrated assembler.  It doesn't grok
        # some neon instructions.
        'clang_use_integrated_as': 0,
      }],
    ],
  },
  'conditions': [
    ['clang==0 or clang_use_integrated_as==0', {
      'cflags': [
        # The neon assembly contains conditional instructions which
        # aren't enclosed in an IT block. The GNU assembler complains
        # without this option.
        # See #86592.
        '-Wa,-mimplicit-it=always',
      ],
    }],
    # In component mode (shared_lib), we build all of skia as a single DLL.
    # However, in the static mode, we need to build skia as multiple targets
    # in order to support the use case where a platform (e.g. Android) may
    # already have a copy of skia as a system library.
    ['component=="static_library" and  component_skia=="static_library"', {
      'targets': [
        {
          'target_name': 'skia_library',
          'type': 'static_library',
          'includes': [
            'skia_common.gypi',
            'skia_library.gypi',
            '../build/android/increase_size_for_speed.gypi',
            # Disable LTO due to compiler error
            # in mems_in_disjoint_alias_sets_p, at alias.c:393
            # crbug.com/422255
            '../build/android/disable_lto.gypi',
          ],
        },
      ],
    }],
    ['component=="static_library" and component_skia=="static_library"', {
      'targets': [
        {
          'target_name': 'skia',
          'type': 'none',
          'dependencies': [
            'skia_library',
            'skia_chrome',
          ],
          'export_dependent_settings': [
            'skia_library',
            'skia_chrome',
          ],
          'direct_dependent_settings': {
            'conditions': [
              [ 'OS == "win"', {
                'defines': [
                  'GR_GL_FUNCTION_TYPE=__stdcall',
                ],
              }],
            ],
          },
        },
        {
          'target_name': 'skia_chrome',
          'type': 'static_library',
          'includes': [
            'skia_chrome.gypi',
            'skia_common.gypi',
            '../build/android/increase_size_for_speed.gypi',
          ],
        },
      ],
    },
    {  # component != static_library
      'targets': [
        {
          'target_name': 'skia',
          'type': '<(component_skia)',
          'product_name': '<(product_name)',
          'includes': [
            # Include skia_common.gypi first since it contains filename
            # exclusion rules. This allows the following includes to override
            # the exclusion rules.
            'skia_common.gypi',
            'skia_chrome.gypi',
            'skia_library.gypi',
            '../build/android/increase_size_for_speed.gypi',
          ],
          'link_settings': {
            'libraries!': [
              '-lc++_static',
            ],
            'libraries' : [
              '-lc++_shared'
            ],
          },
          'ldflags!': [
              '-Wl,--fatal-warnings',
          ],
          'dependencies': [
              '../build/android/setup.gyp:copy_system_libraries',
          ],
          'defines': [
            'SKIA_DLL',
            'SKIA_IMPLEMENTATION=1',
            'GR_GL_IGNORE_ES3_MSAA=0',
          ],
          'direct_dependent_settings': {
            'conditions': [
              [ 'OS == "win"', {
                'defines': [
                  'GR_GL_FUNCTION_TYPE=__stdcall',
                ],
              }],
            ],
            'defines': [
              'SKIA_DLL',
              'GR_GL_IGNORE_ES3_MSAA=0',
            ],
          },
          'conditions': [
            [ 'OS=="android" and product_name=="sweskia"', {
              'link_settings': {
                'libraries' : [
                  '-L<(SHARED_LIB_DIR)/',
                  '-lswecore',
                ],
              },
              'dependencies': [
                '<(DEPTH)/swe/swe_core/swe_core.gyp:swe_core_native',
          ],
              'defines': [
                  'SKIA_VERSION=<(skia_version)',
          ],
            }],
            [ 'component_skia== "none" and OS=="android"', {
              'link_settings': {
                'libraries' : [
                  '-L<(SHARED_LIB_DIR)/',
                  '-lsweskia',
                ],
              },

              'sources/': [
                  ['exclude', '\\.(cc|cpp)$'],
              ],

              'dependencies': [
                'sweskia',
              ],
              'dependencies!': [
                'skia_library_opts.gyp:skia_opts',
              ],
            }],
          ],
        },
        {
          'target_name': 'skia_library',
          'type': 'none',
        },
        {
          'target_name': 'skia_chrome',
          'type': 'none',
        },
      ],
    }],
    ['skia_opts_ext == "True" and ((target_arch == "arm" and arm_version >= 7 and (arm_neon == 1 or arm_neon_optional == 1)) or target_arch == "arm64")', {
      'targets': [
        {
          'target_name': 'D32_A8_Black_Neon_Test',
          'type': 'executable',
          'dependencies': [
            'skia',
          ],
          'include_dirs': [
            '..',
          ],
          'defines': [
            '__ARM_HAVE_NEON',
          ],
          'sources': [
            '../third_party/skia/src/opts/ext/D32_A8_Black_unittest.cc',
          ],
        },
      ],
    }],
  ],

  # targets that are not dependent upon the component type
  'targets': [
    {
      'target_name': 'image_operations_bench',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        'skia',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'ext/image_operations_bench.cc',
      ],
    },
    {
      'target_name': 'filter_fuzz_stub',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:test_support_base',
        'skia.gyp:skia',
      ],
      'sources': [
        'tools/filter_fuzz_stub/filter_fuzz_stub.cc',
      ],
      'includes': [
        '../build/android/increase_size_for_speed.gypi',
      ],
    },
    {
      'target_name': 'skia_mojo',
      'type': 'static_library',
      'dependencies': [
        'skia',
        '../base/base.gyp:base',
      ],
      'includes': [
        '../third_party/mojo/mojom_bindings_generator.gypi',
      ],
      'sources': [
        # Note: file list duplicated in GN build.
        'public/interfaces/bitmap.mojom',
        'public/type_converters.cc',
      ],
    },
  ],
}
