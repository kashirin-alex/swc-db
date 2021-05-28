---
title: CLI SWC-DB(statistics) 
sort: 5
---


# Using the SWC-DB(statistics) CLI - The SWC-DB Statistics monitor Shell

```bash
cd ${SWCDB_INSTALL_PATH}/bin;
```


##### ENTER SWC-DB(statistics) CLI:
```bash
./swcdb --statistics;
```

##### Enter Help:

```bash
SWC-DB(statistics)> help;
```

```bash
Usage Help:  'command' [options];
  quit               Quit or Exit the Console
  help               Commands help information
  switch to          Switch to other CLI, options: rgr|mngr|fs|stats|client
  show               show last='DURATION' since='DATETIME' agg='DURATION'
                          host='STRING' group='STRING' role='STRING' id='STRING'
                          component='STRING' part='STRING' type='STRING' metric='STRING'
                          AND .. ;
                       -> show last=30day group=swcdb role=rgr component=fs part=broker;
                          # show Fs-Broker stats on Ranger for the last 30 days
                       -> show since='2021/05/01 00:00:00' component='cpu';
                          # show CPU on all roles having CPU metrics since May of 2021
                     * STRING:   string-type definition is optional
                     * DURATION: in Decimal[week|day|hour|minute] (4week)
                     * DATETIME: in format YYYY/MM/DD HH:mm:ss or seconds-timestamp
                     * 'last=':  The Duration set or NONE default to 7day
                     * 'since=': The Datetime set or NONE for since (now-'last'|ever)
                     * 'agg=':   The Duration for aggregations or NONE into-single entry
                     * AND:      Another metrics group
  list metrics       list metrics host='STRING' group='STRING' role='STRING' id='STRING'
                                  component='STRING' part='STRING' type='STRING' AND .. ;
                       -> list metrics;
                          # List all metrics definitions
  truncate           truncate last='DURATION' since='DATETIME' agg='DURATION'
                              host='STRING' group='STRING' role='STRING' id='STRING'
                              component='STRING' part='STRING'
                              type='STRING' AND ..;
                       -> truncate;
                          # Truncate all metrics
  set stat-names     set stat-names F1,F2, .. or 'swcdb-default';
                     * swcdb-default: host,group,role,id,component,part,type
  list stat-names    list stat-names;

SWC-DB(statistics)>
```
