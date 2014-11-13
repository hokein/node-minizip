// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include "zip_utils.h"

#include <algorithm>
#include <vector>

#if defined(OS_POSIX)
#include <sys/stat.h>
#endif

namespace zip {
namespace utils {

struct FileSeparatorChecker {
  bool operator()(char a, char b) const {
    return a == '/' && a == b;
  }
};

std::string RemoveExtraFileSeparator(const std::string& file_path) {
  std::string result = file_path;
  std::unique(result.begin(), result.end(), FileSeparatorChecker());
  return result;
}

bool DirectoryExists(const std::string& file_path) {
#if defined(OS_POSIX)
  struct stat64 file_info;
  if (stat64(file_path.c_str(), &file_info) == 0)
    return S_ISDIR(file_info.st_mode);
#endif
  return false;
}

bool CreateDirectory(const std::string& file_path) {
  if (DirectoryExists(file_path))
    return true;
  std::vector<std::string> subpaths;
  std::string path = file_path;
  while (1) {
    subpaths.push_back(path);
    std::string::size_type last_pos = path.find_last_of('/');
    if (last_pos == std::string::npos)
      break;
    path = path.substr(0, last_pos);
  }
  for (std::vector<std::string>::iterator it = subpaths.begin();
       it != subpaths.end(); ++it) {
    if (DirectoryExists(*it))
      continue;
#if defined(OS_POSIX)
    if (mkdir((*it).c_str(), 0700) != 0)
      return false;
#endif
  }
  return true;
}

}  // namespace utils
}  // namespace zip
