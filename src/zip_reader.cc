// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "zip_reader.h"

#include <cstdio>
#include <vector>

#include "zip_internal.h"
#include "zip_utils.h"

#if defined(OS_POSIX)
#include <utime.h>
#endif

namespace {

void UpdateFileTime(const std::string& file_path,
                    uLong dosdate,
                    tm_unz tmu_date) {
#if defined(OS_WIN)
  FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;
  HANDLE handle = CreateFileA(file_path.c_str(), GENERIC_READ | GENERIC_WRITE,
      0, NULL, OPEN_EXISTING, 0, NULL);
  GetFileTime(handle, &ftCreate, &ftLastAcc, &ftLastWrite);
  DosDateTimeToFileTime(static_cast<WORD>(dosdate >> 16),
                        static_cast<WORD>(dosdate),
                        &ftLocal);
  LocalFileTimeToFileTime(&ftLocal, &ftm);
  SetFileTime(handle, &ftm, &ftLastAcc, &ftm);
  CloseHandle(handle);
  return;
#elif defined(OS_POSIX)
  struct utimbuf ut;
  struct tm date;
  date.tm_sec = tmu_date.tm_sec;
  date.tm_min = tmu_date.tm_min;
  date.tm_hour = tmu_date.tm_hour;
  date.tm_mday = tmu_date.tm_mday;
  date.tm_mon = tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
    date.tm_year = tmu_date.tm_year - 1900;
  else
    date.tm_year = tmu_date.tm_year;
  date.tm_isdst = -1;

  ut.actime = ut.modtime = mktime(&date);
  utime(file_path.c_str(), &ut);
#endif
}

}  // namespace

namespace zip {

ZipReader::~ZipReader() {
  if (zip_file_)
    unzClose(zip_file_);
}
  
ZipReader::EntryInfo::EntryInfo(const std::string& file_name_in_zip,
                                const unz_file_info& file_info)
    : file_path(file_name_in_zip), raw_file_info(file_info),
      is_directory(false) {
  // Directory entries in zip files end with "/".
  if (file_name_in_zip.size())
    is_directory = file_name_in_zip[file_name_in_zip.size()-1] == '/';
}

bool ZipReader::Open(const std::string& zip_file_path) {
  zip_file_ = internal::OpenForUnzipping(zip_file_path.c_str());
  if (!zip_file_)
    return false;

  return OpenInternal();
}

bool ZipReader::HasMore() {
  return !reached_end_;
}

bool ZipReader::AdvanceToNextEntry() {
  // Should not go further if we already reached the end.
  if (reached_end_)
    return false;

  unz_file_pos position = {};
  if (unzGetFilePos(zip_file_, &position) != UNZ_OK)
    return false;
  const int current_entry_index = position.num_of_file;
  // If we are currently at the last entry, then the next position is the
  // end of the zip file, so mark that we reached the end.
  if (current_entry_index + 1 == num_entries_) {
    reached_end_ = true;
  } else {
    if (unzGoToNextFile(zip_file_) != UNZ_OK) {
      return false;
    }
  }
  return true;
}

bool ZipReader::OpenCurrentEntryInZip() {
  unz_file_info raw_file_info = {};
  char raw_file_name_in_zip[internal::kZipMaxPath] = {};
  const int result = unzGetCurrentFileInfo(zip_file_,
                                           &raw_file_info,
                                           raw_file_name_in_zip,
                                           sizeof(raw_file_name_in_zip) - 1,
                                           NULL,  // extraField.
                                           0,  // extraFieldBufferSize.
                                           NULL,  // szComment.
                                           0);  // commentBufferSize.
  if (result != UNZ_OK)
    return false;
  if (raw_file_name_in_zip[0] == '\0')
    return false;
  current_entry_info_ = EntryInfo(std::string(raw_file_name_in_zip),
                                  raw_file_info);
  return true;
}

bool ZipReader::ExtractCurrentEntryIntoDirectory(
    const std::string& output_directory_path) {
  if (!utils::DirectoryExists(output_directory_path))
    if (!utils::CreateDir(output_directory_path))
      return false;
  const std::string output_file_path = output_directory_path + "/" +
      current_entry_info()->file_path;
  return ExtractCurrentEntryToFilePath(output_file_path);
}

bool ZipReader::ExtractCurrentEntryToFilePath(
    const std::string& output_file_path) {
  std::string absolute_path = utils::RemoveExtraFileSeparator(output_file_path);
  // If this is a directory, just create it and return.
  if (current_entry_info()->is_directory)
    return utils::CreateDir(absolute_path);
  // Make sure the parent directory is created.
  // The absolute_path is guaranteed valid(must has at lease '/').
  if (!utils::CreateDir(
      absolute_path.substr(0, absolute_path.find_last_of('/'))))
    return false;

  const int open_result = unzOpenCurrentFile(zip_file_);
  if (open_result != UNZ_OK)
    return false;

  FILE* file = fopen(output_file_path.c_str(), "wb");
  if (!file)
    return false;

  bool success = true;
  while (true) {
    char buf[internal::kZipBufSize];
    const int num_bytes_read = unzReadCurrentFile(zip_file_, buf,
                                                  internal::kZipBufSize);
    if (num_bytes_read == 0) {
      // Reached the end of the file.
      break;
    } else if (num_bytes_read < 0) {
      // If num_bytes_read < 0, then it's a specific UNZ_* error code.
      success = false;
      break;
    } else if (num_bytes_read > 0) {
      if (num_bytes_read != static_cast<int>(fwrite(
          buf, sizeof(char), num_bytes_read, file))) {
        success = false;
        break;
      }
    }
  }

  fclose(file);
  unzCloseCurrentFile(zip_file_);

  UpdateFileTime(output_file_path,
                 current_entry_info()->raw_file_info.dosDate,
                 current_entry_info()->raw_file_info.tmu_date);

  return success;
}

bool ZipReader::OpenInternal() {
  unz_global_info zip_info = {};
  if (unzGetGlobalInfo(zip_file_, &zip_info) != UNZ_OK)
    return false;
  num_entries_ = zip_info.number_entry;
  if (num_entries_ < 0)
    return false;
  reached_end_ = (num_entries_ == 0);
  return true;
}

}  // namespace zip
