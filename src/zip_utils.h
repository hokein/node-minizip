// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#ifndef SRC_ZIP_UTILS_H_
#define SRC_ZIP_UTILS_H_

#include <string>

namespace zip {
namespace utils {

std::string RemoveExtraFileSeparator(const std::string& file_path);

bool CreateDir(const std::string& file_path);

bool DirectoryExists(const std::string& file_path);

}  // namespace utils
}  // namespace zip

#endif  // SRC_ZIP_UTILS_H_
