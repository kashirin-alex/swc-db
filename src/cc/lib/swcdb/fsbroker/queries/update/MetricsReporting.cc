/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fsbroker/queries/update/MetricsReporting.h"


namespace SWC { namespace FsBroker { namespace Metric {



class Item_Fds : public Item_CountVolume {
  public:
  Item_Fds() : Item_CountVolume("fds") { }

  virtual ~Item_Fds() { }

  void definitions(client::Query::Update::Handlers::Base::Column* colp,
                   const DB::Cell::KeyVec& parent_key) override {
    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 1);
    key.copy(parent_key);
    key.add(name);

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.key.add(key);

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.add(std::vector<int64_t>({FIELD_ID_VOLUME, FIELD_ID_COUNT}));
    wfields.add(std::vector<std::string>({"open", "opened"}));
    wfields.add(std::vector<std::string>({"Open FDs", "Openings"}));
    wfields.add(std::vector<int64_t>({Aggregation::MAX}));

    cell.set_value(DB::Types::Encoder::ZSTD, wfields.base, wfields.fill());
    colp->add(cell);
  }
};



Reporting::Reporting(const Comm::IoContextPtr& io,
                     Config::Property::V_GINT32::Ptr cfg_intval)
          : Common::Query::Update::Metric::Reporting(io, cfg_intval),
            net(nullptr), fds(new Item_Fds()) {
}

void Reporting::configure_fsbroker(const char*, const Comm::EndPoints& endpoints) {
  char hostname[256];
  if(gethostname(hostname, sizeof(hostname)) == -1)
    SWC_THROW(errno, "gethostname");

  auto level = Common::Query::Update::Metric::Reporting::configure(
    "swcdb", "fsbroker", hostname, endpoints
  );

  level->metrics.emplace_back(
    net = new Item_Net<Comm::Protocol::FsBroker::Commands>(
      endpoints, Env::Config::settings()->get_bool("swc.comm.ssl")));

  auto fs = Env::FsInterface::fs();
  if(fs->statistics.enabled)
    level->metrics.emplace_back(new Item_FS(fs));
  level->metrics.emplace_back(fds);
}



}}} // namespace SWC::FsBroker::Metric
