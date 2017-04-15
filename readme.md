# node-minizip

[![Build Status](https://travis-ci.org/hokein/node-minizip.svg?branch=travis-ci)](https://travis-ci.org/hokein/node-minizip)
[![Dependency Status](https://david-dm.org/hokein/node-minizip.svg)](https://david-dm.org/hokein/node-minizip)
[![npm version](https://img.shields.io/npm/v/node-minizip.svg)](https://www.npmjs.com/package/node-minizip)

node-minizip is a node.js addon binding [Minizip](http://www.winimage.com/zLibDll/minizip.html) for handling zip files.

## Install

```
npm install node-minizip
```

## Quick Start

```
var Minizip = require('node-minizip');
Minizip.zip('your/src/dir', 'your/path/name.zip', function(err) {
  if (err)
    console.log(err);
  else
    console.log('zip successfully.');
});

Minizip.unzip('path/to/name.zip', 'dest/dir', function(err) {
  if (err)
    console.log(err);
  else
    console.log('unzip successfully.');
});
```

## APIs

### Minizip.zip(src_dir, dest_file, callback)

* `src_dir` String
* `dest_file` String
* `callback` Function(err)

### Minizip.unzip(zip_file, unzip_dir, callback)

* `zip_file` String
* `unzip_dir` String
* `callback` Function(err)

## License

MIT license. See `LICENSE` file for details.
