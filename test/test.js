var minizip = require('../main');
var fs = require('fs');
var assert = require('assert');

describe('node-minizip API', function() {
  it('test valid zip file', function(done) {
    minizip.zip('./data', './data.zip', function(err) {
      if (err)
        throw err;
      minizip.unzip('./data.zip', './tmp', function(err) {
        if (err)
          throw err;
        assert.notEqual(-1,
          fs.readFileSync('./tmp/foo.txt').toString().indexOf('foo'));
        assert.notEqual(-1,
          fs.readFileSync('./tmp/bar.txt').toString().indexOf('bar'));
        done();
      });
    });
  });
  it('test zip non-existent dir', function(done) {
    minizip.zip('./non-existent', './data.zip', function(err) {
      if (!err)
        assert.fail(err, 'Expecting error');
      done();
    });
  });
  it('test unzip non-existent dir', function(done) {
    minizip.unzip('./non-exitent.zip', './tmp', function(err) {
      if (!err)
        assert.fail(err, 'Expecting error');
      done();
    });
  });
});
