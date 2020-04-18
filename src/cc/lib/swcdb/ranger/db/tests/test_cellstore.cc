/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/ranger/Settings.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/core/sys/Resources.h"

#include "swcdb/ranger/RangerEnv.h"

#include <iostream>


size_t num_cellstores = 9;
size_t num_cells = 1000000;
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
 
  if(key.get_string(0).compare("") == 0) {
    std::cout << "BAD: " << key.to_string() << "\n";
    exit(1);
  }
}

void read_cs(int id, SWC::Ranger::RangePtr range, 
             int expected_blocks, 
             const SWC::DB::Cell::Key& expected_key);

size_t write_cs(int id, SWC::Ranger::RangePtr range, int any) {
  int err = SWC::Error::OK;

  SWC::Ranger::CellStore::Write cs_writer(
    id, range->get_path_cs(id), 
    range->cfg->cell_versions(), range->cfg->block_enc()
  );
  cs_writer.create(err);
  hdlr_err(err);

  SWC::DynamicBuffer buff;
  Cells::Interval blk_intval;
  Cells::Cell cell;

  uint32_t cell_count = 0;
  int64_t rev;
  int expected_blocks = 0;
  SWC::DB::Cell::Key expected_key;

  for(auto i=1; i<=num_cells; ++i) {

    for(int g=1; g<=group_fractions; ++g) {
      

      rev = SWC::Time::now_ns();
      cell.flag = Cells::INSERT;
      cell.set_timestamp(rev-1);
      //cell.set_revision(rev);
      cell.set_time_order_desc(false);

      std::string idn = std::to_string(id);
      std::string n = std::to_string(i);
      std::string gn = std::to_string(g);
      apply_key(idn, n, gn, cell.key);

      std::string v("A-Data-Value-1234567890-"+idn+":"+n+":"+gn);
      cell.set_value(v);

      cell.write(buff);
      ++cell_count;
      blk_intval.expand(cell);   
      if(blk_intval.key_begin.get_string(0).compare("") == 0) {
        std::cout << cell.to_string() << "\n";
        std::cout << "expand: " << blk_intval.to_string() << "\n";
        exit(1);
      }


      if(num_cells == i && group_fractions == g)
        expected_key.copy(cell.key);

      if(buff.fill() >= range->cfg->block_size() 
        || cell_count >= range->cfg->block_cells() 
        || (num_cells == i && group_fractions == g)) {

        ++expected_blocks;
        std::cout << "adding   block:"
                  << " cell_count=" << cell_count
                  << " buff.fill()=" << buff.fill()
                  << " cell_count=" << cell_count
                  << " num-blk=" << expected_blocks
                  << std::flush;
        if(any == -1 && expected_blocks == 1)
          blk_intval.key_begin.free();

        else if(any == 1 && i == num_cells && group_fractions == g)
          blk_intval.key_end.free();

        cs_writer.block(err, blk_intval, buff, cell_count);
        blk_intval.free();
        buff.free();
        hdlr_err(err);

        cell_count = 0;
        std::cout << " cs-size=" << cs_writer.size << "\n";
      }
    }
  }

  cs_writer.finalize(err);
  std::cout << "cs-wrote:    " << cs_writer.to_string() << "\n";
  hdlr_err(err);

  std::cout << "\n  OK-wrote cs-id=" << id << "\n\n";

  // CHECK SINGLE CS-READ
  read_cs(id, range, expected_blocks, expected_key);
  std::cout << "\n  OK-read  cs-id=" << id << "\n\n";
  return expected_blocks;
}

void read_cs(int id, SWC::Ranger::RangePtr range, 
             int expected_blocks, 
             const SWC::DB::Cell::Key& expected_key) {
  int err = SWC::Error::OK;  
  
  SWC::DB::Cells::Interval intval_r;
  SWC::Ranger::Blocks blocks;
  blocks.init(range);
  blocks.cellstores.add(
    SWC::Ranger::CellStore::Read::make(err, id, range, intval_r, true));

  hdlr_err(err);

  if(blocks.cellstores.blocks_count() != expected_blocks) {
    std::cerr << "ERROR: .cellstores.blocks_count() != expected_blocks \n" 
              << " expected=" << expected_blocks << "\n"
              << " counted=" << blocks.cellstores.blocks_count() << "\n";
    exit(1);
  }

  std::cout << blocks.to_string() << "\n";

  auto req = SWC::Ranger::ReqScanTest::make();
  req->spec.flags.max_versions = 2;
  req->cells.reset(req->spec.flags.max_versions, 0, SWC::Types::Column::PLAIN);
  req->spec.flags.limit = num_cells*group_fractions;
  
  std::promise<void> r_promise;
  req->cb = [req, &blocks, expected_key, 
             await=&r_promise, took=SWC::Time::now_ns()]
    (int err) {
    std::cout << " took=" <<  SWC::Time::now_ns()-took << " " << req->cells.to_string() << "\n" ;
    if(err) {
      std::cout << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") " ;
      std::cout << req->to_string() << "\n";
    }
  
    if(req->cells.size() != req->spec.flags.limit) {
      std::cerr << "ERROR: req->cells.size()=" << req->cells.size() 
                << " expected=" << req->spec.flags.limit << "\n\n"
                << blocks.to_string() << "\n";
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
  req->cb = 0; // release cross-ref of req.

  blocks.unload();
}

int main(int argc, char** argv) {

  SWC::Env::Config::init(argc, argv);
  SWC::Env::FsInterface::init(SWC::FS::fs_type(
    SWC::Env::Config::settings()->get_str("swc.fs")));

  
  SWC::Env::IoCtx::init(8);

  SWC::Env::Resources.init(
    SWC::Env::IoCtx::io()->ptr(),
    SWC::Env::Config::settings()->get<SWC::Property::V_GINT32>(
      "swc.rgr.ram.percent"),
    SWC::Env::Config::settings()->get<SWC::Property::V_GINT32>(
      "swc.rgr.ram.release.rate")
  );

  SWC::RangerEnv::init();
  
  auto cid = 11;
  SWC::DB::Schema schema;
  schema.cid = cid;
  schema.col_name = "col-test-cs";
  schema.cell_versions = 1;
  schema.blk_size = 64000000;
  schema.blk_cells = 100000;
  schema.blk_encoding = SWC::Types::Encoding::SNAPPY;
  SWC::Ranger::ColumnCfg col_cfg(cid, schema);
  
  int err = SWC::Error::OK;

  auto range = std::make_shared<SWC::Ranger::Range>(&col_cfg, 1);
  SWC::Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  SWC::Env::FsInterface::interface()->mkdirs(
    err, range->get_path(range->CELLSTORES_DIR));

  size_t expected_blocks = 0;
  for(auto i=1; i<=num_cellstores; ++i) {
    expected_blocks += write_cs(
      i, range, 
      i == 1 ? -1 : (i == num_cellstores ? 1 : 0) // -1:first 1:last
    );
  }


  std::cout << "\n cellstores-scan:\n";
  
  err = SWC::Error::OK;  
  
  SWC::Ranger::Blocks blocks;
  blocks.init(range);
  blocks.cellstores.load_from_path(err);

  hdlr_err(err);

  std::cout << blocks.to_string() << "\n";
  
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
  size_t id = 0;
  for(int n=1; n<=10; ++n) {
    ++id;
    
    threads.push_back(new std::thread(
      [&blocks, match_on_offset, &match_key, id] () {

      auto req = SWC::Ranger::ReqScanTest::make();
      req->spec.flags.max_versions = 2;
      req->cells.reset(req->spec.flags.max_versions, 0, SWC::Types::Column::PLAIN);
      req->spec.flags.offset = match_on_offset;
      req->offset = req->spec.flags.offset;
      req->spec.flags.limit = 1;
      req->cb = [req, id, match_key, &blocks, took=SWC::Time::now_ns()](int err){

        std::cout << " chk=" << id ;
        std::cout << " took=" <<  SWC::Time::now_ns()-took << "\n" ;

        if(err) {
          std::cout << " err=" <<  err 
                    << "(" << SWC::Error::get_text(err) << ") "
                    << req->to_string() << "\n";
        }

        if(req->cells.size() != req->spec.flags.limit) {
          std::cout << "\n" << blocks.to_string() << "\n";
          std::cout << " err=" <<  err 
                    << "(" << SWC::Error::get_text(err) << ")\n";
          std::cerr << "ERROR: req->cells.size()=" << req->cells.size() 
                    << " expected=" << req->spec.flags.limit << " \n"
                    << req->to_string() << "\n";
          exit(1);
        }

        auto cell = req->cells.back();
        if(!cell->key.equal(match_key)) {
          std::cout << "\n" << blocks.to_string() << "\n";

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
    
  blocks.remove(err);

  hdlr_err(err);

  SWC::Env::FsInterface::interface()->rmdir(
    err, SWC::DB::RangeBase::get_column_path(range->cfg->cid));

  std::cout << "\n-   OK   -\n\n";
  exit(0);
}