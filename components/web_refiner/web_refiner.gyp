{
  'targets' : [
    {
      'target_name': 'web_refiner_native',
      'type': 'none',
      'variables': {
        'swe_core_gyp_path%' : '<(DEPTH)/swe/swe_core/swe_core.gyp',
      },
      'dependencies': [
        '<(swe_core_gyp_path):swe_core_native',
      ],
      'copies': [
        {
          'destination': '<(SHARED_LIB_DIR)',
          'files': [
            '<(DEPTH)/components/web_refiner/<(target_arch)/<(CONFIGURATION_NAME)/libswewebrefiner.so',
          ],
        }
      ],
    },

    {
      'target_name': 'web_refiner_java',
      'type': 'none',
      'variables': {
        'jar_path': '<(DEPTH)/components/web_refiner/java/libswewebrefiner_java.jar',
      },
      'includes':['../../build/java_prebuilt.gypi']
    },
  ],
}
