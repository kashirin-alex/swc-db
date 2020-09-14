## Building & Installing on Ubuntu


#### Prerequisites:
```bash

apt-get update;

apt-get install -y cmake build-essential \
 libasio-dev libre2-dev \
 libz-dev libsnappy-dev libzstd-dev \
 libssl-dev \
 libreadline-dev \
 git;

mkdir builds; cd builds; # path for builds

```

##### if libasio-dev under version asio-1.18
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

#### Prerequisites Optional

##### Memory Allocator
   * _default (Compiler's)_
   * _TCMALLOC_
     ```bash

     apt-get install -y libgoogle-perftools-dev; 

     ```

##### Thrift
```bash

apt-get install -y libboost-math-dev libevent-dev libthrift-c-glib-dev libthrift-dev; 

```
   * ###### _Thrift-Compiler: [download & install](https://thrift.apache.org/download) to generate new Thrift Files_

   * ###### _Thrift-glib_c_: (without define to cmake-DWITHOUT_THRIFT_C=ON)
     ```bash

     apt-get install -y libperl-dev libffi-dev libglib2.0-dev; 

     ```
     ```bash

     GLIB_INCLUDE_PATH="$(pkg-config --cflags glib-2.0 | tr ' ' ';' | sed 's/-I//g' )";

     ```

     * ###### _PAM_: (without define to cmake -DWITHOUT_PAM=ON)
       ```bash

       apt-get install -y libpam-dev;

       ```


##### Hadoop    
_download the desired version from [Apache-Hadoop releases](https://hadoop.apache.org/releases.html)
or if there is already an installed Hadoop on the system enough HADOOP_HOME is set in Environment or define cmake with HADOOP_INSTALL_PATH=_
```bash

HADOOP_VERSION="3.2.1";
wget https://archive.apache.org/dist/hadoop/common/hadoop-${HADOOP_VERSION}/hadoop-${HADOOP_VERSION}.tar.gz
tar -xf hadoop-${HADOOP_VERSION}.tar.gz;
HADOOP_INSTALL_PATH="$(pwd)/hadoop-${HADOOP_VERSION}";

```
_tests require full Java & Apache-Hadoop installation and runtime_

   * ###### Native(JVM) - optional
     ```bash

     apt-get install -y default-jdk;

     ```
     _Version rather than JAVA_HOME apply cmake with `-DJAVA_INSTALL_PATH=$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g);`_

   * ###### Native(C++)
     _state depends on progress of [libhdfscpp](https://github.com/apache/hadoop/tree/trunk/hadoop-hdfs-project/hadoop-hdfs-native-client/src/main/native/libhdfspp)_


##### CephFS
```bash

apt-get install -y libcephfs-dev;

```

##### Python (Thrift Client & swcdb_cluster)
```bash

apt-get install -y python3-pip;

```
```bash

python3 -m pip install thrift fabric;

```

##### Maven (Java Thrift Client)
```bash

apt-get install -y maven;

```

##### Documentations
_Thrift-Documentations require as well [Thrift-Compiler](https://github.com/kashirin-alex/swc-db/wiki/Building-&-Installing--on-Ubuntu#thrift-compiler-download--install-to-generate-new-thrift-files)._
```bash

apt-get install -y graphviz doxygen;

```
_apply cmake with `-DSWC_DOCUMENTATION=ON`_
***


#### Get the SWC-DB source:
```bash

git clone https://github.com/kashirin-alex/swc-db.git;
mkdir swcdb; 
cd swcdb;

```

***
#### Configuring & Building:

```cmake

cmake ../swc-db \
 -DO_LEVEL=3 -DSWC_IMPL_SOURCE=ON \
 -DUSE_GNU_READLINE=ON \
 -DLOOKUP_INCLUDE_PATHS=/usr/lib/x86_64-linux-gnu \
 -DJAVA_INSTALL_PATH=$(find /usr/lib/jvm -name jni.h | sed s"/\/include\/jni.h//"g) \
 -DASIO_INCLUDE_PATH=${ASIO_INCLUDE_PATH} \
 -DWITHOUT_THRIFT_C=OFF -DGLIB_INCLUDE_PATH=${GLIB_INCLUDE_PATH} \
 -DWITHOUT_PAM=OFF \
 -DHADOOP_INSTALL_PATH=${HADOOP_INSTALL_PATH} \
 -DSWC_DOCUMENTATION=OFF \
 -DCMAKE_SKIP_RPATH=OFF -DCMAKE_INSTALL_PREFIX=/opt/swcdb \
 -DCMAKE_BUILD_TYPE=Debug;

```

```bash

make -j8;

```


***

#### Installing & Testing: 
```bash

make install;

```


_Test needs **swcdb_cluster** with password-less ssh authentication_
```bash

ssh-keygen -t rsa;

```
```bash

cat /root/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys;
ssh-keyscan -t rsa localhost,ip6-localhost,localhost.localdomain,::1,::,127.0.0.1 >> ~/.ssh/known_hosts;

```

```bash

make test;

```

***

#### Generating Documentation: 
```bash

make doc;

```
_The 'doc' target will generate an Archive **"swc-db-doc.tar.xz"** in the root build directory with Doxygen & Thrift-Compiler source documentations._

***

#### Building States:

 * Ubuntu 20.04 LTS (Focal Fossa) - **Building**

***

#### Running:

 * [Running a standalone cluster](https://github.com/kashirin-alex/swc-db/wiki/Running-a-standalone-cluster)
 * [Running a distributed cluster](https://github.com/kashirin-alex/swc-db/wiki/Running-a-distributed-cluster)

***

