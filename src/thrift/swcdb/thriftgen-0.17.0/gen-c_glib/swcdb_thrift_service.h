/**
 * Autogenerated by Thrift Compiler (0.17.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef SWCDB_THRIFT_SERVICE_H
#define SWCDB_THRIFT_SERVICE_H

#include <thrift/c_glib/processor/thrift_dispatch_processor.h>

#include "swcdb_thrift_service_types.h"

/* Service service interface */
typedef struct _swcdb_thriftServiceIf swcdb_thriftServiceIf;  /* dummy object */

struct _swcdb_thriftServiceIfInterface
{
  GTypeInterface parent;

  gboolean (*sql_mng_column) (swcdb_thriftServiceIf *iface, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_list_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_compact_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_plain) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsPlain ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_counter) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsCounter ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_serial) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsSerial ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select) (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_rslt_on_column) (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_rslt_on_key) (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_rslt_on_fraction) (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_query) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const gchar * sql, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_update) (swcdb_thriftServiceIf *iface, const gchar * sql, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*exec_sql) (swcdb_thriftServiceIf *iface, swcdb_thriftResult ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*updater_create) (swcdb_thriftServiceIf *iface, gint64* _return, const gint32 buffer_size, swcdb_thriftException ** e, GError **error);
  gboolean (*updater_close) (swcdb_thriftServiceIf *iface, const gint64 id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_plain) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_counter) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsCounter * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_serial) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsSerial * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_by_types) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * plain, const swcdb_thriftUCCellsCounter * counter, const swcdb_thriftUCCellsSerial * serial, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*mng_column) (swcdb_thriftServiceIf *iface, const swcdb_thriftSchemaFunc func, const swcdb_thriftSchema * schema, swcdb_thriftException ** e, GError **error);
  gboolean (*list_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*compact_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan) (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on_column) (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on_key) (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on_fraction) (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const swcdb_thriftSpecScan * spec, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);
};
typedef struct _swcdb_thriftServiceIfInterface swcdb_thriftServiceIfInterface;

GType swcdb_thrift_service_if_get_type (void);
#define SWCDB_THRIFT_TYPE_SERVICE_IF (swcdb_thrift_service_if_get_type())
#define SWCDB_THRIFT_SERVICE_IF(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SWCDB_THRIFT_TYPE_SERVICE_IF, swcdb_thriftServiceIf))
#define SWCDB_THRIFT_IS_SERVICE_IF(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SWCDB_THRIFT_TYPE_SERVICE_IF))
#define SWCDB_THRIFT_SERVICE_IF_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), SWCDB_THRIFT_TYPE_SERVICE_IF, swcdb_thriftServiceIfInterface))

gboolean swcdb_thrift_service_if_sql_mng_column (swcdb_thriftServiceIf *iface, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_list_columns (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_compact_columns (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_select_plain (swcdb_thriftServiceIf *iface, swcdb_thriftCellsPlain ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_select_counter (swcdb_thriftServiceIf *iface, swcdb_thriftCellsCounter ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_select_serial (swcdb_thriftServiceIf *iface, swcdb_thriftCellsSerial ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_select (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_select_rslt_on_column (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_select_rslt_on_key (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_select_rslt_on_fraction (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_query (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const gchar * sql, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_sql_update (swcdb_thriftServiceIf *iface, const gchar * sql, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_exec_sql (swcdb_thriftServiceIf *iface, swcdb_thriftResult ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_updater_create (swcdb_thriftServiceIf *iface, gint64* _return, const gint32 buffer_size, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_updater_close (swcdb_thriftServiceIf *iface, const gint64 id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_update_plain (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_update_counter (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsCounter * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_update_serial (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsSerial * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_update_by_types (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * plain, const swcdb_thriftUCCellsCounter * counter, const swcdb_thriftUCCellsSerial * serial, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_mng_column (swcdb_thriftServiceIf *iface, const swcdb_thriftSchemaFunc func, const swcdb_thriftSchema * schema, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_list_columns (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_compact_columns (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_scan (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_scan_rslt_on_column (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_scan_rslt_on_key (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_scan_rslt_on_fraction (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_if_scan_rslt_on (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const swcdb_thriftSpecScan * spec, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);

/* Service service client */
struct _swcdb_thriftServiceClient
{
  GObject parent;

  ThriftProtocol *input_protocol;
  ThriftProtocol *output_protocol;
};
typedef struct _swcdb_thriftServiceClient swcdb_thriftServiceClient;

struct _swcdb_thriftServiceClientClass
{
  GObjectClass parent;
};
typedef struct _swcdb_thriftServiceClientClass swcdb_thriftServiceClientClass;

GType swcdb_thrift_service_client_get_type (void);
#define SWCDB_THRIFT_TYPE_SERVICE_CLIENT (swcdb_thrift_service_client_get_type())
#define SWCDB_THRIFT_SERVICE_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SWCDB_THRIFT_TYPE_SERVICE_CLIENT, swcdb_thriftServiceClient))
#define SWCDB_THRIFT_SERVICE_CLIENT_CLASS(c) (G_TYPE_CHECK_CLASS_CAST ((c), SWCDB_THRIFT_TYPE_SERVICE_CLIENT, swcdb_thriftServiceClientClass))
#define SWCDB_THRIFT_SERVICE_IS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SWCDB_THRIFT_TYPE_SERVICE_CLIENT))
#define SWCDB_THRIFT_SERVICE_IS_CLIENT_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE ((c), SWCDB_THRIFT_TYPE_SERVICE_CLIENT))
#define SWCDB_THRIFT_SERVICE_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SWCDB_THRIFT_TYPE_SERVICE_CLIENT, swcdb_thriftServiceClientClass))

gboolean swcdb_thrift_service_client_sql_mng_column (swcdb_thriftServiceIf * iface, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_mng_column (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_mng_column (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_list_columns (swcdb_thriftServiceIf * iface, swcdb_thriftSchemas ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_list_columns (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_list_columns (swcdb_thriftServiceIf * iface, swcdb_thriftSchemas ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_compact_columns (swcdb_thriftServiceIf * iface, swcdb_thriftCompactResults ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_compact_columns (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_compact_columns (swcdb_thriftServiceIf * iface, swcdb_thriftCompactResults ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_select_plain (swcdb_thriftServiceIf * iface, swcdb_thriftCellsPlain ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_select_plain (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_select_plain (swcdb_thriftServiceIf * iface, swcdb_thriftCellsPlain ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_select_counter (swcdb_thriftServiceIf * iface, swcdb_thriftCellsCounter ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_select_counter (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_select_counter (swcdb_thriftServiceIf * iface, swcdb_thriftCellsCounter ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_select_serial (swcdb_thriftServiceIf * iface, swcdb_thriftCellsSerial ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_select_serial (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_select_serial (swcdb_thriftServiceIf * iface, swcdb_thriftCellsSerial ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_select (swcdb_thriftServiceIf * iface, swcdb_thriftCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_select (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_select (swcdb_thriftServiceIf * iface, swcdb_thriftCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_select_rslt_on_column (swcdb_thriftServiceIf * iface, swcdb_thriftCCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_select_rslt_on_column (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_select_rslt_on_column (swcdb_thriftServiceIf * iface, swcdb_thriftCCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_select_rslt_on_key (swcdb_thriftServiceIf * iface, swcdb_thriftKCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_select_rslt_on_key (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_select_rslt_on_key (swcdb_thriftServiceIf * iface, swcdb_thriftKCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_select_rslt_on_fraction (swcdb_thriftServiceIf * iface, swcdb_thriftFCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_select_rslt_on_fraction (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_select_rslt_on_fraction (swcdb_thriftServiceIf * iface, swcdb_thriftFCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_query (swcdb_thriftServiceIf * iface, swcdb_thriftCellsGroup ** _return, const gchar * sql, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_query (swcdb_thriftServiceIf * iface, const gchar * sql, const swcdb_thriftCellsResult rslt, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_query (swcdb_thriftServiceIf * iface, swcdb_thriftCellsGroup ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_sql_update (swcdb_thriftServiceIf * iface, const gchar * sql, const gint64 updater_id, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_sql_update (swcdb_thriftServiceIf * iface, const gchar * sql, const gint64 updater_id, GError ** error);
gboolean swcdb_thrift_service_client_recv_sql_update (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_exec_sql (swcdb_thriftServiceIf * iface, swcdb_thriftResult ** _return, const gchar * sql, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_exec_sql (swcdb_thriftServiceIf * iface, const gchar * sql, GError ** error);
gboolean swcdb_thrift_service_client_recv_exec_sql (swcdb_thriftServiceIf * iface, swcdb_thriftResult ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_updater_create (swcdb_thriftServiceIf * iface, gint64* _return, const gint32 buffer_size, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_updater_create (swcdb_thriftServiceIf * iface, const gint32 buffer_size, GError ** error);
gboolean swcdb_thrift_service_client_recv_updater_create (swcdb_thriftServiceIf * iface, gint64* _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_updater_close (swcdb_thriftServiceIf * iface, const gint64 id, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_updater_close (swcdb_thriftServiceIf * iface, const gint64 id, GError ** error);
gboolean swcdb_thrift_service_client_recv_updater_close (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_update_plain (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsPlain * cells, const gint64 updater_id, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_update_plain (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsPlain * cells, const gint64 updater_id, GError ** error);
gboolean swcdb_thrift_service_client_recv_update_plain (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_update_counter (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsCounter * cells, const gint64 updater_id, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_update_counter (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsCounter * cells, const gint64 updater_id, GError ** error);
gboolean swcdb_thrift_service_client_recv_update_counter (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_update_serial (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsSerial * cells, const gint64 updater_id, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_update_serial (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsSerial * cells, const gint64 updater_id, GError ** error);
gboolean swcdb_thrift_service_client_recv_update_serial (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_update_by_types (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsPlain * plain, const swcdb_thriftUCCellsCounter * counter, const swcdb_thriftUCCellsSerial * serial, const gint64 updater_id, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_update_by_types (swcdb_thriftServiceIf * iface, const swcdb_thriftUCCellsPlain * plain, const swcdb_thriftUCCellsCounter * counter, const swcdb_thriftUCCellsSerial * serial, const gint64 updater_id, GError ** error);
gboolean swcdb_thrift_service_client_recv_update_by_types (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_mng_column (swcdb_thriftServiceIf * iface, const swcdb_thriftSchemaFunc func, const swcdb_thriftSchema * schema, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_mng_column (swcdb_thriftServiceIf * iface, const swcdb_thriftSchemaFunc func, const swcdb_thriftSchema * schema, GError ** error);
gboolean swcdb_thrift_service_client_recv_mng_column (swcdb_thriftServiceIf * iface, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_list_columns (swcdb_thriftServiceIf * iface, swcdb_thriftSchemas ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_list_columns (swcdb_thriftServiceIf * iface, const swcdb_thriftSpecSchemas * spec, GError ** error);
gboolean swcdb_thrift_service_client_recv_list_columns (swcdb_thriftServiceIf * iface, swcdb_thriftSchemas ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_compact_columns (swcdb_thriftServiceIf * iface, swcdb_thriftCompactResults ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_compact_columns (swcdb_thriftServiceIf * iface, const swcdb_thriftSpecSchemas * spec, GError ** error);
gboolean swcdb_thrift_service_client_recv_compact_columns (swcdb_thriftServiceIf * iface, swcdb_thriftCompactResults ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_scan (swcdb_thriftServiceIf * iface, swcdb_thriftCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_scan (swcdb_thriftServiceIf * iface, const swcdb_thriftSpecScan * spec, GError ** error);
gboolean swcdb_thrift_service_client_recv_scan (swcdb_thriftServiceIf * iface, swcdb_thriftCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_scan_rslt_on_column (swcdb_thriftServiceIf * iface, swcdb_thriftCCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_scan_rslt_on_column (swcdb_thriftServiceIf * iface, const swcdb_thriftSpecScan * spec, GError ** error);
gboolean swcdb_thrift_service_client_recv_scan_rslt_on_column (swcdb_thriftServiceIf * iface, swcdb_thriftCCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_scan_rslt_on_key (swcdb_thriftServiceIf * iface, swcdb_thriftKCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_scan_rslt_on_key (swcdb_thriftServiceIf * iface, const swcdb_thriftSpecScan * spec, GError ** error);
gboolean swcdb_thrift_service_client_recv_scan_rslt_on_key (swcdb_thriftServiceIf * iface, swcdb_thriftKCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_scan_rslt_on_fraction (swcdb_thriftServiceIf * iface, swcdb_thriftFCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_scan_rslt_on_fraction (swcdb_thriftServiceIf * iface, const swcdb_thriftSpecScan * spec, GError ** error);
gboolean swcdb_thrift_service_client_recv_scan_rslt_on_fraction (swcdb_thriftServiceIf * iface, swcdb_thriftFCells ** _return, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_scan_rslt_on (swcdb_thriftServiceIf * iface, swcdb_thriftCellsGroup ** _return, const swcdb_thriftSpecScan * spec, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError ** error);
gboolean swcdb_thrift_service_client_send_scan_rslt_on (swcdb_thriftServiceIf * iface, const swcdb_thriftSpecScan * spec, const swcdb_thriftCellsResult rslt, GError ** error);
gboolean swcdb_thrift_service_client_recv_scan_rslt_on (swcdb_thriftServiceIf * iface, swcdb_thriftCellsGroup ** _return, swcdb_thriftException ** e, GError ** error);
void service_client_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
void service_client_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

/* Service handler (abstract base class) */
struct _swcdb_thriftServiceHandler
{
  GObject parent;
};
typedef struct _swcdb_thriftServiceHandler swcdb_thriftServiceHandler;

struct _swcdb_thriftServiceHandlerClass
{
  GObjectClass parent;

  gboolean (*sql_mng_column) (swcdb_thriftServiceIf *iface, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_list_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_compact_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_plain) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsPlain ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_counter) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsCounter ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_serial) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsSerial ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select) (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_rslt_on_column) (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_rslt_on_key) (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_select_rslt_on_fraction) (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_query) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const gchar * sql, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);
  gboolean (*sql_update) (swcdb_thriftServiceIf *iface, const gchar * sql, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*exec_sql) (swcdb_thriftServiceIf *iface, swcdb_thriftResult ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
  gboolean (*updater_create) (swcdb_thriftServiceIf *iface, gint64* _return, const gint32 buffer_size, swcdb_thriftException ** e, GError **error);
  gboolean (*updater_close) (swcdb_thriftServiceIf *iface, const gint64 id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_plain) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_counter) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsCounter * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_serial) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsSerial * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*update_by_types) (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * plain, const swcdb_thriftUCCellsCounter * counter, const swcdb_thriftUCCellsSerial * serial, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
  gboolean (*mng_column) (swcdb_thriftServiceIf *iface, const swcdb_thriftSchemaFunc func, const swcdb_thriftSchema * schema, swcdb_thriftException ** e, GError **error);
  gboolean (*list_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*compact_columns) (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan) (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on_column) (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on_key) (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on_fraction) (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
  gboolean (*scan_rslt_on) (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const swcdb_thriftSpecScan * spec, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);
};
typedef struct _swcdb_thriftServiceHandlerClass swcdb_thriftServiceHandlerClass;

GType swcdb_thrift_service_handler_get_type (void);
#define SWCDB_THRIFT_TYPE_SERVICE_HANDLER (swcdb_thrift_service_handler_get_type())
#define SWCDB_THRIFT_SERVICE_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SWCDB_THRIFT_TYPE_SERVICE_HANDLER, swcdb_thriftServiceHandler))
#define SWCDB_THRIFT_IS_SERVICE_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SWCDB_THRIFT_TYPE_SERVICE_HANDLER))
#define SWCDB_THRIFT_SERVICE_HANDLER_CLASS(c) (G_TYPE_CHECK_CLASS_CAST ((c), SWCDB_THRIFT_TYPE_SERVICE_HANDLER, swcdb_thriftServiceHandlerClass))
#define SWCDB_THRIFT_IS_SERVICE_HANDLER_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE ((c), SWCDB_THRIFT_TYPE_SERVICE_HANDLER))
#define SWCDB_THRIFT_SERVICE_HANDLER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SWCDB_THRIFT_TYPE_SERVICE_HANDLER, swcdb_thriftServiceHandlerClass))

gboolean swcdb_thrift_service_handler_sql_mng_column (swcdb_thriftServiceIf *iface, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_list_columns (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_compact_columns (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_select_plain (swcdb_thriftServiceIf *iface, swcdb_thriftCellsPlain ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_select_counter (swcdb_thriftServiceIf *iface, swcdb_thriftCellsCounter ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_select_serial (swcdb_thriftServiceIf *iface, swcdb_thriftCellsSerial ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_select (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_select_rslt_on_column (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_select_rslt_on_key (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_select_rslt_on_fraction (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_query (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const gchar * sql, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_sql_update (swcdb_thriftServiceIf *iface, const gchar * sql, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_exec_sql (swcdb_thriftServiceIf *iface, swcdb_thriftResult ** _return, const gchar * sql, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_updater_create (swcdb_thriftServiceIf *iface, gint64* _return, const gint32 buffer_size, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_updater_close (swcdb_thriftServiceIf *iface, const gint64 id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_update_plain (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_update_counter (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsCounter * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_update_serial (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsSerial * cells, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_update_by_types (swcdb_thriftServiceIf *iface, const swcdb_thriftUCCellsPlain * plain, const swcdb_thriftUCCellsCounter * counter, const swcdb_thriftUCCellsSerial * serial, const gint64 updater_id, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_mng_column (swcdb_thriftServiceIf *iface, const swcdb_thriftSchemaFunc func, const swcdb_thriftSchema * schema, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_list_columns (swcdb_thriftServiceIf *iface, swcdb_thriftSchemas ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_compact_columns (swcdb_thriftServiceIf *iface, swcdb_thriftCompactResults ** _return, const swcdb_thriftSpecSchemas * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_scan (swcdb_thriftServiceIf *iface, swcdb_thriftCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_scan_rslt_on_column (swcdb_thriftServiceIf *iface, swcdb_thriftCCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_scan_rslt_on_key (swcdb_thriftServiceIf *iface, swcdb_thriftKCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_scan_rslt_on_fraction (swcdb_thriftServiceIf *iface, swcdb_thriftFCells ** _return, const swcdb_thriftSpecScan * spec, swcdb_thriftException ** e, GError **error);
gboolean swcdb_thrift_service_handler_scan_rslt_on (swcdb_thriftServiceIf *iface, swcdb_thriftCellsGroup ** _return, const swcdb_thriftSpecScan * spec, const swcdb_thriftCellsResult rslt, swcdb_thriftException ** e, GError **error);

/* Service processor */
struct _swcdb_thriftServiceProcessor
{
  ThriftDispatchProcessor parent;

  /* protected */
  swcdb_thriftServiceHandler *handler;
  GHashTable *process_map;
};
typedef struct _swcdb_thriftServiceProcessor swcdb_thriftServiceProcessor;

struct _swcdb_thriftServiceProcessorClass
{
  ThriftDispatchProcessorClass parent;

  /* protected */
  gboolean (*dispatch_call) (ThriftDispatchProcessor *processor,
                             ThriftProtocol *in,
                             ThriftProtocol *out,
                             gchar *fname,
                             gint32 seqid,
                             GError **error);
};
typedef struct _swcdb_thriftServiceProcessorClass swcdb_thriftServiceProcessorClass;

GType swcdb_thrift_service_processor_get_type (void);
#define SWCDB_THRIFT_TYPE_SERVICE_PROCESSOR (swcdb_thrift_service_processor_get_type())
#define SWCDB_THRIFT_SERVICE_PROCESSOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SWCDB_THRIFT_TYPE_SERVICE_PROCESSOR, swcdb_thriftServiceProcessor))
#define SWCDB_THRIFT_IS_SERVICE_PROCESSOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SWCDB_THRIFT_TYPE_SERVICE_PROCESSOR))
#define SWCDB_THRIFT_SERVICE_PROCESSOR_CLASS(c) (G_TYPE_CHECK_CLASS_CAST ((c), SWCDB_THRIFT_TYPE_SERVICE_PROCESSOR, swcdb_thriftServiceProcessorClass))
#define SWCDB_THRIFT_IS_SERVICE_PROCESSOR_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE ((c), SWCDB_THRIFT_TYPE_SERVICE_PROCESSOR))
#define SWCDB_THRIFT_SERVICE_PROCESSOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SWCDB_THRIFT_TYPE_SERVICE_PROCESSOR, swcdb_thriftServiceProcessorClass))

#endif /* SWCDB_THRIFT_SERVICE_H */
