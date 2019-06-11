/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/db/Comparators/Comparators.h"
#include <cstring>

using namespace SWC;
int main() {

    ComparatorBase* matcher = {};
    const char * a1;
    const char * a2;
    
    // prefix positive
    matcher = matcher->get_matcher(Comparator::PF);
    a1 = "A1";
    a2 = "A1B11111";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "PF ERROR!"; exit(1);}
    // prefix negative
    matcher = matcher->get_matcher(Comparator::PF);
    a1 = "A1";
    a2 = "A2";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "PF ERROR!"; exit(1);}

        
    // greater positive
    matcher = matcher->get_matcher(Comparator::GT);
    a1 = "A1";
    a2 = "A1B11111";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "GT ERROR!"; exit(1);}
    // greater negative
    matcher = matcher->get_matcher(Comparator::GT);
    a1 = "Ab";
    a2 = "Aa";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "GT ERROR!"; exit(1);}
        
    // greater equal positive
    matcher = matcher->get_matcher(Comparator::GE);
    a1 = "A1";
    a2 = "A1";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "GE ERROR!"; exit(1);}
    // greater equal negative
    matcher = matcher->get_matcher(Comparator::GE);
    a1 = "A2";
    a2 = "A1";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "GE ERROR!"; exit(1);}
        
    // equal positive
    matcher = matcher->get_matcher(Comparator::EQ);
    a1 = "A1";
    a2 = "A1";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "EQ ERROR!"; exit(1);}
    // equal negative
    matcher = matcher->get_matcher(Comparator::EQ);
    a1 = "A2";
    a2 = "A1";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "EQ ERROR!"; exit(1);}
        
    // lower equal positive
    matcher = matcher->get_matcher(Comparator::LE);
    a1 = "A1000";
    a2 = "A0";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "LE ERROR!"; exit(1);}
    // lower equal negative
    matcher = matcher->get_matcher(Comparator::LE);
    a1 = "A2";
    a2 = "A2222";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "LE ERROR!"; exit(1);}
        
    // lower positive
    matcher = matcher->get_matcher(Comparator::LT);
    a1 = "A1";
    a2 = "A0";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "LT ERROR!"; exit(1);}
    // lower negative
    matcher = matcher->get_matcher(Comparator::LT);
    a1 = "A0";
    a2 = "A0";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "LT ERROR!"; exit(1);}

    // not equal positive
    matcher = matcher->get_matcher(Comparator::NE);
    a1 = "A1";
    a2 = "A0";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "NE ERROR!"; exit(1);}
    // not equal negative
    matcher = matcher->get_matcher(Comparator::NE);
    a1 = "A0";
    a2 = "A0";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "NE ERROR!"; exit(1);}

    // regexp positive
    matcher = matcher->get_matcher(Comparator::RE);
    a1 = "^A.*0$";
    a2 = "A5432454350";
    if(!matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "RE ERROR!"; exit(1);}
    // not regexp negative
    matcher = matcher->get_matcher(Comparator::RE);
    a1 = "^A.*1$";
    a2 = "A5432454350";
    if(matcher->is_matching(&a1, strlen(a1), &a2, strlen(a2))) 
       { std::cout << "RE ERROR!"; exit(1);}
}