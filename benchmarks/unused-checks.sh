#!/usr/bin/env bash


# the script creates a benchmarks.csv 


if [ -z $SWCDB_INSTALL_PATH ];then 
  SWCDB_INSTALL_PATH="/opt/swcdb";
fi

if [ -z $NUM_CELLS ];then 
  NUM_CELLS=100000;
fi



col_seqs=('LEXIC VOLUME FC_LEXIC FC_VOLUME');
col_types=('PLAIN'); # COUNTER
blk_encodings=('ZSTD'); #  PLAIN SNAPPY ZLIB

num_fractions=('1 5 10 100 1000 10000 100000');

if [ -z $VALUE_SIZE ];then 
  cell_value=('0');
else
  cell_value=('0 ${VALUE_SIZE}');
fi;

check_cellatime=('false true');



generate_data() {

  res=$(${SWCDB_INSTALL_PATH}/bin/swcdb_load_generator -l=warn --gen-progress=0 \
   --gen-col-seq=${col_seq} \
   --gen-col-type=${col_type} \
   --gen-col-name="${column_name}" \
   --gen-blk-encoding=${blk_encoding} \
   --gen-cells=${ncells} \
   --gen-cell-a-time=${cellatime} \
   --gen-fractions=${nfractions} \
   --gen-value-size=${VALUE_SIZE} | \
   grep "Total Time Took:" -A20 | tr "\n" ",");

  echo "${load_type},${cellatime},${nfractions},${res}" >> benchmarks.csv;
}


select_data() {

  res=$(echo "select where col('${column_name}')=(cells=()) DISPLAY_STATS;quit;" | \
    ${SWCDB_INSTALL_PATH}/bin/swcdb | \
   grep "Total Time Took:" -A20 | tr "\n" ",");

  echo "${load_type},${cellatime},${nfractions},${res}" >> benchmarks.csv;
}


compact_columns() {

  echo "compact column ${column_name};quit;" |\
   ${SWCDB_INSTALL_PATH}/bin/swcdb| \
   grep "Compactig Column";

  c=$(($ncells / 7200));
  echo "sleeping for ${c}sec";
  sleep ${c};
}


execute_load() {
  
  case $load_type in
    generate_data)
      generate_data;;

    select_data_*)
      select_data;;

    compact_columns)
      compact_columns;;
  esac
}


load_types=('
  generate_data 
  select_data_before_compaction 
  compact_columns
  select_data_after_compaction
  select_data_with_some_in_rss
');

nc=$(($NUM_CELLS / 1000));

do_checks() {
  for nfractions in $num_fractions; do 

    nf=$(($nfractions / 1000));
    if [[ $nf != 0 && $nc != 0 ]]; then
      ncells=$(($nc * nf));
    else
      ncells=$NUM_CELLS;
    fi;

    for cellatime in $check_cellatime; do 
      for col_seq in $col_seqs; do 
        for col_type in $col_types; do 
          for blk_encoding in $blk_encodings; do 
          
            column_name="benchmark_${col_seq}-${col_type}-${blk_encoding}-${nfractions}-${cellatime}";
            echo "--- Benchmarking column=${column_name} cells=${ncells}"; 
          
            for load_type in $load_types; do 
              echo "-- EXECUTING-LOAD: ${load_type}";
              execute_load;
            done;

          done;
        done;
      done;
    done;
  done;
}




# START BENCHMARK
echo "Benchmark Wrting Output to benchmarks benchmarks.out"

echo "'Load Type',\
'Is cell a time',\
'Number of Fractions',\
'Total Time Took',\
'Total Cells Count',\
'Total Cells Size',\
'Average Transfer Rate',\
'Average Cells Rate',\
'Mngr Locate',\
'Mngr Resolve',\
'Rgr Locate Master',\
'Rgr Locate Meta',\
'Rgr Data'"  >  benchmarks.csv;

do_checks &> benchmarks.out;

echo "Benchmark Finished"




