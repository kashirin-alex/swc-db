---
title: Documentations
sort: 5 
---


# Generating Documentation

_The generate documentation target requires the SWC_DOCUMENTATION=ON as by [the Build Configuration Option]({{ site.baseurl }}/build/configure/#swc-db-configuration-options)_


## Generate
*  while at builds [path as by instructions]({{ site.baseurl }}/build/prerequisites/)

```bash
cd swcdb;
```

**The 'doc' target will generate documentations in the `doc` folder of the build directory with the source documentations and archive by languages & formats to `swc-db-doc-[lang]-[format].tar.xz`.**
> * [The Generated SWC-DB Documentations](https://www.swcdb.org/additional-docs/)

```bash
make doc;
```


