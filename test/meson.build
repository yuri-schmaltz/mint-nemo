
test('Search Engine test',
  executable('test-nemo-search-engine',
    [ 'test-nemo-search-engine.c' ],
    include_directories: [ rootInclude, ],
    dependencies: [ gtk, nemo_private ],
  ),
  args: []
)

test('Directory Async test',
  executable('test-nemo-directory-async',
    [ 'test-nemo-directory-async.c' ],
    include_directories: [ rootInclude, ],
    dependencies: [ gtk, nemo_private ],
  ),
  args: []
)

test('Copy test',
  executable('test-nemo-copy',
    [ 'test-copy.c', 'test.c' ],
    include_directories: [ rootInclude, ],
    dependencies: [ gtk, nemo_private ],
    c_args: nemo_definitions,
  ),
  args: []
)

test('Icon Text Cache test',
  executable('test-icon-text-cache',
    [ 'test-icon-text-cache.c' ],
    include_directories: [ rootInclude, ],
    dependencies: [ gtk, nemo_private ],
  ),
  args: []
)

  test('Lazy Thumbnail Loader test',
    executable('test-lazy-thumbnail-loader',
      [ 'test-lazy-thumbnail-loader.c' ],
      include_directories: [ rootInclude, ],
      dependencies: [ gtk, nemo_private ],
    ),
    args: []
  )
