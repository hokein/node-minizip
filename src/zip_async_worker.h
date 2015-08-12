// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#ifndef SRC_ZIP_WORKER_H_
#define SRC_ZIP_WORKER_H_

#include <string>

#include "nan.h"

namespace zip {

class ZipAsyncWorker : public Nan::AsyncWorker {
 public:
  ZipAsyncWorker(const std::string& src_dir, const std::string& dest_file,
      Nan::Callback* callback);

  // Override Nan::AsyncWorker methods.
  virtual void Execute();
 private:
  const std::string src_dir_;
  const std::string dest_file_;
};

class UnzipAsyncWorker : public Nan::AsyncWorker {
 public:
  UnzipAsyncWorker(const std::string& zip_file, const std::string& dest_dir,
      Nan::Callback* callback);

  // Override Nan::AsyncWorker methods.
  virtual void Execute();
 private:
  const std::string zip_file_;
  const std::string dest_dir_;
};

}  // namespace zip

#endif  // SRC_ZIP_WORKER_H_
