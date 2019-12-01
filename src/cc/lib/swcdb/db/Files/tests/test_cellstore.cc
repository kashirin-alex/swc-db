/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/ranger/Settings.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/client/Clients.h"
#include "swcdb/core/Resources.h"
#include "swcdb/db/Columns/Rgr/Columns.h"

#include <iostream>


namespace Cells = SWC::DB::Cells;

void hdlr_err(int err){
  if(err) {
    std::cerr << " error=" << err << "(" << SWC::Error::get_text(err) << ")\n";
    exit(1);
  }
}

int main(int argc, char** argv) {

  SWC::Env::Config::init(argc, argv);
  SWC::Env::FsInterface::init();
  SWC::Env::Schemas::init();

  
  SWC::Env::IoCtx::init(8);

  SWC::Env::Resources.init(
    SWC::Env::IoCtx::io()->ptr(),
    SWC::Env::Config::settings()->get_ptr<SWC::gInt32t>(
      "swc.rgr.ram.percent")
  );

  int err = SWC::Error::OK;
  SWC::Env::Schemas::get()->add(err, SWC::DB::Schema::make(11, "col-test-cs"));

  size_t num_cells = 10000000;
  size_t block_sz = 64000000;
  //Cells::Mutable::Ptr cells_mutable = Cells::Mutable::make(num_cells, 2, 0, SWC::Types::Column::PLAIN;

  int64_t cid = 11;
  SWC::DB::RangeBase::Ptr range = std::make_shared<SWC::DB::RangeBase>(cid,1);
  SWC::Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  SWC::Env::FsInterface::interface()->mkdirs(
    err, range->get_path(SWC::DB::RangeBase::cellstores_dir));

  SWC::Files::CellStore::Write cs_writer(1, range->get_path_cs(1), SWC::Types::Encoding::SNAPPY);
  cs_writer.create(err);
  hdlr_err(err);

  SWC::DynamicBuffer buff;
  Cells::Interval blk_intval = Cells::Interval();

  Cells::Cell cell;
  SWC::DB::Cell::Key key_to_scan;
  uint32_t cell_count = 0;
  int64_t rev;
  size_t expected_blocks = 0;
  for(auto i=1;i<=num_cells;++i){
    std::string n = std::to_string(i);
      
    rev = SWC::Time::now_ns();
    cell.flag = Cells::INSERT;
    cell.set_timestamp(rev-1);
    cell.set_revision(rev);
    cell.set_time_order_desc(false);

    cell.key.free();
    cell.key.add("aKey1");
    cell.key.add("aKey2");
    cell.key.add(n);
    cell.key.add("aKey3");
    cell.key.add("aKey4");
    cell.key.add("aKey5");

    std::string value("A-Data-Value-1234567890-"+n);
    cell.set_value(value.data(), value.length());

    cell.write(buff);
    cell_count++;
    blk_intval.expand(cell);

    if(num_cells == i)
      key_to_scan.copy(cell.key);

    if(buff.fill() > block_sz || num_cells == i){
      std::cout << "add   block: " << cs_writer.to_string() << "\n";
 
      cs_writer.block(err, blk_intval, buff, cell_count);
      blk_intval.free();
      buff.free();
      hdlr_err(err);

      cell_count = 0;
      expected_blocks++;
      std::cout << "added block: " << cs_writer.to_string() << "\n";
    }
    
  }

  cs_writer.finalize(err);
  std::cout << "cs-wrote:    " << cs_writer.to_string() << "\n";
  hdlr_err(err);

  std::cout << "\n-   OK-wrote   -\n\n";

  
  SWC::DB::Cells::Interval intval;
  SWC::Files::CellStore::Read cs(1, range, intval);
  std::cout << "cs-read-init:\n " << cs.to_string() << "\n";

  cs.load_blocks_index(err, true);
  hdlr_err(err);
  std::cout << "cs-read-load_blocks_index:\n " << cs.to_string() << "\n";
  if(cs.blocks_count() != expected_blocks) {
    std::cerr << "ERROR: cs.blocks_count() != expected_blocks \n" 
              << " expected=" << expected_blocks << "\n"
              << " counted=" << cs.blocks_count() << "\n";
    exit(1);
  }
  //cs.close(err);
  //if(err != EBADR){
  //  std::cerr << " FD should been closed after loading blocks-index err=" <<  err << "(" << SWC::Error::get_text(err) << ") \n";
  //  exit(1);
  //}
  err = SWC::Error::OK;
  std::cout << "cs-closed:\n " << cs.smartfd->to_string() << "\n";

  std::cout << "\ncs-read-scan:\n";

  
  
  SWC::DB::Cells::Interval intval_r;
  SWC::server::Rgr::IntervalBlocks blocks;
  blocks.init(range);
  blocks.cellstores->add(
    SWC::Files::CellStore::Read::make(1, range, intval_r));

  std::atomic<int> requests = 110;
  size_t id = 0;
  for(int n=1;n<=20;n++) {

    for(int i=1; i<=(n>10?1:10);i++) {
      id++;
      //std::cout << "cs-req->spec-scan:\n " << req->spec->to_string() << "\n";
    auto t = std::thread([&blocks, &key_to_scan, id, &count=requests](){

      auto req = Cells::ReqScanTest::make();
      req->cells = Cells::Mutable::make(2, 2, 0, SWC::Types::Column::PLAIN);
      req->spec = SWC::DB::Specs::Interval::make_ptr();
      req->spec->key_start.set(key_to_scan, SWC::Condition::GE);
      req->spec->flags.limit = 2;
      req->cb = [req, id, key_to_scan, &requests=count, took=SWC::Time::now_ns()](int err){
        std::cout << " chk=" << id ;
        std::cout << " took=" <<  SWC::Time::now_ns()-took << "\n" ;
        if(err) {
          std::cout << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") " ;
          std::cout << req->to_string() << "\n";
        }
        requests--;
        if(req->cells->size != 1) {
          std::cerr << "ERROR: req->cells.size=" << req->cells->size 
                    << " expected=1\n";
          exit(1);
        }
        SWC::DB::Cells::Cell cell;
        req->cells->get(0, cell);
        if(!cell.key.equal(key_to_scan)) {
          std::cerr << "ERROR: !cell.key.equal(key_to_scan) " << cell.to_string() 
                    << " expected=" << key_to_scan.to_string()  << "\n";
          exit(1);
        }
      };
      blocks.scan(req);
    });
    if((n== 1 && i == 1) || (n== 3 && i == 10))
      t.join();
    else
      t.detach();
    
      
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }


  while(requests>0)
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

  std::cout << "cs-read-scan: OK\n";


  blocks.remove(err);
  hdlr_err(err);

  std::cout << "\n-   OK-read   -\n\n";
  exit(0);
}