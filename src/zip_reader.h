// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_ZIP_READER_H_
#define SRC_ZIP_READER_H_

#include <string>

#include "zlib/contrib/minizip/unzip.h"

namespace zip {

class ZipReader {
 public:
  ZipReader() {}
  ~ZipReader();

  // This class represents information of an entry (file or directory) in
  // a zip file.
  struct EntryInfo {
    EntryInfo() {};
    EntryInfo(const std::string& filename_in_zip,
              const unz_file_info& file_info);

    std::string file_path;
    unz_file_info raw_file_info;
    bool is_directory;
  };

  // Opens the zip file specified by |zip_file_path|. Returns true on
  // success.
  bool Open(const std::string& zip_file_path);

  bool HasMore();

  // Advances the next entry. Returns true on success.
  bool AdvanceToNextEntry();

  bool OpenCurrentEntryInZip();

  // Extracts the current entry to the given output directory path using
  // ExtractCurrentEntryToFilePath(). Sub directories are created as needed
  // based on the file path of the current entry. For example, if the file
  // path in zip is "foo/bar.txt", and the output directory is "output",
  // "output/foo/bar.txt" will be created.
  //
  // Returns true on success. OpenCurrentEntryInZip() must be called
  // beforehand.
  //
  // This function preserves the timestamp of the original entry. If that
  // timestamp is not valid, the timestamp will be set to the current time.
  bool ExtractCurrentEntryIntoDirectory(
      const std::string& output_directory_path);

  bool ExtractCurrentEntryToFilePath(const std::string& output_file_path);

  EntryInfo* current_entry_info() { return &current_entry_info_; }

 private:
  bool OpenInternal();

  EntryInfo current_entry_info_;
  unzFile zip_file_;
  int num_entries_;
  bool reached_end_;
};

}  // namespace zip

#endif  // SRC_ZIP_READER_H_
