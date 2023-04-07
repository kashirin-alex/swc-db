/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


// THE DATA IN THIS EXAMPLE IS NOT REAL WHILE MIGHT HAVE REALISTIC PROPORTIONS



#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Sync.h"
#include "swcdb/common/Stats/FlowRate.h"

#include <random>


/*
### [height, degC, Density, Electronegativity, Thermal Conductivity, latitude, longitude ]

# SQL - for execution with bin/swcdb shell #
#-------------------------------------------
select where col("geospatial_analysis-xyz_and_properties") = (
  cells=(
    [>="67000", >"5020", >="100",   >="",  >="",  >="20",   >=""]
      <=key<=
    [<="69000", >="",    <="1000",  >="",  >="",  <="160",  >=""]
    limit=10
  )
) DISPLAY_STATS;
#-------------------------------------------

#### Height:
	from:
		67000 => (67000/10) - 6371  = 329.0 (meters above sea-level)
	to:
		69000 => (69000/10) - 6371  = 529.0 (meters above sea-leve)

#### Degree Celsius:
	from:
		5020  => (5020-5000) / 1000  = 0.02 (degC)
	to:
		above any

#### Density:
	from:
		100    => 100/1000   = 0.1 (density)
	to:
		1000   => 1000/1000  = 1.0 (density)
		
#### Electronegativity:
	from:
		any
	to:
		any
		
#### Thermal Conductivity:
	from:
		any
	to:
		any
		
#### latitude:
	from:
		20    => 180 - 20  - 90  = 70d  (20d below north pole)
	to:
		160   => 180 - 160 - 90  = -70d (20d above south pole)
		
#### Longitude:
	from:
		any
	to:
		any
*/





namespace Examples {

SWC::DB::Schema::Ptr create_column();


// example with numbers as string (opt. binary of decimal value )

const uint32_t deg_precision_base  = 1;      // ( 180*1 * 360*1 ) = 64800

const uint32_t distance_precision_base = 10;  // ( 2000*10 )      = 20000
/// total cells = 64800 * (20000 + 1)

const uint32_t density_precision_base = 1000;

const uint32_t degree_c_precision_base = 1000;

const uint32_t enegativity_precision_base = 1000;

const uint32_t tc_precision_base = 1000;  // Thermal Conductivity


// [height, degC, Density, Electronegativity, Thermal Conductivity, latitude, longitude ]
// (value => image-id)

void generate_sample_data() {

  size_t added_cells = 0;
  size_t added_bytes = 0;
  size_t resend_cells = 0;
  SWC::Time::Measure_ns t_measure;
  SWC::Time::Measure_ns t_progress;

  SWC::DB::Cells::Cell cell;
  cell.flag = SWC::DB::Cells::INSERT;
  cell.set_time_order_desc(true);


  size_t lat_max = deg_precision_base * 179;
  size_t lng_max = deg_precision_base * 359 + deg_precision_base - 1;

  size_t height_start = distance_precision_base * (6371-1371); // from center of the earth
  size_t height_end   = height_start + distance_precision_base * 2000;

  std::random_device rd;
  std::mt19937 gen(rd());

  // random Density range
  std::uniform_int_distribution<> density_rand(0, 40 * density_precision_base);
  // random Electronegativity range
  std::uniform_int_distribution<> enegativity_rand(0, 5 * enegativity_precision_base);
  // random DegreeC range
  std::uniform_int_distribution<> degree_c_rand(0, 1000 * degree_c_precision_base);
  // 5000 => zero level
  // random Thermal Conductivity range
  std::uniform_int_distribution<> tc_rand(0, 2000 * tc_precision_base);
  // 1000 => zero level

  size_t img_id = 0;
  size_t density;
  size_t enegativity;
  size_t deg_c;
  size_t tc;

  SWC::Core::Vector<std::string> fractions;
  fractions.resize(7);


  auto hdlr = SWC::client::Query::Update::Handlers::Common::make(
    SWC::Env::Clients::get());
  auto schema = create_column();
  auto& col = hdlr->create(schema);

  for(size_t height=height_start; height<=height_end; ++height) {

    for(size_t lat=0; lat<=lat_max; ++lat) {

      for(size_t lng=0; lng<=lng_max; ++lng) {

        density = density_rand(gen);
        enegativity = enegativity_rand(gen);
        deg_c = degree_c_rand(gen);
        tc = tc_rand(gen);

        cell.key.free();
        // [height, degC, Density, Electronegativity, Thermal Conductivity, latitude, longitude ]

        fractions[0] = std::to_string(height);
        fractions[1] = std::to_string(deg_c);
        fractions[2] = std::to_string(density);
        fractions[3] = std::to_string(enegativity);
        fractions[4] = std::to_string(tc);
        fractions[5] = std::to_string(lat);
        fractions[6] = std::to_string(lng);
        cell.key.add(fractions);

        // value => image-id
        auto v = std::to_string(++img_id);
        cell.set_value(v, false);
        col->add(cell);

        hdlr->commit_or_wait(col.get());

        added_bytes += cell.encoded_length();
        if(!(++added_cells % 100000)) {
          SWC_PRINT
            << "progress cells=" << added_cells
            << " avg=" << (t_progress.elapsed() / 100000)
            << "ns/cell";
          hdlr->profile.print(SWC_LOG_OSTREAM << ' ');
          SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
          SWC_PRINT << cell.to_string() << SWC_PRINT_CLOSE;
          t_progress.restart();
        }
      }
      resend_cells += hdlr->get_resend_count();

    }

  }

  hdlr->commit_if_need();
  hdlr->wait();

  resend_cells += hdlr->get_resend_count();
  SWC_ASSERT(added_cells && added_bytes);

  SWC::Common::Stats::FlowRate::Data rate(added_bytes, t_measure.elapsed());
  SWC_PRINT << std::endl << std::endl;
  rate.print_cells_statistics(SWC_LOG_OSTREAM, added_cells, resend_cells);
  hdlr->profile.display(SWC_LOG_OSTREAM);
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}


SWC::DB::Schema::Ptr create_column() {
  auto schema = SWC::DB::Schema::make();

  schema->col_name = "geospatial_analysis-xyz_and_properties";
  schema->col_seq = SWC::DB::Types::KeySeq::VOLUME;
  schema->col_type = SWC::DB::Types::Column::PLAIN;
  schema->cell_versions = 1;
  schema->cell_ttl = 0;
  schema->cs_size = 209715200;
  /* by default ranger-cfg
  schema->blk_encoding = SWC::DB::Types::Encoder::ZSTD;
  schema->blk_size = 33554432;
  schema->blk_cells = 100000;
  schema->cs_replication = 1;
  schema->cs_max = 2;
  schema->log_rollout_ratio = 10;
  schema->compact_percent = 33;
  */

  // CREATE COLUMN
  int err = SWC::Error::OK;
  SWC::Comm::Protocol::Mngr::Req::ColumnMng_Sync::request(
    SWC::Comm::Protocol::Mngr::Params::ColumnMng::Function::CREATE,
    schema,
    10000,
    SWC::Env::Clients::get(),
    err
  );
  SWC_ASSERT(!err);

  schema = SWC::Env::Clients::get()->get_schema(err, schema->col_name);
  SWC_ASSERT(!err);
  SWC_ASSERT(schema);
  SWC_ASSERT(schema->cid);
  return schema;
}



void init_app_options(SWC::Config::Settings* settings) {
  SWC::Config::init_comm_options(settings);
  SWC::Config::init_client_options(settings);
}


}


int main(int argc, char** argv) {

  SWC::Env::Config::init(argc, argv, &Examples::init_app_options, nullptr);

  SWC::Env::Clients::init(
    SWC::client::Clients::make(
      *SWC::Env::Config::settings(),
      SWC::Comm::IoContext::make("Clients", 8),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
    )->init()
  );

  Examples::generate_sample_data();

  SWC::Env::Clients::get()->stop();

  return 0;
}