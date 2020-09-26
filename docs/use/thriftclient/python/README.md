---
title: Python
sort: 3
---


# Using Python Thrift Client
The package `swcdb` has the SWC-DB ```thirft``` module which consist of modules ```service``` the default SWC-DB client and several Thrift implementations ```native```,  ```tornado```, ```twisted``` and ```zopeif```.
* The ```swcdb.thrift.native``` is a python native implementation. It is the implemenation used in the ```swcdb.thrift.service``` module and the Documentaions discuss the use of the default SWC-DB client.
* The ```swcdb.thrift.tornado``` is the implementation for using with [Tornado](https://pypi.org/project/tornado/) a Python web framework.
* The ```swcdb.thrift.twisted``` is the implementation for using with [Twisted](https://pypi.org/project/twisted/) an event-based framework for internet applications.
* The ```swcdb.thrift.zopeif``` is the Zope Interface for using with [Zope](https://pypi.org/project/zope/) implementations.
 


## The Methods
The class `swcdb.thrift.service.Client` inherits the [SWC-DB Thrift Service]({{ site.baseurl }}/use/thriftclient/#service-service) and all the methods/functions in the SWC-DB Thrift Service are available in the python client. The methods with prefix ```.sql_``` require the structual syntax of the [SWC-DB SQL]({{ site.baseurl }}/use/sql/).



## The Object Types
All the Objects **[Enumerations]({{ site.baseurl }}/use/thriftclient/#enumerations), [Type-Declarations]({{ site.baseurl }}/use/thriftclient/#type-declarations), [Struct & Exception]({{ site.baseurl }}/use/thriftclient/#data-structures)** are as [The SWC-DB Thrift Modules]({{ site.baseurl }}/use/thriftclient/#the-swc-db-thrift-modules) and available in module `swcdb.thrift.service`.



## Before Using
Before you can start using the SWC-DB Python Package you need as [by instructions to Install the package]({{ site.baseurl }}/install/thrift_clients/#install-the-swc-db-python-package).



## Importing, Initializing and Connecting to SWC-DB Thrift-Broker & Closing Client
```python
from swcdb.thrift import service

# Establish Connection
client = service.Client(
  "localhost", 
  18000, 
  timeout_ms=900000, 
  socket=None, 
  do_open=True, 
  framed=True
)

# Close Client Connection
client.close()

exit(0)
```
_The `client` is now a swcdb.thrift.service.Client object_




## Examples
```assert``` in examples is for example checking purpose.



#### a SQL - List all Columns - example

```python
from swcdb.thrift import service

# Establish Connection
client = service.Client("localhost",  18000)

schemas = client.sql_list_columns("list columns")
for schema in schemas:
    print(schema)

# Close Client Connection
client.close()

exit(0)
```



#### a Specification - List all Columns - example

```python
from swcdb.thrift import service

# Establish Connection
client = service.Client("localhost",  18000)

spec = None # service.SpecSchemas()
schemas = client.list_columns(spec)
for schema in schemas:
    print(schema)

# Close Client Connection
client.close()

exit(0)
```



#### a SQL - Create & Get column, Update & Select and remove Column - example

```python
from swcdb.thrift import service

# Establish Connection
client = service.Client("localhost",  18000)

test_colm_name = 'py-thrift-example-sql'

# Create Column
client.sql_mng_column("create column(name='" + test_colm_name + "' seq=VOLUME)")

# Get Column
schemas = client.sql_list_columns("get column " + test_colm_name) 
assert(len(schemas) == 1)
schema = schemas[0]
assert(schema.col_name == test_colm_name)

# Update with 1000 cells
n_cells = 1000
client.sql_update(
    "update " +
    ",".join(["cell(INSERT, " + str(schema.cid) + ", [" + str(n) + "], '', 'Value of F(1) is " + str(n) + "')"
              for n in range(0, n_cells, 1)]), 
    0)

# Select all the cells, expect in Volume Sequence
cells = client.sql_select("select where col(" + str(schema.cid) + ")=(cells=())")
assert(len(cells) == n_cells)

n = 0
for cell in cells:
    assert(int(cell.k[0]) == n)
    n += 1

# Remove/Delete Column
client.sql_mng_column("delete column(cid=" + str(schema.cid) + " name='" + schema.col_name + "')")

# check Removed
schemas = []
try:
    schemas = client.sql_list_columns("get column " + test_colm_name) 
except:
    pass
assert(len(schemas) == 0)

# Close Client Connection
client.close()

exit(0)
```



#### a Specification - Create & Get column, Update & Select and remove Column - example

```python
from swcdb.thrift import service

# Establish Connection
client = service.Client("localhost",  18000)

test_colm_name = 'py-thrift-example-spec'

# Create Column
client.mng_column(
    service.SchemaFunc.CREATE,
    service.Schema(col_name=test_colm_name, col_seq=service.KeySeq.VOLUME)
)

# Get Column
spec = service.SpecSchemas(names=[test_colm_name])
schemas = client.list_columns(spec)
assert(len(schemas) == 1)
schema = schemas[0]
assert(schema.col_name == test_colm_name)

# Update with 100000 cells
n_cells = 100000
client.update(
    {
        schema.cid:  [
            service.UCell(
                f=service.Flag.INSERT,
                k=[bytearray(str(n), 'utf8')],
                ts=None, ts_desc=None,
                v=bytearray("Value of F(1) is " + str(n), 'utf8'))
            for n in range(0, n_cells, 1)
            ]},
    0)
#


# Select all the cells, expect in Volume Sequence
specs = service.SpecScan(
    columns=[
        service.SpecColumn(
            cid=schema.cid, 
            intervals=[service.SpecInterval()]
        )
    ],
    flags=None
)
cells = client.scan(specs)
assert(len(cells) == n_cells)

n = 0
for cell in cells:
    assert(int(cell.k[0]) == n)
    n += 1

# Remove/Delete Column
client.mng_column(service.SchemaFunc.DELETE, schema)

# check Removed
schemas = []
try:
    schemas = client.list_columns(service.SpecSchemas(names=[test_colm_name]))
except:
    pass
assert(len(schemas) == 0)

# Close Client Connection
client.close()

exit(0)
```

