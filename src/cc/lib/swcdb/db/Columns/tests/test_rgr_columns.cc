/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/fs/Interface.h"
#include "swcdb/client/Clients.h"
#include "swcdb/core/Resources.h"

#include "swcdb/ranger/RangerEnv.h"

#include <iostream>


void SWC::Config::Settings::init_app_options(){
  init_fs_options();
}
void SWC::Config::Settings::init_post_cmd_args(){}

using namespace SWC;

void rgr(){

    RangerEnv::init();
    
    int err = Error::OK;
    auto cols = RangerEnv::columns();

    for(int64_t c=10; c<=11; c++){
        std::cout << "Loading cid:" << c << "\n";

        for(int64_t r=1; r<=3; r++){

            auto col = cols->initialize(
                err, c, *DB::Schema::make(c, std::to_string(c)).get());
            
            server::Rgr::Range::Ptr range = col->get_range(err, r, true);
            if(range == nullptr){
                std::cerr << "ERROR, loading ! cid:" << c << ", rid:" << r << "\n";
                exit(1);
            }
        }
    }
    for(int64_t c=10; c<=11; c++){
        std::cout << "Getting cid:" << c << "\n";

        for(int64_t r=1; r<=3; r++){

            server::Rgr::Range::Ptr range = cols->get_range(err, c, r);
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

    Env::FsInterface::init(FS::fs_type(
        Env::Config::settings()->get<std::string>("swc.fs")));
    Env::Schemas::init();

    rgr();
    exit(0);
}