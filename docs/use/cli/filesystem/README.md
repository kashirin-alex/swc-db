---
title: CLI SWC-DB(fs-type) 
sort: 4
---


# Using the SWC-DB(fs-Type) CLI - The SWC-DB File-System Shell

```bash
cd ${SWCDB_INSTALL_PATH}/bin;
```


##### ENTER SWC-DB(fs-Type) CLI:
The File-System Type is defined by the configuration property ```swc.fs``` with the cfg-file or with an argument ```--key=value ```.
* To work, for example, with the "local" file-system type, the command is as follow:
```bash
./swcdb -fs --swc.fs=local;
```
* To work, with the property swc.fs defined in a cfg-file, it is:
```bash
./swcdb -fs;
```

##### Enter Help:

```bash
SWC-DB(fs-broker)> help;
```

```bash
Usage Help:  'command' [options];
  quit    Quit or Exit the Console
  help    Commands help information
  list    list directory contents
          list 'path';

SWC-DB(fs-broker)>
```
