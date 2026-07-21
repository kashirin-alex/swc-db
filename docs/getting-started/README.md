---
title: Getting Started
sort: 0
---

# Getting Started with SWC-DB

A minimal first-run path: install binaries, start a local cluster, run a first query.

> **Which client?** Use the [CLI]({{ site.baseurl }}/use/cli/) for interactive exploration. Use [Thrift]({{ site.baseurl }}/use/thriftclient/) (Python, Java, Ruby, C, C++) for application integration. Use the [native C++ client]({{ site.baseurl }}/use/client_library/) (`libswcdb`) for embedded C++ apps. Use [SQL]({{ site.baseurl }}/use/sql/) syntax from the CLI or any client.

Developers building from source should follow [Build Steps]({{ site.baseurl }}/build/steps/) instead of the install path below.

---

## 1. Install dependencies and binaries

1. [Install dependencies]({{ site.baseurl }}/install/dependencies/) for your OS.
2. [Download and install a release package]({{ site.baseurl }}/install/getting_swcdb/) into `/opt/swcdb` (default).
3. Optionally [install Thrift client packages]({{ site.baseurl }}/install/thrift_clients/) for your language.

---

## 2. Configure (minimal)

After a package install into `/opt/swcdb`, the shipped `etc/swcdb/*.cfg` defaults are usually enough for a local smoke test. Create the log directory:

```bash
mkdir -p /opt/swcdb/var/log/swcdb
```

Only edit `.cfg` files if you need a non-default FS backend, ports, or hosts — see [The Config Files]({{ site.baseurl }}/configure/the_config_files/) and [Properties]({{ site.baseurl }}/configure/properties/).

---

## 3. Start a local cluster (pseudomode)

Follow [Running SWC-DB in Pseudomode]({{ site.baseurl }}/run/pseudomode/):

```bash
cd /opt/swcdb
mkdir -p var/log/swcdb
sbin/swcdb_cluster start
```

Or start each daemon from `bin/` in order: FsBroker → Manager → Ranger → Broker → ThriftBroker.

---

## 4. Run a first query

**CLI (interactive):**

```bash
/opt/swcdb/bin/swcdb
```

At the `SWC-DB(client)>` prompt, try `help;` then explore SQL commands — see [Using SQL]({{ site.baseurl }}/use/sql/).

**Thrift (application):** pick your language under [Thrift Client]({{ site.baseurl }}/use/thriftclient/).

---

## Next steps

| Goal | Guide |
|------|-------|
| Distributed deployment | [Running Distributed]({{ site.baseurl }}/run/distributed/) |
| Full install flow | [Installation Steps]({{ site.baseurl }}/install/steps/) |
| Build from source | [Build Steps]({{ site.baseurl }}/build/steps/) |
| C++ API reference | [Additional docs → C++]({{ site.baseurl }}/additional-docs/cpp.html) |
| Run tests | [Test]({{ site.baseurl }}/build/test/) |
