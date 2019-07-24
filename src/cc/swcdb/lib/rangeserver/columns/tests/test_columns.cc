/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/rangeserver/Settings.h"
#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/rangeserver/columns/Columns.h"

#include <iostream>

using namespace SWC;


int main(int argc, char** argv) {

    SWC::Config::settings->init(argc, argv);
    ColumnsPtr cols = std::make_shared<Columns>(
        std::make_shared<FS::Interface>());

    for(int64_t c=1; c<=1000; c++){
        std::cout << "Loading cid:" << c << "\n";

        for(int64_t r=1; r<=1000; r++){

            RangePtr range = cols->get_range(c, r, true);
            if(range == nullptr){
                std::cerr << "ERROR, loading ! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
        }
    }
    for(int64_t c=1; c<=1000; c++){
        std::cout << "Getting cid:" << c << "\n";

        for(int64_t r=1; r<=1000; r++){

            RangePtr range = cols->get_range(c, r);
            if(range == nullptr){
                std::cerr << "ERROR, range-id does not exists! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
            if(range->range_id() != r){
                std::cerr << "ERROR, range-id does not match! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }

        }
    }
}