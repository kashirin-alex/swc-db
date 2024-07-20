// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Service.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::SWC::Thrift;

class ServiceHandler : virtual public ServiceIf {
 public:
  ServiceHandler() {
    // Your initialization goes here
  }

  /**
   * The direct SQL method to Manage Column.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_mng_column(const std::string& sql) {
    // Your implementation goes here
    printf("sql_mng_column\n");
  }

  /**
   * The direct SQL method to List Columns
   * 
   * @param sql The SQL string to Execute
   */
  void sql_list_columns(Schemas& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_list_columns\n");
  }

  /**
   * The direct SQL method to Compact Columns
   * 
   * @param sql The SQL string to Execute
   */
  void sql_compact_columns(CompactResults& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_compact_columns\n");
  }

  /**
   * The direct SQL method to select cells with result in CellsPlain.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_select_plain(CellsPlain& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_select_plain\n");
  }

  /**
   * The direct SQL method to select cells with result in CellsCounter.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_select_counter(CellsCounter& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_select_counter\n");
  }

  /**
   * The direct SQL method to select cells with result in CellsSerial.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_select_serial(CellsSerial& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_select_serial\n");
  }

  /**
   * The direct SQL method to select cells with result in Cells List.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_select(Cells& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_select\n");
  }

  /**
   * The direct SQL method to select cells with result in Columns Cells map.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_select_rslt_on_column(CCells& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_select_rslt_on_column\n");
  }

  /**
   * The direct SQL method to select cells with result in Key Cells list.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_select_rslt_on_key(KCells& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_select_rslt_on_key\n");
  }

  /**
   * The direct SQL method to select cells with result in Fractons Cells.
   * 
   * @param sql The SQL string to Execute
   */
  void sql_select_rslt_on_fraction(FCells& _return, const std::string& sql) {
    // Your implementation goes here
    printf("sql_select_rslt_on_fraction\n");
  }

  /**
   * The SQL method to select cells with result set by the request's type of CellsResult.
   * 
   * @param sql The SQL string to Execute
   * 
   * @param rslt The Type of Cells Result for the response
   */
  void sql_query(CellsGroup& _return, const std::string& sql, const CellsResult::type rslt) {
    // Your implementation goes here
    printf("sql_query\n");
  }

  /**
   * The direct SQL method to update cells optionally to work with updater-id.
   * 
   * @param sql The SQL string to Execute
   * 
   * @param updater_id The Updater ID to work with
   */
  void sql_update(const std::string& sql, const int64_t updater_id) {
    // Your implementation goes here
    printf("sql_update\n");
  }

  /**
   * The SQL method to execute any query.
   * 
   * @param sql The SQL string to Execute
   */
  void exec_sql(Result& _return, const std::string& sql) {
    // Your implementation goes here
    printf("exec_sql\n");
  }

  /**
   * The method to Create an Updater ID with buffering size in bytes.
   * 
   * @param buffer_size The buffer size of the Updater
   */
  int64_t updater_create(const int32_t buffer_size) {
    // Your implementation goes here
    printf("updater_create\n");
  }

  /**
   * The method to Close an Updater ID.
   * 
   * @param id The Updater ID to close
   */
  void updater_close(const int64_t id) {
    // Your implementation goes here
    printf("updater_close\n");
  }

  /**
   * The direct method to update cells with cell in Update-Columns-Cells-Plain,
   * optionally to work with updater-id.
   * 
   * @param cells The Cells to update
   * 
   * @param updater_id The Updater ID to use for write
   */
  void update_plain(const UCCellsPlain& cells, const int64_t updater_id) {
    // Your implementation goes here
    printf("update_plain\n");
  }

  /**
   * The direct method to update cells with cell in Update-Columns-Cells-Counter,
   * optionally to work with updater-id.
   * 
   * @param cells The Counter Cells to update
   * 
   * @param updater_id The Updater ID to use for write
   */
  void update_counter(const UCCellsCounter& cells, const int64_t updater_id) {
    // Your implementation goes here
    printf("update_counter\n");
  }

  /**
   * The direct method to update cells with cell in Update-Columns-Cells-Serial,
   * optionally to work with updater-id.
   * 
   * @param cells The Serial Cells to update
   * 
   * @param updater_id The Updater ID to use for write
   */
  void update_serial(const UCCellsSerial& cells, const int64_t updater_id) {
    // Your implementation goes here
    printf("update_serial\n");
  }

  /**
   * The method is to update cells by several Column-Types,
   * optionally to work with updater-id.
   * 
   * @param plain The PLAIN Cells to update
   * 
   * @param counter The COUNTER Cells to update
   * 
   * @param serial The SERIAL Cells to update
   * 
   * @param updater_id The Updater ID to use for write
   */
  void update_by_types(const UCCellsPlain& plain, const UCCellsCounter& counter, const UCCellsSerial& serial, const int64_t updater_id) {
    // Your implementation goes here
    printf("update_by_types\n");
  }

  /**
   * The direct method to Manage Column
   * 
   * @param func The Action Function to use
   * 
   * @param schema The Schema for the Action
   */
  void mng_column(const SchemaFunc::type func, const Schema& schema) {
    // Your implementation goes here
    printf("mng_column\n");
  }

  /**
   * The direct method to List Columns
   * 
   * @param spec The Schemas Specifications to match Schema for response
   */
  void list_columns(Schemas& _return, const SpecSchemas& spec) {
    // Your implementation goes here
    printf("list_columns\n");
  }

  /**
   * The direct method to Compact Columns
   * 
   * @param spec The Schemas Specifications to match columns to Compact
   */
  void compact_columns(CompactResults& _return, const SpecSchemas& spec) {
    // Your implementation goes here
    printf("compact_columns\n");
  }

  /**
   * The direct method to select cells with result in Cells List.
   * 
   * @param spec The Scan Specifications for the scan
   */
  void scan(Cells& _return, const SpecScan& spec) {
    // Your implementation goes here
    printf("scan\n");
  }

  /**
   * The direct method to select cells with result in Columns Cells map.
   * 
   * @param spec The Scan Specifications for the scan
   */
  void scan_rslt_on_column(CCells& _return, const SpecScan& spec) {
    // Your implementation goes here
    printf("scan_rslt_on_column\n");
  }

  /**
   * The direct method to select cells with result in Key Cells list.
   * 
   * @param spec The Scan Specifications for the scan
   */
  void scan_rslt_on_key(KCells& _return, const SpecScan& spec) {
    // Your implementation goes here
    printf("scan_rslt_on_key\n");
  }

  /**
   * The direct method to select cells with result in Fractons Cells.
   * 
   * @param spec The Scan Specifications for the scan
   */
  void scan_rslt_on_fraction(FCells& _return, const SpecScan& spec) {
    // Your implementation goes here
    printf("scan_rslt_on_fraction\n");
  }

  /**
   * The method to select cells with result set by the request's type of CellsResult.
   * 
   * @param spec The Scan Specifications for the scan
   * 
   * @param rslt The Type of Cells Result for the response
   */
  void scan_rslt_on(CellsGroup& _return, const SpecScan& spec, const CellsResult::type rslt) {
    // Your implementation goes here
    printf("scan_rslt_on\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  ::std::shared_ptr<ServiceHandler> handler(new ServiceHandler());
  ::std::shared_ptr<TProcessor> processor(new ServiceProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}
