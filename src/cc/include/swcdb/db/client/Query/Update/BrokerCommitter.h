/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_Query_Update_BrokerCommitter_h
#define swcdb_db_client_Query_Update_BrokerCommitter_h


#include "swcdb/db/client/Query/Update/Handlers/Base.h"


namespace SWC { namespace client { namespace Query { namespace Update {



class BrokerCommitter final
    : public std::enable_shared_from_this<BrokerCommitter> {
  public:

  static void execute(const Handlers::Base::Ptr& hdlr,
                      Handlers::Base::Column* colp) {
    std::make_shared<BrokerCommitter>(hdlr, colp)->run();
  }

  Query::Update::Handlers::Base::Ptr     hdlr;
  Query::Update::Handlers::Base::Column* colp;

  BrokerCommitter(const Query::Update::Handlers::Base::Ptr& hdlr,
                  Query::Update::Handlers::Base::Column* colp) noexcept;

  //~BrokerCommitter() { }

  void print(std::ostream& out);

  void run();

  private:

  void commit_data();

  Comm::EndPoints endpoints;

};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update/BrokerCommitter.cc"
#endif


#endif // swcdb_db_client_Query_Update_BrokerCommitter_h
