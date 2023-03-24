/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_utils_ShellStatistics_h
#define swcdb_utils_ShellStatistics_h


#include "swcdb/utils/cli/Shell.h"
#include "swcdb/db/client/sql/SQL.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace Utils { namespace shell {



class Statistics final : public Interface {
  public:

  const bool            with_broker;
  client::Clients::Ptr  clients;

  Statistics();

  virtual ~Statistics();

  private:

  bool read(std::string& cmd, bool extended);

  void set_definitions(DB::Specs::Scan& specs);

  bool list_metrics();

  bool show();

  bool truncate();

  struct ReadGroup {
    uint64_t        last;
    int64_t         since;
    uint32_t        agg;
    DB::Specs::Key  key;
    std::string     metric;
    ReadGroup() noexcept : last(0), since(0), agg(0), key(), metric() { }
    void print(std::ostream& out, const Statistics* ptr) const;
    ~ReadGroup() noexcept { }
  };

  struct MetricDefinition {
    uint24_t    relation;
    uint24_t    id;
    uint8_t     agg;
    std::string name;
    std::string label;
    ~MetricDefinition() noexcept { }
  };

  struct StatsDefinition {
    DB::Specs::Key                 property;
    Core::Vector<MetricDefinition> metrics;
    StatsDefinition(const DB::Cells::Cell& cell);
    ~StatsDefinition() noexcept { }
    bool has_metric(const std::string& name, uint24_t fid,
                    size_t& metric_idx) const noexcept;
    void print(std::ostream& out, const Statistics* ptr,
               bool only_property=false) const;
  };

  struct Stats {
    Stats(const StatsDefinition* a_defined, time_t a_ts) noexcept
          : defined(a_defined), ts(a_ts), values() { }
    Stats(Stats&& other) noexcept
          : defined(other.defined),
            ts(other.ts),
            values(std::move(other.values)) {
    }
    Stats(const Stats&) = delete;
    Stats& operator=(const Stats&) = delete;
    ~Stats() noexcept { }
    void print(std::ostream& out, const ReadGroup& group,
               Statistics* ptr) const;
    void add(const Core::Vector<Stats>* datasp,
             size_t metric_idx, uint24_t field_id, int64_t value) noexcept;
    struct Data {
      const uint24_t id;
      uint8_t        flag;
      int64_t        total;
      int64_t        count;
      int64_t        last;
      Data(uint24_t a_id) noexcept
          : id(a_id), flag(0), total(0), count(0), last(0) { }
    };
    const StatsDefinition*  defined;
    const time_t            ts;
    Core::Vector<Data>      values;
  };

  std::string                    m_message;
  Core::Vector<std::string>      m_stat_names;
  Core::Vector<ReadGroup>        m_read_groups;
  Core::Vector<StatsDefinition>  m_definitions;

};



}}} // namespace Utils::shell

#endif // swcdb_utils_ShellStatistics_h
