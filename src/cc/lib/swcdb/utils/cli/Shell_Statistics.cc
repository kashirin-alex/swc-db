/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/utils/cli/Shell_Statistics.h"

#include "swcdb/db/client/Query/Select/Handlers/Common.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"
#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/db/client/Query/Update/Handlers/Metrics.h"
#include <iomanip>




namespace SWC { namespace Utils { namespace shell {


static const Core::Vector<std::string> default_stat_names = {
  "host", "group", "role", "id", "component", "part", "type"
};


Statistics::Statistics()
  : Interface("\033[32mSWC-DB(\033[36mstatistics\033[32m)\033[33m> \033[00m",
              "/tmp/.swc-cli-statistics-history",
              CLI::STATISTICS),
    with_broker(Env::Config::settings()->get_bool("with-broker")),
    clients(
      (with_broker
        ? client::Clients::make(
            *Env::Config::settings(),
            Comm::IoContext::make("Statistics", 8),
            nullptr  // std::make_shared<client::BrokerContext>()
          )
        : client::Clients::make(
            *Env::Config::settings(),
            Comm::IoContext::make("Statistics", 8),
            nullptr, // std::make_shared<client::ManagerContext>()
            nullptr  // std::make_shared<client::RangerContext>()
          )
      )->init()
    ),
    m_stat_names(default_stat_names) {

  add_option(
    "show",
    {"show last='DURATION' since='DATETIME' agg='DURATION'",
     "     host='STRING' group='STRING' role='STRING' id='STRING'",
     "     component='STRING' part='STRING' type='STRING' metric='STRING'",
     "     AND .. ;",
     "  -> show last=30day group=swcdb role=rgr component=fs part=broker;",
     "     # show Fs-Broker stats on Ranger for the last 30 days",
     "  -> show since='2021/05/01 00:00:00' component='cpu';",
     "     # show CPU on all roles having CPU metrics since May of 2021",
     "* STRING:   string-type definition is optional",
     "* DURATION: in Decimal[week|day|hour|minute] (4week)",
     "* DATETIME: in format YYYY/MM/DD HH:mm:ss or seconds-timestamp",
     "* 'last=':  The Duration set or NONE default to 7day",
     "* 'since=': The Datetime set or NONE for since (now-'last'|ever)",
     "* 'agg=':   The Duration for aggregations or NONE into-single entry",
     "* AND:      Another metrics group"
    },
    [this](std::string& cmd) { return read(cmd, true); },
    "(?i)^show(.*|$)"
  );
  add_option(
    "list metrics",
    {"list metrics host='STRING' group='STRING' role='STRING' id='STRING'",
     "             component='STRING' part='STRING' type='STRING' AND .. ;",
     "  -> list metrics;",
     "     # List all metrics definitions"
    },
    [this](std::string& cmd) { return read(cmd, false); },
    "(?i)^list metrics(.*|$)"
  );
  add_option(
    "truncate",
    {"truncate last='DURATION' since='DATETIME' agg='DURATION'",
     "         host='STRING' group='STRING' role='STRING' id='STRING'",
     "         component='STRING' part='STRING'",
     "         type='STRING' AND ..;",
     "  -> truncate;",
     "     # Truncate all metrics"
    },
    [this](std::string& cmd) { return read(cmd, true); },
    "(?i)^truncate(.*|$)"
  );
  add_option(
    "set stat-names",
    {"set stat-names F1,F2, .. or 'swcdb-default';",
     "* swcdb-default: host,group,role,id,component,part,type"},
    [this](std::string& cmd) { return read(cmd, false); },
    "(?i)^set stat-names(.*|$)"
  );
  add_option(
    "list stat-names",
    {"list stat-names;"},
    [this](std::string&) {
      SWC_PRINT << "stat-names: ";
      for(auto it=m_stat_names.cbegin(); it != m_stat_names.cend();) {
        SWC_LOG_OSTREAM << '"' << *it << '"';
        if(++it != m_stat_names.cend())
          SWC_LOG_OSTREAM<< ", ";
      }
      SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
      return true;
    },
    "(?i)^list stat-names(.*|$)"
  );

  SWC_ASSERT(!with_broker || clients->brokers.has_endpoints());
}

Statistics::~Statistics() {
  clients->stop();
}

void Statistics::ReadGroup::print(std::ostream& out,
                                  const Statistics* ptr) const {
  if(last)
    out << "last=" << last << "s ";
  if(since)
    out << "since=" << since << "s ";
  if(agg)
    out << "agg=" << agg << "s ";
  for(size_t i=0; i < key.size(); ++i) {
    if(!key[i].empty())
      key[i].print(out << ' ' << ptr->m_stat_names[i]);
  }
  if(!metric.empty())
    out << " metric=" << metric << ' ';
}


bool Statistics::read(std::string& cmd, bool extended) {
  m_read_groups.clear();
  m_message.clear();
  err = Error::OK;

  uint8_t func = 0;
  client::SQL::Reader parser(cmd, m_message);
  while(parser.remain && !err) {
    if(parser.found_space())
      continue;

    if(parser.found_token("list metrics", 12)) {
      func = 1;
      break;
    }
    if(parser.found_token("show", 4)) {
      func = 2;
      break;
    }
    if(parser.found_token("truncate", 8)) {
      func = 3;
      break;
    }
    if(parser.found_token("set stat-names", 14)) {
      parser.seek_space();
      if(parser.found_token("swcdb-default", 13)) {
        m_stat_names = default_stat_names;
        return true;
      }
      Core::Vector<std::string> tmp;
      std::string buf;
      while(parser.remain && !err) {
        parser.seek_space();
        parser.read(buf, ",");
        if(!buf.empty())
          tmp.emplace_back(std::move(buf));
        parser.seek_space();
        if(!parser.found_char(','))
          break;
      }
      if(!tmp.empty()) {
        m_stat_names = std::move(tmp);
        return true;
      }
      err = Error::SQL_PARSE_ERROR;
      return error("Empty stat names list");
    }
    bool token_cmd = false;
    parser.expect_token("list metrics|show|truncate", 1, token_cmd);
    err = parser.err;
    return error(m_message);
  }

  auto group = &m_read_groups.emplace_back();
  group->key.resize(m_stat_names.size());
  bool ok = false;
  while(parser.remain && !err) {
    if(parser.found_space())
      continue;
    if(extended) {
      if(parser.found_token("last", 4)) {
        parser.seek_space();
        parser.expect_eq();
        bool was_set;
        parser.read_duration_secs(group->last, was_set, " ");
        ok = true;
        continue;
      }
      if(parser.found_token("since", 5)) {
        parser.seek_space();
        parser.expect_eq();
        std::string buf;
        parser.read(buf, " ");
        group->since = Time::parse_ns(err, buf);
        if(err)
          parser.error_msg(Error::SQL_PARSE_ERROR, "Bad datetime format");
        else
          group->since /= 1000000000;
        ok = true;
        continue;
      }
      if(parser.found_token("agg", 3)) {
        parser.seek_space();
        parser.expect_eq();
        bool was_set;
        uint64_t agg;
        parser.read_duration_secs(agg, was_set, " ");
        if(was_set)
          group->agg = agg;
        ok = true;
        continue;
      }
      if(parser.found_token("metric", 6)) {
        parser.seek_space();
        parser.expect_eq();
        parser.read(group->metric, " ");
        ok = true;
        continue;
      }
    }

    bool found = false;
    std::string buf;
    parser.read(buf, " =");
    if(!buf.empty()) for(size_t i=0; i < m_stat_names.size(); ++i) {
      if(Condition::str_case_eq(
          buf.c_str(), m_stat_names[i].c_str(), m_stat_names[i].size())) {
        parser.seek_space();
        parser.expect_eq();
        parser.read(group->key[i], " ");
        found = ok = true;
        break;
      }
    }
    if(found)
      continue;

    if(!Condition::str_case_eq(buf.c_str(), "and", 3))
      break;
    if(!ok) {
      err = Error::CANCELLED;
      m_message = "Empty Metrics-Group";
      break;
    }
    group = &m_read_groups.emplace_back();
    group->key.resize(m_stat_names.size());
    ok = false;
  }

  if(!ok && m_read_groups.size() > 1) {
    err = Error::CANCELLED;
    m_message = "Empty Metrics-Group";
  }
  if(err || (err = parser.err))
    return error(m_message);

  for(auto& g : m_read_groups) {
    auto it = g.key.begin();
    auto it_set = g.key.cbegin();
    for(; it != g.key.cend(); ++it) {
      if(!it->empty()) {
        it->comp = Condition::EQ;
        it_set = it;
        continue;
      }
      it->comp = Condition::FIP;
    }
    if(it == g.key.cbegin()) {
      g.key.clear();
    } else if(it_set != g.key.cend() &&
              ++it_set != g.key.cend() &&
              ++it_set != g.key.cend()) {
      g.key.erase(it_set, g.key.cend());
    }
  }

  switch(func) {
    case 1:
      return list_metrics();
    case 2:
      return show();
    case 3:
      return truncate();
    default:
      if(!err)
        err = Error::CANCELLED;
  }
  return error(m_message);
}



Statistics::StatsDefinition::StatsDefinition(const DB::Cells::Cell& cell) {
  property.set(cell.key, Condition::Comp::EQ);
  property[0].clear();
  property[0].comp = Condition::Comp::GE;

  StaticBuffer v;
  cell.get_value(v);
  const uint8_t* ptr = v.base;
  size_t remain = v.size;

  Core::Vector<int64_t>      ids;
  Core::Vector<int64_t>      aggregations;
  Core::Vector<int64_t>      relations;
  Core::Vector<std::string>  names;
  Core::Vector<std::string>  labels;

  while(remain) {
    switch(DB::Cell::Serial::Value::read_type(&ptr, &remain)) {
      case DB::Cell::Serial::Value::Type::LIST_INT64: {
        DB::Cell::Serial::Value::Field_LIST_INT64 value(&ptr, &remain);
        if(value.fid == 0)
          value.convert_to(ids);
        else if(value.fid == 3)
          value.convert_to(aggregations);
        else if(value.fid == 4)
          value.convert_to(relations);
        break;
      }
      case DB::Cell::Serial::Value::Type::LIST_BYTES: {
        DB::Cell::Serial::Value::Field_LIST_BYTES value(&ptr, &remain);
        if(value.fid == 1)
          value.convert_to(names);
        else if(value.fid == 2)
          value.convert_to(labels);
        break;
      }
      default:
        break;
    }
  }
  metrics.resize(ids.size());

  for(size_t i =0; i < ids.size(); ++i) {
    metrics[i].id = ids[i];
    metrics[i].relation = i < relations.size() ? relations[i] : 0;
    metrics[i].agg = i < aggregations.size() ? aggregations[i] : 0;
    if(i < names.size())
      metrics[i].name = std::move(names[i]);
    else
      metrics[i].name = std::to_string(ids[i]);
    if(i < labels.size())
      metrics[i].label = std::move(labels[i]);
    else
      metrics[i].label = metrics[i].name;
  }

  std::sort(
    metrics.begin(), metrics.end(),
    [](const MetricDefinition& a, const MetricDefinition& b) {
      return a.id < b.id;
  });
}

bool Statistics::StatsDefinition::has_metric(const std::string& name,
                                             uint24_t fid,
                                             size_t& i)
                                             const noexcept {
  for(i=0; i < metrics.size(); ++i) {
    if(metrics[i].id == fid) {
      if(!name.empty() && (metrics[i].name.empty() ||
                           !Condition::str_eq(name, metrics[i].name)))
        return false;
      return true;
    }
  }
  return false;
}

void Statistics::StatsDefinition::print(std::ostream& out,
                                        const Statistics* ptr,
                                        bool only_property) const {
  out << "# property:";
  for(size_t i = 1; i < property.size(); ++i) {
    out << ' ';
    if(i <= ptr->m_stat_names.size())
      out << ptr->m_stat_names[i-1];
    else
      out << "F" << i;
    out << '=' << property[i];
  }
  if(only_property)
    return;

  size_t len_relation = 8;
  size_t len_id    = 2;
  size_t len_name  = 6;
  size_t len_label = 5;
  for(auto& metric : metrics) {
    if(len_name < metric.name.length())
      len_name = metric.name.length();
    if(len_label < metric.label.length())
      len_label = metric.label.length();
    size_t tmp = std::to_string(uint32_t(metric.id)).length();
    if(len_id < tmp)
      len_id = tmp;
    tmp = std::to_string(uint32_t(metric.relation)).length();
    if(len_relation < tmp)
      len_relation = tmp;
  }
  len_relation += 3;
  len_id += 3;
  len_name += 3;
  len_label += 3;

  out << '\n';
  out << std::left << std::setw(3)            << " "
      << std::left << std::setw(len_name)     << "Metric"
      << std::left << std::setw(len_id)       << "ID"
      << std::left << std::setw(len_label)    << "Label"
      << std::left << std::setw(len_relation) << "Relation"
      << std::left << "Aggregation";
  out << std::endl;
  for(auto& metric : metrics) {
    out << std::left << std::setw(3)            << " "
        << std::left << std::setw(len_name)     << metric.name
        << std::left << std::setw(len_id)       << metric.id
        << std::left << std::setw(len_label)    << metric.label
        << std::left << std::setw(len_relation) << metric.relation
        << std::left
        << client::Query::Update::Handlers::Metric::aggregation_to_string(
            metric.agg);
    out << std::endl;
  }
}



void Statistics::set_definitions(DB::Specs::Scan& specs) {
  m_definitions.clear();

  auto& col = specs.columns.emplace_back(
    DB::Types::SystemColumn::SYS_CID_DEFINE_LEXIC, m_read_groups.size());

  for(auto& g : m_read_groups) {
    auto& key_intval = col.add(
      DB::Types::Column::SERIAL)->key_intervals.add();
    if(!g.key.empty())
      key_intval.start.copy(g.key);
    key_intval.start.insert(
      0,
      std::to_string(DB::Types::SystemColumn::SYS_CID_STATS),
      Condition::EQ
    );
    if(g.key.empty())
      key_intval.start.add("", Condition::FIP);
  }

  auto hdlr = client::Query::Select::Handlers::Common::make(
    clients,
    [this]
    (const client::Query::Select::Handlers::Common::Ptr& _hdlr) {
      DB::Cells::Result cells;
      for(cid_t cid : _hdlr->get_cids()) {
        _hdlr->get_cells(cid, cells);
        m_definitions.reserve(cells.size());
        for(auto cell : cells) {
          try {
            m_definitions.emplace_back(*cell);
          } catch(...) {
            SWC_LOG_CURRENT_EXCEPTION("");
            SWC_PRINT << "";
            cell->print(SWC_LOG_OSTREAM, DB::Types::Column::SERIAL);
            SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
          }

        }
      }
    },
    true,
    nullptr,
    with_broker
      ? client::Clients::BROKER
      : client::Clients::DEFAULT
  );

  hdlr->scan(err, specs);
  if(!err)
    hdlr->wait();
}

// LIST-METRICS
bool Statistics::list_metrics() {
  DB::Specs::Scan specs;
  set_definitions(specs);

  SWC_LOG_OUT(
    LOG_DEBUG,
    specs.display(SWC_LOG_OSTREAM << "\n # Definitions ");
    SWC_LOG_OSTREAM << "# Groups:";
    for(auto& g : m_read_groups)
      g.print(SWC_LOG_OSTREAM << "\n\t", this);
  );
  if(err)
    return error(m_message);

  SWC_PRINT << "";
  for(auto& stats : m_definitions)
    stats.print(SWC_LOG_OSTREAM << '\n', this);
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;

  return true;
}



void Statistics::Stats::print(std::ostream& out, const ReadGroup& group,
                              Statistics* ptr) const {
  defined->print(out, ptr, true);
  out << '\n';

  size_t len_name = 5;
  size_t len_value = 5;
  for(auto& m : defined->metrics) {
    for(auto& data : values) {
      if(m.id != data.id)
        continue;
      if(len_name < m.name.length())
        len_name = m.name.length();
      size_t tmp = std::to_string(
        data.count ? data.total/data.count : data.total).length();
      if(data.flag && data.count > 1)
        tmp += 7;
      if(len_value < tmp)
        len_value = tmp;
      break;
    }
  }
  len_name += 3;
  len_value += 3;

  char res[20];
  std::strftime(res, 20, "%Y/%m/%d %H:%M:%S", std::gmtime(&ts));
  out << "   since " << res;
  if(group.agg) {
    time_t tmp = ts + group.agg-1;
    std::strftime(res, 20, "%Y/%m/%d %H:%M:%S", std::gmtime(&tmp));
    out << " to " << res;
  }
  out << std::endl;

  out << std::left << std::setw(3)         << " "
      << std::left << std::setw(len_name)  << "Metric"
      << std::left << std::setw(len_value) << "Value"
      << std::left << "Label";
  out << std::endl;

  for(auto& m : defined->metrics) {
    for(auto& data : values) {
      if(m.id != data.id)
        continue;
      std::string value(
        std::to_string(data.count ? data.total/data.count : data.total));
      if(data.flag && data.count > 1)
        value.append("(!prop)");
      out << std::left << std::setw(3)         << " "
          << std::left << std::setw(len_name)  << m.name
          << std::left << std::setw(len_value) << value
          << std::left << m.label;
      out << std::endl;
      break;
    }
  }
}

void Statistics::Stats::add(const Core::Vector<Stats>* datasp,
                            size_t metric_idx, uint24_t field_id,
                            int64_t value) noexcept {
  size_t idx = 0;
  bool found = false;
  for(;idx < values.size(); ++idx) {
    if((found = values[idx].id == field_id))
      break;
  }
  if(!found) {
    idx = values.size();
    values.emplace_back(field_id);
  }
  auto& data = values[idx];

  auto& m = defined->metrics[metric_idx];
  switch(m.agg) {
    case client::Query::Update::Handlers::Metric::Aggregation::MAX: {
      if(!found || value > data.total)
        data.total = value;
      break;
    }
    case client::Query::Update::Handlers::Metric::Aggregation::MIN: {
      if(!found || value < data.total)
        data.total = value;
      break;
    }
    case client::Query::Update::Handlers::Metric::Aggregation::AVG_PROP: {
      /* Proportional Average, such-for Latency Avg or Ev-Bytes */
      if(!data.flag) {
        for(auto& stat : *datasp) {
          if(defined != stat.defined || ts != stat.ts)
            continue;
          for(auto& other_m : defined->metrics) {
            if(other_m.agg !=
                client::Query::Update::Handlers::Metric::Aggregation::SUM ||
               m.relation != other_m.relation)
              continue;
            for(auto& other_v : stat.values) {
              if(other_v.id == other_m.id) {
                data.count += other_v.last;
                data.total += value * other_v.last;
                return;
              }
            }
          }
        }
        data.flag = 1;
      }
      [[fallthrough]];
    }
    case client::Query::Update::Handlers::Metric::Aggregation::AVG: {
      ++data.count;
      data.total += value;
      break;
    }
    default: {
      data.total += data.last = value;
      break;
    }
  }
}



// SHOW
bool Statistics::show() {

  DB::Specs::Scan specs_definitions;
  set_definitions(specs_definitions);
  if(err)
    return error(m_message);

  for(auto& g : m_read_groups) {
    DB::Specs::Interval spec(DB::Types::Column::SERIAL);
    auto& key_intval = spec.key_intervals.add();

    bool empty = g.key.empty();
    if(!empty)
      key_intval.start.copy(g.key);
    if(g.since) {
      key_intval.start.insert(0, std::to_string(g.since), Condition::GE);
      if(g.last) {
        key_intval.finish.add(
          std::to_string(g.since + g.last), Condition::LT);
        key_intval.finish.add("", Condition::FIP);
      }
    } else {
      int64_t secs = ::time(nullptr);
      key_intval.start.insert(
        0,
        std::to_string(secs - (g.last ? g.last : 7 * 24 * 60 * 60)),
        Condition::GE);
    }
    if(empty && !key_intval.start.empty())
      key_intval.start.add("", Condition::FIP);

    SWC_LOG_OUT(
      LOG_DEBUG,
      spec.display(SWC_LOG_OSTREAM << "\n # Statistics ");
      g.print(SWC_LOG_OSTREAM << "# Group:", this);
    );

    Core::Vector<Stats> stats_datas;
    auto hdlr = client::Query::Select::Handlers::Common::make(
      clients,
      [this, &g, datasp=&stats_datas]
      (const client::Query::Select::Handlers::Common::Ptr& _hdlr) {
        DB::Cells::Result cells;
        if(!err && !(err = _hdlr->state_error)) {
          auto col = _hdlr->get_columnn(DB::Types::SystemColumn::SYS_CID_STATS);
          if(!(err = col->error()) && !col->empty())
            col->get_cells(cells);
        }
        if(err) {
          _hdlr->valid_state.store(false);
          return;
        }

        for(auto cell : cells) {
          for(auto& defined : m_definitions) {
            if(defined.property.is_matching_lexic(cell->key)) {

              time_t ts = (cell->get_timestamp()/1000000000);
              if(g.agg) {
                ts /= g.agg;
                ts *= g.agg;
              }
              StaticBuffer v;
              cell->get_value(v);
              const uint8_t* ptr = v.base;
              size_t remain = v.size;

              while(remain) {
                SWC::DB::Cell::Serial::Value::read_type(&ptr, &remain);
                if(!remain)
                  break;
                size_t metric_idx;
                DB::Cell::Serial::Value::Field_INT64 field(&ptr, &remain);
                if(!defined.has_metric(g.metric, field.fid, metric_idx))
                  continue;

                Stats* statsp = nullptr;
                for(auto& stats : *datasp) {
                  if(stats.defined == &defined &&
                     (!g.agg || (stats.ts + g.agg > ts && stats.ts <= ts))) {
                    statsp = &stats;
                    break;
                  }
                }
                (statsp ? statsp : &datasp->emplace_back(&defined, ts))
                  ->add(datasp, metric_idx, field.fid, field.value);
              }
              /*
              SWC_PRINT << "";
              defined.print(SWC_LOG_OSTREAM << std::endl, this);
              cell->print(SWC_LOG_OSTREAM << std::endl, DB::Types::Column::SERIAL);
              SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
              */
              break;
            }
          }
        }
      },
      true,
      nullptr,
      with_broker
        ? client::Clients::BROKER
        : client::Clients::DEFAULT
    );

    hdlr->scan(
      DB::Types::KeySeq::LEXIC,
      DB::Types::SystemColumn::SYS_CID_STATS,
      std::move(spec)
    );
    hdlr->wait();
    if(err || (err = hdlr->state_error))
      break;

    SWC_PRINT << "";
    for(auto& data : stats_datas)
      data.print(SWC_LOG_OSTREAM << '\n', g, this);
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }

  SWC_LOG_OUT(
    LOG_DEBUG,
    specs_definitions.display(SWC_LOG_OSTREAM << "\n # Definitions ");
    SWC_LOG_OSTREAM << "# Groups:";
    for(auto& g : m_read_groups)
      g.print(SWC_LOG_OSTREAM << "\n\t", this);
  );
  return err ? error(m_message) : true;
}



// TRUNCATE
bool Statistics::truncate() {
  Core::Atomic<size_t> deleted_count(0);

  auto hdlr = client::Query::Select::Handlers::Common::make(
    clients,
    [countp=&deleted_count]
    (const client::Query::Select::Handlers::Common::Ptr& _hdlr) {
      if(_hdlr->state_error) {
        _hdlr->valid_state.store(false);
      } else {
        auto col = _hdlr->get_columnn(DB::Types::SystemColumn::SYS_CID_STATS);
        if(col->error()) {
          _hdlr->valid_state.store(false);
        } else if(!col->empty()) {
          DB::Cells::Result cells;
          col->get_cells(cells);
          countp->fetch_add(cells.size());
        }
      }
    },
    true,
    nullptr,
    with_broker
      ? client::Clients::BROKER
      : client::Clients::DEFAULT
  );

  DB::Specs::Scan specs;
  auto& col = specs.columns.emplace_back(
    DB::Types::SystemColumn::SYS_CID_STATS, m_read_groups.size());
  for(auto& g : m_read_groups) {
    auto& intval = col.add(DB::Types::Column::SERIAL);
    intval->set_opt__deleting();
    intval->flags.set_only_keys();
    auto& key_intval = intval->key_intervals.add();

    bool empty = g.key.empty();
    if(!empty)
      key_intval.start.copy(g.key);
    if(g.since) {
      key_intval.start.insert(0, std::to_string(g.since), Condition::GE);
      if(g.last) {
        key_intval.finish.add(
          std::to_string(g.since + g.last), Condition::LT);
        key_intval.finish.add("", Condition::FIP);
      }
    } else {
      int64_t secs = ::time(nullptr);
      key_intval.start.insert(
        0,
        std::to_string(secs - (g.last ? g.last : 7 * 24 * 60 * 60)),
        Condition::GE);
    }
    if(empty && !key_intval.start.empty())
      key_intval.start.add("", Condition::FIP);
  }

  SWC_LOG_OUT(
    LOG_DEBUG,
    specs.display(SWC_LOG_OSTREAM << "\n # ");
    SWC_LOG_OSTREAM << "# Groups:";
    for(auto& g : m_read_groups)
      g.print(SWC_LOG_OSTREAM << "\n\t", this);
  );

  hdlr->scan(err, std::move(specs));
  if(!err) {
    hdlr->wait();
    err = hdlr->state_error.load();
  }
  SWC_PRINT << " Deleted Cells: " << deleted_count << SWC_PRINT_CLOSE;
  return err ? error(m_message) : true;
}



}}} // namespace Utils::shell
