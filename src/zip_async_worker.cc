// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include "zip_async_worker.h"

#include "zip.h"

namespace zip {

ZipEncryptAsyncWorker::ZipEncryptAsyncWorker(void* buf, const unsigned long buf_size, const std::string& relative_path, const std::string& dest_file, const std::string& password, Nan::Callback *callback) :
     Nan::AsyncWorker(callback),
     buf(buf),
     buf_size(buf_size),
     relative_path(relative_path),
     dest_file(dest_file),
     password(password)
{
}

void ZipEncryptAsyncWorker::Execute() {
  std::string error_message;
  if(!Zip(buf, buf_size, relative_path, dest_file, password, &error_message)){
    SetErrorMessage(error_message.c_str());
  }
}

ZipAsyncWorker::ZipAsyncWorker(const std::string& src_dir,
                               const std::string& dest_file,
                               Nan::Callback* callback):
     Nan::AsyncWorker(callback), src_dir_(src_dir), dest_file_(dest_file) {
}

void ZipAsyncWorker::Execute() {
  std::string error_message;
  if (!Zip(src_dir_, dest_file_, &error_message))
    SetErrorMessage(error_message.c_str());
}

UnzipDecryptAsyncWorker::UnzipDecryptAsyncWorker(const std::string& zip_file, const std::string& dest_dir, const std::string& password, Nan::Callback* callback)
 : Nan::AsyncWorker(callback), 
   zip_file(zip_file), 
   dest_dir(dest_dir), 
   password(password)
{
}

void UnzipDecryptAsyncWorker::Execute() {
  std::string error_message;
  if (!Unzip(zip_file, dest_dir, password, &error_message)){
    SetErrorMessage(error_message.c_str());
  }
}

UnzipAsyncWorker::UnzipAsyncWorker(const std::string& zip_file,
                                   const std::string& dest_dir,
                                   Nan::Callback* callback):
     Nan::AsyncWorker(callback), zip_file_(zip_file), dest_dir_(dest_dir) {
}

void UnzipAsyncWorker::Execute() {
  std::string error_message;
  if (!Unzip(zip_file_, dest_dir_, &error_message))
    SetErrorMessage(error_message.c_str());
}

}  // namespace zip
