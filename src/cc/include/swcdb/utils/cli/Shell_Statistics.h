/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_utils_ShellStatistics_h
#define swcdb_utils_ShellStatistics_h


#include "swcdb/utils/cli/Shell.h"
#include "swcdb/db/client/sql/SQL.h"


namespace SWC { namespace Utils { namespace shell {



class Statistics : public Interface {
  public:

  Statistics();

  private:

  bool read(std::string& cmd, bool extended);

  void set_definitions(DB::Specs::Scan& specs);

  bool list_metrics();

  bool show();

  bool truncate();

  struct ReadGroup {
    uint64_t last = 0;
    int64_t since = 0;
    uint32_t agg  = 0;
    DB::Specs::Key key;
    std::string metric;
    void print(std::ostream& out, const Statistics* ptr) const;
  };

  struct MetricDefinition {
    uint24_t    id;
    uint8_t     agg;
    std::string name;
    std::string label;
  };

  struct StatsDefinition {
    DB::Specs::Key                property;
    std::vector<MetricDefinition> metrics;
    StatsDefinition(const DB::Cells::Cell& cell);
    bool has_metric(const std::string& name, uint24_t fid,
                    size_t& metric_idx) const noexcept;
    void print(std::ostream& out, const Statistics* ptr,
               bool only_property=false) const;
  };

  struct Stats {
    StatsDefinition*      defined = nullptr;
    time_t                ts = 0;
    std::vector<uint24_t> ids;
    std::vector<int64_t>  values;
    std::vector<int64_t>  counts;
    void print(std::ostream& out, const ReadGroup& group,
               Statistics* ptr) const;
    void add(size_t metric_idx, uint24_t field_id, int64_t value) noexcept;
  };

  std::string                   m_message;
  std::vector<std::string>      m_stat_names;
  std::vector<ReadGroup>        m_read_groups;
  std::vector<StatsDefinition>  m_definitions;

};



}}} // namespace Utils::shell

#endif // swcdb_utils_ShellStatistics_h
