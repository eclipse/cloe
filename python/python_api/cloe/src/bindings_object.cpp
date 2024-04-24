#include <cloe/component/object.hpp>

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_cloe_object(pybind11::module_ &m) {
  py::enum_<cloe::Object::Class>(m, "ObjectClass")
      .value("Unknown", cloe::Object::Class::Unknown)
      .value("Pedestrian", cloe::Object::Class::Pedestrian)
      .value("Bike", cloe::Object::Class::Bike)
      .value("Motorbike", cloe::Object::Class::Motorbike)
      .value("Car", cloe::Object::Class::Car)
      .value("Truck", cloe::Object::Class::Truck)
      .value("Trailer", cloe::Object::Class::Trailer);

  py::enum_<cloe::Object::Type>(m, "ObjectType")
      .value("Unknown", cloe::Object::Type::Unknown)
      .value("Static", cloe::Object::Type::Static)
      .value("Dynamic", cloe::Object::Type::Dynamic);

  py::class_<cloe::Object> obj (m, "Object");
  obj.def_readonly("id", &cloe::Object::id);
  obj.def_readonly("exist_prob", &cloe::Object::exist_prob);
  obj.def_readonly("type", &cloe::Object::type);
  obj.def_readonly("classification", &cloe::Object::classification);
  obj.def_property_readonly("pose", [](const cloe::Object &self) {
    return self.pose.matrix();
  });
  obj.def_readonly("cog_offset", &cloe::Object::cog_offset);
  obj.def_readonly("velocity", &cloe::Object::velocity);
  obj.def_readonly("acceleration", &cloe::Object::acceleration);
  obj.def_readonly("angular_velocity", &cloe::Object::angular_velocity);
}
