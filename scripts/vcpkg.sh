# SPDX-License-Identifier: BSD-3-Clause-No-Nuclear-License-2014
#! /bin/bash

git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh -disableMetrics
./vcpkg integrate install