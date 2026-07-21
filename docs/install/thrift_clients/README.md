---
title: Thrift Clients
sort: 4
---

# Installing Thrift Clients



### SWC-DB Thrift Client as part of a package installations:
* [The SWC-DB Python Package](#install-the-swc-db-python-package)
* [The SWC-DB Ruby Package](#install-the-swc-db-ruby-package)
* [Java / Maven / JDBC](#java--maven--jdbc)
* [C++ Thrift Client](#c-thrift-client)
* [C-Glib Thrift Client](#c-glib-thrift-client)



***



## Install the SWC-DB Python Package

* The [SWC-DB Python package ```swcdb```](https://pypi.org/project/swcdb/) is available at PyPi.org
and it can be installed with:
```python
pip install swcdb;
```
or for other python versions/implementations
```python
YOUR_PY_EXEC -m pip install swcdb;
```

* The SWC-DB Release Binaries include the `swcdb` Python package:
```
SWCDB_VERSION_PYTHON=${SWCDB_VERSION}; # Python package can have another sub-version ".#"
YOUR_PY_EXEC -m pip install wheel ${SWCDB_INSTALL_PATH}/lib/py/swcdb-${SWCDB_VERSION_PYTHON}.tar.gz;
```

Documentations for using the SWC-DB Python package are available at [Using Python Thrift Client]({{ site.baseurl }}/use/thriftclient/python/)



***



## Install the SWC-DB Ruby Package

* The [SWC-DB Ruby gem-package ```swcdb```](https://rubygems.org/gems/swcdb) is available at RubyGems.org
and it can be installed with:
```bash
gem install swcdb;
```

Documentations for using the SWC-DB Ruby package are available at [Using Ruby Thrift Client]({{ site.baseurl }}/use/thriftclient/ruby/)



***



## Java / Maven / JDBC

* The Thrift client artifact `org.swcdb:thrift` is published to Maven Central. Use the latest version listed on [Maven Central](https://search.maven.org/artifact/org.swcdb/thrift) (it may track the source tree version even when binary server packages lag). See [Using Java Thrift Client]({{ site.baseurl }}/use/thriftclient/java/) for `pom.xml` and classpath examples.

* When Java is built with the tree (`SWC_WITHOUT_JAVA=OFF`), CMake also produces a JDBC driver JAR (`swcdb-jdbc-thrift-*-bundled.jar`) installed under the package `lib/` (or Maven `target/`). JDBC URL form: `jdbc:swcdb:thrift:hostname:18000/`.



***



## C++ Thrift Client

Build or install `libswcdb_thrift` (full build or `SWC_BUILD_PKG=lib-thrift`). Usage and link notes: [Using C++ Thrift Client]({{ site.baseurl }}/use/thriftclient/cpp/).



***



## C-Glib Thrift Client

Build `libswcdb_thrift_c` with `WITHOUT_THRIFT_C=OFF` (or `SWC_BUILD_PKG=lib-thrift-c`) and set `GLIB_INCLUDE_PATH` as needed. See [Using C-Glib Thrift Client]({{ site.baseurl }}/use/thriftclient/c_glib/).
