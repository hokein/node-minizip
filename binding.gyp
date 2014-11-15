{
  'targets': [
    {
      'target_name': 'node-minizip',
      'sources': [
        'src/zip.cc',
        'src/zip.h',
        'src/zip_api.cc',
        'src/zip_async_worker.cc',
        'src/zip_async_worker.h',
        'src/zip_internal.cc',
        'src/zip_internal.h',
        'src/zip_reader.cc',
        'src/zip_reader.h',
        'src/zip_utils.cc',
        'src/zip_utils.h',
      ],
      'dependencies': [ 'deps/zlib/zlib.gyp:zlib' ],
      'include_dirs': [
        'deps/',
        '<!(node -e "require(\'nan\')")'
      ],
      'conditions': [
        ['OS=="win"', {
          'defines': [
            'OS_WIN',
            # _HAS_EXCEPTIONS must match ExceptionHandling in msvs_settings.
            '_HAS_EXCEPTIONS=0',
          ],
        }],
        ['OS=="mac" or OS=="linux"', {
          'defines': [
            'OS_POSIX',
          ],
        }],
        ['OS=="linux"', {
          'cflags':[
            # Don't warn about the "struct foo f = {0};" initialization pattern.
            '-Wno-missing-field-initializers',
          ],
        }],
      ],
    },
  ]
}
