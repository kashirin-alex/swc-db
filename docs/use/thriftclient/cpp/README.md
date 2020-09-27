---
title: C++
sort: 1
---


# Using SWC-DB C++ Thrift Client

The SWC-DB C++ Thrift Client is defined in ```swcdb/thrift/client/Client.h``` and implemented with ```libswcdb_thrift```.

The following namespaces will be available by including the ```swcdb/thrift/client/Client.h```:
  * ```SWC``` the base namespace of SWC-DB
  * ```SWC::Thrift``` the namespace of SWC-DB Thrift Client and the [SWC-DB Thrift Modules]({{ site.baseurl }}/use/thriftclient/#the-swc-db-thrift-modules)
  * ```SWC::thrift``` is the ```apache::thrift``` namespace

The ```SWC::Thrift::Client``` is a class based on the ```SWC::Thrift::ServiceClient```. Which is a helper in configuring and making of the `ServiceClient` instance.



## Compiling

Add compiler flags to your program:
* include-flags ```-I${SWCDB_INSTALL_PREFIX}/include/```
* link-flags
  * shared built - ```-lswcdb_thrift``` or ```${SWCDB_INSTALL_PREFIX}/lib/libswcdb_thrift.so```
  * static built - ```${SWCDB_INSTALL_PREFIX}/lib/libswcdb_thrift.a```
  * dependencies  +  ```libthrift``` while depends on the usage, the following might be required ```libthriftnb```, ```libthriftz```, ```libevent```, ```libz```, ```libssl```, ```libcrypto```


## a Making of a Basic Program
1. Establish a connection to Thrift-Broker 
2. List all Columns
3. Create a Column with name `cpp-thrift-example`
4. Get the Created Column
5. Check the Created Column
6. Update the Column with Cells
7. Select with SpecScan the cells available in the Column
8. Delete the Column
9. Check the Column Deleted
10. Close the Client Connection


The **`basic_swcdb_thrift_program.cc`** source:
```cpp

#include "swcdb/thrift/client/Client.h"
#include <iostream> /* for the example purpose */

int main() {
  std::cout << "Enter OK" << std::endl;



  /* 1. Establish a connection to Thrift-Broker
    auto client = SWC::Thrift::Client::make("localhost", 18000);
  */
  std::cout << std::endl << "Establishing a connection" << std::endl;
  SWC::Thrift::Client client("localhost", 18000);



  /* 2. List all Columns */
  SWC::Thrift::SpecSchemas spec;
  SWC::Thrift::Schemas schemas;
  client.list_columns(schemas, spec);

  std::cout << std::endl << "All Columns List size=" 
            << schemas.size() << std::endl;
  for(auto& schema : schemas) {
    schema.printTo(std::cout << " ");
    std::cout << std::endl;
  }



  /* 3. Create a Column with name `cpp-thrift-example` */
  SWC::Thrift::Schema schema;
  schema.__set_col_name("cpp-thrift-example");
  schema.__set_col_seq(SWC::Thrift::KeySeq::VOLUME);

  std::cout << std::endl << "Creating Column: \n " << schema << std::endl;
  client.mng_column(SWC::Thrift::SchemaFunc::CREATE, schema);



  /* 4. Get the Created Column */
  schemas.clear();
  spec.__isset.names = true; // important !!!
  spec.names.push_back(schema.col_name);
  std::cout << "Get Column spec: \n " << spec << std::endl;
  client.list_columns(schemas, spec);

  std::cout << std::endl << "Got Column: \n ";
  for(auto& schema : schemas) {
    std::cout << ' ' << schema << std::endl;
  }



  /* 5. Check the Created Column */
  assert(schemas.size() == 1);
  assert(schemas.back().col_name.compare(schema.col_name) == 0);
  schema = schemas.back();
  assert(schema.col_seq == SWC::Thrift::KeySeq::VOLUME);



  /* 6. Update the Column with Cells */
  std::cout << "\nGenerating Cells \n";
  int n_cells = 100000;
  int num_fractions = 10;

  // value for a cell, 256 bytes bin
  std::string value;
  for(int n=0; n < 256; ++n)
    value += (char)n;

  SWC::Thrift::UCCells ucells;
  auto& col = ucells[schema.cid];
  col.resize(n_cells);

  int n = 0;
  for(auto& cell : col) {
    cell.f = SWC::Thrift::Flag::INSERT;

    // N-Fractions for the Key
    cell.k.resize(num_fractions);
    cell.k[0].append(std::to_string(n++)); // 1st F(unique)
    for(uint8_t f=1; f < num_fractions; ++f) {
      cell.k[f] += (char)(f + 97);
    }
    
    // cell.__set_ts(now_ns());
    // cell.__set_ts_desc(true);
    cell.__set_v(value);
  }

  std::cout << "Updating col with cells=" << col.size() << "\n";
  assert(col.size() == n_cells);

  size_t updater_id = 0; // without an updater
  client.update(ucells, updater_id);



  /* 7. Select the cells available in the Column */
  SWC::Thrift::SpecScan specs;
  specs.columns.emplace_back();

  auto& speccol = specs.columns.back();
  speccol.__set_cid(schema.cid);

  // set cell matching F(2) == "b", expect all
  speccol.intervals.emplace_back();
  speccol.__isset.intervals = true;
  auto& intval = speccol.intervals.back();

  intval.__isset.key_start = true;
  intval.key_start.resize(3);
  intval.key_start[0].__set_comp(SWC::Thrift::Comp::GT);
  intval.key_start[0].__set_f("");
  intval.key_start[1].__set_comp(SWC::Thrift::Comp::EQ);
  intval.key_start[1].__set_f("b");
  intval.key_start[2].__set_comp(SWC::Thrift::Comp::GE);
  intval.key_start[2].__set_f(""); // all on the depth
  
  specs.printTo(std::cout << " Select SpecScan='");
  std::cout << "'\n";

  SWC::Thrift::Cells cells;
  client.scan(cells, specs);

  std::cout << "Select cells.size()=" << cells.size()  << "\n";
  assert(cells.size() == n_cells);
  for(auto& cell : cells)
    assert(value.compare(cell.v) == 0);



  /* 8. Delete the Column */
  std::cout << std::endl << "Deleting the Column" << std::endl;
  client.mng_column(SWC::Thrift::SchemaFunc::DELETE, schema);



  /* 9. Check the Column Deleted */
  schemas.clear();
  try {
    client.list_columns(schemas, spec);
  } catch(const SWC::Thrift::Exception& e) {
    // OK 
  }
  assert(schemas.empty());



  /* 10. Close the Client Connection */
  std::cout << std::endl << "Closing the Client" << std::endl;
  client.close();



  std::cout << std::endl << "Exit OK" << std::endl;
  return 0;
}

```




Build the **`basic_swcdb_thrift_program`**:
```bash 

c++ basic_swcdb_thrift_program.cc \
  -I${SWCDB_INSTALL_PREFIX}/include \
  -L${SWCDB_INSTALL_PREFIX}/lib \
  -lswcdb_thrift -lthrift \
  -o basic_swcdb_thrift_program;
```



Run the **`basic_swcdb_thrift_program`**:
```bash 

LD_LIBRARY_PATH=/opt/swcdb/lib ./basic_swcdb_thrift_program;
```


