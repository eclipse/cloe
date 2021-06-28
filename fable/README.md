Fable
=====

The Fable library provides a convenient mechanism for validating, serializing,
and deserializing C++ data structures to and from JSON. While this library is
developed as a part of Cloe, it can be used as a stand-alone library without
any requirement to Cloe.

Building and Installing with Conan
----------------------------------
If you cloned the Cloe repository, building Fable is simple. Navigate to the
`fable/` directory and run `make package`. This will use Conan and CMake to
build a Conan package with a version derived from the checked out commit.
If you want to have a different version, you can use the Conan `copy` command
to copy the package to a different reference.

Once you've built the package, it is available in your Conan cache and you can
refer to it from elsewhere in your system. You can also upload it to your own
private Conan repository (such as with Artifactory).

Building and Installing without Conan
-------------------------------------
While we recommend to use Conan, if your project is already established and you
don't want to add an extra build dependency, you can modify the
`CMakeLists.txt` file to replace the Conan specific parts with a `find_package`
implementation.

1. Replace the Conan initialization (lines 9-10) with `find_package` statements
   for all the required libraries:

        find_package(Boost COMPONENTS filesystem iostreams REQUIRED)
        find_package(fmt CONFIG REQUIRED)
        find_package(nlohmann_json CONFIG REQUIRED)
        find_package(CLI11 CONFIG REQUIRED)
        find_package(GTest CONFIG REQUIRED)

   The GTest dependency is for the tests and the CLI11 dependency is for the
   example binaries. If you don't need these, you can omit them and disable
   the options in the `CMakeLists.txt` file.

2. Replace all instances of `CONAN_PKG::*` with the appropriate variables that
   `find_package` sets. For example, at lines 34-40:

        target_link_libraries(${target}
          PUBLIC
            ${Boost_LIBRARIES}
            fmt::fmt
            nlohmann_json
        )
        target_include_directories(${target}
          PUBLIC
            ${Boost_INCLUDE_DIR}
        )

Example Applications
--------------------
To learn how Fable can be used, it is recommended to have a look at the example
applications included: contacts and simple_config. Each of these is normally
compiled as part of the tests, but they are stand-alone applications that can
be copied to a completely different location and built:
```console
$ make clean all
rm -r build
cmake -E make_directory build
conan install --install-folder build -g cmake .
Configuration:
[settings]
arch=x86_64
arch_build=x86_64
build_type=RelWithDebInfo
compiler=gcc
compiler.libcxx=libstdc++11
compiler.version=8
os=Linux
os_build=Linux
[options]
[build_requires]
[env]

WARN: fable/0.18.0@cloe/develop: requirement fmt/[~=6.2.0] overridden by your conanfile to fmt/6.2.0
Version ranges solved
    Version range '~=1.9.1' required by 'conanfile.txt' resolved to 'cli11/1.9.1' in local cache
    Version range '>0.17.0' required by 'conanfile.txt' resolved to 'fable/0.18.0@cloe/develop' in local cache
    Version range '>=6.0.0' required by 'conanfile.txt' resolved to 'fmt/6.2.0' in local cache
    Version range '>=1.65.1' required by 'fable/0.18.0@cloe/develop' resolved to 'boost/1.69.0' in local cache
    Version range '~=6.2.0' required by 'fable/0.18.0@cloe/develop' valid for downstream requirement 'fmt/6.2.0'
    Version range '~=3.9.1' required by 'fable/0.18.0@cloe/develop' resolved to 'nlohmann_json/3.9.1' in local cache

conanfile.txt: Installing package
Requirements
    boost/1.69.0 from 'artifactory' - Cache
    bzip2/1.0.8 from 'artifactory' - Cache
    cli11/1.9.1 from 'artifactory' - Cache
    fable/0.18.0@cloe/develop from local cache - Cache
    fmt/6.2.0 from 'artifactory' - Cache
    nlohmann_json/3.9.1 from 'artifactory' - Cache
    zlib/1.2.11 from 'artifactory' - Cache
Packages
    boost/1.69.0:723638df3c23ba2e9b285a2bd692e0abeea9b6e3 - Cache
    bzip2/1.0.8:958b011845d8998ef3cc45a6238d89fc6cb91e6b - Cache
    cli11/1.9.1:5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9 - Cache
    fable/0.18.0@cloe/develop:28e8909fd0e5c5a6d409867acf35d1334f2902b9 - Cache
    fmt/6.2.0:5c1963f6d66f4f5bd8178a39b6b31db7bb1731a1 - Cache
    nlohmann_json/3.9.1:d1091b2ed420e6d287293709a907ae824d5de508 - Cache
    zlib/1.2.11:85d958111e9ae25b990062d9726ea88cea5a01d1 - Cache

Installing (downloading, building) binaries...
bzip2/1.0.8: Already installed!
cli11/1.9.1: Already installed!
fmt/6.2.0: Already installed!
nlohmann_json/3.9.1: Already installed!
zlib/1.2.11: Already installed!
boost/1.69.0: Already installed!
boost/1.69.0: LIBRARIES: ['boost_wave', 'boost_container', ...]
boost/1.69.0: Package folder: $HOME/.conan/data/boost/1.69.0/_/_/package/723638df3c23ba2e9b285a2bd692e0abeea9b6e3
fable/0.18.0@cloe/develop: Already installed!
conanfile.txt: Generator cmake created conanbuildinfo.cmake
conanfile.txt: Generator txt created conanbuildinfo.txt
conanfile.txt: Generated conaninfo.txt
conanfile.txt: Generated graphinfo
cmake -E chdir build cmake -Wdev ..
-- The CXX compiler identification is GNU 8.4.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/lib/ccache/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Conan: Adjusting output directories
-- Conan: Using cmake targets configuration
-- Library fable found $HOME/.conan/data/fable/0.18.0/cloe/develop/package/28e8909fd0e5c5a6d409867acf35d1334f2902b9/lib/libfable.a
-- Library boost_exception found $HOME/.conan/data/boost/1.69.0/_/_/package/723638df3c23ba2e9b285a2bd692e0abeea9b6e3/lib/libboost_exception.a
-- Library boost_iostreams found $HOME/.conan/data/boost/1.69.0/_/_/package/723638df3c23ba2e9b285a2bd692e0abeea9b6e3/lib/libboost_iostreams.a
-- Library boost_regex found $HOME/.conan/data/boost/1.69.0/_/_/package/723638df3c23ba2e9b285a2bd692e0abeea9b6e3/lib/libboost_regex.a
-- Library boost_thread found $HOME/.conan/data/boost/1.69.0/_/_/package/723638df3c23ba2e9b285a2bd692e0abeea9b6e3/lib/libboost_thread.a
-- Library boost_filesystem found $HOME/.conan/data/boost/1.69.0/_/_/package/723638df3c23ba2e9b285a2bd692e0abeea9b6e3/lib/libboost_filesystem.a
-- Library boost_system found $HOME/.conan/data/boost/1.69.0/_/_/package/723638df3c23ba2e9b285a2bd692e0abeea9b6e3/lib/libboost_system.a
-- [... redacted boost output ...]
-- Library fmt found $HOME/.conan/data/fmt/6.2.0/_/_/package/5c1963f6d66f4f5bd8178a39b6b31db7bb1731a1/lib/libfmt.a
-- Library z found $HOME/.conan/data/zlib/1.2.11/_/_/package/85d958111e9ae25b990062d9726ea88cea5a01d1/lib/libz.a
-- Library bz2 found $HOME/.conan/data/bzip2/1.0.8/_/_/package/958b011845d8998ef3cc45a6238d89fc6cb91e6b/lib/libbz2.a
-- Conan: Adjusting default RPATHs Conan policies
-- Conan: Adjusting language standard
-- Current conanbuildinfo.cmake directory: $HOME/cloe/fable/examples/contacts/build
-- Conan: Compiler GCC>=5, checking major version 8
-- Conan: Checking correct version: 8
-- Configuring done
-- Generating done
-- Build files have been written to: $HOME/cloe/fable/examples/contacts/build
cmake --build build
make[1]: Entering directory '$HOME/cloe/fable/examples/contacts/build'
make[2]: Entering directory '$HOME/cloe/fable/examples/contacts/build'
make[3]: Entering directory '$HOME/cloe/fable/examples/contacts/build'
make[3]: Leaving directory '$HOME/cloe/fable/examples/contacts/build'
make[3]: Entering directory '$HOME/cloe/fable/examples/contacts/build'
[ 50%] Building CXX object CMakeFiles/contacts.dir/src/main.cpp.o
[100%] Linking CXX executable bin/contacts
make[3]: Leaving directory '$HOME/cloe/fable/examples/contacts/build'
[100%] Built target contacts
make[2]: Leaving directory '$HOME/cloe/fable/examples/contacts/build'
make[1]: Leaving directory '$HOME/cloe/fable/examples/contacts/build'
```
The output has been slightly edited for better legibility. At this point, the
example program `bin/contacts` has been created in the `build/` directory.

You can run this with `--help` to gain an impression of what works out of the
box:
```console
$ build/bin/contacts --help
Fable Contact Example
Usage: build/bin/contacts [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -f,--file TEXT              input JSON filepath
  --print-example             print example data
  --print-schema              print data schema
```
Try running the application with the example input files to see how it
deserializes the files:
```
$ build/bin/contacts -f example_addressbook.json
NAME                  AGE  ADDRESS
--------------------  ---  ----------------------------------
John Smith            27   21 2nd Street, 10021-3100 New York
Julia Smith           28   21 2nd Street, 10021-3100 New York
Elbow Dorado          N/A  N/A
Hannibal Mercurial    34   N/A
```

The source code in `src/main.cpp` is documented and serves as a great
starting point for making use of Fable in your own application.
