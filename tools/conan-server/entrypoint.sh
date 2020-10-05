#!/bin/bash

set -ex

/usr/local/bin/conan_server --migrate
/usr/local/bin/conan_server
