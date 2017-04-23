var minizip = require('../main');
var fs = require('fs');
var assert = require('assert');
var path = require('path');

describe('node-minizip API', function() {
  it('test valid zip file', function(done) {
    minizip.zip('test/data', 'test/data.zip', function(err) {
      if (err)
        throw err;
      minizip.unzip('test/data.zip', 'test/tmp', function(err) {
        if (err)
          throw err;
        assert.notEqual(-1,
          fs.readFileSync('test/tmp/foo.txt').toString().indexOf('foo'));
        assert.notEqual(-1,
          fs.readFileSync('test/tmp/bar.txt').toString().indexOf('bar'));
        done();
      });
    });
  });
  it('test zip non-existent dir', function(done) {
    minizip.zip('test/non-existent', 'test/data.zip', function(err) {
      if (!err)
        assert.fail(err, 'Expecting error');
      done();
    });
  });
  it('test unzip non-existent dir', function(done) {
    minizip.unzip('test/non-exitent.zip', 'test/tmp', function(err) {
      if (!err)
        assert.fail(err, 'Expecting error');
      done();
    });
  });
  it('test valid zip file for single file with encryption from buffer', function(done) {
    var stream = fs.createReadStream('test/tmp/foo.txt');
    var data = [];
    stream.on('data', function(chunk) {
        data.push(chunk);
    });

    stream.on('end', function() {
        var buffer = Buffer.concat(data);
        minizip.zip_encrypt(buffer, 'foo.txt', 'test/data-password.zip', 'password', function(err) {
            if (err){
              throw err;
            }

            minizip.unzip_decrypt('test/data-password.zip', 'test/tmp2', 'password', function(err) {
                if (err){
                    throw err;
                }

                assert.notEqual(-1, fs.readFileSync('test/tmp2/foo.txt').toString().indexOf('foo'));
                done();
          });
        });
    });
  });
});
