# How to build

Follow the instructions in the README.md in the repo root for building Cloe.

# Debugging this plugin

First, make this plugin editable:
```
cd plugins/frustum_culling && make editable
```

Follow the instructions in the README.md in the repo root for building Cloe, e.g. call this from root to export the packages:
```
make export-vendor export
```

For the next step, you need to set your version the profile you want to use.
Here in the example the profile is "debug_default" and the version is "0.22.0-83-gbe9777d-dirty".
Replace them in the following lines of code so they fit your setup.

From plugins/frustum_culling, call:
```
conan lock create --lockfile-out "build/conan.lock" -pr=debug_default --build=missing ../../conanfile.py
conan install . cloe-plugin-frustum-culling/0.22.0-83-gbe9777d-dirty@cloe/develop --install-folder="build" --build=missing --lockfile="build/conan.lock"
conan build . --source-folder="." --install-folder="build"
```

Make sure to build with a debug profile, i.e. use a conan profile with build_type "Debug".
If you have built before with a different profile, redo the steps in "How to build" with the correct profile.

We provide a launch.json in the .vscode folder.
In order for vscode to find the launch.json, you need to open code from that directory.
```
cd plugins/frustum_culling && code .
```

Then in the newly opened vs code window, you can select the target "Debug plugins/frustum_culling [local]" and run the debugger.
