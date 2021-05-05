/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_queries_update_MetricsReporting_h
#define swcdb_common_queries_update_MetricsReporting_h


#include "swcdb/common/sys/Resources.h"
#include "swcdb/db/client/Query/Update/Handlers/Metrics.h"


namespace SWC { namespace Common { namespace Query { namespace Update {
namespace Metric {

using namespace client::Query::Update::Handlers::Metric;


class Item_Net : public Base {
  /* Cell-Serialization:
      key:    [levels, "net", "{address}", is_secure()?"SECURE":"PLAIN"]
      value:  {FIELD_ID}:I:{value}, ...
  */

  public:

  static const uint8_t FIELD_CONN_OPEN        = 0;
  static const uint8_t FIELD_CONN_ACC         = 1;
  static const uint8_t FIELD_CONN_EST         = 2;

  static const uint8_t FIELD_BYTES_SENT_MIN   = 3;
  static const uint8_t FIELD_BYTES_SENT_MAX   = 4;
  static const uint8_t FIELD_BYTES_SENT_AVG   = 5;
  static const uint8_t FIELD_BYTES_SENT_TRX   = 6;

  static const uint8_t FIELD_BYTES_RECV_MIN   = 7;
  static const uint8_t FIELD_BYTES_RECV_MAX   = 8;
  static const uint8_t FIELD_BYTES_RECV_AVG   = 9;
  static const uint8_t FIELD_BYTES_RECV_TRX   = 10;

  static const uint8_t FIELD_EV_COMMAND_START = 100; // {100 + CMD}

  typedef std::unique_ptr<Item_CountVolume> Ptr;

  struct Addr {
    const asio::ip::address             addr;
    Core::Atomic<uint64_t>              conn_open;
    Core::Atomic<uint64_t>              conn_accept;
    Core::Atomic<uint64_t>              conn_est;
    MinMaxAvgCount                      bytes_sent;
    MinMaxAvgCount                      bytes_recv;
    std::vector<Core::Atomic<uint64_t>> commands;

    Addr(const Comm::EndPoint& endpoint, uint8_t max_ev_cmd)
        : addr(endpoint.address()),
          conn_open(0), conn_accept(0), conn_est(0),
          commands(max_ev_cmd) {
    }
  };

  Item_Net(const Comm::EndPoints& endpoints,
           bool using_secure, uint8_t max_ev_cmd) {
    for(uint8_t secure = 0; secure <= using_secure; ++secure) {
      m_addresses[secure].reserve(endpoints.size());
      for(auto& endpoint : endpoints)
        m_addresses[secure].emplace_back(new Addr(endpoint, max_ev_cmd));
    }
  }

  virtual ~Item_Net() { }

  Addr* get(const asio::ip::address& for_addr, bool secure) {
    for(auto& addr : m_addresses[secure]) {
      if(addr->addr.is_unspecified() || addr->addr == for_addr)
        return addr.get();
    }
    return nullptr;
  }

  void accepted(const Comm::EndPoint& endpoint, bool secure) {
    if(Addr* addr = get(endpoint.address(), secure))
      addr->conn_accept.fetch_add(1);
  }

  void connected(const Comm::ConnHandlerPtr& conn) {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure())) {
      addr->conn_open.fetch_add(1);
      addr->conn_est.fetch_add(1);
    }
  }

  void disconnected(const Comm::ConnHandlerPtr& conn) {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->conn_open.fetch_sub(1);
  }

  void command(const Comm::ConnHandlerPtr& conn, uint8_t cmd) {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->commands[cmd].fetch_add(1);
  }

  void error(const Comm::ConnHandlerPtr& conn) {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->commands.back().fetch_add(1);
  }

  void sent(const Comm::ConnHandlerPtr& conn, size_t bytes) {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->bytes_sent.add(bytes);
  }

  void received(const Comm::ConnHandlerPtr& conn, size_t bytes) {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->bytes_recv.add(bytes);
  }


  virtual void report(uint64_t for_ns,
                      client::Query::Update::Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override {
    int64_t conn_open = 0;
    int64_t conn_accept = 0;
    int64_t conn_est = 0;
    uint64_t bytes_sent_min = 0;
    uint64_t bytes_sent_max = 0;
    uint64_t bytes_sent_avg = 0;
    uint64_t bytes_sent_trx = 0;
    uint64_t bytes_recv_min = 0;
    uint64_t bytes_recv_max = 0;
    uint64_t bytes_recv_avg = 0;
    uint64_t bytes_recv_trx = 0;

    bool secure = false;
    _do_layer:
    for(auto& addr : m_addresses[secure]) {
      DB::Cell::KeyVec key;
      key.reserve(parent_key.size() + 3);
      key.copy(parent_key);
      key.add("net");
      key.add(addr->addr.to_string());
      key.add(secure ? "SECURE" : "PLAIN");

      DB::Cells::Cell cell;
      cell.flag = DB::Cells::INSERT;
      cell.set_time_order_desc(true);
      cell.set_timestamp(for_ns);
      cell.key.add(key);

      conn_open = addr->conn_open.load();
      size_t sz = 2 + Serialization::encoded_length_vi64(conn_open);
      if((conn_accept = addr->conn_accept.exchange(0)))
        sz += 2 + Serialization::encoded_length_vi64(conn_accept);
      if((conn_est = addr->conn_est.exchange(0)))
        sz += 2 + Serialization::encoded_length_vi64(conn_est);

      addr->bytes_sent.exchange_reset(
        bytes_sent_min, bytes_sent_max, bytes_sent_avg, bytes_sent_trx);
      if(bytes_sent_trx) {
        sz += 8;
        sz += Serialization::encoded_length_vi64(bytes_sent_min);
        sz += Serialization::encoded_length_vi64(bytes_sent_max);
        sz += Serialization::encoded_length_vi64(bytes_sent_avg);
        sz += Serialization::encoded_length_vi64(bytes_sent_trx);
      }
      addr->bytes_recv.exchange_reset(
        bytes_recv_min, bytes_recv_max, bytes_recv_avg, bytes_recv_trx);
      if(bytes_recv_trx) {
        sz += 8;
        sz += Serialization::encoded_length_vi64(bytes_recv_min);
        sz += Serialization::encoded_length_vi64(bytes_recv_max);
        sz += Serialization::encoded_length_vi64(bytes_recv_avg);
        sz += Serialization::encoded_length_vi64(bytes_recv_trx);
      }
      std::vector<int64_t> counts(addr->commands.size());
      auto it = counts.begin();
      for(auto& c : addr->commands) {
        if((*it = c.exchange(0)))
          sz += 2 + Serialization::encoded_length_vi64(*it);
        ++it;
      }

      DB::Cell::Serial::Value::FieldsWriter wfields;
      wfields.ensure(sz);

      wfields.add(FIELD_CONN_OPEN,  conn_open);
      if(conn_accept) {
        wfields.add(FIELD_CONN_ACC, conn_accept);
      }
      if(conn_est) {
        wfields.add(FIELD_CONN_EST, conn_est);
      }
      if(bytes_sent_trx) {
        wfields.add(FIELD_BYTES_SENT_MIN, int64_t(bytes_sent_min));
        wfields.add(FIELD_BYTES_SENT_MAX, int64_t(bytes_sent_max));
        wfields.add(FIELD_BYTES_SENT_AVG, int64_t(bytes_sent_avg));
        wfields.add(FIELD_BYTES_SENT_TRX, int64_t(bytes_sent_trx));
      }
      if(bytes_recv_trx) {
        wfields.add(FIELD_BYTES_RECV_MIN, int64_t(bytes_recv_min));
        wfields.add(FIELD_BYTES_RECV_MAX, int64_t(bytes_recv_max));
        wfields.add(FIELD_BYTES_RECV_AVG, int64_t(bytes_recv_avg));
        wfields.add(FIELD_BYTES_RECV_TRX, int64_t(bytes_recv_trx));
      }
      for(size_t i=0; i < counts.size(); ++i) {
        if(counts[i])
          wfields.add(i + FIELD_EV_COMMAND_START, counts[i]);
      }
      cell.set_value(wfields.base, wfields.fill(), false);
      colp->add(cell);
    }

    if(!secure) {
      secure = true;
      if(!m_addresses[secure].empty())
        goto _do_layer;
    }
  }

  virtual void reset() override {
    bool secure = false;
    _do_layer:
      for(auto& addr : m_addresses[secure]) {
        addr->conn_open.store(0);
        addr->conn_accept.store(0);
        addr->conn_est.store(0);
        addr->bytes_sent.reset();
        addr->bytes_recv.reset();
        for(auto& c : addr->commands) {
          c.store(0);
        }
      }
    if(!secure) {
      secure = true;
      if(!m_addresses[secure].empty())
        goto _do_layer;
    }
  }


  protected:
  std::vector<std::unique_ptr<Addr>> m_addresses[2];
};


class Item_Memory : public Base {
  /* Cell-Serialization:
      key:    [levels, "mem"]
      value:  {FIELD_ID}:I:{value}, ...
  */

  public:

  static const uint8_t FIELD_RSS_MIN = 0;
  static const uint8_t FIELD_RSS_MAX = 1;
  static const uint8_t FIELD_RSS_AVG = 2;

  typedef std::unique_ptr<Item_Memory> Ptr;

  MinMaxAvgCount  rss;

  Item_Memory() noexcept { }

  virtual ~Item_Memory() { }

  void add_rss(uint64_t mbytes) noexcept {
    rss.add(mbytes);
  }

  virtual void report(uint64_t for_ns, client::Query::Update::Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override {
    uint64_t rss_min = 0;
    uint64_t rss_max = 0;
    uint64_t rss_avg = 0;
    uint64_t rss_samples = 0;
    rss.exchange_reset(rss_min, rss_max, rss_avg, rss_samples);
    if(!rss_samples)
      return;

    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 1);
    key.copy(parent_key);
    key.add("mem");

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.set_timestamp(for_ns);
    cell.key.add(key);

    size_t sz = 6;
    sz += Serialization::encoded_length_vi64(rss_min);
    sz += Serialization::encoded_length_vi64(rss_max);
    sz += Serialization::encoded_length_vi64(rss_avg);

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(sz);
    wfields.add(FIELD_RSS_MIN,  int64_t(rss_min));
    wfields.add(FIELD_RSS_MAX,  int64_t(rss_max));
    wfields.add(FIELD_RSS_AVG,  int64_t(rss_avg));

    cell.set_value(wfields.base, wfields.fill(), false);
    colp->add(cell);
  }

  virtual void reset() override {
    rss.reset();
  }

};


class Item_CPU : public Base {
  /* Cell-Serialization:
      key:    [levels, "cpu"]
      value:  {FIELD_ID}:I:{value}, ...
  */

  public:

  static const uint8_t FIELD_NTHREADS_MIN = 0;
  static const uint8_t FIELD_NTHREADS_MAX = 1;
  static const uint8_t FIELD_NTHREADS_AVG = 2;

  static const uint8_t FIELD_CPU_U_PERC_MIN = 3;
  static const uint8_t FIELD_CPU_U_PERC_MAX = 4;
  static const uint8_t FIELD_CPU_U_PERC_AVG = 5;

  static const uint8_t FIELD_CPU_S_PERC_MIN = 3;
  static const uint8_t FIELD_CPU_S_PERC_MAX = 4;
  static const uint8_t FIELD_CPU_S_PERC_AVG = 5;

  typedef std::unique_ptr<Item_CPU> Ptr;

  MinMaxAvgCount  nthreads;
  MinMaxAvgCount  cpu_u_perc;
  MinMaxAvgCount  cpu_s_perc;

  Item_CPU() noexcept { }

  virtual ~Item_CPU() { }

  void add_threads(uint64_t num) noexcept {
    nthreads.add(num);
  }

  void add_cpu_u(uint64_t perc) noexcept {
    cpu_u_perc.add(perc);
  }
  void add_cpu_s(uint64_t perc) noexcept {
    cpu_s_perc.add(perc);
  }

  virtual void report(uint64_t for_ns, client::Query::Update::Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override {
    uint64_t nthreads_min = 0;
    uint64_t nthreads_max = 0;
    uint64_t nthreads_avg = 0;
    uint64_t nthreads_samples = 0;
    nthreads.exchange_reset(
      nthreads_min, nthreads_max, nthreads_avg, nthreads_samples);

    uint64_t cpu_u_perc_min = 0;
    uint64_t cpu_u_perc_max = 0;
    uint64_t cpu_u_perc_avg = 0;
    uint64_t cpu_u_perc_samples = 0;
    cpu_u_perc.exchange_reset(
      cpu_u_perc_min, cpu_u_perc_max, cpu_u_perc_avg, cpu_u_perc_samples);

    uint64_t cpu_s_perc_min = 0;
    uint64_t cpu_s_perc_max = 0;
    uint64_t cpu_s_perc_avg = 0;
    uint64_t cpu_s_perc_samples = 0;
    cpu_s_perc.exchange_reset(
      cpu_s_perc_min, cpu_s_perc_max, cpu_s_perc_avg, cpu_s_perc_samples);

    if(!nthreads_samples && !cpu_u_perc_samples && !cpu_s_perc_samples)
      return;

    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 1);
    key.copy(parent_key);
    key.add("cpu");

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.set_timestamp(for_ns);
    cell.key.add(key);

    size_t sz = 0;
    DB::Cell::Serial::Value::FieldsWriter wfields;
    if(nthreads_samples) {
      sz += 6;
      sz += Serialization::encoded_length_vi64(nthreads_min);
      sz += Serialization::encoded_length_vi64(nthreads_max);
      sz += Serialization::encoded_length_vi64(nthreads_avg);
    }
    if(cpu_u_perc_samples) {
      sz += 6;
      sz += Serialization::encoded_length_vi64(cpu_u_perc_min);
      sz += Serialization::encoded_length_vi64(cpu_u_perc_max);
      sz += Serialization::encoded_length_vi64(cpu_u_perc_avg);
    }
    if(cpu_s_perc_samples) {
      sz += 6;
      sz += Serialization::encoded_length_vi64(cpu_s_perc_min);
      sz += Serialization::encoded_length_vi64(cpu_s_perc_max);
      sz += Serialization::encoded_length_vi64(cpu_s_perc_avg);
    }

    wfields.ensure(sz);
    if(nthreads_samples) {
      wfields.add(FIELD_NTHREADS_MIN,  int64_t(nthreads_min));
      wfields.add(FIELD_NTHREADS_MAX,  int64_t(nthreads_max));
      wfields.add(FIELD_NTHREADS_AVG,  int64_t(nthreads_avg));
    }
    if(cpu_u_perc_samples) {
      wfields.add(FIELD_CPU_U_PERC_MIN,  int64_t(cpu_u_perc_min));
      wfields.add(FIELD_CPU_U_PERC_MAX,  int64_t(cpu_u_perc_max));
      wfields.add(FIELD_CPU_U_PERC_AVG,  int64_t(cpu_u_perc_avg));
    }
    if(cpu_s_perc_samples) {
      wfields.add(FIELD_CPU_S_PERC_MIN,  int64_t(cpu_s_perc_min));
      wfields.add(FIELD_CPU_S_PERC_MAX,  int64_t(cpu_s_perc_max));
      wfields.add(FIELD_CPU_S_PERC_AVG,  int64_t(cpu_s_perc_avg));
    }

    cell.set_value(wfields.base, wfields.fill(), false);
    colp->add(cell);
  }

  virtual void reset() override {
    nthreads.reset();
    cpu_u_perc.reset();
    cpu_s_perc.reset();
  }

};



class Reporting : public client::Query::Update::Handlers::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval_ms)
            : client::Query::Update::Handlers::Metric::Reporting(
                io, cfg_intval_ms),
              net(nullptr) {
  }

  virtual Level* configure(const char* group_name, const char* inst_name,
                           const char* host, const Comm::EndPoints& endpoints,
                           uint8_t max_ev_cmd) {
    auto level = host ? get_level(host) : nullptr;
    if(group_name)
      level = level ? level->get_level(group_name) : get_level(group_name);
    if(inst_name)
      level = level ? level->get_level(inst_name) : get_level(inst_name);
    if(!endpoints.empty()) {
      std::string port(std::to_string(endpoints.front().port()));
      level = level ? level->get_level(port.c_str()) : get_level(port.c_str());
    }
    SWC_ASSERT(level);

    level->metrics.emplace_back(hardware._mem);
    level->metrics.emplace_back(hardware._cpu);

    bool using_net_secure = Env::Config::settings()->get_bool("swc.comm.ssl");
    net = new Item_Net(endpoints, using_net_secure, max_ev_cmd);
    level->metrics.emplace_back(net);
    return level;
  }

  virtual ~Reporting() { }


  struct Hardware final : Common::Resources::Notifiers {
    Item_Memory*  _mem;
    Item_CPU*     _cpu;

    Hardware() noexcept
      : _mem(new Item_Memory()),
        _cpu(new Item_CPU()) {
    }

    void rss(uint64_t bytes) override {
      _mem->add_rss(bytes / 1048576);
    }

    void threads(uint64_t num) override {
      _cpu->add_threads(num);
    }

    void cpu_user(uint64_t perc) override {
      _cpu->add_cpu_u(perc);
    }

    void cpu_sys(uint64_t perc) override {
      _cpu->add_cpu_s(perc);
    }

  } hardware;

  Item_Net* net;

};



}}}}} // namespace SWC::Common::Query::Update::Metric


#endif // swcdb_common_queries_update_MetricsReporting_h
