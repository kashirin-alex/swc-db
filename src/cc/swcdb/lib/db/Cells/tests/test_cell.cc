/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/db//Cells/Cell.h"
#include <memory>

using namespace SWC;
using namespace Cells;

int main() {

   size_t num_cells = 10000;
   
   size_t i;
   std::vector<Cell*> cells;
   //cells.reserve(num_cells);

   std::cout << "\n--------Init Cells-------------\n";
   for(i=0;i<num_cells;++i){
      std::string n = std::to_string(i);
      char* cell_num = new char[n.length()];
      strcpy(cell_num, n.c_str());
      
      Cell* cell = new Cell();
      
      cell->set_flag(255);
      cell->set_timestamp(111);
      cell->set_revision(222);
      cell->set_time_order_desc(true);

      cell->keys.push_back(Key("aKey1"));
      cell->keys.push_back(Key("aKey2|"));
      cell->keys.push_back(Key(cell_num));
      cell->keys.push_back(Key("|aKey3"));
      cell->keys.push_back(Key("aKey4"));
      cell->keys.push_back(Key("aKey5"));

      char* v_tmp = new char[n.length()+24];
      char* v1 = v_tmp;
      strcpy(v_tmp, "A-Data-Value-1234567890-");
      v_tmp+=24;
      strcpy(v_tmp, n.c_str());
      cell->set_value(v1, strlen(v1));

      cells.push_back(cell);

      std::cout << "Inital Cell-"<< i << ":\n";
      std::cout << *cells.back() << "\n\n";
   }
   std::cout << "\n-------------------------------\n";


/**/
   std::cout << "\n--------Copy Cells-------------\n";

   i=0;
   std::vector<Cell> cells_copied;
   cells_copied.reserve(num_cells);
   for(auto it=cells.begin();it<cells.end();it++){
      std::cout << "Copying Cell-"<< i << ":\n";
         cells_copied.push_back(*(*it)->make_copy()); // OK
      // cells_copied.push_back(Cell((*it)));         // BAD, on-stack
      // cells_copied.push_back(*(new Cell(*it)));    // OK 

         std::cout << " ptrs-orig skey:"<<(size_t)(*it)->skey 
                   << " value:"<<(size_t)(*it)->value << "\n";
         std::cout << " ptrs-copy skey:"<<(size_t)cells_copied.back().skey 
                  << " value:"<<(size_t)cells_copied.back().value << "\n";

      if((*it)->skey == cells_copied.back().skey || (*it)->value == cells_copied.back().value){
         std::cout << "COPY PTRs SHOUT NOT BE EQUAL:\n";
         exit(1);
      }
      if(!(*(*it) == cells_copied.back())){
         std::cout << "COPY NOT EQUAL:\n";
         std::cout << *(*cells.begin()) << "\n\n";
         std::cout << cells_copied.back() << "\n\n";
         exit(1);
      }

      i++;
   }
   i=0;
   for(auto it=cells_copied.begin();it<cells_copied.end();it++){
      std::cout << "Copied Cell-"<< i << ":\n";
      std::cout << (*it) << "\n\n";
      i++;
   }
   std::cout << "\n-------------------------------\n";
/**/


   std::cout << "\n---Write Serialized Buffer-----\n";
   
   const uint8_t * ptr_state;
   DynamicBuffer buff = DynamicBuffer();

   for(auto it=cells.begin();it<cells.end();it++){
      std::cout << "Serialized buff, fill:" << buff.fill() << " size:" << buff.size << "\n";
      (*it)->write(buff);
   }
   std::cout << "Serialized final-buff, fill:" << buff.fill() << " size:" << buff.size << "\n\n";
   std::cout << "\n-------------------------------\n";




   std::cout << "\n---Load Serialized Cells-----\n";
   buff.set_mark();
   uint8_t* mark = buff.mark;
   uint8_t* bptr = buff.base;
   std::cout << "base:"<< (size_t)bptr << ", mark:"<< (size_t)mark << ":\n";

   i=0;
   auto it1=cells.begin();
   auto it2=cells_copied.begin();

   while(bptr < mark){
      Cell* cell = new Cell();
      cell->read(&bptr);
      
      std::cout << "Loaded Cell-"<< i << ":\n";
      std::cout << *cell << "\n\n";
      std::cout << "base:"<< (size_t)buff.base  << ",bptr:"<< (size_t)bptr << ",mark:"<< (size_t)mark << "\n";
      i++;
      
      if(!(*(*it1) == *cell) || !((*it2) == *cell) ){
         std::cout << "LOADED SERIALIZED CELL NOT EQUAL:\n";
         std::cout << *cell << "\n\n";
         std::cout << *(*it1) << "\n\n";
         std::cout << (*it2) << "\n\n";
         exit(1);
      }

      it1++;
      it2++;
   }

   std::cout << "\n-------------------------------\n";

}