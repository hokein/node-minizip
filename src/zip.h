// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_ZIP_H_
#define SRC_ZIP_H_

#include <string>

namespace zip {

bool Zip(const std::string& src_dir, const std::string& dest_file,
         std::string* error);

bool Unzip(const std::string& zip_file, const std::string& dest_file,
           std::string* error);

}  // namespace zip

#endif  // SRC_ZIP_H_
