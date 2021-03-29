#!/usr/bin/env python3
# -- coding: utf-8 --
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


# the script creates a benchmarks.csv

# benchmarks/checks.py [options multiple-kwarg]
#   --n_cells_relative=100000
#   --n_cells_relative=1000000
#   --n_fractions=1
#   --n_fractions=1000
#   --n_cells_min=1000
#   --value_size=256
#   --install_path=/opt/swcdb
#   --col_seq=LEXIC
#   --col_seq=VOLUME
#   --col_seq=FC_LEXIC
#   --col_seq=FC_VOLUME
#   --col_type=PLAIN
#   --col_type=COUNTER
#   --blk_encoding=ZSTD
#   --blk_encoding=PLAIN
#   --blk_encoding=ZLIB
#   --blk_encoding=SNAPPY
#   --cell_a_time=false
#   --cell_a_time=true
#


import sys
import os
import time
import csv

columns = [
    'Load Type',
    'Column Type',
    'Column Seq',
    'Blk Encoding',
    'Is cell a time',
    'Number of Cells',
    'Number of Fractions',
    'Total Time(sec) Took',
    'Total Cells Count',
    'Total Cells Size',
    'Average Transfer Rate',
    'Average Cells/sec Rate',
    'Mngr Locate',
    'Mngr Resolve',
    'Rgr Locate Master',
    'Rgr Locate Meta',
    'Rgr Data'
]
csv_file = open('benchmarks.csv', 'w')
csv_writer = csv.writer(csv_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
csv_writer.writerow(columns)
csv_file.flush()

field_op = {
    'Total Time Took':      ((lambda x, by: x / float(by)), (lambda x, by: x * float(by))),
    'Average Cells Rate':   ((lambda x, by: x * float(by)), (lambda x, by: x / float(by)))
}

swcdb_cfg = {}


class BenchMark:
    def __init__(self, n_cells, n_fractions, col_seq, col_type, blk_encoding, cell_a_time):
        self.load_type = ""
        self.n_cells = n_cells
        self.n_fractions = n_fractions
        self.col_seq = col_seq
        self.col_type = col_type
        self.blk_encoding = blk_encoding
        self.cell_a_time = cell_a_time

        self.column_name = "benchmark_"
        self.column_name += col_seq + "-"
        self.column_name += col_type + "-"
        self.column_name += blk_encoding + "-"
        self.column_name += n_fractions + "-"
        self.column_name += cell_a_time + "-"
        print ("--- Benchmarking column=" + self.column_name + "1 cells=" + self.n_cells)
        #

    def write_stats_csv(self, output):
        print (output.replace(",", "\n"))

        line = [self.load_type, self.col_type, self.col_seq, self.blk_encoding,
                self.cell_a_time, self.n_cells, self.n_fractions]

        for raw in output.split(','):
            if ':' not in raw:
                continue
            field, value = raw.split(":", 2)
            field = field.strip()
            value = value.strip()

            op = field_op.get(field, None)
            if op:
                under, above = op
                tmp = float(value.split(" ")[0])
                if 'nanosec' in value:
                    tmp = under(tmp, 1000000000)
                elif 'microsec' in value:
                    tmp = under(tmp, 1000000)
                elif 'millisec' in value:
                    tmp = under(tmp, 1000)
                elif 'sec' in value:
                    pass
                elif 'minute' in value:
                    tmp = above(tmp, 60)
                elif 'hour' in value:
                    tmp = above(tmp, 3600)
                value = str(round(tmp, 6))
            line.append(value)
        csv_writer.writerow(line)
        csv_file.flush()
        #

    def load_generator(self, args):
        insert, select, delete, select_empty = args
        cmd = [
            swcdb_cfg['install_path'][0], '/bin/swcdb_load_generator --gen-progress=0',
            ' --gen-select=', str(select),
            ' --gen-insert=', str(insert),
            ' --gen-delete=', str(delete),
            ' --gen-select-empty=', str(select_empty),
            ' --gen-delete-column=', str(select_empty),
            ' --gen-col-type=', self.col_type,
            ' --gen-col-seq=', self.col_seq,
            ' --gen-col-type=', self.col_type,
            ' --gen-col-name=', self.column_name,
            ' --gen-blk-encoding=', self.blk_encoding,
            ' --gen-cells=', self.n_cells,
            ' --gen-cell-a-time=', self.cell_a_time,
            ' --gen-fractions=', self.n_fractions,
            ' --gen-value-size=', swcdb_cfg['value_size'][0],
            ' | grep "Total Time Took:" -A20 | tr "\\n" ",";'
        ]
        print ('exec: ', ''.join(cmd))
        stream = os.popen(''.join(cmd))
        self.write_stats_csv(stream.read())
        #

    def select_data(self, args):
        cmd = [
            'echo "select where col(', self.column_name, '1)=(cells=()) DISPLAY_STATS;quit;"',
            ' | ', swcdb_cfg['install_path'][0], '/bin/swcdb',
            ' | grep \"Total Time Took:\" -A20 | tr "\\n" ",";'
        ]
        print ('exec: ', ''.join(cmd))
        stream = os.popen(''.join(cmd))
        self.write_stats_csv(stream.read())
        #

    def compact_column(self, args):
        cmd = [
            'echo "compact column ', self.column_name, '1;quit;"',
            ' | ', swcdb_cfg['install_path'][0], '/bin/swcdb',
            ' | grep \"Total Time Took:\" -A20 | tr "\\n" ",";'
        ]
        print ('exec: ', ''.join(cmd))
        stream = os.popen(''.join(cmd))
        output = stream.read()
        print (output.replace(",", "\\n"))
        print ("sleeping for ", (int(self.n_cells) / 7200), "sec ")

        time.sleep(int(self.n_cells) / 7200)
        #
#


load_types = [
    (BenchMark.load_generator, 'insert',       (True, False, False, False)),
    (BenchMark.load_generator, 'select',       (False, True, False, False)),
    (BenchMark.load_generator, 'delete',       (False, False, True, False)),
    (BenchMark.load_generator, 'select-empty', (False, True, False, True)),

    # (BenchMark.compact_column,  'compact_columns',              ()),
    # (BenchMark.select_data,     'select_data_after_compaction', ()),
]


def do_checks():
    n_cells_min = int(swcdb_cfg['n_cells_min'][0])

    for nc in swcdb_cfg['n_cells_relative']:
        nc = int(nc)

        for n_fractions in swcdb_cfg['n_fractions']:
            nc = int(nc / int(n_fractions))
            if nc < n_cells_min:
                nc = n_cells_min
            n_cells = str(nc)

            for cell_a_time in swcdb_cfg['cell_a_time']:
                for col_seq in swcdb_cfg['col_seq']:
                    for col_type in swcdb_cfg['col_type']:
                        for blk_encoding in swcdb_cfg['blk_encoding']:

                            run = BenchMark(n_cells, n_fractions, col_seq, col_type, blk_encoding, cell_a_time)
                            for method, load_type, args in load_types:
                                run.load_type = load_type
                                print ("-- EXECUTING-LOAD: " + load_type)
                                method(run, args)
    #


if __name__ == '__main__':
    for i, data in enumerate(sys.argv):
        if not data.startswith("--"):
            continue
        n, v = data.split("=")
        n = n.strip()[2:]
        v = v.strip()
        if n not in swcdb_cfg:
            swcdb_cfg[n] = []
        if v not in swcdb_cfg[n]:
            swcdb_cfg[n].append(v)
    for n, v in [
        ('n_cells_min',         ["1000"]),
        ('n_cells_relative',    ["1000", "100000"]),
        ('value_size',          ["256"]),
        ('install_path',        ["/opt/swcdb"]),
        ('col_seq',             ['LEXIC', 'VOLUME', 'FC_LEXIC', 'FC_VOLUME']),
        ('col_type',            ['PLAIN']),
        ('blk_encoding',        ['ZSTD']),
        ('n_fractions',         ['1', '5', '10', '100', '1000', '10000', '100000']),
        ('cell_a_time',         ['false', 'true']),
    ]:
        if n not in swcdb_cfg:
            swcdb_cfg[n] = v

    print ("Benchmark Writing Output to benchmarks.csv")

    do_checks()

    csv_file.close()
    print ("Benchmark Finished")
    #

