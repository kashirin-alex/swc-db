#!/bin/sh


SWCDB_INSTALL_PATH=$1; shift
SWCDB_SOURCE_PATH=$1; shift
COL_TYPE=$1; shift
COL_BY=$1; shift
COL_EXT=$1; shift
SWCDB_WITH_BROKER=$1; shift

if [ -z $SWCDB_INSTALL_PATH ];then
  SWCDB_INSTALL_PATH="/opt/swcdb";
fi

if [ -z $NUM_CELLS ];then
  NUM_CELLS=1000000;
fi

if [[ $COL_BY == "NAME" ]]; then
	col='test-shell-dbclient-1'
else
	col='12'
fi

if [[ $COL_TYPE == "COUNTER" ]]; then
	gen_value="--gen-value-size=1";
else
	gen_value="--gen-value-size=100";
fi

if [[ $COL_EXT == "zst" ]]; then
	fmt=' ext=zst'
	ext='.zst'
else
	fmt=''
	ext=''
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
  --gen-col-name="test-shell-dbclient-" --gen-col-type=${COL_TYPE}\
  --gen-cells=${NUM_CELLS} ${gen_value} --swc.cfg.path=${SWCDB_SOURCE_PATH}/ ${SWCDB_WITH_BROKER};


## DUMP 1st sample
do_test echo "dump col='${col}' into path='dumps/dbclient/' ${fmt} DISPLAY_STATS; quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/ ${SWCDB_WITH_BROKER};
do_test ls -l ${SWCDB_DATA_PATH}/dumps/dbclient/1.tsv${ext}

## DELETE 1st sample
do_test echo "delete column(name='test-shell-dbclient-1' cid=12); quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/ ${SWCDB_WITH_BROKER};


## LOAD 1st sample
do_test echo "create column(name='test-shell-dbclient-1' type=${COL_TYPE} cid=12); quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/ ${SWCDB_WITH_BROKER};

do_test echo "load from path='dumps/dbclient/' into col='${col}' DISPLAY_STATS; quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/ ${SWCDB_WITH_BROKER};


## DUMP 2nd sample
do_test echo "dump col='${col}' into path='dumps/dbclient_afterload/' ${fmt} DISPLAY_STATS; quit;" | \
				${SWCDB_INSTALL_PATH}/bin/swcdb shell --debug --swc.cfg.path=${SWCDB_SOURCE_PATH}/ ${SWCDB_WITH_BROKER};




## COMPARE 1st with 2nd sample

do_test ls -l ${SWCDB_DATA_PATH}/dumps/dbclient_afterload/1.tsv${ext}
diff -u ${SWCDB_DATA_PATH}/dumps/dbclient/1.tsv${ext} ${SWCDB_DATA_PATH}/dumps/dbclient_afterload/1.tsv${ext};


# STOP & CLEAR
${SWCDB_INSTALL_PATH}/sbin/swcdb_cluster stop --swc.cfg.path=${SWCDB_SOURCE_PATH}/

rm -r /tmp/swcdb;
