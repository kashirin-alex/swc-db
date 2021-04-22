---
title: Backup & Restore
sort: 1
---


# Backup & Restore data on SWC-DB
One of the ways backup and restore can be done is with `SWC-DB(client)>` CLI and the available commands.

* Dump & Load can use additional FileSystem configurations and write/read to other storage space.

> It is incompatible and There is no need of META-COLUMNS backup & restore.


***


### Backing Up
* List all the columns CID
* Save the Schema
* Dump the Column


```bash
#!/usr/bin/env bash

### * List all the columns CID
COLUMNS_SELECTOR="r'.*'";
# ALL="" or only the columns-selector, "r'.*'"==EXCLUDING_SYS

SCHEMAS_PATH="./swcdb-schemas";
mkdir -p ${SCHEMAS_PATH};

_cmd="list schemas OUTPUT_ONLY_CID ${COLUMNS_SELECTOR};";
cids=`echo "${_cmd} quit;" | ${SWCDB_INSTALL_PATH}/bin/swcdb --quiet --swc.logging.level=WARN;`;

for cid in ${cids}; do
     ### * Save the Schema
     _cmd="get schema ${cid};";
     schema=`echo "${_cmd} quit;" | ${SWCDB_INSTALL_PATH}/bin/swcdb --quiet --swc.logging.level=WARN`;

     echo "Backing Up: ${schema}";
     echo ${schema} > ${SCHEMAS_PATH}/${cid}.schema;

     ### * Dump the Column
     _cmd="dump col=${cid} into fs=LOCAL path='dumps/${cid}' ext=zst level=6 DISPLAY_STATS;";
     echo "${_cmd} quit;" | ${SWCDB_INSTALL_PATH}/bin/swcdb --quiet --swc.logging.level=WARN;
     echo "###";
done;
```


***


### Restoring
* Create all the Columns Schema
* Load Column Data


```bash
#!/usr/bin/env bash

SCHEMAS_PATH="./swcdb-schemas";

### * Create all the Columns Schema
for f_schema in ${SCHEMAS_PATH}/*.schema; do
     _schema=`cat ${f_schema}`;

     echo "Restoring: ${_schema}";
     echo "add ${_schema}; quit;" | ${SWCDB_INSTALL_PATH}/bin/swcdb --quiet --swc.logging.level=WARN;

     filename=$(basename -- "${f_schema}");
     cid="${filename%.*}"

     ### * Load Column Data
     _cmd="load from fs=LOCAL path='dumps/${cid}' into col=${cid} DISPLAY_STATS;";
     echo "${_cmd} quit;" | ${SWCDB_INSTALL_PATH}/bin/swcdb --quiet --swc.logging.level=WARN;
     echo "###";

done;
```


****

