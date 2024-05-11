---
title: Getting SWC-DB
sort: 2
---



# Getting SWC-DB




### _Ways to Download & Install_

* [Install the SWC-DB .tar.xz package](#the-swc-db-tarxz-package)
* [Install the SWC-DB deb package](#the-swc-db-deb-package)
* [Install with SWC-DB installation pack](#the-swc-db-installation-pack)
* [Install SWC-DB via Archlinux (AUR) keyword=swcdb](#swc-db-via-archlinux-aur)






## The SWC-DB .tar.xz package:
Choose the `.tar.xz package` from the  [Available for Download](#available-for-download).

Fill the ```SWCDB_INSTALL_PATH``` ```SWCDB_VERSION``` ```SWCDB_VERSION_SPECIFIC``` as required.

* #### download:

```bash
SWCDB_INSTALL_PATH="/opt/swcdb";
SWCDB_VERSION="0.5.13";
SWCDB_VERSION_SPECIFIC="debug.amd64";

wget https://github.com/kashirin-alex/swc-db/releases/download/v${SWCDB_VERSION}/swcdb-${SWCDB_VERSION}.${SWCDB_VERSION_SPECIFIC}.tar.xz;
```

* #### install:

```bash
mkdir ${SWCDB_INSTALL_PATH};
tar -xf swcdb-${SWCDB_VERSION}.${SWCDB_VERSION_SPECIFIC}.tar.xz -C ${SWCDB_INSTALL_PATH};
```



***



## The SWC-DB deb package:

Choose the `deb package` from the  [Available for Download](#available-for-download).
_installation path defaults to "/opt/swcdb"_


* #### download:

```bash
SWCDB_VERSION="0.5.13";
SWCDB_VERSION_SPECIFIC="Ubuntu_20_04_6_LTS.amd64";

wget https://github.com/kashirin-alex/swc-db/releases/download/v${SWCDB_VERSION}/swcdb-${SWCDB_VERSION}.${SWCDB_VERSION_SPECIFIC}.deb;
```

* #### install:

```bash
dpkg -i swcdb-${SWCDB_VERSION}.${SWCDB_VERSION_SPECIFIC}.deb;
```
* For details on making a Debian Package [packaging/debian](https://github.com/kashirin-alex/swc-db/tree/master/packaging/debian)



***



## The SWC-DB installation pack:
The steps of proceeding with SWC-DB cluster installation. The pack includes only the `sbin` and `etc` folders of the release.

> In-case the source-host needs to be a machine running SWC-DB, the [release package should be installed](#the-swc-db-tarxz-package) and proceed to step-3.

1. [download and install the 'install-pack' version-specific](#download)
2. use the default archive-url or use the archive-path of a [downloaded SWC-DB .tar.xz package](#download)
3. follow-up with instructions on [Running SWC-DB Distributed Cluster]({{ site.baseurl }}/run/distributed/)



***



## SWC-DB via Archlinux (AUR):
SWC-DB on AUR is separate into packages on scope/component and configured for OS base path.

The Packages of SWC-DB are available at [Archlinux(AUR) keyword=swcdb](https://aur.archlinux.org/packages/?K=swcdb).
* Further details and help on creating and installing the packages via AUR is available at [packaging/archlinux](https://github.com/kashirin-alex/swc-db/tree/master/packaging/archlinux)



***


## Available for Download

|   Version   |   Version-Specific                 |  Package Type     |     Build Type    | Compiler    | Architectures / Platforms           | Link           |
|     ---     |          ---                       |       ---         |        ---        |     ---     | ---                                 |  ---           |
| 0.5.12       | asan.amd64                        | tar.xz            | asan             | GCC-9.5    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.12/swcdb-0.5.12.asan.amd64.tar.xz) |
| 0.5.12       | debug.amd64                        | tar.xz            | debug             | GCC-9.5    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.12/swcdb-0.5.12.debug.amd64.tar.xz) |
| 0.5.12       | amd64                              | tar.xz            | standard          | GCC-9.5    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.12/swcdb-0.5.12.amd64.tar.xz) |
| 0.5.12       | optimized.amd64                    | tar.xz            | optimized         | GCC-9.5    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.12/swcdb-0.5.12.optimized.amd64.tar.xz) |
| 0.5.12       | Ubuntu_20_04_6_LTS.amd64           | deb               | standard          | GCC-9.4     | GLIBC-2.31 Ubuntu-20.04.6 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.12/swcdb-0.5.12.Ubuntu_20_04_6_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.11       | debug.amd64                        | tar.xz            | debug             | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.11/swcdb-0.5.11.debug.amd64.tar.xz) |
| 0.5.11       | amd64                              | tar.xz            | standard          | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.11/swcdb-0.5.11.amd64.tar.xz) |
| 0.5.11       | optimized.amd64                    | tar.xz            | optimized         | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.11/swcdb-0.5.11.optimized.amd64.tar.xz) |
| 0.5.11       | Ubuntu_20_04_5_LTS.amd64           | deb               | standard          | GCC-9.4     | GLIBC-2.31 Ubuntu-20.04.5 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.11/swcdb-0.5.11.Ubuntu_20_04_5_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.10       | debug.amd64                        | tar.xz            | debug             | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.10/swcdb-0.5.10.debug.amd64.tar.xz) |
| 0.5.10       | amd64                              | tar.xz            | standard          | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.10/swcdb-0.5.10.amd64.tar.xz) |
| 0.5.10       | optimized.amd64                    | tar.xz            | optimized         | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.10/swcdb-0.5.10.optimized.amd64.tar.xz) |
| 0.5.10       | Ubuntu_20_04_5_LTS.amd64           | deb               | standard          | GCC-9.4     | GLIBC-2.31 Ubuntu-20.04.5 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.10/swcdb-0.5.10.Ubuntu_20_04_5_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.9       | debug.amd64                        | tar.xz            | debug             | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.9/swcdb-0.5.9.debug.amd64.tar.xz) |
| 0.5.9       | amd64                              | tar.xz            | standard          | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.9/swcdb-0.5.9.amd64.tar.xz) |
| 0.5.9       | optimized.amd64                    | tar.xz            | optimized         | GCC-11.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.9/swcdb-0.5.9.optimized.amd64.tar.xz) |
| 0.5.9       | Ubuntu_20_04_4_LTS.amd64           | deb               | standard          | GCC-9.4     | GLIBC-2.31 Ubuntu-20.04.4 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.9/swcdb-0.5.9.Ubuntu_20_04_4_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.8       | debug.amd64                        | tar.xz            | debug             | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.8/swcdb-0.5.8.debug.amd64.tar.xz) |
| 0.5.8       | amd64                              | tar.xz            | standard          | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.8/swcdb-0.5.8.amd64.tar.xz) |
| 0.5.8       | optimized.amd64                    | tar.xz            | optimized         | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.8/swcdb-0.5.8.optimized.amd64.tar.xz) |
| 0.5.8       | Ubuntu_20_04_3_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04.3 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.8/swcdb-0.5.8.Ubuntu_20_04_3_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.7       | debug.amd64                        | tar.xz            | debug             | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.7/swcdb-0.5.7.debug.amd64.tar.xz) |
| 0.5.7       | amd64                              | tar.xz            | standard          | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.7/swcdb-0.5.7.amd64.tar.xz) |
| 0.5.7       | optimized.amd64                    | tar.xz            | optimized         | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.7/swcdb-0.5.7.optimized.amd64.tar.xz) |
| 0.5.7       | Ubuntu_20_04_3_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04.3 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.7/swcdb-0.5.7.Ubuntu_20_04_3_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.6       | debug.amd64                        | tar.xz            | debug             | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.6/swcdb-0.5.6.debug.amd64.tar.xz) |
| 0.5.6       | amd64                              | tar.xz            | standard          | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.6/swcdb-0.5.6.amd64.tar.xz) |
| 0.5.6       | optimized.amd64                    | tar.xz            | optimized         | GCC-11.2    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.6/swcdb-0.5.6.optimized.amd64.tar.xz) |
| 0.5.6       | Ubuntu_20_04_3_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04.3 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.6/swcdb-0.5.6.Ubuntu_20_04_3_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.5       | debug.amd64                        | tar.xz            | debug             | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.5/swcdb-0.5.5.debug.amd64.tar.xz) |
| 0.5.5       | amd64                              | tar.xz            | standard          | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.5/swcdb-0.5.5.amd64.tar.xz) |
| 0.5.5       | optimized.amd64                    | tar.xz            | optimized         | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.5/swcdb-0.5.5.optimized.amd64.tar.xz) |
| 0.5.5       | Ubuntu_20_04_3_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04.3 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.5/swcdb-0.5.5.Ubuntu_20_04_3_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.4       | debug.amd64                        | tar.xz            | debug             | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.4/swcdb-0.5.4.debug.amd64.tar.xz) |
| 0.5.4       | amd64                              | tar.xz            | standard          | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.4/swcdb-0.5.4.amd64.tar.xz) |
| 0.5.4       | optimized.amd64                    | tar.xz            | optimized         | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.4/swcdb-0.5.4.optimized.amd64.tar.xz) |
| 0.5.4       | Ubuntu_20_04_2_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04.2 LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.4/swcdb-0.5.4.Ubuntu_20_04_2_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.3       | debug.amd64                        | tar.xz            | debug             | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.3/swcdb-0.5.3.debug.amd64.tar.xz) |
| 0.5.3       | amd64                              | tar.xz            | standard          | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.3/swcdb-0.5.3.amd64.tar.xz) |
| 0.5.3       | optimized.amd64                    | tar.xz            | optimized         | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.3/swcdb-0.5.3.optimized.amd64.tar.xz) |
| 0.5.3       | Ubuntu_20_04_2_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.3/swcdb-0.5.3.Ubuntu_20_04_2_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.2       | debug.amd64                        | tar.xz            | debug             | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.2/swcdb-0.5.2.debug.amd64.tar.xz) |
| 0.5.2       | amd64                              | tar.xz            | standard          | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.2/swcdb-0.5.2.amd64.tar.xz) |
| 0.5.2       | optimized.amd64                    | tar.xz            | optimized         | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.2/swcdb-0.5.2.optimized.amd64.tar.xz) |
| 0.5.2       | Ubuntu_20_04_2_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.2/swcdb-0.5.2.Ubuntu_20_04_2_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.1       | debug.amd64                        | tar.xz            | debug             | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.1/swcdb-0.5.1.debug.amd64.tar.xz) |
| 0.5.1       | amd64                              | tar.xz            | standard          | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.1/swcdb-0.5.1.amd64.tar.xz) |
| 0.5.1       | optimized.amd64                    | tar.xz            | optimized         | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.1/swcdb-0.5.1.optimized.amd64.tar.xz) |
| 0.5.1       | Ubuntu_20_04_2_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.1/swcdb-0.5.1.Ubuntu_20_04_2_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.5.0       | debug.amd64                        | tar.xz            | debug             | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.0/swcdb-0.5.0.debug.amd64.tar.xz) |
| 0.5.0       | amd64                              | tar.xz            | standard          | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.0/swcdb-0.5.0.amd64.tar.xz) |
| 0.5.0       | optimized.amd64                    | tar.xz            | optimized         | GCC-10.3    | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.0/swcdb-0.5.0.optimized.amd64.tar.xz) |
| 0.5.0       | Ubuntu_20_04_2_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.5.0/swcdb-0.5.0.Ubuntu_20_04_2_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.4.19      | debug.amd64                        | tar.xz            | debug             | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.19/swcdb-0.4.19.debug.amd64.tar.xz) |
| 0.4.19      | amd64                              | tar.xz            | standard          | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.19/swcdb-0.4.19.amd64.tar.xz) |
| 0.4.19      | optimized.amd64                    | tar.xz            | optimized         | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.19/swcdb-0.4.19.optimized.amd64.tar.xz) |
| 0.4.19      | Ubuntu_20_04_2_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.19/swcdb-0.4.19.Ubuntu_20_04_2_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.4.18      | debug.amd64                        | tar.xz            | debug             | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.18/swcdb-0.4.18.debug.amd64.tar.xz) |
| 0.4.18      | amd64                              | tar.xz            | standard          | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.18/swcdb-0.4.18.amd64.tar.xz) |
| 0.4.18      | optimized.amd64                    | tar.xz            | optimized         | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.18/swcdb-0.4.18.optimized.amd64.tar.xz) |
| 0.4.18      | Ubuntu_20_04_2_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.18/swcdb-0.4.18.Ubuntu_20_04_2_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |
| 0.4.17      | debug.amd64                        | tar.xz            | debug             | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.17/swcdb-0.4.17.debug.amd64.tar.xz) |
| 0.4.17      | amd64                              | tar.xz            | standard          | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.17/swcdb-0.4.17.amd64.tar.xz) |
| 0.4.17      | optimized.amd64                    | tar.xz            | optimized         | GCC-9.3     | GLIBC-2.27 amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.17/swcdb-0.4.17.optimized.amd64.tar.xz) |
| 0.4.17      | Ubuntu_20_04_1_LTS.amd64           | deb               | standard          | GCC-9.3     | GLIBC-2.31 Ubuntu-20.04LTS amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.17/swcdb-0.4.17.Ubuntu_20_04_1_LTS.amd64.deb) |
|             |                       |                   |                   |             |                                     |                 |


_**[Releases on Github](https://github.com/kashirin-alex/swc-db/releases)**_


