---
title: Documentations
sort: 5 
---


# Generating Documentation

_The generate documentation target requires the SWC_DOCUMENTATION=ON as by [the Build Configuration Option](/swc-db/build/configure/#swc-db-configuration-options)_


## Generate
*  while at builds [path as by instructions](/swc-db/build/prerequisites/)

```bash
cd swcdb;
```

**The 'doc' target will generate an Archive "swc-db-doc.tar.xz" in the root build directory with Doxygen & Thrift-Compiler source documentations.**
```bash
make doc;
```


