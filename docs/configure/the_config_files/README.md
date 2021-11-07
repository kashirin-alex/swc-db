---
title: The Config Files
sort: 1
---


# The SWC-DB Configuration-Files with extension .cfg (etc/swcdb/*.cfg)
SWC-DB by default for the configuration files, relatively to the installation-prefix, use the folder `etc/swcdb/`. The default path for configuration files is a run-time configurable property `swc.cfg.path` argv for using with swcdbProgram 'args', the ```swcdbProgram --help;``` will indicate whether it is optional.


## The file Format
The file is a single propery a line and tree-wise grouping format.
The structure of property is a ```key=value``` whereas if key is delimmited with dot(.) ```k.e.y=value``` each dot is a sub-grouping to it's parent.
Duplicate keys with different values, value function of single-value will overwrite it's earlier value while value of many-values function will have a list of values to the key in the given order of the configurations.


### Single Key Propery
_unique key:_
```
NameOfProp.1.name=value
NameOfProp.2.name=value
```
_mulit-value key:_
```
NameOfProp.3.name=value_1
NameOfProp.3.name=value_2
```

### Grouped Key Properties
```
[NameOfProp]
1.name=value
2.name=value
3.name=value_1
3.name=value_2
[NameOfProp=end]
```

### Property Value
The value of a property defaults to type String, the available specialized value types by function are:
  * single-value [
  TYPE_BOOL,
  TYPE_UINT8,
  TYPE_UINT16,
  TYPE_INT32,
  TYPE_INT64,
  TYPE_DOUBLE,
  TYPE_STRING,
  TYPE_ENUM]
  * many-values [
  TYPE_STRINGS,
  TYPE_INT64S,
  TYPE_DOUBLES]
  * guarded-value/s [
  TYPE_BOOL_G,
  TYPE_UINT8_G,
  TYPE_UINT16_G,
  TYPE_INT32_G,
  TYPE_UINT64_G,
  TYPE_ENUM_G,
  TYPE_STRINGS_G
  ]


## The file with extended extension (.dyn.cfg)
Additionally to the `.cfg` configuration filename there are filenames with extended extension of dynamic type `.dyn.cfg`. These files are designated for use with value of function guarded-value/s which are allowed for reload of configurations at swcdbPrograms run-time.


## The Roles and cfg filename
The configuration files are changeable by configuration properties applied to a program, while by default-setup these
filenames cfg properties structured on corresponding role/service/program naming.

_**The following cfg-files for roles are used:**_

| Roles         |  type       | filename.cfg                | filename.dyn.cfg                |
| ---           | ---         | ---                         | ---                             |
| All           |             | ```swc.cfg```               | ```swc.dyn.cfg```               |
|               |             |                             |                                 |
| Manager       |             | ```swc_mngr.cfg```          | ```swc_mngr.dyn.cfg```          |
| Ranger        |             | ```swc_rgr.cfg```           | ```swc_rgr.dyn.cfg```           |
| FsBroker      |             | ```swc_fsbroker.cfg```      | ```swc_fsbroker.dyn.cfg```      |
| Broker        |             | ```swc_bkr.cfg```           | ```swc_bkr.dyn.cfg```           |
| ThriftBroker  |             | ```swc_thriftbroker.cfg```  | ```swc_thriftbroker.dyn.cfg```  |
|               |             |                             |                                 |
| libswcdb_fs_* | Type of FS  |                             |                                 |
|               | local       | ```swc_fs_local.cfg```      | ```swc_fs_local.dyn.cfg```      |
|               | broker      | ```swc_fs_broker.cfg```     | ```swc_fs_broker.dyn.cfg```     |
|               | hadoop_jvm  | ```swc_fs_hadoop_jvm.cfg``` | ```swc_fs_hadoop_jvm.dyn.cfg``` |
|               | hadoop      | ```swc_fs_hadoop.cfg```     |                                 |
|               | ceph        | ```swc_fs_ceph.cfg```       | ```swc_fs_ceph.dyn.cfg```       |
|               |             |                             |                                 |
| swcdb_cluster |             | ```swc_cluster.cfg```       |                                 |
|               |             |                             |                                 |


