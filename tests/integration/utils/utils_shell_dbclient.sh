#!/bin/sh


SWCDB_INSTALL_PATH=$1; shift
SWCDB_SOURCE_PATH=$1; shift

if [ -z $SWCDB_INSTALL_PATH ];then 
  SWCDB_INSTALL_PATH="/opt/swcdb";
fi

if [ -z $NUM_CELLS ];then 
  NUM_CELLS=1000000;
fi


# START with /tmp/swcdb
mkdir -p /tmp/swcdb;
${SWCDB_INSTALL_PATH}/sbin/swcdb_cluster start --swc.cfg.path=${SWCDB_SOURCE_PATH}/


SWCDB_DATA_PATH="/tmp/swcdb/utils_test";


do_test() {
	if $@
	then
		echo "OK: " $@;
	else 
		exit 1;
	fi
}

## CREATE TEST DATA
${SWCDB_INSTALL_PATH}/bin/swcdb_load_generator -l=warn \
  --gen-col-name="test-shell-dbclient" \
  --gen-cells=${NUM_CELLS} --swc.cfg.path=${SWCDB_SOURCE_PATH}/;


## DUMP 1st sample
do_test echo "dump col='test-shell-dbclient' into 'dumps/dbclient/' DISPLAY_STATS; quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/;


## LOAD 1st sample
do_test echo "create column(name='test-shell-dbclient_load'); quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/;

do_test echo "load from 'dumps/dbclient/' into col='test-shell-dbclient_load' DISPLAY_STATS; quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/;


## DUMP 2nd sample
do_test echo "dump col='test-shell-dbclient_load' into 'dumps/dbclient_afterload/' DISPLAY_STATS; quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/;


## COMPARE 1st with 2nd sample
diff -u ${SWCDB_DATA_PATH}/dumps/dbclient/1.tsv ${SWCDB_DATA_PATH}/dumps/dbclient_afterload/1.tsv;


# STOP & CLEAR
${SWCDB_INSTALL_PATH}/sbin/swcdb_cluster stop --swc.cfg.path=${SWCDB_SOURCE_PATH}/

rm -r /tmp/swcdb;
