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
 * \file gndtruth_extractor.cpp
 */

#include <cassert>  // for assert
#include <chrono>   // for chrono, duration_cast<>
#include <memory>   // for unique_ptr<>

#include <cloe/controller.hpp>                 // for Controller, ControllerFactory, ...
#include <cloe/plugin.hpp>                     // for EXPORT_CLOE_PLUGIN
#include <cloe/sync.hpp>                       // for Sync
#include <cloe/utility/output_serializer.hpp>  // for Outputstream, JSONSerializer, ...
#include <cloe/vehicle.hpp>                    // for Vehicle
using namespace cloe::utility;                 // NOLINT(build/namespaces)

#include "enumconfable.hpp"  // for EnumConfable

namespace cloe {

enum OutputTypeEnum {
  JSON_BZIP2,
  JSON_GZIP,
  JSON_ZIP,
  JSON,
  MSGPACK_BZIP2,
  MSGPACK_GZIP,
  MSGPACK_ZIP,
  MSGPACK,
};

typedef EnumStringConfable<OutputTypeEnum> OutputType;

// clang-format off
IMPLEMENT_ENUMSTRINGMAP(cloe::OutputTypeEnum)
    (cloe::OutputTypeEnum::JSON_BZIP2   , "json.bz2"    )
    (cloe::OutputTypeEnum::JSON_GZIP    , "json.gz"     )
    (cloe::OutputTypeEnum::JSON_ZIP     , "json.zip"    )
    (cloe::OutputTypeEnum::JSON         , "json"        )
    (cloe::OutputTypeEnum::MSGPACK_BZIP2, "msgpack.bz2" )
    (cloe::OutputTypeEnum::MSGPACK_GZIP , "msgpack.gz"  )
    (cloe::OutputTypeEnum::MSGPACK_ZIP  , "msgpack.zip" )
    (cloe::OutputTypeEnum::MSGPACK      , "msgpack"     )
;
// clang-format on

namespace controller {

struct GndTruthExtractorConfiguration : public Confable {
  std::string output_file;
  OutputType output_type;
  std::vector<std::string> components;

  CONFABLE_SCHEMA(GndTruthExtractorConfiguration) {
    return Schema{
        {"components", Schema(&components, "array of components to be extracted")},
        {"output_file", Schema(&output_file, "file path to write groundtruth output to")},
        {"output_type", Schema(&output_type, "type of output file to write")},
    };
  }

  void to_json(Json& j) const override {
    j = Json{
        {"components", components},
        {"output_file", output_file},
        {"output_type", output_type},
    };
  }

  // TODO(ben): Is this override still necessary?
  virtual void from_conf(const Conf& conf) override {
    Confable::from_conf(conf);
    conf.try_from("output_type", &output_type);
  }
};

// Approximate size of GndTruth is minimally: 1K
// Simulation with 100s * 50/s * 1K = 5000 K
struct GndTruth {
  Duration sim_time;
  uint64_t sim_step;
  std::map<std::string, std::shared_ptr<Component> > components;

  friend void to_json(Json& j, const GndTruth& g) {
    j = Json{
        {"sim_time", std::chrono::duration_cast<Seconds>(g.sim_time).count()},
        {"sim_step", g.sim_step},
        {"components", g.components},
    };
  }
};

class GndTruthJsonSerializer : public AbstractJsonSerializer<const Sync&, const GndTruth&> {
 public:
  typedef AbstractJsonSerializer<const Sync&, const GndTruth&> base;
  using base::base;
  virtual void serialize(const Sync& sync, const GndTruth& gt) override {
    if (sync.step() > 1) {
      write(",\n");  // serialize delimiting comma, if already one dataset was serialized
    }
    auto txt = (Json(gt)).dump(3);  // serialize to json with level 3 indent
    write(txt);
  }
};

class GndTruthMsgPackSerializer
    : public AbstractMsgPackSerializer<GndTruth, const Sync&, const GndTruth&> {
 public:
  typedef AbstractMsgPackSerializer<GndTruth, const Sync&, const GndTruth&> base;
  using base::base;
  virtual void serialize(const Sync&, const GndTruth& gt) override { data_.emplace_back(gt); }
};

/// GndTruthSerializer is
/// 1) Interface for the GndTruthExtractor
/// 2) the anchor point for exactly one instance of the default_filename
class GndTruthSerializer {
 public:
  virtual ~GndTruthSerializer() = 0;
  virtual void open_file(const std::string& filename) = 0;
  virtual void serialize(const Sync& sync, const GndTruth& gt) = 0;
  virtual void close_file() = 0;

 protected:
  static const std::string default_filename;
};

GndTruthSerializer::~GndTruthSerializer() {}

const std::string GndTruthSerializer::default_filename = "/tmp/cloe_gndtruth";

/// GndTruthSerializerImpl is the implementation of GndTruthSerializer
template <typename TSerializer, typename TOutputStream>
class GndTruthSerializerImpl
    : public SequentialFileSerializer<TSerializer, TOutputStream, const Sync&, const GndTruth&>,
      public GndTruthSerializer {
  typedef SequentialFileSerializer<TSerializer, TOutputStream, const Sync&, const GndTruth&> base1;

 public:
  GndTruthSerializerImpl(Logger& logger) : base1(logger), GndTruthSerializer() {}
  virtual ~GndTruthSerializerImpl() override;
  using base1::open_file;
  virtual void open_file(const std::string& filename) override {
    std::string default_name = this->outputstream_.make_default_filename(
        this->serializer_.make_default_filename(default_filename));
    base1::open_file(filename, default_name);
  }
  virtual void serialize(const Sync& sync, const GndTruth& gt) override {
    base1::serialize(sync, gt);
  }
  virtual void close_file() override { base1::close_file(); }

 protected:
  virtual void on_file_opened() override { this->serializer_.start_array(); }
  virtual void on_file_closing() override { this->serializer_.end_array(); }
};

template <typename TSerializer, typename TOutputStream>
GndTruthSerializerImpl<TSerializer, TOutputStream>::~GndTruthSerializerImpl() {}

template <typename TOutput>
using JsonOutputSerializer = GndTruthSerializerImpl<GndTruthJsonSerializer, TOutput>;
typedef JsonOutputSerializer<FileOutputStream> JsonSerializer;
typedef JsonOutputSerializer<ZlibOutputStream> ZlibJsonSerializer;
typedef JsonOutputSerializer<GzipOutputStream> GZipJsonSerializer;
typedef JsonOutputSerializer<Bzip2OutputStream> BZip2JsonSerializer;

template <typename TOutput>
using MsgPackOutputSerializer = GndTruthSerializerImpl<GndTruthMsgPackSerializer, TOutput>;
typedef MsgPackOutputSerializer<FileOutputStream> MsgPackSerializer;
typedef MsgPackOutputSerializer<ZlibOutputStream> ZlibMsgPackSerializer;
typedef MsgPackOutputSerializer<GzipOutputStream> GZipMsgPackSerializer;
typedef MsgPackOutputSerializer<Bzip2OutputStream> BZip2MsgPackSerializer;

std::unique_ptr<GndTruthSerializer> makeGndTruthSerializer(OutputTypeEnum type, Logger log) {
  std::unique_ptr<GndTruthSerializer> result;
  switch (type) {
    case JSON_BZIP2:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<BZip2JsonSerializer>(log));
      break;
    case JSON_GZIP:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<GZipJsonSerializer>(log));
      break;
    case JSON_ZIP:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<ZlibJsonSerializer>(log));
      break;
    case JSON:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<JsonSerializer>(log));
      break;
    case MSGPACK_BZIP2:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<BZip2MsgPackSerializer>(log));
      break;
    case MSGPACK_GZIP:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<GZipMsgPackSerializer>(log));
      break;
    case MSGPACK_ZIP:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<ZlibMsgPackSerializer>(log));
      break;
    case MSGPACK:
      result = std::unique_ptr<GndTruthSerializer>(std::make_unique<MsgPackSerializer>(log));
      break;
    default:
      result = makeGndTruthSerializer(OutputTypeEnum::JSON, log);
      break;
  }
  return result;
}

class GndTruthExtractor : public Controller {
 public:
  GndTruthExtractor(const std::string& name, const GndTruthExtractorConfiguration& c)
      : Controller(name), config_(c) {}

  virtual ~GndTruthExtractor() noexcept = default;

  void start(const Sync& sync) override {
    Controller::start(sync);
    auto log = this->logger();
    serializer_ = makeGndTruthSerializer(config_.output_type.value, log);
    open_file();
  }

  Duration process(const Sync& sync) override {
    assert(veh_ != nullptr);

    GndTruth gt;
    gt.sim_time = sync.time();
    gt.sim_step = sync.step();

    for (const auto& component : config_.components) {
      auto c = veh_->get<Component>(component);
      gt.components.emplace(component, std::move(c));
    }

    // Serialize data
    if (serializer_) {
      serializer_->serialize(sync, gt);
    }

    return sync.time();
  }

  void stop(const Sync& sync) override {
    Controller::stop(sync);
    close_file();
  }

  void abort() override {
    // Nothing to do here.
  }

  void reset() override {
    // Works by default.
  }

 private:
  void open_file() {
    if (serializer_) {
      serializer_->open_file(config_.output_file);
    }
  }
  void close_file() {
    if (serializer_) {
      serializer_->close_file();
    }
  }

 protected:
  GndTruthExtractorConfiguration config_;
  std::unique_ptr<GndTruthSerializer> serializer_;
};

DEFINE_CONTROLLER_FACTORY(GndTruthExtractorFactory, GndTruthExtractorConfiguration,
                          "gndtruth_extractor", "extracts information from the simulation")

DEFINE_CONTROLLER_FACTORY_MAKE(GndTruthExtractorFactory, GndTruthExtractor)

}  // namespace controller
}  // namespace cloe

// Register factory as plugin entrypoint:
EXPORT_CLOE_PLUGIN(cloe::controller::GndTruthExtractorFactory)
