{
  'variables' : {
# if client wants to compile this library with openmp support, they will override
# this variable. By default we assume it is not supported.
# PS: With openmp I get 2x performance boost on my MBP 2011.
    'openmp%': 'false',
  },
  'target_defaults': {
    'cflags' : [ '-std=c++11' ],
    'target_conditions': [
      ['_type=="executable"', {
          'xcode_settings': {
            'OTHER_LDFLAGS': [ '-stdlib=libc++' ],
          },
        }
      ],
    ],
    'conditions': [
        [ 'OS=="mac"', {
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS' : [ '-std=c++11', '-stdlib=libc++' ],
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES', # make sure we support exceptions
          },
        }],
        [ 'OS == "mac" and openmp == "true"', {
# TODO: consumers have to export CXX=clang-omp++ before they can use this. How to avoid this?
# TODO: How do I find this library dynamically?
          "libraries+": [ "/usr/local/lib/libiomp5.dylib" ],
          "all_dependent_settings": {
            "libraries+": [ "/usr/local/lib/libiomp5.dylib" ],
          },
          "xcode_settings": {
            "OTHER_CPLUSPLUSFLAGS+" : [ "-fopenmp"],
          },
        }],
      ],
  },
}
