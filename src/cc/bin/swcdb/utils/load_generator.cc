/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/db/client/Clients.h"

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Sync.h"

#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Update/Committer.h"

#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/db/client/Query/Select/Handlers/Common.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"

#include "swcdb/common/Stats/FlowRate.h"

#include <random>



namespace SWC {


namespace Utils {


/**
 * @brief The SWC-DB Load-Generator C++ namespace 'SWC::Utils::LoadGenerator'
 *
 * \ingroup Applications
 */
namespace LoadGenerator {


// Distrbution Sequence

enum Distrib : uint8_t {
  SEQUENTIAL = 0,
  STEPPING   = 1,
  UNIFORM    = 2
};

int from_string_distrib(const std::string& typ) {
  switch(typ.length()) {
    case 1: {
      switch(*typ.data()) {
        case '0':
          return Distrib::SEQUENTIAL;
        case '1':
          return Distrib::UNIFORM;
        case '2':
          return Distrib::STEPPING;
        default:
          SWC_THROW(Error::CONFIG_BAD_VALUE, "Unrecognized Distribution");
      }
      break;
    }
    default: {
      if(Condition::str_case_eq(typ.data(), "SEQUENTIAL", 10))
        return Distrib::SEQUENTIAL;
      if(Condition::str_case_eq(typ.data(),"UNIFORM", 7))
        return Distrib::UNIFORM;
      if(Condition::str_case_eq(typ.data(),"STEPPING", 8))
        return Distrib::STEPPING;
      break;
    }
  }
  SWC_THROW(Error::CONFIG_BAD_VALUE, "Unrecognized Distribution");
}

std::string repr_distrib(int typ) {
  switch(typ) {
    case 0:
      return "SEQUENTIAL";
    case 1:
      return "UNIFORM";
    case 2:
      return "STEPPING";
  }
  SWC_THROW(Error::CONFIG_BAD_VALUE,"Unrecognized Distribution");
}

// Distrbution Course
enum DistribCourse : uint8_t {
  STEP      = 0,
  R_STEP    = 1,
  SINGLE    = 2,
  R_SINGLE  = 3,
  LEVELS    = 4,
  R_LEVELS  = 5
};

int from_string_distrib_course(const std::string& typ) {
  switch(typ.length()) {
    case 1: {
      switch(*typ.data()) {
        case '0':
          return DistribCourse::STEP;
        case '1':
          return DistribCourse::R_STEP;
        case '2':
          return DistribCourse::SINGLE;
        case '3':
          return DistribCourse::R_SINGLE;
        case '4':
          return DistribCourse::LEVELS;
        case '5':
          return DistribCourse::R_LEVELS;
        default:
          SWC_THROW(
            Error::CONFIG_BAD_VALUE, "Unrecognized Distribution Course");
      }
      break;
    }
    default: {
      if(Condition::str_case_eq(typ.data(), "STEP", 4))
        return DistribCourse::STEP;
      if(Condition::str_case_eq(typ.data(),"R_STEP", 6))
        return DistribCourse::R_STEP;
      if(Condition::str_case_eq(typ.data(),"SINGLE", 6))
        return DistribCourse::SINGLE;
      if(Condition::str_case_eq(typ.data(),"R_SINGLE", 8))
        return DistribCourse::R_SINGLE;
      if(Condition::str_case_eq(typ.data(),"LEVELS", 6))
        return DistribCourse::LEVELS;
      if(Condition::str_case_eq(typ.data(),"R_LEVELS", 8))
        return DistribCourse::R_LEVELS;
      break;
    }
  }
  SWC_THROW(Error::CONFIG_BAD_VALUE, "Unrecognized Distribution Course");
}

std::string repr_distrib_course(int typ) {
  switch(typ) {
    case 0:
      return "STEP";
    case 1:
      return "R_STEP";
    case 2:
      return "SINGLE";
    case 3:
      return "R_SINGLE";
    case 4:
      return "LEVELS";
    case 5:
      return "R_LEVELS";
  }
  SWC_THROW(Error::CONFIG_BAD_VALUE,"Unrecognized Distribution Course");
}

} } // namespace Utils::LoadGenerator




namespace Config {

void Settings::init_app_options() {
  Property::Value::get_pointer<Property::V_GENUM>(
    cmdline_desc.get_default("swc.logging.level")
  )->set(LOG_ERROR); // default level

  init_comm_options();
  init_client_options();

  cmdline_desc.definition(usage_str(
    "SWC-DB(load_generator) Usage: swcdb_load_generator [options]\n\nOptions:")
  );

  cmdline_desc.add_options()
    ("with-broker", boo(false)->zero_token(),
     "Query applicable requests with Broker")

    ("gen-handlers", i32(8), "Number of client Handlers")

    ("gen-insert", boo(true),   "Generate new data")
    ("gen-select", boo(false),  "Select generated data")
    ("gen-delete", boo(false),  "Delete generated data")

    ("gen-select-empty", boo(false),  "Expect empty select results")
    ("gen-delete-column", boo(false),  "Delete Column after")

    ("gen-progress", i32(100000),
      "display progress every N cells or 0 for quiet")
    ("gen-cell-a-time", boo(false), "Write one cell at a time")

    ("gen-cells", i64(1000),
      "number of cells, total=cells*versions*(is SINGLE?1:fractions*onLevel)")
    ("gen-cells-on-level", i64(1),
      "number of cells, on fraction level")

    ("gen-fractions", i32(10),
      "Number of Fractions per cell key")
    ("gen-fraction-size", i32(10),
      "fraction size in bytes at least")

    ("gen-distrib-seed", i64(1),
      "Use this seed/step for Distribution injection")
    ("gen-distrib-course",
      new Config::Property::V_ENUM(
        int(Utils::LoadGenerator::DistribCourse::STEP),
        Utils::LoadGenerator::from_string_distrib_course,
        Utils::LoadGenerator::repr_distrib_course
      ),
     "Fractions distrib Course STEP|R_STEP|SINGLE|R_SINGLE|LEVELS|R_LEVELS")
    ("gen-distrib",
      new Config::Property::V_ENUM(
        int(Utils::LoadGenerator::Distrib::SEQUENTIAL),
        Utils::LoadGenerator::from_string_distrib,
        Utils::LoadGenerator::repr_distrib
      ),
     "Distribution SEQUENTIAL|STEPPING|UNIFORM")

    ("gen-value-size", i32(256),
      "cell value in bytes or counts for a col-counter")

    ("gen-col-name", str("load_generator-"),
     "Gen. load column name, joins with colm-number")
    ("gen-col-number", i32(1),
      "Number of columns to generate")

    ("gen-col-seq",
      g_enum(
        int(DB::Types::KeySeq::LEXIC),
        nullptr,
        DB::Types::from_string_range_seq,
        DB::Types::repr_range_seq
      ),
     "Schema col-seq FC_+|LEXIC|VOLUME")

    ("gen-col-type",
      g_enum(
        int(DB::Types::Column::PLAIN),
        nullptr,
        DB::Types::from_string_col_type,
        DB::Types::repr_col_type
      ),
     "Schema col-type PLAIN|COUNTER_I{64,32,16,8}|SERIAL")

    ("gen-cell-versions", i32(1), "cell key versions")
    ("gen-cell-encoding",
      g_enum(
        int(DB::Types::Encoder::PLAIN),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding
      ),
     "Cell's Value encoding ZSTD|SNAPPY|ZLIB")

    ("gen-cs-count", i8(0), "Schema cs-count")
    ("gen-cs-size", i32(0), "Schema cs-size")
    ("gen-cs-replication", i8(0), "Schema cs-replication")

    ("gen-blk-size", i32(0), "Schema blk-size")
    ("gen-blk-cells", i32(0), "Schema blk-cells")
    ("gen-blk-encoding",
      g_enum(
        int(DB::Types::Encoder::DEFAULT),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding
      ),
     "Schema blk-encoding NONE|ZSTD|SNAPPY|ZLIB")

    ("gen-log-rollout", i8(0),
     "CommitLog rollout block ratio")
    ("gen-log-compact-cointervaling", i8(0),
     "CommitLog minimal cointervaling Fragments for compaction")
    ("gen-log-preload", i8(0),
     "Number of CommitLog Fragments to preload")

    ("gen-compaction-percent", i8(0),
     "Compaction threshold in % applied over size of either by cellstore or block")
  ;
}

void Settings::init_post_cmd_args() { }

} // namespace Config




namespace Utils { namespace LoadGenerator {


void quit_error(int err) {
  if(!err)
    return;
  SWC_PRINT << "Error " << err << "(" << Error::get_text(err) << ")"
            << SWC_PRINT_CLOSE;
  std::quick_exit(EXIT_FAILURE);
}


struct FractionState {
  FractionState(size_t a_value) : value(a_value) { }
  virtual ~FractionState() { }
  virtual void reset() {
    value = 0;
  }
  virtual void set_value() = 0;
  size_t value;
};

struct FractionStateDistSeq : public FractionState {
  FractionStateDistSeq(size_t a_ncells, bool a_reverse)
    : FractionState(a_reverse ? (a_ncells + 1) : 0),
      ncells(a_ncells), reverse(a_reverse) {
  }
  virtual void set_value() override {
    if(reverse) {
      if(!--value) {
        value = ncells;
      }
    } else {
      if(++value > ncells)
        value = 1;
    }
  }
  const size_t  ncells;
  const bool    reverse;
};

struct FractionStateDistStepping : public FractionState {
  FractionStateDistStepping(size_t a_ncells, size_t a_step, bool a_reverse)
    : FractionState(a_reverse ? (a_ncells + 1) : 0),
      ncells(a_ncells), step(a_step), reverse(a_reverse), offset(0) {
  }

  virtual void reset() override {
    FractionState::reset();
    offset = 0;
  }

  virtual void set_value() override {
    if(reverse) {
      if(value <= step) {
        if(offset == ncells)
          offset = 0;
        value = ncells - offset;
        ++offset;
      } else {
        value -= step;
      }
    } else {
      value += step;
      if(value > ncells) {
        if(++offset > ncells)
          offset = 1;
        value = offset;
      }
    }
  }
  const size_t  ncells;
  const size_t  step;
  const bool    reverse;
  size_t        offset;
};

struct FractionStateDistUniform : public FractionState {
  FractionStateDistUniform(size_t a_ncells, size_t seed)
    : FractionState(0), gen(seed), distrib(1, a_ncells) {
  }
  virtual void set_value() override {
    value = distrib(gen);
  }
  std::mt19937                          gen;
  std::uniform_int_distribution<size_t> distrib;
};



class KeyGenerator {
  public:
  const size_t        ncells;
  const size_t        ncells_onlevel;
  const uint24_t      nfractions;
  const uint24_t      fraction_size;
  const Distrib       distribution;
  const DistribCourse  course;

  KeyGenerator(size_t a_ncells, size_t a_ncells_onlevel,
                uint24_t a_fraction_size, uint24_t a_nfractions,
                Distrib a_distribution, DistribCourse a_course, size_t seed)
              : ncells(a_ncells), ncells_onlevel(a_ncells_onlevel),
                nfractions(a_nfractions), fraction_size(a_fraction_size),
                distribution(a_distribution), course(a_course),
                _ncells(0), _ncells_onlevel(0), _nfractions(0) {
    if(ncells_onlevel > 1 && (
        course == DistribCourse::SINGLE ||
        course == DistribCourse::R_SINGLE ||
        course == DistribCourse::LEVELS ||
        course == DistribCourse::R_LEVELS))
      SWC_THROW(
        Error::CONFIG_BAD_VALUE,
        "Not supported SINGLE|LEVELS Distribution Course with onlevel > 1");

    bool reverse = course == DistribCourse::R_STEP ||
                   course == DistribCourse::R_SINGLE ||
                   course == DistribCourse::R_LEVELS;
    for(uint24_t n=1; n<=nfractions; ++n) {
      switch(distribution) {
        case Distrib::SEQUENTIAL: {
          if(seed > 1) {
            SWC_THROW(Error::CONFIG_BAD_VALUE,
              "Not supported SEQUENTIAL Distribution with seed > 1");
          }
          _fractions_state.emplace_back(
            new FractionStateDistSeq(
              ncells * ncells_onlevel, reverse));
          break;
        }
        case Distrib::STEPPING: {
          if(course == DistribCourse::LEVELS ||
             course == DistribCourse::R_LEVELS) {
            SWC_THROW(Error::CONFIG_BAD_VALUE,
              "Not supported STEPPING Distribution with LEVELS");
          }
          if(ncells_onlevel > 1 && seed > 1 && (
              course == DistribCourse::STEP ||
              course == DistribCourse::R_STEP)) {
            SWC_THROW(Error::CONFIG_BAD_VALUE,
              "Not supported STEP Distribution Course with"
              " onlevel > 1 and step > 1");
          }

          _fractions_state.emplace_back(
            new FractionStateDistStepping(
              ncells * ncells_onlevel, seed, reverse));
          break;
        }
        case Distrib::UNIFORM:
          _fractions_state.emplace_back(
            new FractionStateDistUniform(
              ncells * ncells_onlevel, seed));
          break;
      }
    }
  }

  bool next_n_fraction() {
    switch(course) {
      case DistribCourse::SINGLE:
      case DistribCourse::R_SINGLE: {
        if(++_ncells > ncells)
          return false;
        for(auto& state : _fractions_state)
          state->set_value();
        _nfractions = nfractions;
        return true;
      }

      case DistribCourse::LEVELS: {
        if(++_ncells > ncells || !_nfractions) {
          if(++_nfractions > nfractions)
            return false;
          _ncells = 1;
        }
        for(uint24_t n=0; n<_nfractions;++n)
          _fractions_state[n]->set_value();
        return true;
      }

      case DistribCourse::R_LEVELS: {
        if(!_nfractions)
          _nfractions = nfractions;
        if(++_ncells > ncells) {
          if(!--_nfractions)
            return false;
          _ncells = 1;
        }
        for(auto& state : _fractions_state)
          state->set_value();
        return true;
      }

      case DistribCourse::STEP: {
        if(!_nfractions || ++_ncells_onlevel == ncells_onlevel) {
          if(!_nfractions || ++_nfractions > nfractions) {
            ++_ncells;
            _nfractions = 1;
          }
          _ncells_onlevel = 0;
        }
        _fractions_state[_nfractions - 1]->set_value();
        return _ncells <= ncells;
      }

      case DistribCourse::R_STEP: {
        if(!_nfractions || ++_ncells_onlevel == ncells_onlevel) {
          if(!_nfractions || !--_nfractions) {
            ++_ncells;
            _nfractions = nfractions;
            for(auto& state : _fractions_state)
              state->set_value();
          }
          _ncells_onlevel = 0;
        } else {
          _fractions_state[_nfractions - 1]->set_value();
        }
        return _ncells <= ncells;
      }
    }

    SWC_THROW(Error::CONFIG_BAD_VALUE, "Unrecognized Distribution Course");
  }

  protected:
  size_t                                        _ncells;
  size_t                                        _ncells_onlevel;
  uint24_t                                      _nfractions;
  Core::Vector<std::unique_ptr<FractionState>>  _fractions_state;
};


class KeyGeneratorUpdate : public KeyGenerator {
  public:
  KeyGeneratorUpdate(size_t a_ncells, size_t a_ncells_onlevel,
                     uint24_t a_fraction_size, uint24_t a_nfractions,
                     Distrib a_distribution, DistribCourse a_course, size_t seed)
          : KeyGenerator(
              a_ncells, a_ncells_onlevel,
              a_fraction_size, a_nfractions,
              a_distribution, a_course, seed) {
    _fractions.reserve(nfractions);
  }

  bool next(DB::Cell::Key& key) {
    if(!next_n_fraction())
      return false;

    _fractions.resize(_nfractions);
    size_t fn = 0;
    for(auto& fraction : _fractions) {
      fraction = std::to_string(_fractions_state[fn]->value);
      if(fraction_size > fraction.length())
        fraction.insert(0, fraction_size - fraction.length(), '0');
      ++fn;
    }
    key.free();
    key.add(_fractions);
    _fractions.clear();
    return true;
  }

  private:
  Core::Vector<std::string>    _fractions;
};


class KeyGeneratorSelect : public KeyGenerator {
  public:
  KeyGeneratorSelect(size_t a_ncells, size_t a_ncells_onlevel,
                     uint24_t a_fraction_size, uint24_t a_nfractions,
                     Distrib a_distribution, DistribCourse a_course, size_t seed)
          : KeyGenerator(
              a_ncells, a_ncells_onlevel,
              a_fraction_size, a_nfractions,
              a_distribution, a_course, seed) {
  }

  bool next(DB::Specs::Key& key) {
    if(!next_n_fraction())
      return false;

    key.clear();
    key.resize(_nfractions);
    size_t fn = 0;
    for(auto& fraction : key) {
      fraction.comp = Condition::EQ;
      fraction = std::to_string(_fractions_state[fn]->value);
      if(fraction_size > fraction.length())
        fraction.insert(0, fraction_size - fraction.length(), '0');
      ++fn;
    }
    return true;
  }

};




void update_data(const DB::SchemasVec& schemas, uint8_t flag, size_t seed) {
  auto settings = Env::Config::settings();

  uint32_t versions = flag == DB::Cells::INSERT
    ? settings->get_i32("gen-cell-versions")
    : 1;

  uint32_t nfractions = settings->get_i32("gen-fractions");
  uint32_t fraction_size = settings->get_i32("gen-fraction-size");

  uint64_t ncells = settings->get_i64("gen-cells");
  uint64_t ncells_onlevel = settings->get_i64("gen-cells-on-level");

  Distrib distribution = Distrib(settings->get_enum("gen-distrib"));
  DistribCourse course = DistribCourse(settings->get_enum("gen-distrib-course"));

  uint32_t value = flag == DB::Cells::INSERT
    ? settings->get_i32("gen-value-size")
    : 1;

  uint32_t progress = settings->get_i32("gen-progress");
  bool cellatime = settings->get_bool("gen-cell-a-time");

  auto cell_encoder = DB::Types::Encoder(settings->get_genum(
    "gen-cell-encoding"));

  client::Query::Update::Handlers::Base::Colms colms;

  auto hdlr = client::Query::Update::Handlers::Common::make(
    Env::Clients::get(),
    nullptr,
    nullptr,
    settings->get_bool("with-broker")
      ? client::Clients::BROKER
      : client::Clients::DEFAULT
  );
  colms.reserve(schemas.size());
  for(auto& schema : schemas)
    colms.push_back(hdlr->create(schema).get());

  size_t added_count = 0;
  size_t resend_cells = 0;
  size_t added_bytes = 0;
  DB::Cells::Cell cell;
  cell.flag = flag;
  cell.set_time_order_desc(true);


  bool is_counter = DB::Types::is_counter(schemas.front()->col_type);
  bool is_serial = schemas.front()->col_type == DB::Types::Column::SERIAL;
  std::string value_data;
  if(flag == DB::Cells::INSERT && !is_counter) {
    uint8_t c=122;
    for(uint32_t n=0; n<value;++n)
      value_data += char(c == 122 ? c = 97 : ++c);
  }

  Time::Measure_ns t_measure;
  Time::Measure_ns t_progress;

  for(uint32_t v=0; v<versions; ++v) {
    for(uint32_t count=is_counter ? value : 1; count > 0; --count) {

      KeyGeneratorUpdate key_gen(
        ncells, ncells_onlevel,
        fraction_size, nfractions,
        distribution, course, seed
      );
      while(key_gen.next(cell.key)) {
        if(flag == DB::Cells::INSERT) {
          if(is_counter) {
            cell.set_counter(0, 1, schemas.front()->col_type);

          } else if(is_serial) {

            DB::Cell::Serial::Value::FieldsWriter wfields;
            wfields.ensure(value_data.size() * 10);
            auto t = DB::Cell::Serial::Value::Type::INT64;
            for(auto it= value_data.cbegin(); it != value_data.cend(); ++it) {
              if(t == DB::Cell::Serial::Value::Type::INT64) {
                wfields.add(int64_t(*it));
                t = DB::Cell::Serial::Value::Type::DOUBLE;
              } else if(t == DB::Cell::Serial::Value::Type::DOUBLE) {
                long double v_d(*it);
                wfields.add(v_d);
                t = DB::Cell::Serial::Value::Type::BYTES;
              } else if(t == DB::Cell::Serial::Value::Type::BYTES) {
                const uint8_t c = *it;
                wfields.add(&c, 1);
                t = DB::Cell::Serial::Value::Type::INT64;
              }
            }

            if(cell_encoder != DB::Types::Encoder::PLAIN) {
              cell.set_value(cell_encoder, wfields.base, wfields.fill());
            } else {
              cell.set_value(wfields.base, wfields.fill(), true);
            }

          } else if(cell_encoder != DB::Types::Encoder::PLAIN) {
            cell.set_value(cell_encoder, value_data);
          } else {
            cell.set_value(value_data);
          }
        }

        for(auto& col : colms) {
          col->add(cell);

          ++added_count;
          added_bytes += cell.encoded_length();
          if(cellatime) {
            hdlr->commit(col);
            hdlr->wait();
          } else {
            hdlr->commit_or_wait();
          }
        }

        if(progress && !(added_count % progress)) {
          SWC_PRINT
            << "update-progress(time_ns=" <<  Time::now_ns()
            << " cells=" << added_count
            << " bytes=" << added_bytes
            << " avg=" << t_progress.elapsed()/progress << "ns/cell) ";
          hdlr->profile.finished();
          hdlr->profile.print(SWC_LOG_OSTREAM);
          SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
          t_progress.restart();
        }
      }
      resend_cells += hdlr->get_resend_count();
    }
  }

  hdlr->commit_if_need();
  hdlr->wait();

  resend_cells += hdlr->get_resend_count();
  SWC_ASSERT(added_count && added_bytes);

  Common::Stats::FlowRate::Data rate(added_bytes, t_measure.elapsed());
  SWC_PRINT << std::endl << std::endl;
  rate.print_cells_statistics(SWC_LOG_OSTREAM, added_count, resend_cells);
  hdlr->profile.display(SWC_LOG_OSTREAM);
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}


void select_data(const DB::SchemasVec& schemas, size_t seed) {
  auto settings = Env::Config::settings();

  bool expect_empty = settings->get_bool("gen-select-empty");

  uint32_t versions = settings->get_i32("gen-cell-versions");
  uint32_t nfractions = settings->get_i32("gen-fractions");
  uint32_t fraction_size = settings->get_i32("gen-fraction-size");

  uint64_t ncells = settings->get_i64("gen-cells");
  uint64_t ncells_onlevel = settings->get_i64("gen-cells-on-level");

  Distrib distribution = Distrib(settings->get_enum("gen-distrib"));
  DistribCourse course = DistribCourse(settings->get_enum("gen-distrib-course"));

  uint32_t progress = settings->get_i32("gen-progress");
  bool cellatime = settings->get_bool("gen-cell-a-time");
  bool with_broker = settings->get_bool("with-broker");

  size_t select_count = 0;
  size_t select_bytes = 0;

  if(DB::Types::is_counter(schemas.front()->col_type))
    versions = 1;

  Time::Measure_ns t_measure;
  Time::Measure_ns t_progress;

  if(cellatime) {
    client::Query::Profiling profiling;
    KeyGeneratorSelect key_gen(
      ncells, ncells_onlevel,
      fraction_size, nfractions,
      distribution, course, seed
    );

    DB::Specs::Key key_spec;
    while(key_gen.next(key_spec)) {

      DB::Specs::Interval intval;
      intval.key_intervals.add().start.move(key_spec);
      intval.set_opt__key_equal();
      intval.flags.limit = versions;

      auto hdlr = client::Query::Select::Handlers::Common::make(
        Env::Clients::get(),
        nullptr, false, nullptr,
        with_broker
          ? client::Clients::BROKER
          : client::Clients::DEFAULT
      );
      for(auto& schema : schemas)
        hdlr->scan(schema, std::move(intval));

      hdlr->wait();
      if(expect_empty) {
        SWC_ASSERT(hdlr->empty());
      } else {
        for(auto& schema : schemas)
          SWC_ASSERT(hdlr->get_size(schema->cid) == versions);
      }

      select_bytes += hdlr->get_size_bytes();
      ++select_count;

      profiling += hdlr->profile;
      if(progress && !(select_count % progress)) {
        SWC_PRINT
          << "select-progress(time_ns=" << Time::now_ns()
          << " cells=" << select_count
          << " avg=" << t_progress.elapsed()/progress << "ns/cell) ";
        hdlr->profile.print(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
        t_progress.restart();
      }
    }
    if(expect_empty)
      select_count = 0;
    profiling.finished();

    Common::Stats::FlowRate::Data rate(select_bytes, t_measure.elapsed());
    SWC_PRINT << std::endl << std::endl;
    rate.print_cells_statistics(SWC_LOG_OSTREAM, select_count, 0);
    profiling.display(SWC_LOG_OSTREAM);
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;

  } else {
    DB::Specs::Scan specs;
    for(auto& schema : schemas) {
      specs.columns.emplace_back(schema->cid).add(schema->col_type);
    }
    auto hdlr = client::Query::Select::Handlers::Common::make(
      Env::Clients::get(),
      [&select_bytes, &select_count, &schemas]
      (const client::Query::Select::Handlers::Common::Ptr& _hdlr) {
        for(auto& schema : schemas) {
          DB::Cells::Result cells;
          _hdlr->get_cells(schema->cid, cells);
          select_count += cells.size();
          select_bytes += cells.size_bytes();
        }
      },
      true,
      nullptr,
      with_broker
        ? client::Clients::BROKER
        : client::Clients::DEFAULT
    );
    int err = Error::OK;
    hdlr->scan(err, std::move(specs));
    SWC_ASSERT(!err);

    hdlr->wait();
    SWC_ASSERT(
      expect_empty
      ? hdlr->empty()
      : select_count == versions * ncells * ncells_onlevel * schemas.size() * (
          course == DistribCourse::SINGLE || course == DistribCourse::R_SINGLE
            ? 1 : nfractions)
    );

    Common::Stats::FlowRate::Data rate(select_bytes, t_measure.elapsed());
    SWC_PRINT << std::endl << std::endl;
    rate.print_cells_statistics(SWC_LOG_OSTREAM, select_count, 0);
    hdlr->profile.display(SWC_LOG_OSTREAM);
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }
}


void make_work_load(const DB::SchemasVec& schemas) {
  auto settings = Env::Config::settings();

  size_t seed = settings->get_i64("gen-distrib-seed");

  if(settings->get_bool("gen-insert"))
    update_data(schemas, DB::Cells::INSERT, seed);

  if(settings->get_bool("gen-select"))
    select_data(schemas, seed);

  if(settings->get_bool("gen-delete"))
    update_data(schemas, DB::Cells::DELETE, seed);

  if(settings->get_bool("gen-delete-column")) {
    int err = Error::OK;
    bool with_broker = Env::Config::settings()->get_bool("with-broker");
    auto func = Comm::Protocol::Mngr::Params::ColumnMng::Function::DELETE;
    for(auto& schema : schemas) {
      with_broker
        ? Comm::Protocol::Bkr::Req::ColumnMng_Sync::request(
            func, schema, 10000, Env::Clients::get(), err)
        : Comm::Protocol::Mngr::Req::ColumnMng_Sync::request(
            func, schema, 10000, Env::Clients::get(), err);
      quit_error(err);
    }
  }

}


void generate() {
  auto settings = Env::Config::settings();

  std::string col_name(settings->get_str("gen-col-name"));

  uint32_t ncolumns(settings->get_i32("gen-col-number"));

  DB::Schemas::SelectorPatterns patterns;
  patterns.names.push_back(DB::Schemas::Pattern(Condition::PF, col_name));

  DB::SchemasVec schemas;
  int err = Error::OK;
  Env::Clients::get()->get_schema(err, patterns, schemas);
  if(schemas.size() == ncolumns) {
    quit_error(err);
    make_work_load(schemas);
    return;
  }
  err = Error::OK;

  for(uint32_t ncol=1; ncol<=ncolumns; ++ncol) {
  auto schema = DB::Schema::make();
  schema->col_name = col_name + std::to_string(ncol);
  schema->col_seq = DB::Types::KeySeq(
    settings->get_genum("gen-col-seq"));
  schema->col_type = DB::Types::Column(
    settings->get_genum("gen-col-type"));
  schema->cell_versions = settings->get_i32("gen-cell-versions");
  schema->cell_ttl = 0;
  schema->blk_encoding = DB::Types::Encoder(
    settings->get_genum("gen-blk-encoding"));
  schema->blk_size = settings->get_i32("gen-blk-size");
  schema->blk_cells = settings->get_i32("gen-blk-cells");
  schema->cs_replication = settings->get_i8("gen-cs-replication");
  schema->cs_size = settings->get_i32("gen-cs-size");
  schema->cs_max = settings->get_i8("gen-cs-count");
  schema->log_rollout_ratio = settings->get_i8("gen-log-rollout");
  schema->log_compact_cointervaling = settings->get_i8(
    "gen-log-compact-cointervaling");
  schema->log_fragment_preload = settings->get_i8("gen-log-preload");

  schema->compact_percent = settings->get_i8("gen-compaction-percent");

  // CREATE COLUMN
  auto func = Comm::Protocol::Mngr::Params::ColumnMng::Function::CREATE;
  Env::Config::settings()->get_bool("with-broker")
    ? Comm::Protocol::Bkr::Req::ColumnMng_Sync::request(
        func, schema, 10000, Env::Clients::get(), err)
    : Comm::Protocol::Mngr::Req::ColumnMng_Sync::request(
        func, schema, 10000, Env::Clients::get(), err);
  quit_error(err);
  }

  schemas.clear();
  Env::Clients::get()->get_schema(err, patterns, schemas);
  if(schemas.size() != ncolumns)
    quit_error(Error::INVALID_ARGUMENT);

  make_work_load(schemas);
}


}}} // namespace SWC::Utils::LoadGenerator



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);

  auto settings = SWC::Env::Config::settings();

  auto io = SWC::Comm::IoContext::make(
    "LoadGenerator",
    settings->get_i32("gen-handlers")
  );

  SWC::Env::Clients::init(
    (settings->get_bool("with-broker")
      ? SWC::client::Clients::make(
          *settings,
          io,
          nullptr  // std::make_shared<client::BrokerContext>()
        )
      : SWC::client::Clients::make(
          *settings,
          io,
          nullptr, // std::make_shared<client::ManagerContext>()
          nullptr  // std::make_shared<client::RangerContext>()
        )
    )->init()
  );

  auto period = settings->get<
    SWC::Config::Property::V_GINT32>("swc.cfg.dyn.period");
  if(period->get()) {
    io->set_periodic_timer(
      period,
      [](){SWC::Env::Config::settings()->check_dynamic_files();}
    );
  }

  SWC::Utils::LoadGenerator::generate();

  SWC_CAN_QUICK_EXIT(EXIT_SUCCESS);
  SWC::Env::Clients::get()->stop();

  std::this_thread::sleep_for(std::chrono::seconds(2));
  SWC::Env::Clients::reset();
  SWC::Env::Config::reset();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}
