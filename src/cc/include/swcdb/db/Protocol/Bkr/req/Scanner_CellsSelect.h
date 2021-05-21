/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_Scanner_CellsSelect_h
#define swcdb_db_protocol_bkr_req_Scanner_CellsSelect_h


#include "swcdb/db/Protocol/Bkr/params/CellsSelect.h"
#include "swcdb/db/client/Query/Select/BrokerScanner.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class Scanner_CellsSelect: public client::ConnQueue::ReqBase {
  public:

  static void request(
        const SWC::client::Query::Select::BrokerScanner::Ptr& scanner,
        const Params::CellsSelectReq& params) {
    std::make_shared<Scanner_CellsSelect>(scanner, params)->run();
  }

  typedef std::shared_ptr<Scanner_CellsSelect>    Ptr;
  SWC::client::Query::Select::BrokerScanner::Ptr  scanner;

  Scanner_CellsSelect(
        const SWC::client::Query::Select::BrokerScanner::Ptr& scanner,
        const Params::CellsSelectReq& params);

  virtual ~Scanner_CellsSelect() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

};


}}}}}



#endif // swcdb_db_protocol_bkr_req_Scanner_CellsSelect_h
