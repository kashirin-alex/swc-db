---
title: CLI SWC-DB(rgr) 
sort: 3
---


# Using the SWC-DB(rgr) CLI - The SWC-DB Ranger Shell

```bash
cd ${SWCDB_INSTALL_PATH}/bin;
```


##### ENTER SWC-DB(rgr) CLI:
```bash
./swcdb -rgr;
```


##### Enter Help:

```bash
SWC-DB(rgr)> help;
```

```bash
Usage Help:  'command' [options];
  quit                Quit or Exit the Console
  help                Commands help information
  switch to           Switch to other CLI, options: rgr|mngr|fs|stats|client
  report-resources    report Ranger resources
                      report-resources endpoint/hostname[|port];
  report              report loaded column or all and opt. ranges on a Ranger
                      report [cid=NUM/column='name'] [ranges] endpoint/hostname[|port];

SWC-DB(rgr)>
```
