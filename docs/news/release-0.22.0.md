# Version 0.22.0 Release

This version comes relatively soon after the previous version,
but includes some additions that have been several months in the
making.

For the entire changelog, see the [Git commit history](https://github.com/eclipse/cloe/compare/v0.21.0...v0.22.0).

## ESMini Simulator Plugin

This release marks the initial release of the ESMini simulator
plugin binding.

You can use it by adding it to your test configuration `conanfile.py`
at the appropriate location:

```python
require("cloe-plugin-esmini/0.22.0@cloe/develop")
```

The ESMini simulator uses OpenScenario XML files as inputs, and
this needs to be configured in the stackfile in order for you
to use it:

```yaml
version: "4"
simulators:
  binding: esmini
  args:
    headless: true
    scenario: "${ESMINI_XOSC_PATH}/test-driver.xosc"
    vehicles:
      Ego:
        closed_loop: true
        filter_distance: 200.0
vehicles:
  name: default
  from:
    simulator: esmini
    index: 0
```

The `${ESMINI_XOSC_PATH}` is defined by the `esmini-data` Conan package
and made available in the environment through `cloe-launch`. (You
can of course use your own environment variables if you want,
or none at all.)

The above example code assumes it will integrate in an existing vehicle
and controller configuration and be passed to cloe-engine as JSON.

See {doc}`the reference documentation <../reference/plugins/esmini>` for more.

## Clothoid-Fit Component Plugin

The clothoid-fit component plugin for lane boundaries makes its
entrance.

You can use it by adding it to your test configuration `conanfile.py`
at the appropriate location:

```python
require("cloe-plugin-clothoid-fit/0.22.0@cloe/develop")
```

And then you need to configure your vehicle in the stackfile:

```yaml
version: "4"
vehicles:
  name: default
  from:
    simulator: minimator
    name: ego1
  components:
    "cloe::clothoid_fit":
      binding: clothoid_fit
      from: "cloe::default_lane_sensor"
      args:
        enable: true
        estimation_distance: 40
```

The above example code assumes it will integrate in an existing vehicle
and simulator configuration and be passed to cloe-engine as JSON.

See {doc}`the reference documentation <../reference/plugins/clothoid_fit>` for more.

## Fable Library

Compile-time improvements for the Fable library radically
speed up the compilation of code that use the `make_schema` functions.

However, the following definitions were removed from the Fable
library in order to make the `make_schema()` functions consistent:

- `Struct make_schema(TPropertyList&&)`
- `Struct make_schema(std::string&&, TPropertyList&&)`
- `Struct make_schema(std::string&&, const Box&, TPropertyList&&)`
- `Struct make_schema(std::string&&, const Struct&, TPropertyList&&)`
- `Variant make_schema(std::string&&, std::initializer_list<Box>)`
- `Variant make_schema(std::string&&, std::vector<Box>&&)`
- `Variant make_schema(std::initializer_list<Box>)`
- `Variant make_schema(std::vector<Box>&&)`
- `Ignore make_schema(std::string&&, JsonType t = Jsontype::object)`

The purpose of `make_schema()` is to make it easy to derive the
correct schema for any C++ standard type you may have, even
more complex ones such as `std::vector<std::map<std::string, MyConfableClass>>`.
The removed `make_schema()` definitions do not fall into this
use-case, and were rarely if ever used.

If you want to create a struct, please use `fable::Schema` or
`fable::schema::Struct` directly:

```cpp
return fable::Schema{
    { "foo", fable::Schema {
        { "nested", make_schema(&variable, "description") },
    }}
};
```

If you want to create a variant, please use `fable::schema::Variant`
directly:

```cpp
return fable::schema::Variant{
    fable::Schema{ /* variant 1 * / },
    /* ... */
    fable::Schema{ /* variant N * / },
};
```

If you want to name a key but ignore it, please use `fable::schema::Ignore`
directly:

```cpp
return fable::Schema{
    { "field", make_schema(&value, "description") },
    { "ignore", fable::schema::Ignore("description") },
};
```

Please see the respective header files for more initialization forms.
