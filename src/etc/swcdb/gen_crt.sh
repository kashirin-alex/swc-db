#!/usr/bin/env bash

# self-signed PEM 
DOMAIN_NAME="swcdb-cluster-1"; 
CONFIG_PATH=/opt/swcdb/etc/swcdb; 
CONFIG_FILE=/opt/swcdb/etc/swcdb/openssl.cnf; 

# CA -- keep rootCA.key safe or delete it.
openssl req -x509 -new \
  -nodes -newkey rsa:4096 -keyout rootCA.key \
  -sha256 -days 3650 -subj "/CN=${DOMAIN_NAME}-CA" \
  -reqexts v3_req -extensions v3_ca \
  -out rootCA.crt -config ${CONFIG_FILE};

#CRT-REQ
openssl req -new -sha256 -newkey rsa:4096 -nodes \
    -keyout server.key \
    -subj "/CN=${DOMAIN_NAME}" \
    -reqexts v3_req \
    -config <(cat ${CONFIG_FILE} <(printf "\n[v3_req]\nsubjectAltName=DNS:${DOMAIN_NAME}")) \
    -out server.csr;

#CRT-SIGN
openssl x509 -req \
    -extfile <(printf "\n[v3_req]\nsubjectAltName=DNS:${DOMAIN_NAME}") \
    -days 3650 -in server.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial \
    -out cluster.crt -sha256;

# PUT IN CONFIG PATH
mv cluster.crt ${CONFIG_PATH}/;
mv server.key ${CONFIG_PATH}/;
cat rootCA.crt > ${CONFIG_PATH}/ca.pem;

rm rootCA.crt rootCA.key server.csr rootCA.srl;

openssl verify -CAfile ${CONFIG_PATH}/ca.pem ${CONFIG_PATH}/cluster.crt