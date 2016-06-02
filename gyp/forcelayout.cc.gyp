{
  'includes': [
    './common.gypi'
  ],
  'targets': [{
      'target_name': 'forcelayout',
      'type': 'static_library',
      'sources': [
        '../src/layout.cc',
      ],
      'include_dirs': [
          '../include'
      ],
      'includes': [
        './deps.gypi'
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include'
        ],
      },
    },
  ]
}
