/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/fs/Settings.h"
#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/db/Columns/RS/Columns.h"
#include "swcdb/lib/db/Columns/MNGR/Columns.h"

#include <iostream>


void SWC::Config::Settings::init_app_options(){
  init_fs_options();
}
void SWC::Config::Settings::init_post_cmd_args(){}

using namespace SWC;

void rs(){
    EnvRsColumns::init();
    server::RS::ColumnsPtr cols = EnvRsColumns::get();

    for(int64_t c=1; c<=1000; c++){
        std::cout << "Loading cid:" << c << "\n";

        for(int64_t r=1; r<=1000; r++){

            server::RS::RangePtr range = cols->get_range(c, r, true);
            if(range == nullptr){
                std::cerr << "ERROR, loading ! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
        }
    }
    for(int64_t c=1; c<=1000; c++){
        std::cout << "Getting cid:" << c << "\n";

        for(int64_t r=1; r<=1000; r++){

            server::RS::RangePtr range = cols->get_range(c, r);
            if(range == nullptr){
                std::cerr << "ERROR, range-id does not exists! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
            if(range->rid != r){
                std::cerr << "ERROR, range-id does not match! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }

        }
    }
}
void mngr(){
    EnvMngrColumns::init();
    server::Mngr::ColumnsPtr cols = EnvMngrColumns::get();

    for(int64_t c=1; c<=1000; c++){
        std::cout << "Loading cid:" << c << "\n";

        for(int64_t r=1; r<=1000; r++){

            server::Mngr::RangePtr range = cols->get_range(c, r, true);
            if(range == nullptr){
                std::cerr << "ERROR, loading ! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
        }
    }
    for(int64_t c=1; c<=1000; c++){
        std::cout << "Getting cid:" << c << "\n";

        for(int64_t r=1; r<=1000; r++){

            server::Mngr::RangePtr range = cols->get_range(c, r);
            if(range == nullptr){
                std::cerr << "ERROR, range-id does not exists! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
            if(range->rid != r){
                std::cerr << "ERROR, range-id does not match! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }

        }
    }
}

int main(int argc, char** argv) {
    EnvConfig::init(argc, argv);

    EnvFsInterface::init();
    rs();
    mngr();
}