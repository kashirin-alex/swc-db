

```python

from swcdb.thrift import service

try:
    client = service.Client(
        "localhost", 
        18000, 
        timeout_ms=900000, 
        socket=None, 
        do_open=True, 
        framed=True
    )
except Exception as e:
    print(e)
    exit(1)


schemas_all = client.sql_list_columns("list columns")
print(schemas_all)
client.sql_mng_column("create column(name='py-thrift-test-1')")
client.sql_mng_column("create column(name='py-thrift-test-2' cell_versions=5)")

schemas_added = client.sql_list_columns("get columns py-thrift-test-1, py-thrift-test-2")
print(schemas_added)

schema = client.sql_list_columns("get column py-thrift-test-2")
client.sql_mng_column("modify column(cid=" + str(schema[0].cid) + " name='py-thrift-test-2' cell_versions=3)")

client.sql_update("update cell(INSERT, py-thrift-test-1, [1], '', 'VALUE-DATA-1-0')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1], '', 'VALUE-DATA-1-1')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1], '', 'VALUE-DATA-1-2')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1], '', 'VALUE-DATA-1-3')", 0);

client.sql_update("update cell(INSERT, py-thrift-test-1, [1,2], '', 'VALUE-DATA-12-0')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1,2], '', 'VALUE-DATA-12-1')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1,2], '', 'VALUE-DATA-12-2')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1,2], '', 'VALUE-DATA-12-3')", 0);

client.sql_update("update cell(INSERT, py-thrift-test-1, [1,2,3], '', 'VALUE-DATA-123-0')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1,2,3], '', 'VALUE-DATA-123-1')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1,2,3], '', 'VALUE-DATA-123-2')", 0);
client.sql_update("update cell(INSERT, py-thrift-test-2, [1,2,3], '', 'VALUE-DATA-123-3')", 0);

cells = client.sql_select("select where col(py-thrift-test-2)=(cells=())")
print(cells)
print("number of cell: "+str(len(cells)))

client.sql_query("select where col(py-thrift-test-2)=(cells=())", service.CellsResult.ON_FRACTION)

cells = client.sql_select("select where col(py-thrift-test-1)=(cells=())")
print(cells)
print("number of cell: "+str(len(cells)))




client.sql_query(
    "select where col(py-thrift-test-1, py-thrift-test-2)=(cells=())", 
    service.CellsResult.ON_FRACTION
).fcells

client.sql_query(
    "select where col(py-thrift-test-1, py-thrift-test-2)=(cells=(key=[='1']))", 
    service.CellsResult.ON_COLUMN
).ccells

client.sql_query(
    "select where col(py-thrift-test-1, py-thrift-test-2)=(cells=(key=[='1'] MAX_VERSIONS=1))", 
    service.CellsResult.ON_KEY
).kcells



client.sql_update(
    "update cell(INSERT, py-thrift-test-1, [a9, 2], '', 'VALUE-DATA-9-2-0'),"
    "cell(INSERT, py-thrift-test-1, [a10, 3], '', 'VALUE-DATA-10-0'),"
    "cell(INSERT, py-thrift-test-1, [a11, 4], '', 'VALUE-DATA-11-0')"
    ,  0
)
cells = client.sql_select(
    "select where col(py-thrift-test-1, py-thrift-test-2)=(cells=(key<=[=^'a',>='2'] and value re'-0$' ))) ")
for cell in cells:
    print(cell)

print("number of cell: "+str(len(cells)))


for schema in schemas_added:
    client.sql_mng_column("delete column(cid=" + str(schema.cid) + " name='" + schema.col_name + "')")

try:
    schemas_added = client.sql_list_columns("get columns py-thrift-test-1, py-thrift-test-2")
except service.Exception as e:
    print(e)


client.close()

try:
    client.sql_list_columns("list columns")
except service.TTransport.TTransportException as e:
    print(e)



dir(service)
dir(service.Client)

```