/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/Comparators.h"
#include <cstring>
#include <iostream>

namespace Condition = SWC::Condition;

void test(bool vol) {

    const char * a1;
    const char * a2;
    
    // prefix positive
    a1 = "A1";
    a2 = "A1B11111";
    if(!Condition::is_matching(vol, Condition::PF, a1, strlen(a1), a2, strlen(a2)))
       { std::cout << "~PF ERROR!"; exit(1);}
    // prefix negative
    a1 = "A1";
    a2 = "A2";
    if(Condition::is_matching(vol, Condition::PF, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "PF ERROR!"; exit(1);}

        
    // greater positive
    a1 = "A1";
    a2 = "A1B11111";
    if(!Condition::is_matching(vol, Condition::GT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~GT ERROR!"; exit(1);}
    // greater negative
    a1 = "Ab";
    a2 = "Aa";
    if(Condition::is_matching(vol, Condition::GT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "GT ERROR!"; exit(1);}
        
    // greater equal positive
    a1 = "A1";
    a2 = "A1";
    if(!Condition::is_matching(vol, Condition::GE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~GE ERROR!"; exit(1);}
    // greater equal negative
    a1 = "A2";
    a2 = "A1";
    if(Condition::is_matching(vol, Condition::GE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "GE ERROR!"; exit(1);}
        
    // equal positive
    a1 = "A1";
    a2 = "A1";
    if(!Condition::is_matching(vol, Condition::EQ, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~EQ ERROR!"; exit(1);}
    // equal negative
    a1 = "A2";
    a2 = "A1";
    if(Condition::is_matching(vol, Condition::EQ, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "EQ ERROR!"; exit(1);}
        
    // lower equal positive
    a1 = "A1000";
    a2 = "A0";
    if(!Condition::is_matching(vol, Condition::LE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~LE ERROR!"; exit(1);}
    // lower equal negative
    a1 = "A2";
    a2 = "A2222";
    if(Condition::is_matching(vol, Condition::LE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "LE ERROR!"; exit(1);}
        
    // lower positive
    a1 = "A1";
    a2 = "A0";
    if(!Condition::is_matching(vol, Condition::LT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~LT ERROR!"; exit(1);}
    // lower negative
    a1 = "A0";
    a2 = "A0";
    if(Condition::is_matching(vol, Condition::LT, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "LT ERROR!"; exit(1);}

    // not equal positive
    a1 = "A1";
    a2 = "A0";
    if(!Condition::is_matching(vol, Condition::NE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~NE ERROR!"; exit(1);}
    // not equal negative
    a1 = "A0";
    a2 = "A0";
    if(Condition::is_matching(vol, Condition::NE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "NE ERROR!"; exit(1);}

    // regexp positive
    a1 = "^A.*0$";
    a2 = "A5432454350";
    if(!Condition::is_matching(vol, Condition::RE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "~RE ERROR!"; exit(1);}
    // not regexp negative
    a1 = "^A.*1$";
    a2 = "A5432454350";
    if(Condition::is_matching(vol, Condition::RE, a1, strlen(a1), a2, strlen(a2))) 
       { std::cout << "RE ERROR!"; exit(1);}
}
       

int main() {
   test(false);
   test(true);
   return 0;
}