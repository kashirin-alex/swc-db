---
title: Command Line Interfaces 
sort: 1
---



# Using the SWC-DB Command Line Interfaces (CLI)

The SWC-DB CLI/Shell is available in the utillity program `bin/swcdb` and the default Shell is the ```SWC-DB(client)>``` interface.


**The Command Line Interfaces and argument-values available to work with are:**

| Argument Token                    | Shell                                                           |  Command Line Interface |
| ---                               | ---                                                             | ---                     |
|  _without an argument_            | **[Work with the DB-Client](db_client/)**                       | ```SWC-DB(client)>```   |
| ```--manager```     / ```-mngr``` | **[Work with a Manager](manager/)**                             | ```SWC-DB(mngr)>```     |
| ```--ranger```      / ```-rgr```  | **[Work with a Ranger](ranger/)**                               | ```SWC-DB(rgr)>```      |
| ```--filesystem```  / ```-fs```   | **[Work with a FileSystem](filesystem/)** type by swc.fs='TYPE' | ```SWC-DB(fs-TYPE)>```  |


To use the specific CLI apply the argument-value-token to ```bin/swcdb --ARG;```


* For additional help information with running, use: ```bin/swcdb --help;``` and ```bin/swcdb --help-config;``` 
* For help with available shell-commands and help with the commands execution, use: ```SWC-DB('cli-name')> help;``` in the selected CLI.
