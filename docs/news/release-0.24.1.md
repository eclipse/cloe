# Version 0.24.1 Release

This patch release primarily fixes problems when consuming esmini simulator in Linux
distribution under certain corner cases (in a context where more than one Conan package
makes use of open-simulation-interface and Protobuf) and activates full_package_mode for
all dependencies in the Cloe Conan packages.

## Motivation to activate full_package_mode for the different dependencies

Normally when working in a private project one wants to take advantage of distributing
already compiled packages with a certain configuration to reduce the compile time of an
application. Also, it happens quite often that the Conan configuration is out of your
control (so going for a solution like changing the default configuration in the Conan
configuration of `default_package_id_mode` from `semver_direct_mode` to something
else is not an option). And since `semver_direct_mode` only rebuilds a package if a
dependency is overriden to a newer minor version (it does not care about if a newer
patch version is consumed), it might happen that from your application recipe you
override to a newer patch version of a Conan package (let's call it `A/1.0.2`) but the
Cloe release uploaded to your artifactory which was produced with `A/1.0.1` is still
consumed in your build (Conan will download the packages from artifactory because,
with the current configuration, it detects that the uploaded binaries satisfy the
conditions to build your application). Which is wrong since you are overriding package
`A` to be version `1.0.2` from your main recipe. If the patch release of package `A`
is not backward compatible with its old patch version you will face runtime issues.
To avoid this situation and to make Conan detect properly that the Cloe conan packages
have to be rebuild with the correct overriden dependencies we calculate the package_id
of a package setting the full_package_mode for all its dependencies. This way Conan will
rebuild the package considering also if the version of the package changed (including
a patch version), the user, the channel or even the package_id of the dependencies. 

## Recipe fix for esmini

Support to WSL and Linux distribution at the same time with open-simulation-interface and
Protobuf is a challenge, especially due to the fact that many packages may make use of them.
Using a different combination of how we produce the libraries will end up in runtime issues
when we load an application using Protobuf under the hood (for example having Protobuf as
a shared library). Using open-simulation-interface as a shared
library and Protobuf as static library works well in both WSL and linux. This change in the
esmini's recipe was to be more strict in which modes we use OSI and Protobuf which is actually
what we currently support in other packages as well.


