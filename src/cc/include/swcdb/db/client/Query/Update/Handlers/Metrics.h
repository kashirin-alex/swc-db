/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_Query_Update_Handlers_Metrics_h
#define swcdb_db_client_Query_Update_Handlers_Metrics_h


#include "swcdb/core/Compat.h"
#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/db/client/Query/UpdateHandlerBaseSingleColumn.h"
#include "swcdb/common/Stats/Stat.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {


//! The SWC-DB Update Metric Handler C++ namespace 'SWC::client::Query::Update::Handlers::Metric'
namespace Metric {



static const uint24_t FIELD_ID_MIN    = uint24_t(0);
static const uint24_t FIELD_ID_MAX    = uint24_t(1);
static const uint24_t FIELD_ID_AVG    = uint24_t(2);
static const uint24_t FIELD_ID_COUNT  = uint24_t(3);
static const uint24_t FIELD_ID_VOLUME = uint24_t(4);



class Level;

struct Base {
  typedef std::unique_ptr<Base> Ptr;

  virtual ~Base() { }

  virtual void report(uint64_t for_ns, Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) = 0;

  virtual void reset() = 0;

  virtual Level* get_level(const char*, bool) {
    return nullptr;
  }

};



class Level : public Base {
  public:
  typedef std::unique_ptr<Level> Ptr;

  const std::string      name;
  std::vector<Base::Ptr> metrics;

  Level(const char* name) : name(name) { }

  virtual ~Level() { }

  virtual void report(uint64_t for_ns, Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override;

  virtual void reset() override;

  Level* get_level(const char* _name, bool inner=true) override;

};



class Item_MinMaxAvgCount :
      public SWC::Common::Stats::MinMaxAvgCount_Safe<uint64_t>,
      public Base {
  public:
  typedef std::unique_ptr<Item_MinMaxAvgCount> Ptr;

  using SWC::Common::Stats::MinMaxAvgCount_Safe<uint64_t>::add;

  const std::string name;

  Item_MinMaxAvgCount(const char* name) : name(name) { }

  virtual ~Item_MinMaxAvgCount() { }

  virtual void report(uint64_t for_ns, Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override;

  virtual void reset() override {
    SWC::Common::Stats::MinMaxAvgCount_Safe<uint64_t>::reset();
  }

};



class Item_Count : public Base {
  public:
  typedef std::unique_ptr<Item_Count> Ptr;

  const std::string name;

  Item_Count(const char* name) : name(name), m_count(0) { }

  virtual ~Item_Count() { }

  void increment() {
    m_count.fetch_add(1);
  }

  virtual void report(uint64_t for_ns, Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override;

  virtual void reset() override {
    m_count.store(0);
  }

  protected:
  Core::Atomic<uint64_t> m_count;

};



class Item_Volume : public Base {
  public:
  typedef std::unique_ptr<Item_Volume> Ptr;

  const std::string name;

  Item_Volume(const char* name) : name(name), m_volume(0) { }

  virtual ~Item_Volume() { }

  void increment() {
    m_volume.fetch_add(1);
  }

  void decrement() {
    m_volume.fetch_sub(1);
  }

  virtual void report(uint64_t for_ns, Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override;

  virtual void reset() override {
    m_volume.store(0);
  }

  protected:
  Core::Atomic<uint64_t> m_volume;

};



class Item_CountVolume : public Base {
  public:
  typedef std::unique_ptr<Item_CountVolume> Ptr;

  const std::string name;

  Item_CountVolume(const char* name)
                    : name(name), m_count(0), m_volume(0) { }

  virtual ~Item_CountVolume() { }

  void increment() {
    m_count.fetch_add(1);
    m_volume.fetch_add(1);
  }

  void decrement() {
    m_volume.fetch_sub(1);
  }

  virtual void report(uint64_t for_ns, Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override;

  virtual void reset() override {
    m_count.store(0);
    m_volume.store(0);
  }

  protected:
  Core::Atomic<uint64_t> m_count;
  Core::Atomic<uint64_t> m_volume;

};





class Reporting : public BaseSingleColumn {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  const Comm::IoContextPtr           io;
  Config::Property::V_GINT32::Ptr    cfg_intval;
  std::vector<Metric::Base::Ptr>     metrics;
  Core::AtomicBool                   running;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval);

  virtual ~Reporting() { }

  virtual void start() {
    bool at = false;
    if(running.compare_exchange_weak(at, true))
      schedule();
  }

  virtual void stop();

  virtual void wait();

  Level* get_level(const char* name);

  virtual uint64_t apply_time(uint32_t intval, DB::Cell::KeyVec& key);

  protected:

  Core::MutexSptd m_mutex;

  virtual bool valid() noexcept override {
    return running;
  }

  virtual void response(int err=Error::OK) override;


  private:

  void report();

  void schedule();

  asio::high_resolution_timer m_timer;

};




}}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update/Handlers/Metrics.cc"
#endif


#endif // swcdb_db_client_Query_Update_Handlers_Metrics_h
