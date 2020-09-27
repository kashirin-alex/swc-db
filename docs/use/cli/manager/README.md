---
title: CLI SWC-DB(mngr) 
sort: 2
---


# Using the SWC-DB(mngr) CLI - The SWC-DB Manager Shell

```bash
cd ${SWCDB_INSTALL_PATH}/bin;
```


##### ENTER SWC-DB(mngr) CLI:
```bash
./swcdb -mngr;
```


##### Enter Help:

```bash
SWC-DB(mngr)> help;
```

```bash
Usage Help:  'command' [options];
  quit              Quit or Exit the Console
  help              Commands help information
  cluster           report cluster-status state by error
                    cluster;
  status            report managers status by mngr-endpoint, of Schema-Role or by All
                    status [Schemas(default) | ALL | endpoint=HOST/IP(|PORT)];
  column-status     report column status with ranges status
                    column-status [cid=CID | name=NAME];
  rangers-status    report rangers status by Manager of column or Rangers-Role
                    rangers-status [without | cid=CID | name=NAME];

SWC-DB(mngr)>
```
