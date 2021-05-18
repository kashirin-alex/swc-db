#!/usr/bin/env bash



### Runnable after Prepare `makepkg` environment:


_user="swcdb_builder"

_packages=("swcdb-env
swcdb-lib-core
swcdb-lib
swcdb-lib-fs
swcdb-lib-fs-local
swcdb-lib-fs-broker
swcdb-lib-fs-ceph
swcdb-lib-fs-hadoop
swcdb-lib-fs-hadoop-jvm
swcdb-lib-thrift
swcdb-lib-thrift-c
swcdb-manager
swcdb-ranger
swcdb-fsbroker
swcdb-broker
swcdb-thriftbroker
swcdb-utils")


pacman -Syu;
pacman -S git binutils sudo fakeroot;


for pkg in ${_packages}; do

  git clone https://aur.archlinux.org/${pkg}.git;

  cd ${pkg};
  chmod -R 777 ./;

  sudo -u ${_user} makepkg --printsrcinfo > .SRCINFO;
  sudo -u ${_user} makepkg -s -r;

  sudo -u ${_user} makepkg -i;

  cd ..;

done;
