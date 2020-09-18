

# HADOOP VERSION

_download the desired version from [Apache-Hadoop releases](https://hadoop.apache.org/releases.html)
or if there is already an installed Hadoop on the system enough HADOOP_HOME is set in Environment or define cmake with HADOOP_INSTALL_PATH=_

```bash
HADOOP_VERSION="3.2.1";
wget https://archive.apache.org/dist/hadoop/common/hadoop-${HADOOP_VERSION}/hadoop-${HADOOP_VERSION}.tar.gz
tar -xf hadoop-${HADOOP_VERSION}.tar.gz;
HADOOP_INSTALL_PATH="$(pwd)/hadoop-${HADOOP_VERSION}";
```
