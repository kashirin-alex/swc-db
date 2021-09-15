/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/ranger/Settings.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/db/client/Clients.h"

#include "swcdb/ranger/RangerEnv.h"


size_t num_cellstores = 9;
size_t num_cells = 100000;
size_t num_len = 7;
size_t group_fractions = 9; // Xnum_cells = total in a cs

namespace Cells = SWC::DB::Cells;

void hdlr_err(int err){
  if(err) {
    std::cerr << " error=" << err << "(" << SWC::Error::get_text(err) << ")\n";
    exit(1);
  }
}

void apply_key(const std::string& idn,
               std::string n,
               const std::string& gn,
               SWC::DB::Cell::Key& key) {
  key.free();
  key.add("F1");
  key.add("cs"+idn);
  if(num_len > n.size())
    n.insert(0, num_len - n.size(), '0');
  key.add(n);
  key.add("F4");
  key.add("F5");
  key.add("F6");
  key.add(gn);

  if(key.get_string(0).empty()) {
    std::cout << "BAD: " << key.to_string() << "\n";
    exit(1);
  }
}

void read_cs(SWC::csid_t csid, SWC::Ranger::RangePtr range,
             size_t expected_blocks,
             const SWC::DB::Cell::Key& expected_key);

size_t write_cs(SWC::csid_t csid, SWC::Ranger::RangePtr range, int any) {
  int err = SWC::Error::OK;

  SWC::Ranger::CellStore::Write cs_writer(
    csid, range->get_path_cs(csid),
    range, range->cfg->cell_versions(),
    SWC::DB::Cell::Key()
  );
  cs_writer.create(err);
  hdlr_err(err);

  SWC::DynamicBuffer buff(range->cfg->block_size());
  SWC::Ranger::CellStore::Block::Header header(range->cfg->key_seq);
  Cells::Cell cell;

  int64_t rev;
  int expected_blocks = 0;
  SWC::DB::Cell::Key expected_key;

  for(size_t i=1; i<=num_cells; ++i) {

    for(size_t g=1; g<=group_fractions; ++g) {


      rev = SWC::Time::now_ns();
      cell.flag = Cells::INSERT;
      cell.set_timestamp(rev-1);
      //cell.set_revision(rev);
      cell.set_time_order_desc(false);

      std::string idn = std::to_string(csid);
      std::string n = std::to_string(i);
      std::string gn = std::to_string(g);
      apply_key(idn, n, gn, cell.key);

      std::string v("A-Data-Value-1234567890-"+idn+":"+n+":"+gn);
      cell.set_value(v);

      cell.write(buff);
      ++header.cells_count;
      header.interval.expand(cell);
      if(header.interval.key_begin.get_string(0).empty()) {
        std::cout << cell.to_string() << "\n";
        header.interval.print(std::cout << "expand: ");
        std::cout << "\n";
        exit(1);
      }


      if(num_cells == i && group_fractions == g)
        expected_key.copy(cell.key);

      if(buff.fill() >= range->cfg->block_size()
        || header.cells_count >= range->cfg->block_cells()
        || (num_cells == i && group_fractions == g)) {

        ++expected_blocks;
        std::cout << "adding   block:"
                  << " cell_count=" << header.cells_count
                  << " buff.fill()=" << buff.fill()
                  << " cell_count=" << header.cells_count
                  << " num-blk=" << expected_blocks
                  << std::flush;
        if(any == -1 && expected_blocks == 1)
          header.interval.key_begin.free();

        else if(any == 1 && i == num_cells && group_fractions == g)
          header.interval.key_end.free();

        cs_writer.block_encode(err, buff, std::move(header));
        SWC_PRINT << " cs-size=" << cs_writer.size << SWC_PRINT_CLOSE;
        /*        << "Write::block_encode header(";
        header.print(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << ')' << SWC_PRINT_CLOSE; */

        buff.clear();
        hdlr_err(err);

        //header.interval.free();
        header.cells_count = 0;
      }
    }
  }

  cs_writer.finalize(err);
  cs_writer.print(std::cout << "cs-wrote:    ");
  std::cout << "\n";
  hdlr_err(err);

  std::cout << "\n  OK-wrote csid=" << csid << "\n\n";

  // CHECK SINGLE CS-READ
  read_cs(csid, range, expected_blocks, expected_key);
  std::cout << "\n  OK-read  csid=" << csid << "\n\n";
  return expected_blocks;
}

void read_cs(SWC::csid_t csid, SWC::Ranger::RangePtr range,
             size_t expected_blocks,
             const SWC::DB::Cell::Key& expected_key) {
  int err = SWC::Error::OK;

  SWC::Ranger::Blocks blocks(range->cfg->key_seq);
  blocks.init(range);
  blocks.cellstores.add(
    SWC::Ranger::CellStore::Read::make(err, csid, range, true));

  hdlr_err(err);

  if(blocks.cellstores.blocks_count() != expected_blocks) {
    std::cerr << "ERROR: .cellstores.blocks_count() != expected_blocks \n"
              << " expected=" << expected_blocks << "\n"
              << " counted=" << blocks.cellstores.blocks_count() << "\n";
    blocks.cellstores.print(std::cerr, true);
    exit(1);
  }

   blocks.print(std::cout, true);
   std::cout << '\n';

  auto req = SWC::Ranger::ReqScanTest::make();
  req->spec.flags.max_versions = 2;
  req->spec.flags.limit = num_cells*group_fractions;

  std::promise<void> r_promise;
  req->cb = [req, &blocks, expected_key,
             await=&r_promise, took=SWC::Time::now_ns()] (int _err) {
    std::cout << " took=" <<  SWC::Time::now_ns()-took << " ";
    req->cells.print(std::cout);
    std::cout << "\n" ;
    if(_err) {
      std::cout << " err=" <<  _err << "(" << SWC::Error::get_text(_err) << ") " ;
      req->print(std::cout);
      std::cout << "\n";
    }

    if(req->cells.size() != req->spec.flags.limit) {
      std::cerr << "ERROR: req->cells.size()=" << req->cells.size()
                << " expected=" << req->spec.flags.limit << "\n\n";
      blocks.print(std::cerr, true);
      std::cerr << '\n';
      exit(1);
    }

    auto cell = req->cells.back();
    if(!cell->key.equal(expected_key)) {
      std::cerr << "ERROR: !cell.key.equal(expected_key) " << cell->to_string()
                << " expected=" << expected_key.to_string()  << "\n";
      exit(1);
    }

    await->set_value();
  };

  blocks.scan(req);
  r_promise.get_future().wait();
  req->cb = nullptr; // release cross-ref of req.

  blocks.unload();
}

int run() {

  SWC::cid_t cid = 11;
  SWC::DB::Schema schema;
  schema.cid = cid;
  schema.col_name = "col-test-cs";
  schema.cell_versions = 1;
  schema.blk_size = 640000;
  schema.blk_cells = 10000;
  schema.blk_encoding = SWC::DB::Types::Encoder::SNAPPY;
  SWC::Ranger::ColumnCfg::Ptr col_cfg(
    new SWC::Ranger::ColumnCfg(cid, schema));

  int err = SWC::Error::OK;

  auto range = std::make_shared<SWC::Ranger::Range>(col_cfg, 1);
  range->set_state(SWC::Ranger::Range::State::LOADED);
  range->compacting(SWC::Ranger::Range::COMPACT_CHECKING);

  SWC::Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  SWC::Env::FsInterface::interface()->mkdirs(
    err, range->get_path(SWC::DB::RangeBase::CELLSTORES_DIR));
  SWC_ASSERT(!err);

  size_t expected_blocks = 0;
  for(size_t i=1; i<=num_cellstores; ++i) {
    expected_blocks += write_cs(
      i, range,
      i == 1 ? -1 : (i == num_cellstores ? 1 : 0) // -1:first 1:last
    );
  }


  std::cout << "\n cellstores-scan:\n";

  err = SWC::Error::OK;

  SWC::Ranger::Blocks blocks(range->cfg->key_seq);
  blocks.init(range);
  blocks.cellstores.load_from_path(err);

  hdlr_err(err);

  blocks.print(std::cout, true);
  std::cout << '\n';

  if(blocks.cellstores.blocks_count() != expected_blocks) {
    std::cerr << "ERROR: .cellstores.blocks_count() != expected_blocks \n"
              << " expected=" << expected_blocks << "\n"
              << " counted=" << blocks.cellstores.blocks_count() << "\n";
    exit(1);
  }


  size_t match_on_offset = num_cellstores * num_cells * group_fractions - 1;
  SWC::DB::Cell::Key match_key;
  apply_key(
    std::to_string(num_cellstores),
    std::to_string(num_cells),
    std::to_string(group_fractions),
    match_key
  );

  std::vector<std::thread*> threads;
  SWC::csid_t csid = 0;
  for(int n=1; n<=10; ++n) {
    ++csid;

    threads.push_back(new std::thread(
      [&blocks, match_on_offset, &match_key, csid] () {

      auto req = SWC::Ranger::ReqScanTest::make();
      req->spec.flags.max_versions = 2;
      req->spec.flags.offset = match_on_offset;
      req->offset = req->spec.flags.offset;
      req->spec.flags.limit = 1;
      req->cb = [req, csid, match_key, &blocks, took=SWC::Time::now_ns()]
      (int _err){

        std::cout << " chk-csid=" << csid ;
        std::cout << " took=" <<  SWC::Time::now_ns()-took << "\n" ;

        if(_err) {
          std::cout << " err=" <<  _err
                    << "(" << SWC::Error::get_text(_err) << ") ";
          req->print(std::cout);
          std::cout << "\n";
        }

        if(req->cells.size() != req->spec.flags.limit) {
          blocks.print(std::cout << '\n', true);
          std::cout << '\n';
          std::cout << " err=" <<  _err
                    << "(" << SWC::Error::get_text(_err) << ")\n";
          std::cerr << "ERROR: req->cells.size()=" << req->cells.size()
                    << " expected=" << req->spec.flags.limit << " \n";
          req->print(std::cout);
          std::cout << "\n";
          exit(1);
        }

        auto cell = req->cells.back();
        if(!cell->key.equal(match_key)) {
          blocks.print(std::cout << '\n', true);
          std::cout << '\n';

          std::cerr << "ERROR: !cell.key.equal(match_key) " << cell->to_string()
                    << " expected=" << match_key.to_string()  << "\n"
                    << req->spec.to_string() << "\n";
          exit(1);
        }
        req->cb = nullptr;
      };

      blocks.scan(req);
    }));
  }

  while(!threads.empty()) {
    threads.front()->join();
    delete threads.front();
    threads.erase(threads.begin());
  }

  blocks.remove(err); // waiting-completion-here
  hdlr_err(err);

  range->compacting(SWC::Ranger::Range::COMPACT_NONE);
  range->internal_remove(err);
  hdlr_err(err);

  SWC::Env::FsInterface::interface()->rmdir(
    err, SWC::DB::RangeBase::get_column_path(range->cfg->cid));
  return 0;
}



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  SWC::Env::FsInterface::init(
    SWC::Env::Config::settings(),
    SWC::FS::fs_type(SWC::Env::Config::settings()->get_str("swc.fs"))
  );

  SWC::Env::Clients::init(
    SWC::client::Clients::make(
      *SWC::Env::Config::settings(),
      SWC::Comm::IoContext::make("Clients", 8),
      nullptr, // std::make_shared<SWC::client::ManagerContext>()
      nullptr  // std::make_shared<SWC::client::RangerContext>()
    )->init()
  );

  SWC::Env::Rgr::init();


  int s = run();


  SWC::Env::Rgr::shuttingdown();
  SWC::Env::Rgr::wait_if_in_process();

  SWC::Env::Clients::get()->stop();
  SWC::Env::FsInterface::interface()->stop();
  SWC::Env::Rgr::io()->stop();

  if(SWC::Env::Rgr::metrics_track())
    SWC::Env::Rgr::metrics_track()->wait();

  SWC::Env::Rgr::reset();
  SWC::Env::Clients::reset();
  SWC::Env::FsInterface::reset();
  SWC::Env::Config::reset();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::cout << "\n-   OK   -\n\n";
  return s;
}
