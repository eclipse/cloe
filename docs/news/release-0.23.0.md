# Version 0.23.0 Release

This release primarily refactors and improves the Fable library,
with several breaking changes.

**New:**

- Add `fable/fable_fwd.hpp` containing many forward-declarations.

  This can allow you to write code that compiles faster.

- Add schema support for `std::filesystem::path` from C++17.

  Usage is identical to that of `boost::filesystem::path` (which now requires
  inclusion of a separate header.)

- Add schema support for `std::optional` from C++17.

  Usage is identical to that of `boost::optional` (which now requires inclusion
  of a separate header).

- Add schema support for `std::array`.

  JSON can be deserialized from a JSON array or a JSON object. The array is
  deserialization is as expected, and replaces the target value simply:
  ```json
  [ 1, 2, 3 ]
  ```

  If only a specific field in the array should be set, this can be accomplished
  with the use of a map:
  ```json
  {
    "0": 0,
    "2": 42
  }
  ```
  In which case, only indexes 0 and 2 are set, to result in a serialization of:
  ```json
  [ 0, 2, 42 ]
  ```
  Serialization of an array is always the full array.

  It is possible when creating a new array, such as with `make_schema`, to
  disable the setting of individual fields:
  ```cpp
  make_schema(array_ptr, "this is my array").require_all(true)
  ```
  Currently, C arrays are not supported. With modern C++, it should be possible
  to use `std::array` everywhere where a C array would be used otherwise.

- Add `fable/utility/sol.hpp` with an implementation of `nlohmann_json::adl_serializer<sol::object>`.

  This means that `to_json` and `from_json` can be used with `sol::object`.
  The `fable::into_sol_object()` helps deserialize a JSON into a specific Lua
  object.

- Add `fable/utility/string.hpp` with several helper functions:
  ```cpp
  namespace fable {
    bool starts_with(std::string_view s, std::string_view prefix);
    bool ends_with(std::string_view s, std::string_view suffix);
    std::string join_vector(const std::vector<std::string>& v, std::string_view sep);
    std::vector<std::string> split_string(std::string_view s, std::string_view sep);
  };
  ```

- Add `fable/utility/templates.hpp` with several helper functions:
  ```cpp
  namespace fable {
    template <typename T, typename S>
    constexpr bool is_cast_safe(S value);

    template <typename T>
    struct typeinfo {
        static const constexpr char* name;
    };
  }
  ```
  These are primarily used for fable implementation.

- Add `fable/utility/path.hpp` with `nlohmann_json::adl_serializer<std::filesystem::path>`
  implementation and the following helper functions:
  ```cpp
  namespace fable {
    bool is_executable(const std::filesystem::path&);
  }
  ```

- Add `fable/utility/optional.hpp` with `nlohmann_json::adl_serializer<std::optional<T>>`
  implementation.

- Add `fable/utility/chrono.hpp` with `nlohmann_json::adl_serializer<std::chrono::duration<Rep, Period>>`
  implementation and the following helper functions:
  ```cpp
  namespace fable {
    std::chrono::nanoseconds parse_duration_to_nanoseconds(const std::string&);

    template <typename Duration>
    Duration parse_duration(const std::string&);

    template <typename Duration>
    std::string to_string(const Duration&);
  }
  ```

**Fixed:**

- Fix rare segfault in GCC 8.

- Fix number and duration bound incorrect validation.

- Force compilation stop when C++ standard is less than 17.

- Fix use of `nlohmann::detail` namespace.

- Fix bugs related to use of JSON pointer with `Conf`.

**Changed:**

- Use `fable` namespace in cloe code instead of cloe aliases.

- Renamed type `fable::schema::Array` to `fable::schema::Vector`.

- Renamed file `fable/schema/array.hpp` to `fable/schema/vector.hpp`.

  This is makes more clear what type is contained and supported
  through this fable schema, namely `std::vector`.

  **Migration:**
  > If you have files containing `#include <fable/schema/array.hpp>` then
  > these should be updated to `#include <fable/schema/vector.hpp>`.

- Renamed file `fable/schema/magic.hpp` to `fable/schema/xmagic.hpp`.

  This makes it clearer that the header should be included after all other
  schema headers, and if someone sorts the schema commits by accident, this
  will preserve that.

  **Migration:**
  > If you have files containing `#include <fable/schema/magic.hpp>` then
  > these should be updated to `#include <fable/schema/xmagic.hpp>`.

- Remove Boost dependency

  All schemas are now C++17 first, but support for Boost datatypes can be
  transparently re-enabled by including the appropriate header files:

      #include <fable/utility/boost_path.hpp>
      #include <fable/utility/boost_optional.hpp>

  This means that the Fable library no longers forces users to use Boost,
  but it also means that users can no longer expect Fable to include Boost
  for them.

  *Migration:*
  > For CMake, ensure that your `CMakeLists.txt` includes Boost if you use it:
  >
  >     find_package(Boost COMPONENTS headers filesystem REQUIRED)
  >     target_link_libraries(${yourTarget} PUBLIC Boost::headers Boost::filesystem)
  >
  > In Conan, ensure that your `conanfile.py` includes a Boost of your liking:
  >
  >     class ConanFile:
  >       def requirements(self):
  >         self.requires("boost/1.74.0")
  >
  > In your C++ code, ensure that you include the new header files for each
  > type you use:
  >
  >     #include <fable/utility/boost_path.hpp>
  >     #include <fable/utility/boost_optional.hpp>
  >

- Change `schema::Interface` to use and return `std::unique_ptr` instead
  of raw pointers.

  *Migration:*
  > If you use the `clone()` method from any schema, you need to update
  > your code to receive a `std::unique_ptr<Interface>` instead of
  > a raw pointer `Interface*`.
  >
  > If you have implemented a custom schema, you should update your
  > `clone()` method appropriately.

- Change `Conf::try_from()` and `Conf::try_from_pointer()` signatures to
  take reference instead of pointer.

  *Migration:*
  > If you use `Conf::try_from` or `Conf::try_from_pointer`, you need
  > to update your call from passing a pointer to passing a value.
  >
  > That means you may need to use one of these transformations:
  >
  >     conf.try_from(key, value)       ->      conf.try_from(key, *value)
  >     conf.try_from(key, &value)      ->      conf.try_from(key, value)
  >

- Add `[[nodiscard]]` to methods which are not intended to be used for
  side-effects.

  *Migration:*
  > If your code stops compiling because of `[[nodiscard]]`, then you should
  > either use another variant, or employ the `std::ignore = ...` pattern.

- Change schema `validate()` method to a non-throwing interface.

  This should result in better debugability and performance, since exceptions
  are not used under-the-hood anymore for validating. Validation could
  previously throw in a non-error state when for example differentiating
  between variants.

  *Migration:*
  > Previous use of `Schema::validate(const Conf&)` should now use
  > `Schema::validate_or_throw(const Conf&)`.
  >
  > Custom schemas should change their implementation of `validate(const Conf&)`
  > to `bool validate(const Conf&, std::optional<SchemaError>&)`.

**Removed:**

- Remove the following `SchemaError` constructors:

      template <typename... Args>
      SchemaError(const Conf& c, const Json& s, const Json& ctx, std::string_view forma
t, Args&&... args);
      SchemaError(const ConfError& err, const Json& schema, const Json& context);

  These definitions were error-prone and primarily used within fable and cloe.
  Instead, the new `SchemaError::with_context(Conf)` method.

  *Migration:*
  > If `SchemaError` is manually thrown/created with context, update it to
  > add the context through chaining:
  >
  >     throw SchemaError(err, schema).with_context(ctx);
  >

- Remove `make_const_str()`

  Instead, `make_const_schema()` should be used, which is orthogonal with
  `make_schema` and works the same as `make_const_str`, except that you
  need to make sure that `std::string` is actually used instead of something
  else, such as `char *` or `std::string_view`.

  *Migration:*
  > The following statement:
  >
  >     make_const_str("constant");
  >
  > should become:
  >
  >     use namespace std::literals;
  >     make_const_schema("constant"s);
  >
  > or:
  >
  >     make_const_schema(std::string{"constant"});
  >

### Automatic Migration

The following script can be saved and run to automatically migrate some of the
changes.

Ensure that you are running these commands in a clean working directory of a
Git repository. This allows you to track the changes that the scripts make to
your repo and revert incorrect modifications.

#### Include Paths

The following line should automatically modify files located in different
locations.
```
find -type f -exec sed -r \
    -e 's|(#include [<"]fable/schema)(/array.hpp)([>"])|\1/vector.hpp\3|' \
    -e 's|(#include [<"]fable/schema)(/magic.hpp)([>"])|\1/xmagic.hpp\3|' \
    -e 's|(#include [<"]fable)(/json/with_eigen.hpp)([>"])|\1/utility/eigen.hpp\3|' \
    -e 's|(#include [<"]fable)(/json/with_std.hpp)([>"])|\1/utility/memory.hpp\3|' \
    -e 's|(#include [<"]fable)(/json/with_boost.hpp)([>"])|\1/utility/boost_optional.hpp\3|' \
    -i {} \;
```

You may still need to manually add

    #include <fable/utility/boost_path.hpp>

if you are using `boost::filesystem::path` with Fable types.

#### Array to Vector Type Rename

If the fully scoped name is being used, the following line should rename the
type successfully:

    find -type f -exec sed -r -e 's|schema::Array|schema::Vector|' -i {} \;

However, if the code contains code like:

    using namespace fable::schema;
    return Array{ptr, "description"};

then we would not find it. You can search your repository for all instances of
`Array` and check each instance manually:

    grep -r Array
