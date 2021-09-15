/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/Settings.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/db/client/Clients.h"

#include "swcdb/ranger/RangerEnv.h"


using namespace SWC;


void count_all_cells(size_t num_cells, Ranger::Blocks& blocks) {
  std::cout << " count_all_cells: \n";
  Core::Semaphore sem(1, 1);

  auto req = Ranger::ReqScanTest::make();
  req->spec.flags.max_versions = blocks.range->cfg->cell_versions();
  req->spec.flags.limit = num_cells * blocks.range->cfg->cell_versions();

  req->cb = [&req, &sem](int err) { // , blocks=&blocks
    std::cout << " err=" <<  err
              << "(" << Error::get_text(err) << ") \n" ;
    if(req->cells.size() != req->spec.flags.limit) {
      std::cerr << "all-ver, req->cells.size() != req->spec.flags.limit  \n"
                << " " << req->cells.size()
                << " != "  << req->spec.flags.limit <<"\n";
      exit(1);
    }
    sem.release();
  };

  blocks.scan(req);

  sem.wait_all();

  std::cout << " count_all_cells, OK\n";
}


int run() {

  cid_t cid = 11;
  DB::Schema schema;
  schema.cid = cid;
  schema.col_name = "col-test";
  schema.cell_versions = 2;
  schema.blk_size = 64000000;
  schema.blk_cells = 100000;
  Ranger::ColumnCfg::Ptr col_cfg(new Ranger::ColumnCfg(cid, schema));

  int err = Error::OK;
  int num_cells = 1000000;
  uint32_t versions = 3;

  auto range = std::make_shared<Ranger::Range>(col_cfg, 1);
  Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  SWC_ASSERT(!err);
  range->internal_create_folders(err);
  SWC_ASSERT(!err);

  range->set_state(Ranger::Range::State::LOADED);
  //range->compacting(Ranger::Range::COMPACT_CHECKING);

  { // create logs with temp commitlog
  Ranger::CommitLog::Fragments commitlog(col_cfg->key_seq);
  commitlog.init(range);


  commitlog.print(std::cout << " init:\n", true);
  std::cout << "\n";

  int num_threads = 8;
  Core::Semaphore sem(num_threads, num_threads);

  for(int t=0;t<num_threads;++t) {

    std::thread([t, versions, commitlog = &commitlog, &sem,
                 num=num_cells/num_threads]() {
      std::cout << "thread-adding=" << t
                << " offset=" << t*num
                << " until=" << t*num+num << "\n";
      DB::Cells::Cell cell;
      int64_t rev;
      for(uint32_t v=0;v<versions;++v) {
      for(auto i=t*num;i<t*num+num;++i){

        std::string n = std::to_string(i);

        rev = Time::now_ns();
        cell.flag = DB::Cells::INSERT;
        cell.set_timestamp(rev-1);
        //cell.set_revision(rev);
        cell.set_time_order_desc(false);

        cell.key.free();
        cell.key.add("aFraction1");
        cell.key.add("aFraction2");
        cell.key.add(n);
        cell.key.add("aFraction3");
        cell.key.add("aFraction4");
        cell.key.add("aFraction5");
        //if(num_revs == r)
        //  cell.set_counter(Cells::OP_EQUAL, 0);
        //else
        //  cell.set_counter(0, 1);
        std::string s("A-Data-Value-1234567890-"+n);
        cell.set_value(s.data(), s.length());

        commitlog->add(cell);

        if(!(i % 100000))
          std::cout << "thread-adding=" << t
                    << " progress=" << i << "\n";
      }
      }
      sem.release();
    }).detach();
  }

  sem.wait_all();

  commitlog.commit_finalize();

  commitlog.print(std::cout << " added cell=" << num_cells << ": \n", true);

  std::cout << "\n cells_count=" << commitlog.cells_count() << "\n";
  if((versions == 1 || versions == col_cfg->cell_versions())
      && num_cells*col_cfg->cell_versions() != commitlog.cells_count()) {
    exit(1);
  }
  commitlog.unload();
  std::cout << "\n FINISH CREATE LOG\n\n ";
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));


  ///


  range->init();
  auto& blocks = range->blocks;
  blocks.print(std::cout << "new loading: \n", true);
  std::cout << '\n';
  blocks.cellstores.add(Ranger::CellStore::create_initial(err, range));
  blocks.load(err);
  blocks.print(std::cout << "loaded: \n", true);
  std::cout << '\n';

  int num_chks = 10;
  Core::Semaphore sem2(num_chks, num_chks);
  std::vector<Ranger::ReqScanTest::Ptr> requests(num_chks);
  for(int i = 0;i<num_chks; ++i){

    auto& req = requests[i] = Ranger::ReqScanTest::make();
    req->spec.flags.limit = num_cells;

    req->cb = [&req, &sem2, i](int _err) { // , blocks=&blocks
      std::cout << " chk=" << i
                << " err=" <<  _err << "(" << Error::get_text(_err) << ") \n" ;
      if(req->cells.size() != req->spec.flags.limit) {
        std::cerr << "one-ver, req->cells.size() != req->spec.flags.limit  \n"
                  << " " << req->cells.size()
                  << " !="  << req->spec.flags.limit <<"\n";
        exit(1);
      }
      sem2.release();
    };
    blocks.scan(req);
  }

  blocks.print(std::cout << "scanned blocks: \n", true);
  std::cout << '\n';

  sem2.wait_all();
  requests.clear();

  count_all_cells(num_cells, blocks);

  std::cout << " scanned blocks, OK\n";



  std::cout << "blocks.add_logged: \n";

  blocks.print(std::cout << "adding to block: \n", true);
  std::cout << '\n';

  int added_num = 1000000;
  DB::Cells::Cell cell;
  int64_t rev;
  for(uint32_t v=0;v<versions;++v) {
    for(auto i=0;i<added_num;++i){

        std::string n = std::to_string(i)+"-added";

        rev = Time::now_ns();
        cell.flag = DB::Cells::INSERT;
        cell.set_timestamp(rev-1);
        //cell.set_revision(rev);
        cell.set_time_order_desc(false);

        cell.key.free();
        cell.key.add("aFraction1");
        cell.key.add("aFraction2");
        cell.key.add(n);
        cell.key.add("aFraction3");
        cell.key.add("aFraction4");
        cell.key.add("aFraction5");
        //if(num_revs == r)
        //  cell.set_counter(Cells::OP_EQUAL, 0);
        //else
        //  cell.set_counter(0, 1);
        std::string s("A-Data-Value-1234567890-"+n);
        cell.set_value(s.data(), s.length());

        blocks.add_logged(cell);
    }
    blocks.print(std::cout << " add_logged ver=" << v << " : \n", true);
    std::cout << '\n';
  }


  count_all_cells(num_cells+added_num, blocks);

  blocks.print(std::cout << " scanned blocks (add_logged): \n", true);
  std::cout << '\n';
  std::cerr << " scanned blocks (add_logged), OK\n";

  range->internal_remove(err);
  SWC_ASSERT(!err);

  Env::FsInterface::interface()->rmdir(
    err, DB::RangeBase::get_column_path(range->cfg->cid));

  std::cerr << " range use-count=" << range.use_count() << '\n';
  return 0;
}


int main(int argc, char** argv) {
  Env::Config::init(argc, argv);

  Env::FsInterface::init(
    Env::Config::settings(),
    FS::fs_type(Env::Config::settings()->get_str("swc.fs"))
  );

  Env::Clients::init(
    client::Clients::make(
      *Env::Config::settings(),
      Comm::IoContext::make("Clients", 8),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
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
