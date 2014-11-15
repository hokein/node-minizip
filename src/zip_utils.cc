// Copyright 2014 Haojian Wu. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include "zip_utils.h"

#include <algorithm>
#include <vector>

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_POSIX)
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
#if defined(OS_WIN)
  DWORD fileattr = GetFileAttributes(file_path.c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#elif defined(OS_POSIX)
  struct stat64 file_info;
  if (stat64(file_path.c_str(), &file_info) == 0)
    return S_ISDIR(file_info.st_mode);
#endif
  return false;
}

bool CreateDir(const std::string& file_path) {
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
#if defined(OS_WIN)
    if (!::CreateDirectory(it->c_str(), NULL))
      return false;
#elif defined(OS_POSIX)
    if (mkdir((*it).c_str(), 0700) != 0)
      return false;
#endif
  }
  return true;
}

}  // namespace utils
}  // namespace zip
