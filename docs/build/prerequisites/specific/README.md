---
title: Component Specific
sort: 2
---

# Version Specific of Prerequisites



## ASIO VERSION

_**if libasio-dev under version asio-1.18**_
_use latest version from https://sourceforge.net/projects/asio/_
```bash

mkdir asio; cd asio;
ASIO_VERSION="1.18.0";
wget https://sourceforge.net/projects/asio/files/asio/${ASIO_VERSION}%20%28Stable%29/asio-${ASIO_VERSION}.tar.gz/download \
 -O asio-${ASIO_VERSION}.tar.gz;
tar -xf asio-${ASIO_VERSION}.tar.gz;

ASIO_INCLUDE_PATH="$(pwd)/asio-${ASIO_VERSION}/include";
cd ..;

```



## HADOOP VERSION

_download the desired version from [Apache-Hadoop releases](https://hadoop.apache.org/releases.html)
or if there is already an installed Hadoop on the system enough HADOOP_HOME is set in Environment or define cmake with HADOOP_INSTALL_PATH=_

```bash
HADOOP_VERSION="3.2.1";
wget https://archive.apache.org/dist/hadoop/common/hadoop-${HADOOP_VERSION}/hadoop-${HADOOP_VERSION}.tar.gz
tar -xf hadoop-${HADOOP_VERSION}.tar.gz;
HADOOP_INSTALL_PATH="$(pwd)/hadoop-${HADOOP_VERSION}";
```



## COMPILER THRIFT

In order to generate:
* new Thrift Files
* Thrift Documentations

[download & install](https://thrift.apache.org/download)
