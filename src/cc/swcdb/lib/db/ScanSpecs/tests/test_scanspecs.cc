/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/common/Compat.h"
#include "swcdb/lib/db/ScanSpecs/ScanSpecs.h"


using namespace SWC;
using namespace ScanSpecs;


void test_encode_decode(ColumnIntervals* ci, uint8_t** buf, uint8_t** buf2);
int main() {

    ScanSpec ss = ScanSpec();
    ss.flags.limit = 111;
    ss.flags.offset = 222;
    ss.flags.max_versions = 2;
    
    const char * a1 = "a1";
    const char * a2 = "a2";
    const char * a3 = "a3";
    const char * a4 = "a4";
    const char * b1 = "b1";
    const char * b2 = "b2";
    const char * b3 = "b3";
    const char * b4 = "b4";
    std::string a_string_v = std::string("A-Value1");
    const char * va1 = a_string_v.c_str();
    const char * va2 = "A-Value2";
    const char * vb1 = "B-Value1";
    const char * vb2 = "B-Value2";

    int64_t ts1 = 111;
    int64_t ts2 = 112;
    int64_t ts3 = 113;
    int64_t ts4 = 114;
    int64_t ts5 = 115;
    int64_t ts6 = 116;
    int64_t ts7 = 117;
    int64_t ts8 = 118;
    
    Keys keys_start;
    keys_start.keys.push_back(Key(a1, Comparator::EQ));
    keys_start.keys.push_back(Key(a2, 2, Comparator::EQ));
    
    Keys keys_finish;
    keys_finish.keys.push_back(Key(a3, Comparator::EQ));
    keys_finish.keys.push_back(Key(a4, Comparator::EQ));
    
    ColumnIntervals cs_is_1 = ColumnIntervals(5555);
    cs_is_1.cells_interval.push_back(CellsInterval(
        keys_start,
        keys_finish,
        Value(va1, Comparator::EQ),
        Timestamp(ts1,Comparator::EQ),
        Timestamp(ts2,Comparator::EQ),
        ss.flags
        ));
    cs_is_1.cells_interval.push_back(CellsInterval(
        keys_start,
        keys_finish,
        Value(va2, Comparator::EQ),
        Timestamp(ts3,Comparator::EQ),
        Timestamp(ts4,Comparator::EQ),
        ss.flags
        ));
    ss.columns.push_back(cs_is_1);
    
    ColumnIntervals cs_is_2 = ColumnIntervals(11111);
    cs_is_2.cells_interval.push_back(CellsInterval(
        keys_start,
        keys_finish,
        Value(vb1, Comparator::EQ),
        Timestamp(ts5,Comparator::EQ),
        Timestamp(ts6,Comparator::EQ)
        ));

        
    Keys keys_start2;
    keys_start2.keys.push_back(Key(b1, Comparator::EQ));
    keys_start2.keys.push_back(Key(b2, 2, Comparator::EQ));
    
    Keys keys_finish2;
    keys_finish2.keys.push_back(Key(b3, Comparator::EQ));
    keys_finish2.keys.push_back(Key(b4, Comparator::EQ));
    cs_is_2.cells_interval.push_back(CellsInterval(
        keys_start2,
        keys_finish2,
        Value(vb2, Comparator::EQ),
        Timestamp(ts7,Comparator::EQ),
        Timestamp(ts8,Comparator::EQ)
        ));
    ss.columns.push_back(cs_is_2);
    std::cout << "- " << ss << '\n';
    
    std::cout << "\n\n";
    
    ScanSpec passed_ss = ss;
    ScanSpec passed_ss2;
    passed_ss2 = ss;
    // 1st interval - column 1
    if(!passed_ss.columns[0].cells_interval[0].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "passed_ss: k_comp ERROR";
        exit(1);
    }
    if(!passed_ss2.columns[0].cells_interval[0].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "passed_ss2: k_comp ERROR";
        exit(1);
    }
    ScanSpec* passed_ss_ptr = &ss;
    if(!passed_ss_ptr->columns[0].cells_interval[0].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "passed_ss_ptr: k_comp ERROR";
        exit(1);
    }
    ScanSpec* passed_ss_ptr2;
    passed_ss_ptr2 = &ss;
    if(!passed_ss_ptr2->columns[0].cells_interval[0].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "passed_ss_ptr2: k_comp ERROR";
        exit(1);
    }

    if(!ss.columns[0].cells_interval[0].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[0].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[0].keys_start.keys[1].is_matching(&a2, strlen(a2))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[0].keys_finish.keys[0].is_matching(&a3, strlen(a3))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[0].keys_finish.keys[1].is_matching(&a4, strlen(a4))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[0].value.is_matching(&va1, strlen(va1))){
        std::cout << "v_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[0].ts_start.is_matching(ts1)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[0].ts_finish.is_matching(ts2)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }

    // 2nd interval
    
    if(!ss.columns[0].cells_interval[1].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[1].keys_start.keys[1].is_matching(&a2, strlen(a2))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[1].keys_finish.keys[0].is_matching(&a3, strlen(a3))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[1].keys_finish.keys[1].is_matching(&a4, strlen(a4))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[1].value.is_matching(&va2, strlen(va1))){
        std::cout << "v_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[1].ts_start.is_matching(ts3)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }
    if(!ss.columns[0].cells_interval[1].ts_finish.is_matching(ts4)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }
    
    // 3rd interval - column 2
    
    if(!ss.columns[1].cells_interval[0].keys_start.keys[0].is_matching(&a1, strlen(a1))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[0].keys_start.keys[1].is_matching(&a2, strlen(a2))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[0].keys_finish.keys[0].is_matching(&a3, strlen(a3))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[0].keys_finish.keys[1].is_matching(&a4, strlen(a4))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[0].value.is_matching(&vb1, strlen(vb1))){
        std::cout << "v_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[0].ts_start.is_matching(ts5)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[0].ts_finish.is_matching(ts6)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }
    
    // 4th interval
    
    if(!ss.columns[1].cells_interval[1].keys_start.keys[0].is_matching(&b1, strlen(b1))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[1].keys_start.keys[1].is_matching(&b2, strlen(b2))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[1].keys_finish.keys[0].is_matching(&b3, strlen(b3))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[1].keys_finish.keys[1].is_matching(&b4, strlen(b4))){
        std::cout << "k_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[1].value.is_matching(&vb2, strlen(vb2))){
        std::cout << "v_comp ERROR";
        exit(1);
    }
    if(!passed_ss_ptr->columns[1].cells_interval[1].value.is_matching(&vb2, strlen(vb2))){
        std::cout << "passed_ss_ptr v_comp ERROR";
        exit(1);
    }

    if(!ss.columns[1].cells_interval[1].ts_start.is_matching(ts7)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }
    if(!ss.columns[1].cells_interval[1].ts_finish.is_matching(ts8)){
        std::cout << "ts_comp ERROR";
        exit(1);
    }

    ss.columns[1].cells_interval[1].ts_finish.comp = Comparator::GE;
    ss.columns[1].cells_interval[1].ts_finish.reset_matcher();
    if(!ss.columns[1].cells_interval[1].ts_finish.is_matching(ts8+1111)){
        std::cout << "ts_comp(changed) ERROR";
        exit(1);
    }



    uint8_t* buf = new uint8_t[256]; //!! (6-ensure)
    uint8_t* base = buf;
    uint8_t* buf2 = new uint8_t[256]; //!! (6-ensure)
    uint8_t* base2 = buf;
    for(size_t i=1000;--i;){
        std::cout << "try:" << i << "\n\n";
        for(auto it=ss.columns.begin();it<ss.columns.end();++it){
            buf = base;
            buf2 = base2;
            test_encode_decode(&(*it), &buf, &buf2);

        }
    }

}
void test_encode_decode(ColumnIntervals* ci, uint8_t** buf, uint8_t** buf2){
        size_t len = ci->encoded_length();
        
        const uint8_t* base = *buf;
        const uint8_t* mark1 = base;
        
        std::cout << "Encoding: \n" << *ci << "\n"
                  << "Encoded, cid:" << ci->cid 
                  << ", encoded-len: "<< len << " base:" << (size_t)base << "\n";

        ci->encode(buf);
        std::cout << "data:\"" << std::string((const char*)base, len) << "\"\n"
                  << "calc-len: "<< *buf-base << "\n";
        if(*buf-base != len) {
            std::cout << "ERROR, encode wrote less than expected";
            exit(1);
        }

        ColumnIntervals ci_decoded = ColumnIntervals(&base, &len);
        std::cout << "Decoded, cid:" << ci_decoded.cid << ", remain-len: "<< len 
                   << "\n" << ci_decoded << "\n";
        if(len > 0) {
            std::cout << "ERROR, remain-len\n";
            exit(1);
        }
        
        //ci_decoded.cid = 444;
        
        len = ci_decoded.encoded_length();
        const uint8_t* mark2 = *buf2;
        ci_decoded.encode(buf2);
        
        if(memcmp(mark1, mark2, len) != 0){
            std::cout << "ERROR, encoding mismatch (memcmp) \n";
            std::cout << ColumnIntervals(&mark2, &len) << "\n\n";
            std::cout << "data:\"" << std::string((const char*)mark1, len) << "\"\n";
            std::cout << "!==\n";
            std::cout << "data:\"" << std::string((const char*)mark2, len) << "\"\n";
            exit(1);
        }
        
}