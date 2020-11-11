/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_RangeData_h
#define swcdb_ranger_db_RangeData_h


#include "swcdb/core/Buffer.h"

namespace SWC { namespace Ranger { 


//! The SWC-DB Range-Data C++ namespace 'SWC::Ranger::RangeData'
namespace RangeData {


const int HEADER_SIZE=13;
const int HEADER_OFFSET_CHKSUM=9;
const int8_t VERSION=1;

/* file-format: 
    header: i8(version), i32(data-len), 
            i32(data-checksum), i32(header-checksum)
    data:   vi32(num-cellstores) ,[vi32(cellstore-csid), interval]
*/


// SET 
void write(DynamicBuffer& dst_buf, CellStore::Readers& cellstores);

void save(int& err, CellStore::Readers& cellstores);


//  GET
void read(int& err, const uint8_t **ptr, size_t* remain, 
          CellStore::Readers& cellstores);

void load(int& err, CellStore::Readers& cellstores);


}}}

#endif // swcdb_ranger_db_RangeData_h
