// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "zip.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include "zlib/contrib/minizip/zip.h"
#include "zip_internal.h"
#include "zip_reader.h"
#include "zip_utils.h"

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_POSIX)
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#endif

namespace {

/**
 * calculate the CRC32 of a data buffer, used to put in the header when encrypting a file
 */
int getCrc(void *data,unsigned long data_size,unsigned long* result_crc)
{
    int err = ZIP_OK;
    unsigned long buf_size = 16384;
    unsigned char buf[buf_size];
    unsigned long size_read = 0;
    unsigned long total_read = 0;
    unsigned long calculate_crc=0;
    const char *dataPtr = (char *)data;

    do
    {
        err = ZIP_OK;
        size_read = (data_size-total_read) > buf_size ? buf_size : data_size-total_read;
        memcpy(buf, dataPtr, size_read);

        if (size_read>0){
            calculate_crc = crc32(calculate_crc,buf,size_read);
        }
        dataPtr += size_read;
        total_read += size_read;

    } while ((err == ZIP_OK) && (size_read>0));

   *result_crc=calculate_crc;

   return err;
}


// Get all relative file paths from root_path.
std::vector<std::string> GetAllFilesFromDirectory(
    const std::string& root_path, const std::string& relative_path) {
  std::string absolute_path = root_path + "/" + relative_path;
  std::vector<std::string> files;
#if defined(OS_WIN)
  WIN32_FIND_DATA fd;
  HANDLE hFind = ::FindFirstFile((absolute_path + "/*").c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      std::string entry_relative_file_path;
      if (relative_path.empty())
        entry_relative_file_path = std::string(fd.cFileName);
      else
        entry_relative_file_path = relative_path + "/" +
            std::string(fd.cFileName);
      if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        files.push_back(entry_relative_file_path);
      } else if (fd.cFileName[0] != '.') { // skip "." and ".." directory.
        std::vector<std::string> sub_files = GetAllFilesFromDirectory(
            absolute_path, entry_relative_file_path);
        files.insert(files.end(), sub_files.begin(), sub_files.end());
      }
    } while(::FindNextFile(hFind, &fd));
    ::FindClose(hFind);
  }
#elif defined(OS_POSIX)
  DIR* dir = opendir(absolute_path.c_str());
  if (dir != NULL) {
    struct dirent* entry;
    while((entry = readdir(dir))) {
      if (strcmp(entry->d_name, "..") == 0 ||
          strcmp(entry->d_name, ".") == 0) {
        continue;
      }
      std::string entry_relative_file_path;
      if (relative_path.empty())
        entry_relative_file_path = std::string(entry->d_name);
      else
        entry_relative_file_path = relative_path + "/" +
            std::string(entry->d_name);
      if (entry->d_type & DT_DIR) {
        std::vector<std::string> sub_files = GetAllFilesFromDirectory(
            absolute_path, entry_relative_file_path);
        files.insert(files.end(), sub_files.begin(), sub_files.end());
      } else {
        files.push_back(entry_relative_file_path);
      }
    }
  }
#endif
  return files;
}

//Returns a zip_fileinfo with the last modification date of |path| set.
bool GetFileInfoForZipping(const std::string& path, zip_fileinfo* zip_info) {
#if defined(OS_WIN)
  SYSTEMTIME localTime;

  GetLocalTime(&localTime);
  zip_info->tmz_date.tm_sec = localTime.wSecond;
  zip_info->tmz_date.tm_min = localTime.wMinute;
  zip_info->tmz_date.tm_hour = localTime.wHour;
  zip_info->tmz_date.tm_mday = localTime.wDay;
  zip_info->tmz_date.tm_mon = localTime.wMonth;
  zip_info->tmz_date.tm_year = localTime.wYear;
  return true;
#elif defined(OS_POSIX)
  struct stat s;
  if (stat(path.c_str(), &s) == 0) {
    struct tm* file_date = localtime(&s.st_mtime);
    zip_info->tmz_date.tm_sec  = file_date->tm_sec;
    zip_info->tmz_date.tm_min  = file_date->tm_min;
    zip_info->tmz_date.tm_hour = file_date->tm_hour;
    zip_info->tmz_date.tm_mday = file_date->tm_mday;
    zip_info->tmz_date.tm_mon  = file_date->tm_mon ;
    zip_info->tmz_date.tm_year = file_date->tm_year;
    return true;
  }
#endif
  return false;
}

//Returns a zip_fileinfo with the current time as the modification date
bool GetBufferInfoForZipping(zip_fileinfo* zip_info) {
#if defined(OS_WIN)
  FILETIME file_time;
  HANDLE file_handle;
  WIN32_FIND_DATAA find_data;

    GetLocalTime(&(find_data.ftLastWriteTime), &file_time);
    FileTimeToDosDateTime(&file_time,
                          (LPWORD)(&zip_info->dosDate) + 1,
                          (LPWORD)(&zip_info->dosDate) + 0);
    FindClose(file_handle);
    return true
#elif defined(OS_POSIX)
    time_t rawtime;
    struct tm* file_date = localtime(&rawtime);
    zip_info->tmz_date.tm_sec  = file_date->tm_sec;
    zip_info->tmz_date.tm_min  = file_date->tm_min;
    zip_info->tmz_date.tm_hour = file_date->tm_hour;
    zip_info->tmz_date.tm_mday = file_date->tm_mday;
    zip_info->tmz_date.tm_mon  = file_date->tm_mon ;
    zip_info->tmz_date.tm_year = file_date->tm_year;
    return true;
#endif
}

bool AddFileToZip(zipFile zip_file, const std::string& absolute_path) {
  FILE* file = fopen(absolute_path.c_str(), "rb");
  if (!file)
    return false;
  size_t num_bytes;
  char buf[zip::internal::kZipBufSize];

  do {
    num_bytes = fread(buf, sizeof(char), zip::internal::kZipBufSize, file);
    if (num_bytes > 0) {
      if (ZIP_OK != zipWriteInFileInZip(
          zip_file, buf, static_cast<unsigned int>(num_bytes)))
        return false;
    }
  } while (num_bytes > 0);

  std::fclose(file);
  return true;
}

bool AddFileToZip(zipFile zip_file, void* buf, unsigned long buf_size) {
  return ZIP_OK == zipWriteInFileInZip(zip_file, buf, buf_size);
}


bool AddEntryToZip(zipFile zip_file, const std::string& root_path,
                   const std::string& relative_path) {
  std::string parent_path = root_path;
#if defined(OS_WIN)
  std::replace(parent_path.begin(), parent_path.end(), '\\', '/');
#endif
  zip_fileinfo file_info = {};
  std::string absolute_path = zip::utils::RemoveExtraFileSeparator(
      parent_path + "/" + relative_path);
  GetFileInfoForZipping(absolute_path, &file_info);

  if (ZIP_OK != zipOpenNewFileInZip3(
                    zip_file,  // file
                    relative_path.c_str(),  // relative filename
                    &file_info,  // zipfi
                    NULL,  // extrafield_local,
                    0u,  // size_extrafield_local
                    NULL,  // extrafield_global
                    0u,  // size_extrafield_global
                    NULL,  // comment
                    Z_DEFLATED,  // method
                    Z_DEFAULT_COMPRESSION,  // level
                    0,  // raw
                    -MAX_WBITS,  // windowBits
                    DEF_MEM_LEVEL,  // memLevel
                    Z_DEFAULT_STRATEGY,  // strategy
                    NULL,  // password
                    0)) {  // crcForCrypting
    return false;
  }

  bool success = true;
  success = AddFileToZip(zip_file, absolute_path);

  if (ZIP_OK != zipCloseFileInZip(zip_file))
    return false;

  return success;
}

bool AddEncryptedEntryToZip(zipFile zip_file, void *buf, unsigned long buf_size, const std::string& relative_path, const char* password) {
  unsigned long crcFile=0;
  int zip64 = 0;
  int err=ZIP_OK;

#if defined(OS_WIN)
  std::replace(parent_path.begin(), parent_path.end(), '\\', '/');
#endif

  zip_fileinfo file_info = {};
  GetBufferInfoForZipping( &file_info);

  if (password != NULL){ 
    err = getCrc(buf,buf_size,&crcFile);
  }

  if (err==ZIP_OK){
      zip64 = buf_size >= 0xffffffff;
      err = zipOpenNewFileInZip3_64(
                        zip_file,  // file
                        relative_path.c_str(),  // relative filename
                        &file_info,  // zipfi
                        NULL,  // extrafield_local,
                        0u,  // size_extrafield_local
                        NULL,  // extrafield_global
                        0u,  // size_extrafield_global
                        NULL,  // comment
                        Z_DEFLATED,  // method
                        Z_DEFAULT_COMPRESSION,  // level
                        0,  // raw
                        -MAX_WBITS,  // windowBits
                        DEF_MEM_LEVEL,  // memLevel
                        Z_DEFAULT_STRATEGY,  // strategy
                        password,  // password
                        crcFile,
                        zip64);  // crcForCrypting
  }
  
  if(err == ZIP_OK) {
      bool success = true;
      success = AddFileToZip(zip_file, buf, buf_size);

      err = zipCloseFileInZip(zip_file);
  }

  return err == ZIP_OK;
}

}  // namespace

namespace zip {

bool Zip(void* buf, const unsigned long buf_size, const std::string& relative_path, const std::string& dest_file, const std::string& password, std::string* error) {
  zipFile zip_file = internal::OpenForZipping(dest_file.c_str(), APPEND_STATUS_CREATE);
  if (!zip_file) {
    if (error){
      *error = "Couldn't create file " + dest_file + ".";
    }
    return false;
  }

  if(!AddEncryptedEntryToZip(zip_file, buf, buf_size, relative_path, password.c_str())) {
    if (error){
      *error = "Couldn't add encrypted entry to zip file " + dest_file + ".";
    }
    return false;
  }

  if (ZIP_OK != zipClose(zip_file, NULL)) {
    if (error){
      *error = "Error closing zip file " + dest_file + ".";
    }
    return false;
  }
  return true;
}
bool Zip(const std::string& src_dir, const std::string& dest_file, std::string* error) {
  if (!utils::DirectoryExists(src_dir)) {
    if (error)
      *error = "Dir " + src_dir + " is not existent.";
    return false;
  }

  zipFile zip_file = internal::OpenForZipping(dest_file.c_str(),
                                              APPEND_STATUS_CREATE);
  if (!zip_file) {
    if (error)
      *error = "Couldn't create file " + dest_file + ".";
    return false;
  }

  std::vector<std::string> files = GetAllFilesFromDirectory(src_dir, "");
  for (size_t i = 0; i < files.size(); ++i) {
    if (!AddEntryToZip(zip_file, src_dir, files[i])) {
      if (error)
        *error = "Fail to add " + files[i] + " to zip.";
      return false;
    }
  }
  if (ZIP_OK != zipClose(zip_file, NULL)) {
    if (error)
      *error = "Error closing zip file " + dest_file + ".";
    return false;
  }
  return true;
}

bool Unzip(const std::string& zip_file, const std::string& dest_dir,
           std::string* error) {
  if (dest_dir.empty()) {
    if (error)
      *error = "dest_dir shouldn't be empty.";
    return false;
  }

  ZipReader reader;
  if (!reader.Open(zip_file)) {
    if (error)
      *error = "Failed to open " + zip_file + ".";
    return false;
  }
  while (reader.HasMore()) {
    if (!reader.OpenCurrentEntryInZip()) {
      if (error)
        *error = "Failed to open the current file in zip.";
      return false;
    }
    if (!reader.ExtractCurrentEntryIntoDirectory(dest_dir)) {
      if (error)
        *error = "Failed to extract " +
            reader.current_entry_info()->file_path;
      return false;
    }
    if (!reader.AdvanceToNextEntry()) {
      if (error)
        *error = "Failed to advance to the next file";
      return false;
    }
  }
  return true;
}

bool Unzip(const std::string& zip_file, const std::string& dest_dir, const std::string& password, std::string* error) {
  if (dest_dir.empty()) {
    if (error)
      *error = "dest_dir shouldn't be empty.";
    return false;
  }

  ZipReader reader;
  if (!reader.Open(zip_file)) {
    if (error)
      *error = "Failed to open " + zip_file + ".";
    return false;
  }
  while (reader.HasMore()) {
    if (!reader.OpenCurrentEntryInZip()) {
      if (error)
        *error = "Failed to open the current file in zip.";
      return false;
    }
    if (!reader.ExtractCurrentEntryIntoDirectory(dest_dir, password)) {
      if (error)
        *error = "Failed to extract " +
            reader.current_entry_info()->file_path;
      return false;
    }
    if (!reader.AdvanceToNextEntry()) {
      if (error)
        *error = "Failed to advance to the next file";
      return false;
    }
  }
  return true;
}

}  // namespace zip
