#include "Aspen/Python/Registry.hpp"
#include <unordered_map>

using namespace Aspen;
using namespace pybind11;

namespace {
  std::unordered_map<const _typeobject*, Boxers> box_registry;

  auto VALUE_BOXERS = [] {
    return Boxers{nullptr, nullptr, nullptr};
  }();
}

void Aspen::register_reactor(const object& type,
    void (*boxer)(const object&, void*, const std::type_info&),
    SharedBox<object> (*object_boxer)(const object&),
    SharedBox<void> (*void_boxer)(const object&)) {
  box_registry.insert(std::make_pair(
    reinterpret_cast<const _typeobject*>(type.ptr()),
    Boxers{boxer, object_boxer, void_boxer}));
}

const Boxers& Aspen::find_boxers(const object& value) {
  auto i = box_registry.find(value.ptr()->ob_type);
  if(i == box_registry.end()) {
    return VALUE_BOXERS;
  }
  return i->second;
}
