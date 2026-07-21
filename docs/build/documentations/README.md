---
title: Documentations
sort: 5 
---


# Generating Documentation

_The generate documentation target requires the SWC_DOCUMENTATION=ON as by [the Build Configuration Option]({{ site.baseurl }}/build/configure/#swc-db-configuration-options)_


## Generate
* while at the build directory ([layouts]({{ site.baseurl }}/build/configure/#configuring)):

```bash
cd builds/swcdb;   # or your cmake build dir
```

**The `doc` target** generates API documentation under the build directory’s `doc/` folder and archives by language/format as `swc-db-doc-[lang]-[format].tar.xz`.

```bash
make doc;
```

### What you get

| Output | Location / role |
|--------|-----------------|
| Doxygen HTML (and related formats) | `doc/` inside the build directory |
| Packaged archives | `swc-db-doc-*.tar.xz` in the build directory |
| Published C++ API site | [https://cpp.swcdb.org](https://cpp.swcdb.org/) — mirrored from generated docs; see [Additional docs → C++]({{ site.baseurl }}/additional-docs/cpp.html) |

> Local `make doc` does not publish to cpp.swcdb.org; that site is updated from release/doc packaging workflows outside the user install path.
