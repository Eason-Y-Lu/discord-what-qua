# SPDX-License-Identifier: BSD-3-Clause-No-Nuclear-License-2014
#! /bin/bash

apt-get update -y
apt-get install ca-certificates gpg wget -y
test -f /usr/share/doc/kitware-archive-keyring/copyright || wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null 
apt-get update -y 
test -f /usr/share/doc/kitware-archive-keyring/copyright || rm /usr/share/keyrings/kitware-archive-keyring.gpg
apt-get install kitware-archive-keyring -y
apt-get full-upgrade -y --autoremove --purge
apt-get -y install clang cmake ninja-build libstdc++-10-dev lld linux-libc-dev git-all make build-essential curl zip unzip tar bash-completion pkg-config software-properties-common openssl libssl-dev