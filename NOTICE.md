Notices for Eclipse Cloe
========================

This content is produced and maintained by the Eclipse Cloe project.

- Project home: https://projects.eclipse.org/projects/technology.cloe

## Trademarks

Eclipse Cloe is a trademark of the Eclipse Foundation.

## Copyright

All content is the property of the respective authors or their employers.
For more information regarding authorship of content, please consult the
listed source code repository logs.

## Declared Project Licenses

This program and the accompanying materials are made available under the
terms of the Apache License, Version 2.0, which is available at:

    https://www.apache.org/licenses/LICENSE-2.0

SPDX-License-Identifier: Apache-2.0

## Source Code

The project maintains the following source code repositories:

- https://github.com/eclipse/cloe

## Third-Party Content

The following third-party libraries are included in the Cloe repository:

- Bootstrap
  - License: MIT
  - License-Source: https://github.com/twbs/bootstrap/blob/v4.0.0-beta.2/LICENSE
  - Website: http://getbootstrap.com
  - Source: engine/webui/bootstrap.min.css

- Doxygen-Awesome-CSS
  - License: MIT
  - License-Source: docs/_vendor/doxygen-awesome/LICENSE
  - Website: https://jothepro.github.io/doxygen-awesome-css
  - Source: docs/_vendor/doxygen-awesome

- Inspect.lua
  - License: MIT
  - License-Source: https://raw.githubusercontent.com/kikito/inspect.lua/master/MIT-LICENSE.txt
  - Website: https://github.com/kikito/inspect.lua
  - Source: engine/lua/inspect.lua

- Typecheck
  - License: MIT
  - License-Source: https://github.com/gvvaughan/typecheck/raw/master/LICENSE.md
  - Website: https://github.com/gvvaughan/typecheck
  - Source: engine/lua/typecheck.lua

- Tableshape
  - License: MIT
  - License-Source: https://github.com/leafo/tableshape/blob/v2.6.0/README.md
  - Website: https://github.com/leafo/tableshape
  - Source: engine/lua/tableshape.lua

- Linenoise
  - License: BSD2
  - License-Source: https://raw.githubusercontent.com/antirez/linenoise/master/LICENSE
  - Website: https://github.com/antirez/linenoise
  - Source: engine/vendor/linenoise

- LRDB
  - License: BSL-1.0
  - License-Source: https://www.boost.org/LICENSE_1_0.txt
  - Website: https://github.com/satoren/LRDB
  - Source: engine/vendor/lrdb

- Lust
  - License: MIT
  - License-Source: https://raw.githubusercontent.com/bjornbytes/lust/master/LICENSE
  - Website: https://github.com/bjornbytes/lust
  - Source: engine/lua/lust.lua

The following third-party libraries are used by this project (these are usually
installed with the help of Conan):

- Google Test with Google Mock
  - License: BSD-3
  - License-Source: https://github.com/google/googletest/blob/master/LICENSE
  - Website: https://github.com/google/googletest
  - Conan-Package: gtest

- CLI11
  - License: BSD-3
  - License-Source: https://github.com/cliutils/cli11/blob/master/LICENSE
  - Website: https://github.com/cliutils/cli11
  - Conan-Package: CLI11@cliutils/stable

- gabime/spdlog
  - License: MIT
  - License-Source: https://github.com/gabime/spdlog/blob/v1.x/LICENSE
  - Source: https://github.com/gabime/spdlog
  - Conan-Package: spdlog

- fmtlib/fmt
  - License: BSD-2
  - License-Source: https://github.com/fmtlib/fmt/blob/master/LICENSE.rst
  - Website: fmtlib.net
  - Source: https://github.com/fmtlib/fmt
  - Conan-Package: fmt

- nlohmann/json
  - License: MIT
  - License-Source: https://github.com/nlohmann/json/blob/master/LICENSE.MIT
  - Source: https://github.com/nlohmann/json
  - Conan-Package: nlohmann_json

- incbin
  - License: Unlicense
  - License-Source: https://github.com/graphitemaster/blob/master/UNLICENSE
  - Website: https://github.com/graphitemaster/incbin
  - Conan-Package: incbin

- inja
  - License: MIT
  - License-Source: https://github.com/pantor/inja/blob/master/LICENSE
  - Source: https://github.com/pantor/inja
  - Conan-Package: inja

- Boost
  - License: BSL-1.0
  - License-Source: https://www.boost.org/LICENSE_1_0.txt
  - Website: https://www.boost.org
  - Conan-Package: boost

- Eigen3
  - License: MPL2
  - License-Source: https://www.mozilla.org/en-US/MPL/2.0
  - Website: https://eigen.tuxfamily.org
  - Conan-Package: eigen

- Oat++
  - License: Apache-2.0
  - License-Source: https://github.com/oatpp/oatpp/blob/master/LICENSE
  - Website: https://oatpp.io/
  - Source: https://github.com/oatpp/oatpp
  - Conan-Package: oatpp

Additional third-party content that is used for the web user-interface
is listed in the `ui/LICENSE-3RD-PARTY.txt` file.
