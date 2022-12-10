###
#  SWC-DB regression test example (benchmark):
#   * proceed with rounds of insert + select + delete for each column of aggregated cells-step until cells-max reach
#   * insert & delete can be defined in batch-size (cells_batch_size = 1 - equal cell-a-time, = 0 equal single-batch)
#   * logging statistics in CSV: "round #", "number of cells", "CID", "insert time", "select time", "delete time"
#   Anyhow Modifying and Redistributing the file is allowed!
###

from swcdb.thrift import service
from swcdb.thrift.pool import PoolService
import sys
import time
import csv
import signal
#


def avg_ns(took, n_cells):
    return int(((took / n_cells) if n_cells else took) * 1000000000)
    #


class Test(object):

    col_name_prefix = 'py-test-'
    colm_batch = 30
    colm_start = 1
    columns = 100000
    rounds = 3
    cells_max = 10
    cells_step = 10
    cells_batch_size = 1  # 0 == single-batch

    def __init__(self):
        self.client = PoolService(1)

        self.orig_sig = [(s, signal.getsignal(s)) for s in [signal.SIGTERM, signal.SIGINT]]
        for s in self.orig_sig:
            signal.signal(s[0], self.signal_term_handler)

        self.csv_file = open('benchmarks.csv', 'w')
        self.csv_writer = csv.writer(self.csv_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        self.csv_writer.writerow([
            "round #",
            "CID",
            "create time",
            "insert time",
            "select time",
            "delete time",
            "number of cells",
        ])
        self.csv_file.flush()
        #

    def signal_term_handler(self, *args):
        print(args)
        for h in self.orig_sig:
            signal.signal(*h)

        self.client.stop()
        #

    def write_stats(self, r, cid, create_time, insert_time, select_time, delete_time, ncells):
        self.csv_writer.writerow([r, cid, create_time, insert_time, select_time, delete_time, ncells])
        self.csv_file.flush()
        #

    def get_batch_size(self, sz):
        return self.cells_batch_size if self.cells_batch_size else sz
        #

    def run(self):
        # ensure clear
        self.remove_columns()
        # return

        for r in range(1, self.rounds + 1):
            print("# round:", r)
            for c in range(self.colm_start, self.columns + self.colm_batch, self.colm_batch):
                self.run_column(r, c)
                if c == self.columns:
                    break
            self.colm_start = 1
            self.remove_columns()

        self.csv_file.close()
        self.client.close()
        #

    def run_column(self, r, c):

        cols_create_time = {}
        names = []
        for n_col in range(1, self.colm_batch + 1):
            tmp_ts = time.time()
            schema = service.Schema(col_name=self.col_name_prefix + str(c), col_seq=service.KeySeq.LEXIC)
            names.append(schema.col_name)
            self.client.mng_column(service.SchemaFunc.CREATE, schema)
            cols_create_time[schema.col_name] = time.time() - tmp_ts
            c += 1
            if c == self.columns:
                break

        schemas = self.client.list_columns(service.SpecSchemas(names=names))
        assert (len(schemas) == len(names))

        for schema in schemas:

            insert_time = 0
            select_time = 0
            delete_time = 0
            n_cells = 0
            for numcells in range(self.cells_step, self.cells_max + self.cells_step, self.cells_step):
                n_cells += self.cells_step
                ts_start = time.time()
                self.insert_cells(schema, numcells)
                insert_time += time.time() - ts_start

                ts_start = time.time()
                #cells = self.select_cells(schema, n_cells)
                cells = self.select_cells_cell_a_time(schema, numcells)
                select_time += time.time() - ts_start

                ts_start = time.time()
                self.delete_cells(schema, cells)
                delete_time += time.time() - ts_start

                self.select_cells(schema, 0)  # expect-empty

            create_time = cols_create_time[schema.col_name]
            self.write_stats(r, schema.cid, create_time, insert_time, select_time, delete_time, n_cells)
            ts_took = create_time + insert_time + select_time + delete_time
            print(
                "round:", r, "cells:", n_cells, "cid:", schema.cid,
                "took:", round(ts_took, 3), "avg:", avg_ns(ts_took, n_cells),
                "create-took:", round(create_time, 3),
                "insert-took:", round(insert_time, 3), "insert-avg:", avg_ns(insert_time, n_cells),
                "select-took:", round(select_time, 3), "select-avg:", avg_ns(select_time, n_cells),
                "delete-took:", round(delete_time, 3), "delete-avg:", avg_ns(delete_time, n_cells),
            )
            sys.stdout.flush()
        #

    def insert_cells(self, schema, n_cells):
        # print("# insert_cells", "cid:", schema.cid, "cells:", n_cells)
        batch_sz = self.get_batch_size(n_cells)
        for batch in range(0, n_cells, batch_sz):
            self.client.update_plain({
                schema.cid: [
                    service.UCellPlain(
                        f=service.Flag.INSERT,
                        k=[bytearray(str(n), 'utf8')],
                        ts=None, ts_desc=None,
                        v=bytearray("F(1)==" + str(n), 'utf8'),
                        encoder=None)
                    for n in range(batch, batch + batch_sz, 1)
                    ]},
                0
            )
        #

    def select_cells(self, schema, n_cells):
        # print("# select_cells", "cid:", schema.cid, "cells:", n_cells)
        specs = service.SpecScan(
            columns_plain=[
                service.SpecColumnPlain(
                    cid=schema.cid,
                    intervals=[service.SpecIntervalPlain()]
                )
            ],
            flags=None
        )
        container = self.client.scan(specs)
        assert (len(container.plain_cells) == n_cells)
        return container.plain_cells
        #

    def select_cells_cell_a_time(self, schema, n_cells):
        # print("# select_cells", "cid:", schema.cid, "cells:", n_cells)
        results = []
        for n in range(0, n_cells, 1):
            specs = service.SpecScan(
                columns_plain=[
                    service.SpecColumnPlain(
                        cid=schema.cid,
                        intervals=[
                            service.SpecIntervalPlain(
                                key_intervals=[
                                    service.SpecKeyInterval(
                                        start=[
                                            service.SpecFraction(
                                                f=bytearray(str(n), 'utf8'),
                                                comp=service.Comp.EQ
                                            )
                                        ]
                                    )
                                ],
                                flags=service.SpecFlags(limit=1)
                            )
                        ]
                    )
                ],
                flags=None
            )
            container = self.client.scan(specs)
            results += container.plain_cells
        assert (len(results) == n_cells)
        return results
        #

    def delete_cells(self, schema, cells):
        # print("# delete_cells ", "cid:", schema.cid, "cells:", len(cells))
        n_cells = len(cells)
        batch_sz = self.get_batch_size(n_cells)
        for batch in range(0, n_cells, batch_sz):
            self.client.update_plain({
                schema.cid: [
                    service.UCellPlain(
                        f=service.Flag.DELETE_LE,
                        k=cells[n].k,
                        ts=None, ts_desc=None, v=None, encoder=None)
                    for n in range(batch, batch + batch_sz, 1)
                    ]},
                0
            )
        #
    #

    def remove_columns(self):
        print("# remove_columns #")
        spec = service.SpecSchemas(
            patterns=service.SchemaPatterns(
                names=[service.SchemaPattern(
                    comp=service.Comp.VGE,
                    value=self.col_name_prefix + str(self.colm_start)
                )]
            ))
        schemas = self.client.list_columns(spec)
        for schema in schemas:
            ts_start = time.time()
            self.client.mng_column(service.SchemaFunc.REMOVE, schema)
            print("remove cid:", schema.cid, "took:", time.time() - ts_start)
            sys.stdout.flush()
        # check Removed
        schemas = self.client.list_columns(spec)
        assert (len(schemas) == 0)
        #
#


#
if __name__ == "__main__":
    test = Test()
    test.run()
    exit(0)
