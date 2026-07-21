---
title: C-Glib
sort: 5
---


# Using C-Glib Thrift Client

The C-Glib Thrift client wraps Apache Thrift’s C GLib bindings around the generated SWC-DB Service. Headers ship with `libswcdb_thrift_c`.


## Build / install

1. Install GLib development packages and Apache Thrift with C GLib support.
2. Configure the SWC-DB build with `WITHOUT_THRIFT_C=OFF` and set `GLIB_INCLUDE_PATH` if needed (see [Build Configure]({{ site.baseurl }}/build/configure/)).
3. Or build only the library: `-DSWC_BUILD_PKG=lib-thrift-c`.

Install notes: [Thrift Clients — C-Glib]({{ site.baseurl }}/install/thrift_clients/#c-glib-thrift-client).


## API surface

Include `swcdb/thrift/client/client.h`. The helper type and connect/disconnect functions:

```c
#include "swcdb/thrift/client/client.h"

swcdb_thrift_client client;
GError* error = NULL;

if (!swcdb_thrift_client_connect(&client, "localhost", 18000, &error)) {
  /* handle error */
}

/* client.service is a swcdb_thriftServiceIf* — call generated Service methods */

swcdb_thrift_client_disconnect(&client);
```

Generated types and Service methods match the [Thrift Service reference]({{ site.baseurl }}/use/thriftclient/). Method names follow the C GLib Thrift conventions from `swcdb/thrift/gen-c_glib/`.


## Link flags

* Include: `-I${SWCDB_INSTALL_PREFIX}/include`
* Library: `-lswcdb_thrift_c` (plus Thrift C GLib, GLib, and related deps)
