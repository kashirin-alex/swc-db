/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include <iostream>

#include "swcdb/core/Time.h"
#include "swcdb/db/Cells/Cell.h"
#include <vector>

#include "swcdb/core/config/Settings.h"
namespace SWC { namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}}

namespace Cells = SWC::DB::Cells;

void check_load() {
  Cells::Cell cell;

  cell.flag = Cells::INSERT;
  cell.set_timestamp(SWC::Time::now_ns());
  cell.set_time_order_desc(true);

  cell.key.add("aKey1");
  cell.key.add("aKey2");
  cell.key.add("aKey3");
  cell.key.add("aKey4");
  cell.key.add("aKey5");

  std::string value = "(";
  for(uint32_t n=0; n<4; ++n)
    for(uint32_t chr=0; chr<=255; ++chr)
      value += (char)chr;
  value += ")END";
  cell.set_value(value);

  auto ts = SWC::Time::now_ns();
  auto chks = 10000;
  for(auto n = chks; n; --n){
    auto c = new Cells::Cell(cell);
    delete c;
  }
  ts = SWC::Time::now_ns() - ts;
  std::cout << "Constructor took=" << ts << " avg=" << ts/chks << "\n";

  ts = SWC::Time::now_ns();
  for(auto n = chks; n; --n){
    auto c = new Cells::Cell();
    c->copy(cell);
    delete c;
  }
  ts = SWC::Time::now_ns() - ts;
  std::cout << "Copy took=" << ts << " avg=" << ts/chks << "\n";

  
  
  ts = SWC::Time::now_ns();
  for(auto n = chks; n; --n){
    auto c = new SWC::DB::Cell::Key(cell.key);
    delete c;
  }
  ts = SWC::Time::now_ns() - ts;
  std::cout << "Key Constructor took=" << ts << " avg=" << ts/chks << "\n";

  ts = SWC::Time::now_ns();
  for(auto n = chks; n; --n){
    auto c = new SWC::DB::Cell::Key();
    c->copy(cell.key);
    delete c;
  }
  ts = SWC::Time::now_ns() - ts;
  std::cout << "Key Copy took=" << ts << " avg=" << ts/chks << "\n";


  ts = SWC::Time::now_ns();
  SWC::DynamicBuffer buff;
  for(auto n = chks; n; --n)
    cell.write(buff);
  ts = SWC::Time::now_ns() - ts;
  std::cout << "Cell::write took=" << ts << " avg=" << ts/chks << " without pre-alloc\n";
  
  auto sz = buff.fill();
  buff.free();
  buff.ensure(sz);
  ts = SWC::Time::now_ns();
  for(auto n = chks; n; --n)
    cell.write(buff);
  ts = SWC::Time::now_ns() - ts;
  std::cout << "Cell::write took=" << ts << " avg=" << ts/chks << "\n";


  const uint8_t* bptr = buff.base;
  size_t remain = buff.fill();
  ts = SWC::Time::now_ns();
  while(remain)
    cell.read(&bptr, &remain);
  ts = SWC::Time::now_ns() - ts;
  std::cout << "Cell::read took=" << ts << " avg=" << ts/chks << "\n";
      
}

int main() {
   
   size_t num_cells = 10000;
   
   size_t i;
   std::vector<Cells::Cell*> cells;
   //cells.reserve(num_cells);

   std::cout << "\n--------Init Cells-------------\n";
   for(i=0;i<num_cells;++i){
      std::string n = std::to_string(i);
      char* cell_num = new char[n.length()];
      strcpy(cell_num, n.c_str());
      
      Cells::Cell* cell = new Cells::Cell();
      
      cell->flag = Cells::INSERT;
      cell->set_timestamp(111);
      //cell->set_revision(222);
      cell->set_time_order_desc(true);

      cell->key.add("aKey1");
      cell->key.add("aKey2|");
      cell->key.add(cell_num);
      cell->key.add("|aKey3");
      cell->key.add("aKey4");
      cell->key.add("aKey5");

      char* v_tmp = new char[n.length()+24];
      char* v1 = v_tmp;
      strcpy(v_tmp, "A-Data-Value-1234567890-");
      v_tmp+=24;
      strcpy(v_tmp, n.c_str());
      cell->set_value(v1, strlen(v1));

      cells.push_back(cell);

      std::cout << "Initial Cell-"<< i << ":\n";
      std::cout << cells.back()->to_string() << "\n\n";
   }
   std::cout << "\n-------------------------------\n";


/**/
   std::cout << "\n--------Copy Cells-------------\n";

   i=0;
   std::vector<Cells::Cell*> cells_copied;
   cells_copied.reserve(num_cells);
   for(auto it=cells.begin();it<cells.end();++it){
      std::cout << "Copying Cell-"<< i << ":\n";
      //std::cout << (*it)->to_string() << "\n\n";
      cells_copied.push_back(new Cells::Cell(**it));  // OK 
      //cells_copied.push_back(Cells::Cell(**it));      // !move
      auto last_cell = cells_copied.back();
      std::cout << " ptrs-orig key:"<<(size_t)(*it)->key.data 
                << " value:"<<(size_t)(*it)->value << "\n";
      std::cout << " ptrs-copy key:"<<(size_t)last_cell->key.data  
                << " value:"<<(size_t)last_cell->value << "\n";

      if((*it)->key.data == last_cell->key.data || (*it)->value == last_cell->value){
         std::cout << "COPY PTRs SHOUT NOT BE EQUAL:\n";
         exit(1);
      }
      if(!(*it)->equal(*last_cell)) {
         std::cout << "COPY NOT EQUAL:\n";
         std::cout << (*cells.begin())->to_string() << "\n\n";
         std::cout << last_cell->to_string() << "\n\n";
         exit(1);
      }

      ++i;
   }
   i=0;
   for(auto it=cells_copied.begin();it<cells_copied.end();++it){
      std::cout << "Copied Cell-"<< i << ":\n";
      std::cout << (*it)->to_string() << "\n\n";
      ++i;
   }
   std::cout << "\n-------------------------------\n";


   std::cout << "\n---Write Serialized Buffer-----\n";
   
   const uint8_t * ptr_state;
   SWC::DynamicBuffer buff;

   for(auto it=cells.begin();it<cells.end();++it){
      std::cout << "Serialized buff, fill:" << buff.fill() << " size:" << buff.size << "\n";
      (*it)->write(buff);
   }
   std::cout << "Serialized final-buff, fill:" << buff.fill() << " size:" << buff.size << "\n\n";
   std::cout << "\n-------------------------------\n";




   std::cout << "\n---Load Serialized Cells-----\n";
   buff.set_mark();
   uint8_t* mark = buff.mark;
   const uint8_t* bptr = buff.base;
   size_t remain = mark-bptr;
   std::cout << " remain=" << remain << " base:"<< (size_t)bptr << ", mark:"<< (size_t)mark << ":\n";

   i=0;
   Cells::Cell cell;
   auto it1=cells.begin();
   auto it2=cells_copied.begin();

   while(mark > bptr) {
    cell.read(&bptr, &remain);
      
    std::cout << "Loaded Cell-"<< i << ":\n";
    std::cout << cell.to_string() << "\n\n";
    std::cout << "base:"<< (size_t)buff.base  << ",bptr:"<< (size_t)bptr << ",mark:"<< (size_t)mark << "\n";
    ++i;
      
    if( !(*it1)->equal(cell) || !(*it2)->equal(cell) ){
      std::cout << "LOADED SERIALIZED CELL NOT EQUAL:\n";
      std::cout << cell.to_string() << "\n\n";
      std::cout << (*it1)->to_string() << "\n\n";
      std::cout << (*it2)->to_string() << "\n\n";
      exit(1);
    }

    ++it1;
    ++it2;
   }

   std::cout << "\n-------------------------------\n";


   std::cout << "\n----Destruction Cells----------\n";

   size_t k_len = cells_copied.back()->key.size;
   uint8_t* last_skey = new uint8_t[k_len];
   uint8_t* last_skey_ptr = cells_copied.back()->key.data;
   memcpy(last_skey, last_skey_ptr, k_len);
   
  for(auto c : cells_copied)
    delete c;
   cells_copied.clear();
   //cells_copied.clear();
   if (*last_skey_ptr != 0 && memcmp(last_skey, last_skey_ptr, k_len) == 0){
         std::cout << "DESTRUCTION DID NOT HAPPEN:\n";
         std::cout << (size_t)last_skey_ptr << ":" << std::string((const char *)last_skey_ptr, k_len) 
            << "==" << std::string((const char *)last_skey, k_len)<< "\n\n";
         exit(1);
   }
   delete []last_skey;


   std::cout << " sizeof(SWC::DB::Cells::Cell)=" << sizeof(SWC::DB::Cells::Cell) << "\n";
   std::cout << " sizeof(SWC::DB::Cell::Key)=" << sizeof(SWC::DB::Cell::Key) << "\n";
   std::cout << " sizeof(SWC::DB::Cell::KeyVec)=" << sizeof(SWC::DB::Cell::KeyVec) << "\n";
   check_load();;
   std::cout << "\n-------------------------------\n";

}