---
title: Introduction
sort: 1
---

# Welcome to the documentations of SWC-DB© !

**SWC-DB© (Super Wide Column Database) - a High Performance Scalable Database**



***



## Introduction to the SWC-DB
The SWC-DB _(Super Wide Column Database)_ a Super Fast database management system designed to handle [Yottabytes+](https://en.wikipedia.org/wiki/Yottabyte) on a [quadrillion](https://en.wikipedia.org/wiki/Orders_of_magnitude_(numbers)#1024) base of entries. Latest release of SWC-DB is version 0.5.4 available at [github.com/kashirin-alex/swc-db](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.4) more details at [Getting SWC-DB]({{ site.baseurl }}/install/getting_swcdb/) .


_The proven test with SWC-DB v0.4.9, on one machine(CPU Intel E3-1246V3 RAM 32GB SW-RAID 2x 2TB-7200rpm) run the DB and the Client, capabilities standing is a dozen terabytes of raw-data with 100+billion of record entries with performace input of ~250,000+ cells a second and random scan/select of one entry surrounds the microseconds to milliseconds, whereas performance varies on applied configurations and hardware. **Testing facillity is required for in deep and further testing.**_





#### The **_```What is SWC-DB?```_** checklist ##

| Is it?                                          | ```?``` | Comments |
| ---                                             | ---     | ---      |
|                                                 |  | Legend: <br/> ```✔``` - Yes <br/>  ```–✔``` - Require Enabling Feature <br/>  ```✗``` - No |
| **a Key Value DB**                                    | **```✔```**  |          |
| **a Wide Column Database DB**                         | **```✔```**  |          |
| **a NOSQL (Not Only Structual Query Language) DB**    | **```✔```**  |          |
| **a [SQL]({{ site.baseurl }}/use/sql/) (Structual Query Language) DB**  | **```✔```**  | no joins |
| **a DB with ACID(concept) for a single data-entry**   | **```✔```**  | write with acknowledgment on base of one data-entry(cell) |
| **a DB with ACID(concept) for a many data-entries**   | **```✗```**  | write with acknowledgment many data-entries(cells) with one request, problem - one entry can exist while other yet to exist |
| **a Relational DB**                                   | **```✗```**  | achievable by the client-side |
| **a Time Series DB**                                  | **```✔```**  | without relational propotions |
| **an Object Oriented DB**                              | **```–✔```** | binary key(Fractions) and value allowed, user defined Serialization IO or/and Serializations available with the SERIAL column-type |
| **a XML DB / Tripel Stores**                          | **```–✔```** | require Native XML Client process XML-schema to Database structure |
| **a Schema Free DB**                                  | **```✔```**  | except to data/column type, schema/column-definition is without inner data schematics |
| **a DB with User Concepts (Access Controls)**         | **```✗```**  |     |
| **a Shard Partitioning DB**                           | **```✔```**  |     |
| **a Spatial DB**                                      | **```✔```**  | user defined Fractions order and sequence in a Cell Key |
| **a Variable Key Sequences DB**                       | **```✔```**  | infinite size (indefinite other limitation apply ) |
| **a Distributed + Scalable DB**                       | **```✔```**  | O(1) performance |
| **a DB with Multihoming Capabilities**                | **```✔```**  |     |
| **a DB with Nanosecond Precision**                    | **```✔```**  |     |
| **a DB supporting variety of File-Systems**           | **```✔```**  | any File System which can implement the Base class SWC::FS::FileSystem |
| **a DB with Data Replication Support**                | **```–✔```** | File System dependant |
| **a Versioned Data DB**                               | **```✔```**  | automatic or user defined versions by Desc/Asc with Max Versions to Keep |
| **a DB supporting an “Atomic“ Counter Value**         | **```✔```**  | |
| **a DB supporting Query of Multiple Sequences**       | **```✔```**  | multiple Columns with multiple Intervals in a Column |
| **a Document/Files stores DB**                        | **```✔```**  | user defined Meta/Header & Key-Fractions(file-path) structure whereas anomaly of use is at file-size above Range-Block-size cfg |
| **a Super Wide Column DB**                            | **```✔```**  | supporting 16777216(2^24,uint24) number of Fractions, max 2^24 bytes a Fraction and total size of a key upto 2^32 bytes |
| **a DB with Communications over Secure Layer(SSL)**   | **```✔```**  | The total security of transactions require the underlying configured FileSystem and Clients to have & support a Secure Layer |


#### The Differences of SWC-DB with Wide Column Databases
The major differences **“Super Wide Column Database”** has to commonly known Wide Column Databases are SWC-DB does not have Tables nor
Namespaces whereas the number of columns is numerous with a support of a selector over Column-Tags and while cell key as known to be in Wide Column Database structured in timestamp, row, column-family and column-family-qualifier in
SWC-DB a cell key is a list of Fractions with timestamp. The differences in [SQL]({{ site.baseurl }}/use/sql/) syntax, it is in-place of ```select columns from “table_name”``` with SWC-DB
It is ```select [ where_clause [Columns-Intervals] ]```.\
Considering to structure a Wide-Column-DB in SWC-DB it can be in these forms ```key=[F(row),
F(column-family), F(column-family-qualifier)]``` or the actual column is named after Namespace, Table and Column Family with ```key=[F(row), F(column-family-qualifier)]```.


#### The Cell Key and Fractions in SWC-DB
The Fractions in SWC-DB cell-key let numerous “qualifiers”, as known to be, with a range-locator able to respond with the ranges applicable to the
fractions of a scan specs. As a result a scan-spec of ```key=[>“”, >=”THIS”]``` will scan ranges that consist the “THIS” on comparator with a help of metacolumn
that include, additionally to the key-begin and key-end of a range, the minimal and maximum values of the fractions in an aligned manner. Hence
the name “Super Wide Column“ a column can have cells with one key ```[F(1st)]``` second key ```[F(1st), F(2nd)]``` third key ```[F(1st), F(2nd), F(3rd), .. ]``` and the
scan/select is possible on ```[F(1st)]``` and above that will return all the cells having fraction one equal “1st ” and so as without further indexations to select
cells with key ```[>F(), F(2nd)]``` returning the cells with second fraction equal “2nd”.


#### The Comparators in SWC-DB
The [Comparators]({{ site.baseurl }}/use/sql/#comparators-syntax) available in SWC-DB are NONE, PF ,GT, GE, EQ, LE, LT, NE, RE and domain-object(Fully/Partially sub/sup set) SBS, SPS, POSBS, POSPS, FOSBS and FOSPS, while some Comparators have limitations for range-locator as regexp is evaluated as
NONE being anything-match. Additionally the conditions of comparators applied on the corresponding [“ key-sequence ”]({{ site.baseurl }}/use/thriftclient/#enumeration-keyseq) by column's schema that include
LEXIC, VOLUME, FC_LEXIC, FC_VOLUME that define the sequence of cells in a range. If a prefix (PF) is desired than the choice will be the LEXIC or with
FC_LEXIC as VOLUME (volumetric) will not correspond to the char-byte sequence while if desired to have for example a decimal sequence of 0, 1, 2 .. 11
the VOLUME is the right choice whereas the FC_VOLUME unlike tree-wise on fraction keeps the sequence of smaller key fractions-count at the beginning in the range.


#### The Master Ranges and Meta Ranges in SWC-DB
SWC-DB use a self-explanatory master-ranges that define ranges to meta-ranges of data-ranges(cells-range) whereas on range-locator scan includes the
Key comparison on the comparators of request, resulting in most narrowed ranges for the scan of cells.\
**SWC-DB have reserved system-columns for the purposes:**
1. Ranges("SYS_MASTER_LEXIC") \
  The Meta-Column ranges with Lexical Key-Sequence.
2. Ranges("SYS_MASTER_VOLUME") \
  The Meta-Column ranges with Volumetric Key-Sequence.
3. Ranges("SYS_MASTER_FC_LEXIC") \
  The Meta-Column ranges with Fractions-Count Lexical Key-Sequence.
4. Ranges("SYS_MASTER_FC_VOLUME") \
  The Meta-Column ranges with Fractions-Count Volumetric Key-Sequence.
5. Ranges("SYS_META_LEXIC") \
  The Data-Columns ranges with Lexical Key-Sequence.
6. Ranges("SYS_META_VOLUME") \
  The Data-Columns ranges with Volumetric Key-Sequence.
7. Ranges("SYS_META_FC_LEXIC") \
  The Data-Columns ranges with Fractions-Count Lexical Key-Sequence.
8. Ranges("SYS_META_FC_VOLUME") \
  The Data-Columns ranges with Fractions-Count Volumetric Key-Sequence.
9. System("SYS_RGR_DATA") \
  The Ranger-Data column is an internal system column for mapping/resolution of Column-Range to Ranger ID with Endpoints. Instead of storing RgrData in a file(/C/cid/R/rid/ranger.data) on the file-system, The column exists in favour and the stored data is for columns above/other-than system Ranges-Columns and the SYS_RGR_DATA it self. \
10. Statistics("SYS_STATS") \
  The Statistics column used for internal systems monitoring and it can be used like any other serial-column with Lexic Key-Sequence (keeping for the purpose) with fractions of [period, host, group, role, id, component, part, type] with value of serialized merics data.
11. Definers("SYS_DEFINE_LEXIC") \
  The Definer columns is SERIAL type column with Lexic Key Sequence and the column is used for system's internal column-definitions of metrics-data serialization saved to SYS_STATS. The SYS_DEFINE_LEXIC column can be used as any column, whereas requires first fraction to be the CID of a column for the serialization defintions.



#### The Persistent Storage in SWC-DB
The storage-form in the SWC-DB on FS is based by column-id and range-id, that on path consist CellStores and CommitLog files while at any point one
server is responsible for a range-id on column-id and of a path root. The CellStores are files storing Cells in serialized form that are after latest
compaction whereas Commit-Log is Fragments of current added data, one fragment is written at a time on a threshold reach or on shutdown.


#### The limitations that can be overseen with SWC-DB
  ✗ Maximum number of columns, it is store-size of uint64(2^64) – 11(reserved cols) which can be improved by CID to be a string-type. \
  ✗ Maximum size of Value or Key-Fraction(after serialization), it is 4GB, while for such data size other limitations apply.


#### The capabilities to expect from SWC-DB
  ✔ A Manager-Root with definitions of 1K^2 ranges (a use of 1 GB RAM) is a definition of 1K^4 Meta-Ranges that sums-down to 1K^8 Data-Ranges, with
range-size configuration to 10GB that makes a total storage volume for a cell size average of 256KB to be a quarter of Yotta Byte. \
  ✔ A client can read at 100%(while Client's and Ranger's are equivalent) bandwidth, considering a perfect scan case of each client is requesting on
different ranges, number of clients at a given time can be by the number of data Rangers using 100% bandwidth each. \
  ✔ Maximum number of concurrent connections to a given server instance, it is the total available ports on the server by the number of configured
IPv4 and IPv6 with support of multi-homed / multiple interfaces.


#### The Failure Tolerance of SWC-DB
✔ A failed request to a Manager is a connection fail-over to next in list from 'swc.mngr.host' configuration. \
✔ A failed request to a Ranger(Master, Meta, Data)-N is a fail-over to the new newly assigned Ranger(addr) by Manager. \
✔ Manager, on interval or shut-down state of a managed Ranger(either role), request to load ranges to another Ranger. \
✔ Distribute File System, depends on the system and it's feature of routing to a datanode. \
✔ Managers or Rangers in case of a connection or file-descriptor failure try to reconnect to the DFS. \
✔ Communications security, SSL applicable between servers for non-secure networks. \
✔ Communication over-heads of resolved-data of column-name, RID-location and Ranger-address are kept on TTL/KA.
In worst case of outdated data being used with a request the Ranger return an error of a NOT_LOADED_RANGE.




***



## The Documentations Table of Contents

* [Introduction to the SWC-DB](#introduction-to-the-swc-db)
* USING
  * [SQL]({{ site.baseurl }}/use/sql/)
  * [Thrift Client]({{ site.baseurl }}/use/thriftclient/)
  * [CLI Client]({{ site.baseurl }}/use/cli/)
  * [Load Generator]({{ site.baseurl }}/use/load_generator/)
* RUNNING
  * [Psedomode]({{ site.baseurl }}/run/psedomode/)
  * [Distributed]({{ site.baseurl }}/run/distributed/)
* CONFIGURING
  * [The Config Files]({{ site.baseurl }}/configure/the_config_files/)
  * [The Properties]({{ site.baseurl }}/configure/properties/)
* INSTALLING
  * [Installation Steps]({{ site.baseurl }}/install/steps/)
  * [Dependencies]({{ site.baseurl }}/install/dependencies/)
  * [Getting SWC-DB]({{ site.baseurl }}/install/getting_swcdb/)
  * [Setting up swcdb_cluster]({{ site.baseurl }}/install/swcdb_cluster/)
* BUILDING
  * [Build Steps]({{ site.baseurl }}/build/steps/)
  * [Prerequisites]({{ site.baseurl }}/build/prerequisites/)
  * [Configure]({{ site.baseurl }}/build/configure/)
  * [Make]({{ site.baseurl }}/build/make/)
  * [Documentations]({{ site.baseurl }}/build/documentations/)
  * [Test]({{ site.baseurl }}/build/test/)
* SUPPORT & LICENSE
  * [Support on SWC-DB]({{ site.baseurl }}/support_license/support.html)
  * [License of SWC-DB]({{ site.baseurl }}/support_license/license.html)



<iframe src="https://github.com/sponsors/kashirin-alex/card" title="Sponsor kashirin-alex" width="100%" style="border: 0;width:100%;"></iframe>



***




### DEVELOPMENT ABSTRACT
[https://alex.kashirin.family/swc-DB.pdf](https://alex.kashirin.family/swc-DB.pdf)



***
##### **_USE SIDEBAR TO NAVIGATE TO GUIDE SECTIONS_**
***



![SWC-DB©]({{ site.baseurl }}/logo.svg)


