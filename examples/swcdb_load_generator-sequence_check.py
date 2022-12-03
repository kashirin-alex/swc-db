###
#  SWC-DB "swcdb_load_generator" sequence check:
#   * select all CID cells by LIMIT and MAX_VERSIONS
#   * check cells match to expected load-generator key sequence
#   Anyhow Modifying and Redistributing this file is allowed!
###


from swcdb.thrift import service
from swcdb.thrift.pool import PoolService
import time


# Configurables:
limit = 1000
max_versions = 1 # number of versions to expect
column_cid = 12

## For Insistent PoolService "stop" see:
# https://github.com/kashirin-alex/swc-db/blob/e847ffd9fc0587c4b3c77e702078b3858944d263/examples/swcdb_regression_test-columns.py#L55
# https://github.com/kashirin-alex/swc-db/blob/e847ffd9fc0587c4b3c77e702078b3858944d263/examples/swcdb_regression_test-columns.py#L37


# runtime-state:
kcount = 1
fractions = 0
versions = 0

offset_key = None # ['0000000000'] continue from key
offset_rev = None # max-i64 is 1st at ts-descending

client = PoolService(1)

while True:
    specs = service.SpecScan(
        columns_plan=[
            service.SpecColumnPlain(
                cid=column_cid,
                intervals=[
                    service.SpecIntervalPlain(
                        offset_key=offset_key,
                        offset_rev=offset_rev,
                        flags=service.SpecFlags(offset=0, limit=limit)
                    )
                ]
            )
        ],
        flags=None
    )
    print(specs)

    ts = time.time()
    cells=client.scan(specs).plain_cells
    took = time.time() - ts
    if len(cells) == 0:
        break

    offset_rev = cells[-1].ts
    offset_key = cells[-1].k

    for cell in cells:

        if int(cell.k[0]) != kcount:
            print("bad: ", kcount, ":", cell)
            exit(1)

        versions += 1
        if versions == max_versions:
            fractions += 1
            versions = 0
            if fractions == 10:
                fractions = 0
                kcount += 1

    print("progress: ", kcount, took)

client.close()
