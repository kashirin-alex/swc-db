
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_thrift_c_client_client_h
#define swc_thrift_c_client_client_h


#include <thrift/c_glib/thrift.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol.h>
#include <thrift/c_glib/protocol/thrift_compact_protocol.h>
#include <thrift/c_glib/protocol/thrift_multiplexed_protocol.h>
#include <thrift/c_glib/transport/thrift_buffered_transport.h>
#include <thrift/c_glib/transport/thrift_framed_transport.h>
//#include <thrift/c_glib/transport/thrift_ssl_socket.h>
#include <thrift/c_glib/transport/thrift_socket.h>
#include <thrift/c_glib/transport/thrift_transport.h>

#include "swcdb/thrift/gen-c_glib/service.h"


struct _swcdb_thrift_client {
  ThriftSocket*     socket;
  ThriftTransport*  transport;
  ThriftProtocol*   protocol;
  ServiceIf*        service;
};
typedef struct _swcdb_thrift_client swcdb_thrift_client;

gboolean 
swcdb_thrift_client_connect(swcdb_thrift_client* client, 
                            const char* host, const gint port,
                            GError** error) {
  client->socket = NULL;
  client->transport = NULL;
  client->protocol = NULL;
  client->service = NULL;

  client->socket = g_object_new(
    THRIFT_TYPE_SOCKET,
    "hostname", g_strdup(host),
    "port",     (gint)port,
    NULL
  );

  client->transport = g_object_new(
    THRIFT_TYPE_FRAMED_TRANSPORT,
    "transport", client->socket,
     NULL
  );

  client->protocol = g_object_new (
    THRIFT_TYPE_BINARY_PROTOCOL,
    "transport", client->transport,
    NULL
  );

  client->service = g_object_new(
    TYPE_SERVICE_CLIENT,
    "input_protocol",  client->protocol,
    "output_protocol", client->protocol,
    NULL
  );
  
  return thrift_transport_open(client->transport, error);
}

void
swcdb_thrift_client_free(swcdb_thrift_client* client) {
  g_clear_object(&client->service);
  g_clear_object(&client->protocol);
  g_clear_object(&client->transport);
  g_clear_object(&client->socket);
  client->socket = NULL;
  client->transport = NULL;
  client->protocol = NULL;
  client->service = NULL;
}

gboolean 
swcdb_thrift_client_disconnect(swcdb_thrift_client* client,  GError** error) {
  gboolean res = thrift_transport_close(client->transport, error);
  swcdb_thrift_client_free(client);
  return res;
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/thrift/gen-c_glib/service.c"
#include "swcdb/thrift/gen-c_glib/service_types.c"
#endif 

#endif // swc_thrift_c_client_client_h