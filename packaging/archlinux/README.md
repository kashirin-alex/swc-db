

### Names of the Archlinux Packages

* Runtime Environment:
  * swcdb-env
  * swcdb-doc

* Libraries:
  * swcdb-lib-core
  * swcdb-lib
  * swcdb-lib-fs
  * swcdb-lib-fs-local
  * swcdb-lib-fs-broker
  * swcdb-lib-fs-ceph
  * swcdb-lib-fs-hadoop
  * swcdb-lib-fs-hadoop-jvm
  * swcdb-lib-thrift
  * swcdb-lib-thrift-c

* Applications:
  * swcdb-manager
  * swcdb-ranger
  * swcdb-fsbroker
  * swcdb-thiriftbroker
  * swcdb-utils


***


### Prepare `makepkg` environment:
Additional detail at [Arch User Repository](https://wiki.archlinux.org/index.php/Arch_User_Repository)
```
pacman -Syu;
pacman -S git binutils sudo fakeroot;
```
> The `install_all.sh` and `README.md` in packages describe the use with the user `swcdb_builder`.
```
## add building user, if required.
useradd swcdb_builder -g root;
passwd swcdb_builder ****;
```
  1. user needs to be with permissions to use/install/upgrade with `pacman -S pkgname` such as (/etc/sudoers group root allow)
  2. `userdel swcdb_builder` if no longer require



#### Commit and Push:
while at package folder
```
## add new files if new added
# git add PKGBUILD .SRCINFO README.md;
```

```
git commit -m "useful commit message";
git push;
```
