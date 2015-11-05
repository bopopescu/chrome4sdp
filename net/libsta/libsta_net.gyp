{
  'targets': [
    {
      'target_name': 'libsta_net',
      'type': '<(component)',
      'include_dirs': [
          '../..',
      ],
      'dependencies': [
      ],
      'sources': [
          'common/interfaces/interface_types.cc',
          'common/utils/tp_helper.cc',
          'common/utils/tw_helper.cc',
          '../base/dependant_iobuffer.cc',
      ],
    },
  ],
}
