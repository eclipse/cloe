/*
 * Copyright 2020 Robert Bosch GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * \file cloe/utility/output_serializer.hpp
 */

#pragma once

#include <fstream>  // for ofstream
#include <string>   // for string
#include <vector>   // for vector<>

#include <boost/algorithm/string/join.hpp>          // for boost::algorithm::join
#include <boost/assign/list_of.hpp>                 // for list_of
#include <boost/bimap.hpp>                          // for bimap
#include <boost/iostreams/copy.hpp>                 // for copy
#include <boost/iostreams/filter/bzip2.hpp>         // for bzip2_compressor
#include <boost/iostreams/filter/gzip.hpp>          // for gzip_compressor
#include <boost/iostreams/filtering_streambuf.hpp>  // for filtering_streambuf
#include <boost/range/adaptor/map.hpp>              // for boost::adaptors::map
#include <boost/range/algorithm/copy.hpp>           // for boost::algorithm::copy

#include <cloe/core.hpp>  // for Logger

namespace cloe {
namespace utility {

class OutputStream {
 public:
  using char_iterator = std::vector<char>::iterator;
  using uint8_iterator = std::vector<uint8_t>::iterator;

  explicit OutputStream(Logger logger) : logger_(logger) {}
  virtual ~OutputStream() = default;
  virtual std::string make_default_filename(const std::string& default_filename) = 0;
  virtual bool open_stream() = 0;
  virtual void write(const char* s, std::streamsize count) = 0;
  virtual void close_stream() = 0;

 protected:
  Logger logger_;
};

template <typename... TSerializerArgs>
class Serializer {
 public:
  Serializer(void (OutputStream::*write_function)(const char*, std::streamsize),
             OutputStream* instance)
      : write_function_(write_function), instance_(instance) {}
  virtual ~Serializer() = default;
  virtual std::string make_default_filename(const std::string& default_filename) = 0;
  virtual void start_array() = 0;
  virtual void serialize(TSerializerArgs... args) = 0;
  virtual void end_array() = 0;

 protected:
  void write(const std::string& str) { (instance_->*write_function_)(str.c_str(), str.length()); }
  void write(const std::vector<uint8_t>& data) {
    (instance_->*write_function_)((const char*)data.data(), data.size());
  }
  void (OutputStream::*write_function_)(const char*, std::streamsize);
  OutputStream* instance_;

  template <typename TSerializer, typename TOutputStream>
  friend class GndTruthSerializerImpl;
};

class BasicFileOutputStream : public OutputStream {
 public:
  using OutputStream::OutputStream;
  virtual ~BasicFileOutputStream() = default;
  bool open_stream() final { return false; }

  [[nodiscard]]
  virtual bool open_file(const std::string& filename, const std::string& default_filename);

  void write(const char* s, std::streamsize count) override { ofs_.write(s, count); }
  void close_stream() override;

 protected:
  std::ofstream ofs_;  // output file stream
};

class FileOutputStream : public BasicFileOutputStream {
 public:
  using BasicFileOutputStream::BasicFileOutputStream;
  virtual ~FileOutputStream() = default;
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename;
  }
};

class FilteringOutputStream : public BasicFileOutputStream {
 public:
  explicit FilteringOutputStream(Logger logger)
      : BasicFileOutputStream(logger), filter_(), out_(&filter_) {}
  virtual ~FilteringOutputStream() = default;

  [[nodiscard]]
  bool open_file(const std::string& filename, const std::string& default_filename) override;

  void write(const char* s, std::streamsize count) override { out_.write(s, count); }
  void close_stream() override;

 protected:
  virtual void configure_filter() = 0;

 protected:
  boost::iostreams::filtering_streambuf<boost::iostreams::output> filter_;
  std::ostream out_;
};

class ZlibOutputStream : public FilteringOutputStream {
 public:
  using FilteringOutputStream::FilteringOutputStream;
  virtual ~ZlibOutputStream() = default;
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename + ".zip";
  }

 protected:
  void configure_filter() override {
    filter_.push(boost::iostreams::gzip_compressor(
        boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
  }
};

class GzipOutputStream : public FilteringOutputStream {
 public:
  using FilteringOutputStream::FilteringOutputStream;
  virtual ~GzipOutputStream() = default;
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename + ".gz";
  }

 protected:
  void configure_filter() override {
    filter_.push(boost::iostreams::gzip_compressor(
        boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
  }
};

class Bzip2OutputStream : public FilteringOutputStream {
 public:
  using FilteringOutputStream::FilteringOutputStream;
  virtual ~Bzip2OutputStream() = default;
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename + ".bz2";
  }

 protected:
  void configure_filter() override {
    filter_.push(boost::iostreams::bzip2_compressor(
        boost::iostreams::bzip2_params(boost::iostreams::bzip2::default_block_size)));
  }
};

/// FileSerializer composes a TSerializer with a TOutputStream
template <typename TSerializer, typename TOutputStream, typename... TSerializerArgs>
class FileSerializer {
 public:
  explicit FileSerializer(Logger logger)
      : outputstream_(logger)
      , serializer_((void (OutputStream::*)(const char*, std::streamsize)) & TOutputStream::write,
                    &outputstream_) {}
  virtual ~FileSerializer() = default;

  [[nodiscard]]
  virtual bool open_file(const std::string& filename, const std::string& default_filename) {
    return outputstream_.open_file(filename, default_filename);
  }

  virtual void serialize(TSerializerArgs... args) { serializer_.serialize(args...); }
  virtual void close_file() { outputstream_.close_stream(); }

 protected:
  TOutputStream outputstream_;
  TSerializer serializer_;
};

/// SequentialFileSerializer is a FileSerializer for sequences of objects of the same type.
template <typename TSerializer, typename TOutputStream, typename... TSerializerArgs>
class SequentialFileSerializer
    : public FileSerializer<TSerializer, TOutputStream, TSerializerArgs...> {
  using base = FileSerializer<TSerializer, TOutputStream, TSerializerArgs...>;

 public:
  using base::base;

  [[nodiscard]]
  bool open_file(const std::string& filename, const std::string& default_filename) override {
    bool ok = base::open_file(filename, default_filename);
    if (ok) {
      on_file_opened();
    }
    return ok;
  }

  void close_file() override {
    on_file_closing();
    base::close_file();
  }

 protected:
  virtual void on_file_opened() = 0;
  virtual void on_file_closing() = 0;
};

}  // namespace utility
}  // namespace cloe
