/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/thrift/client/client.h"
#include <stdio.h>
#include <glib/gprintf.h>

#include <inttypes.h>
#define SWC_STRINGIFY(s) #s
#define SWC_FMT_LU SWC_STRINGIFY(%) PRIu64
#define SWC_FMT_LD SWC_STRINGIFY(%) PRId64


int main() {
  puts ("Started Testing\n");

  swcdb_thrift_client client;
  GError*     error = NULL;

  if(!swcdb_thrift_client_connect(&client, "localhost", 18000, &error)) {
    if(error != NULL) {
      printf("Connect failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
    } else {
      puts("Connect failed: Unknown Error\n");
    }
    return 1;
  }


  swcdb_thriftSchemas* schemas = g_object_new (SWCDB_THRIFT_TYPE_SERVICE_SQL_LIST_COLUMNS_RESULT, NULL);
  const gchar*              sql = "list columns";
  swcdb_thriftException*    exception = NULL;

  if(swcdb_thrift_service_client_sql_list_columns(
     client.service, &schemas, sql, &exception, &error)) {
    printf("schemas count=%d \n",  schemas->len);

    swcdb_thriftSchema*  schema;
    for(guint i = 0; i < schemas->len; ++i) {
      schema = g_ptr_array_index(schemas, i);
      if(!schema->__isset_cid || schema->cid == 0) // unexplained (cid=0)
        continue;
      puts    ("schema: ");
      printf  ("  cid=" SWC_FMT_LU " \n",  (guint64)schema->cid);
      g_printf("  name=%s \n",  schema->col_name);
      g_clear_object(&schema);
    }
    g_clear_object(&schemas);
  }

  if(error != NULL) {
    printf("service_handler_sql_list_columns GError: %s\n", error->message);
    g_error_free (error);
    error = NULL;
  }

  if(exception != NULL) {
    printf("service_handler_sql_list_columns Exception: code=%d %s\n",
            error->code, error->message);
    g_free(exception);
    exception = NULL;
  }



  if(!swcdb_thrift_client_disconnect(&client, &error)) {
    if(error != NULL) {
      printf("Disconnect failed: %s\n", error->message);
      g_error_free (error);
      error = NULL;
    } else {
      puts("Disconnect failed: failed: Unknown Error\n");
    }
    return 1;
  }

  puts ("\nFinished Testing");
  return 0;
}