// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include "nan.h"
#include "zip_async_worker.h"

namespace {

#define THROW_BAD_ARGS(msg)    \
    do {                       \
       NanThrowTypeError(msg); \
       NanReturnUndefined();   \
    } while(0);

NAN_METHOD(Zip) {
  NanScope();

  if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() ||
      !args[2]->IsFunction()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string src_dir(*(v8::String::Utf8Value(args[0])));
  std::string dest_dir(*(v8::String::Utf8Value(args[1])));
  NanCallback* callback = new NanCallback(args[2].As<v8::Function>());

  NanAsyncQueueWorker(new zip::ZipAsyncWorker(src_dir, dest_dir, callback));

  NanReturnUndefined();
}

NAN_METHOD(Unzip) {
  NanScope();

  if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() ||
      !args[2]->IsFunction()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string zip_file(*(v8::String::Utf8Value(args[0])));
  std::string dest_dir(*(v8::String::Utf8Value(args[1])));
  NanCallback* callback = new NanCallback(args[2].As<v8::Function>());

  NanAsyncQueueWorker(new zip::UnzipAsyncWorker(zip_file, dest_dir, callback));

  NanReturnUndefined();
}

}  // namespace

void init(v8::Handle<v8::Object> exports) {
  exports->Set(NanNew("zip"),
               NanNew<v8::FunctionTemplate>(Zip)->GetFunction());
  exports->Set(NanNew("unzip"),
               NanNew<v8::FunctionTemplate>(Unzip)->GetFunction());
}

NODE_MODULE(node_minizip, init)
