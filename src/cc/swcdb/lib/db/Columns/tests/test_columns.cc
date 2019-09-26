/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/fs/Settings.h"
#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/db/Columns/Rgr/Columns.h"
#include "swcdb/lib/db/Columns/Mngr/Columns.h"

#include <iostream>


void SWC::Config::Settings::init_app_options(){
  init_fs_options();
}
void SWC::Config::Settings::init_post_cmd_args(){}

using namespace SWC;

void rgr(){
    Env::RgrColumns::init();

    Env::RgrData::init();
    
    int err = Error::OK;
    server::Rgr::ColumnsPtr cols = Env::RgrColumns::get();

    for(int64_t c=10; c<=11; c++){
        std::cout << "Loading cid:" << c << "\n";

        for(int64_t r=1; r<=3; r++){

            server::Rgr::RangePtr range = cols->get_range(err, c, r, true);
            if(range == nullptr){
                std::cerr << "ERROR, loading ! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
        }
    }
    for(int64_t c=10; c<=11; c++){
        std::cout << "Getting cid:" << c << "\n";

        for(int64_t r=1; r<=3; r++){

            server::Rgr::RangePtr range = cols->get_range(err, c, r);
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
    Env::MngrColumns::init();
    server::Mngr::ColumnsPtr cols = Env::MngrColumns::get();
    int err = Error::OK;

    for(int64_t c=10; c<=11; c++){
        std::cout << "Loading cid:" << c << "\n";

        for(int64_t r=1; r<=3; r++){

            server::Mngr::RangePtr range = cols->get_range(err, c, r, true);
            if(range == nullptr){
                std::cerr << "ERROR, loading ! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
        }
    }
    for(int64_t c=10; c<=11; c++){
        std::cout << "Getting cid:" << c << "\n";

        for(int64_t r=1; r<=3; r++){

            server::Mngr::RangePtr range = cols->get_range(err, c, r);
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
    Env::Config::init(argc, argv);

    Env::FsInterface::init();
    Env::Schemas::init();

    rgr();
    mngr();
    exit(0);
}