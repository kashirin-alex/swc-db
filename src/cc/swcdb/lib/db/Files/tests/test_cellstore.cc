/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/lib/fs/Settings.h"
#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/db/Columns/Rgr/Columns.h"

#include <iostream>


void SWC::Config::Settings::init_app_options(){
  init_fs_options();
}
void SWC::Config::Settings::init_post_cmd_args(){}


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
   
  size_t num_cells = 1000000;
  int num_revs = 5;
  size_t block_sz = 64000000;
  //Cells::Mutable::Ptr cells_mutable = Cells::Mutable::make(num_cells, 2, 0, SWC::Types::Column::PLAIN;

  SWC::Files::CellStoreWrite cs_writer("test_file_1.cs", SWC::Types::Encoding::SNAPPY);
  int err = SWC::Error::OK;
  cs_writer.create(err);
  hdlr_err(err);

  SWC::DynamicBuffer buff;
  Cells::Interval::Ptr blk_intval = std::make_shared<Cells::Interval>();

  Cells::Cell cell;
  SWC::DB::Cell::Key key_to_scan;
  uint32_t cell_count = 0;
  int64_t rev;
  
  for(auto r=1;r<=num_revs;++r){
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
      if(r==1&&i==1)
        key_to_scan.copy(cell.key);
      //if(num_revs == r)
      //  cell.set_value(Cells::OP::EQUAL, 0);
      //else
      //  cell.set_value(Cells::OP::PLUS, 1);

      std::string value("A-Data-Value-1234567890-"+n);
      cell.set_value(value.data(), value.length());

      //cells_mutable->add(cell);

      cell.write(buff);
      cell_count++;
      blk_intval->expand(cell);

      if(buff.fill() > block_sz || (num_revs == r && num_cells == i)){
        std::cout << "add   block: " << cs_writer.to_string() << "\n";

        cs_writer.block(err, blk_intval, buff, cell_count);
        hdlr_err(err);

        cell_count = 0;

        std::cout << "added block: " << cs_writer.to_string() << "\n";
      }
    }
  }

  cs_writer.finalize(err);
  std::cout << "cs-wrote:    " << cs_writer.to_string() << "\n";
  hdlr_err(err);

  std::cout << "\n-   OK-wrote   -\n\n";

  
  SWC::Files::CellStore cs(0);
  cs.smartfd = cs_writer.smartfd;
  std::cout << "cs-read-init:\n " << cs.to_string() << "\n";

  cs.load_blocks_index(err, true);
  hdlr_err(err);
  std::cout << "cs-read-load_blocks_index:\n " << cs.to_string() << "\n";

  cs.close(err);
  err = SWC::Error::OK;


  Cells::Mutable::Ptr cells_mutable = Cells::Mutable::make(1234, 2, 0, SWC::Types::Column::PLAIN);
  SWC::server::Rgr::Callback::RangeScan::Ptr req 
    = std::make_shared<SWC::server::Rgr::Callback::RangeScan>(
        SWC::ConnHandlerPtr(nullptr), 
        SWC::EventPtr(nullptr),
        SWC::DB::Specs::Interval::make_ptr(),
        cells_mutable
      );
  req->spec->key_start.set(key_to_scan, SWC::Condition::GT);
  req->spec->flags.limit =2;
  std::cout << "cs-req->spec-scan:\n " << req->spec->to_string() << "\n";


  SWC::Files::CellStore::Ptr cs2 = std::make_shared<SWC::Files::CellStore>(0);
  cs2->smartfd = cs_writer.smartfd;

  SWC::Env::IoCtx::init(8);
  cs2->scan(req);

  std::this_thread::sleep_for(std::chrono::milliseconds(30000));

  std::cout << "cs-read-scan:\n " << cells_mutable->to_string() << "\n";


  cs2->remove(err);
  hdlr_err(err);

  std::cout << "\n-   OK-read   -\n\n";
  exit(0);
}