// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include "nan.h"
#include "zip_async_worker.h"

namespace {

NAN_METHOD(Zip) {
  Nan::HandleScope scope;

  if (info.Length() < 3 || !info[0]->IsString() || !info[1]->IsString() ||
      !info[2]->IsFunction()) {
    Nan::ThrowTypeError("Bad arguments");
    return;
  }

  std::string src_dir(*(v8::String::Utf8Value(info[0])));
  std::string dest_dir(*(v8::String::Utf8Value(info[1])));
  Nan::Callback* callback = new Nan::Callback(info[2].As<v8::Function>());

  Nan::AsyncQueueWorker(new zip::ZipAsyncWorker(src_dir, dest_dir, callback));

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Unzip) {
  Nan::HandleScope scope;

  if (info.Length() < 3 || !info[0]->IsString() || !info[1]->IsString() ||
      !info[2]->IsFunction()) {
    Nan::ThrowTypeError("Bad arguments");
    return;
  }

  std::string zip_file(*(v8::String::Utf8Value(info[0])));
  std::string dest_dir(*(v8::String::Utf8Value(info[1])));

  Nan::Callback* callback = new Nan::Callback(info[2].As<v8::Function>());

  Nan::AsyncQueueWorker(new zip::UnzipAsyncWorker(zip_file, dest_dir, callback));

  info.GetReturnValue().Set(Nan::Undefined());
}

}  // namespace

NAN_MODULE_INIT(init) {
  Nan::Set(target, Nan::New("zip").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(Zip)->GetFunction());
  Nan::Set(target, Nan::New("unzip").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(Unzip)->GetFunction());
}

NODE_MODULE(node_minizip, init)
