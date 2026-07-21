---
title: Using
sort: 1 
---

# Using SWC-DB

Application and operator interfaces for querying and managing SWC-DB.

| Guide | Purpose |
|-------|---------|
| [CLI]({{ site.baseurl }}/use/cli/) | Interactive `swcdb` shell — DB client, Manager, Ranger, FS, statistics |
| [SQL]({{ site.baseurl }}/use/sql/) | SQL syntax, comparators, schema and query reference |
| [Thrift Client]({{ site.baseurl }}/use/thriftclient/) | Thrift API and per-language client guides (C++, Java, Python, Ruby, C-Glib) |
| [C++ Client / libswcdb]({{ site.baseurl }}/use/client_library/) | Native C++ client API (not Thrift) |
| [Load Generator]({{ site.baseurl }}/use/load_generator/) | Benchmark and load tool (`swcdb_load_generator`) |

**Choosing a client:** CLI for exploration and ops; Thrift for most application languages; native C++ client for embedded apps that link `libswcdb` directly.
