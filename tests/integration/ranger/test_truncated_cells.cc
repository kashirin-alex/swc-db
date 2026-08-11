/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/Settings.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/db/client/Clients.h"

#include "swcdb/ranger/RangerEnv.h"


using namespace SWC;


static void fill_cell(DB::Cells::Cell& cell, const std::string& n) {
  cell.flag = DB::Cells::INSERT;
  cell.set_timestamp(Time::now_ns() - 1);
  cell.set_time_order_desc(false);
  cell.key.free();
  cell.key.add("f1");
  cell.key.add(n);
  cell.key.add("f3");
  std::string v("value-" + n);
  cell.set_value(v.data(), v.length());
}


int run() {
  cid_t cid = 11;
  DB::Schema schema;
  schema.cid = cid;
  schema.col_name = "col-test-truncated-cells";
  schema.cell_versions = 1;
  schema.blk_size = 640000;
  schema.blk_cells = 10000;
  Ranger::ColumnCfg::Ptr col_cfg(new Ranger::ColumnCfg(cid, schema));

  int err = Error::OK;
  auto range = std::make_shared<Ranger::Range>(col_cfg, 1);
  Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  SWC_ASSERT(!err);
  range->internal_create_folders(err);
  SWC_ASSERT(!err);
  range->set_state(Ranger::Range::State::LOADED);

  Ranger::Blocks blocks(col_cfg->key_seq);
  blocks.init(range);

  DynamicBuffer cells_buf;
  DB::Cells::Cell cell;
  DB::Cells::Interval interval(col_cfg->key_seq);
  const uint32_t cells_count = 3;
  for(uint32_t i = 0; i < cells_count; ++i) {
    fill_cell(cell, std::to_string(i));
    cell.write(cells_buf);
    interval.expand(cell);
  }
  SWC_ASSERT(cells_buf.fill() > 8);

  // Cut mid-payload so Cell::read throws SERIALIZATION_INPUT_OVERRUN
  size_t truncated_sz = cells_buf.fill() / 2;
  cells_buf.ptr = cells_buf.base + truncated_sz;

  StaticBuffer buff_write;
  auto frag = Ranger::CommitLog::Fragment::make_write(
    err = Error::OK,
    range->get_path("log") + "/truncated.frag",
    std::move(interval),
    DB::Types::Encoder::PLAIN,
    col_cfg->cell_versions(),
    cells_count,
    cells_buf,
    buff_write
  );
  SWC_ASSERT(!err);
  SWC_ASSERT(frag);

  {
    DB::Cells::MutableVec vec(
      col_cfg->key_seq, col_cfg->block_cells(),
      col_cfg->cell_versions(), 0, col_cfg->column_type()
    );
    frag->load_cells(err = Error::OK, vec);
    if(err != Error::SERIALIZATION_INPUT_OVERRUN) {
      std::cerr << "Fragment::load_cells expected SERIALIZATION_INPUT_OVERRUN"
                << " got=" << err << "(" << Error::get_text(err) << ")\n"
                << " vec.size=" << vec.size() << '\n';
      exit(1);
    }
    std::cout << "Fragment::load_cells truncated -> err="
              << Error::get_text(err) << ", OK\n";
  }

  // Rebuild a truncated buffer for Block::load_cells
  DynamicBuffer cells_buf2;
  for(uint32_t i = 0; i < cells_count; ++i) {
    fill_cell(cell, std::to_string(i));
    cell.write(cells_buf2);
  }
  truncated_sz = cells_buf2.fill() / 2;

  auto blk = Ranger::Block::make(
    DB::Cells::Interval(col_cfg->key_seq), blocks.ptr()
  );
  bool was_splitted = false;
  err = Error::OK;
  size_t added = blk->load_cells(
    err, cells_buf2.base, truncated_sz,
    col_cfg->cell_versions(), cells_count,
    was_splitted
  );
  delete blk;

  if(err != Error::SERIALIZATION_INPUT_OVERRUN) {
    std::cerr << "Block::load_cells expected SERIALIZATION_INPUT_OVERRUN"
              << " got=" << err << "(" << Error::get_text(err) << ")\n"
              << " added=" << added << '\n';
    exit(1);
  }
  std::cout << "Block::load_cells truncated -> err="
            << Error::get_text(err) << " added=" << added << ", OK\n";

  blocks.unload();
  range->internal_remove(err = Error::OK);
  Env::FsInterface::interface()->rmdir(
    err, DB::RangeBase::get_column_path(range->cfg->cid));
  return 0;
}


int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv, &SWC::Config::init_app_options, nullptr);

  Env::FsInterface::init(
    Env::Config::settings(),
    FS::fs_type(Env::Config::settings()->get_str("swc.fs"))
  );

  Env::Clients::init(
    client::Clients::make(
      *Env::Config::settings(),
      Comm::IoContext::make("Clients", 8),
      nullptr,
      nullptr
    )->init()
  );

  Env::Rgr::init();
  Env::Rgr::start();

  int s = run();

  Env::Rgr::shuttingdown();
  Env::Rgr::wait_if_in_process();

  Env::Clients::get()->stop();
  Env::FsInterface::interface()->stop();
  Env::Rgr::io()->stop();

  if(Env::Rgr::metrics_track())
    Env::Rgr::metrics_track()->wait();

  Env::Rgr::reset();
  Env::Clients::reset();
  Env::FsInterface::reset();
  Env::Config::reset();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::cout << "\n-   OK   -\n\n";
  return s;
}
