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
#ifndef CLOE_UTILITY_OUTPUT_SERIALIZER_HPP_
#define CLOE_UTILITY_OUTPUT_SERIALIZER_HPP_

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
  typedef typename std::vector<char>::iterator char_iterator;
  typedef typename std::vector<uint8_t>::iterator uint8_iterator;

  explicit OutputStream(Logger& logger) : logger_(logger) {}
  virtual ~OutputStream() = default;
  virtual std::string make_default_filename(const std::string& default_filename) = 0;
  virtual bool open_stream() = 0;
  virtual void write(const char* s, std::streamsize count) = 0;
  virtual void close_stream() = 0;

 protected:
  Logger& logger_;
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

class AbstractJsonSerializerBase {
 protected:
  static const std::string json_array_open;
  static const std::string json_array_close;
};

const std::string AbstractJsonSerializerBase::json_array_open("\n[\n");   // NOLINT
const std::string AbstractJsonSerializerBase::json_array_close("\n]\n");  // NOLINT

template <typename... TSerializerArgs>
class AbstractJsonSerializer : public Serializer<TSerializerArgs...>,
                               public AbstractJsonSerializerBase {
 public:
  typedef Serializer<TSerializerArgs...> base;
  using Serializer<TSerializerArgs...>::Serializer;
  ~AbstractJsonSerializer() override {}
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename + ".json";
  }
  void start_array() override { base::write(json_array_open); }
  void end_array() override { base::write(json_array_close); }
};

template <typename T, typename... TSerializerArgs>
class AbstractMsgPackSerializer : public Serializer<TSerializerArgs...> {
 public:
  typedef Serializer<TSerializerArgs...> base;
  using Serializer<TSerializerArgs...>::Serializer;
  ~AbstractMsgPackSerializer() override {}
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename + ".msg";
  }
  void start_array() override {}
  void end_array() override { base::write(Json::to_msgpack(Json(data_))); }

 protected:
  std::vector<T> data_;
};

class BasicFileOutputStream : public OutputStream {
 public:
  using OutputStream::OutputStream;
  ~BasicFileOutputStream() override {}
  bool open_stream() final { return false; }
  virtual bool open_file(const std::string& filename, const std::string& default_filename);
  void write(const char* s, std::streamsize count) override { ofs_.write(s, count); }
  void close_stream() override;

 protected:
  std::ofstream ofs_;  // output file stream
};

bool BasicFileOutputStream::open_file(const std::string& filename,
                                      const std::string& default_filename) {
  const auto& output_file = filename == "" ? default_filename : filename;
  if (&output_file == &default_filename) {
    logger_->warn("No output file specified, using {}", output_file);
  }

  ofs_.open(output_file);
  bool succes = !ofs_.fail();
  if (succes) {
    logger_->info("Writing output to file: {}", output_file);
  } else {
    logger_->error("Error opening file for writing: {}", output_file);
  }
  return succes;
}

void BasicFileOutputStream::close_stream() {
  if (ofs_.is_open()) {
    ofs_.close();
  }
}

class FileOutputStream : public BasicFileOutputStream {
 public:
  using BasicFileOutputStream::BasicFileOutputStream;
  ~FileOutputStream() override {}
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename;
  }
};

class FilteringOutputStream : public BasicFileOutputStream {
 public:
  explicit FilteringOutputStream(Logger& logger)
      : BasicFileOutputStream(logger), filter_(), out_(&filter_) {}
  ~FilteringOutputStream() override {}
  bool open_file(const std::string& filename, const std::string& default_filename) override {
    auto success = BasicFileOutputStream::open_file(filename, default_filename);
    if (success) {
      configure_filter();
      filter_.push(ofs_);  // attach sink stream to the filter
    }
    return success;
  }
  void write(const char* s, std::streamsize count) override { out_.write(s, count); }
  void close_stream() override {
    filter_.pop();                          // remove sink stream from filter
    BasicFileOutputStream::close_stream();  // close sink
  }

 protected:
  virtual void configure_filter() = 0;

 protected:
  boost::iostreams::filtering_streambuf<boost::iostreams::output> filter_;
  std::ostream out_;
};

class ZlibOutputStream : public FilteringOutputStream {
 public:
  using FilteringOutputStream::FilteringOutputStream;
  ~ZlibOutputStream() override {}
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
  ~GzipOutputStream() override {}
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
  ~Bzip2OutputStream() override {}
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
  explicit FileSerializer(Logger& logger)
      : outputstream_(logger)
      , serializer_((void (OutputStream::*)(const char*, std::streamsize)) & TOutputStream::write,
                    &outputstream_) {}
  virtual ~FileSerializer() = default;
  virtual void open_file(const std::string& filename, const std::string& default_filename) {
    outputstream_.open_file(filename, default_filename);
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
  typedef FileSerializer<TSerializer, TOutputStream, TSerializerArgs...> base;

 public:
  using base::base;
  void open_file(const std::string& filename, const std::string& default_filename) override {
    base::open_file(filename, default_filename);
    on_file_opened();
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

#endif  // CLOE_UTILITY_OUTPUT_SERIALIZER_HPP_
