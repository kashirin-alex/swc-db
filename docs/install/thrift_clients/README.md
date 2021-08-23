---
title: Thrift Clients
sort: 4
---

# Installing Thrift Clients



### SWC-DB Thrift Client as part of a package installations:
* [The SWC-DB Python Package](#install-the-swc-db-python-package)
* [The SWC-DB Ruby Package](#install-the-swc-db-ruby-package)



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

* The SWC-DB Release Binaries inlcude the `swcdb` Python package:
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
