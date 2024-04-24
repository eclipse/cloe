#pragma once

#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>

namespace cloe::py {

nlohmann::json dict2json(const pybind11::dict &dict);

}
