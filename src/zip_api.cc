// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include "nan.h"
#include "zip_async_worker.h"

namespace {

NAN_METHOD(ZipEncrypt) {
  Nan::HandleScope scope;
  if (info.Length() < 5 || !info[0]->IsObject() || !info[1]->IsString() || !info[2]->IsString() ||
                           !info[3]->IsString() || !info[4]->IsFunction()){ 
    Nan::ThrowTypeError("Bad arguments");
  }

  v8::Local<v8::Object> stream = info[0].As<v8::Object>();
  int size = node::Buffer::Length(stream);
  void *buffer = node::Buffer::Data(stream);
  std::string relative_path(*(v8::String::Utf8Value(info[1])));
  std::string dest_file(*(v8::String::Utf8Value(info[2])));
  std::string password(*(v8::String::Utf8Value(info[3])));

  Nan::Callback* callback = new Nan::Callback(info[4].As<v8::Function>());

  Nan::AsyncQueueWorker(new zip::ZipEncryptAsyncWorker(buffer, size, relative_path, dest_file, password, callback)); 

  info.GetReturnValue().Set(Nan::Undefined());
}

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

NAN_METHOD(UnzipDecrypt) {
  Nan::HandleScope scope;

  if (info.Length() < 4 || !info[0]->IsString() || !info[1]->IsString() || !info[2]->IsString() || !info[3]->IsFunction()) {
    Nan::ThrowTypeError("Bad arguments");
    return;
  }

  std::string zip_file(*(v8::String::Utf8Value(info[0])));
  std::string dest_dir(*(v8::String::Utf8Value(info[1])));
  std::string password(*(v8::String::Utf8Value(info[2])));

  Nan::Callback* callback = new Nan::Callback(info[3].As<v8::Function>());

  Nan::AsyncQueueWorker(new zip::UnzipDecryptAsyncWorker(zip_file, dest_dir, password, callback));

  info.GetReturnValue().Set(Nan::Undefined());
}

}  // namespace

NAN_MODULE_INIT(init) {
  Nan::Set(target, Nan::New("zip").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(Zip)->GetFunction());
  Nan::Set(target, Nan::New("unzip").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(Unzip)->GetFunction());
  Nan::Set(target, Nan::New("zip_encrypt").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(ZipEncrypt)->GetFunction());
  Nan::Set(target, Nan::New("unzip_decrypt").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(UnzipDecrypt)->GetFunction());
}

NODE_MODULE(node_minizip, init)
