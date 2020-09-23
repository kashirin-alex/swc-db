---
title: Getting SWC-DB
sort: 3
---

# Getting SWC-DB

## Available for Download 

|   Version   |   Version-Specific    |  Package Types    |     Build Types   | Architectures / Platforms             | Link           |
|     ---     |          ---          |       ---         |        ---        |     ---                               |  ---           |
| 0.4.8       | .debug.amd64          | tar.xz            | debug             |   GLIBC-2.27-amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.8/swcdb-0.4.8.debug.amd64.tar.xz) |
| 0.4.8       | .amd64                | tar.xz            | optimized         |   GLIBC-2.27-amd64                    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.4.8/swcdb-0.4.8.amd64.tar.xz) |
| 0.3.0       | .debug_amd64          | deb               | debug             |   GLIBC-2.28-Ubuntu-20.04LTS-amd64    | [download](https://github.com/kashirin-alex/swc-db/releases/download/v0.3.0/swcdb_0.0.1-0.3.0.debug_amd64.deb) |
|             |                       |                   |                   |                                       |                 |


_**[Releases on Github](https://github.com/kashirin-alex/swc-db/releases)**_




## Ways to Download & Install


### The SWC-DB .tar.xz package:

Fill the ```SWCDB_INSTALL_PATH``` ```SWCDB_VERSION``` ```SWCDB_VERSION_SPECIFIC``` as required.

```bash
SWCDB_INSTALL_PATH="/opt/swcdb";
SWCDB_VERSION="0.4.8";
SWCDB_VERSION_SPECIFIC=${SWCDB_VERSION}".debug.amd64";

wget https://github.com/kashirin-alex/swc-db/releases/download/v${SWCDB_VERSION}/swcdb-${SWCDB_VERSION_SPECIFIC}.tar.xz;

mkdir ${SWCDB_INSTALL_PATH};
tar -xf swcdb-${SWCDB_VERSION_SPECIFIC}.tar.xz -C ${SWCDB_INSTALL_PATH};

# for python thrift-client:
python3 -m pip install wheel ${SWCDB_INSTALL_PATH}/lib/py/swcdb-${SWCDB_VERSION}.tar.gz;

```


### The SWC-DB deb package:

_installation path defaults to "/opt/swcdb"_

```bash
SWCDB_VERSION="0.3.0";
SWCDB_VERSION_SPECIFIC=${SWCDB_VERSION}".debug_amd64";

wget https://github.com/kashirin-alex/swc-db/releases/download/v${SWCDB_VERSION}/swcdb_0.0.1-${SWCDB_VERSION_SPECIFIC}.deb;
dpkg -i swcdb_0.0.1-{SWCDB_VERSION_SPECIFIC}.deb;

```