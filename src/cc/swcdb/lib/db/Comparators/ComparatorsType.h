/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef commmon_ComparatorsType_h
#define commmon_ComparatorsType_h

#include "Comparators.h"


#include <ostream>
#include <memory>

#include <re2/re2.h>



namespace SWC {
  

class ComparatorPrefix: public ComparatorBase {
    public:
    ComparatorPrefix(){}
    virtual ~ComparatorPrefix(){}
    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
};


class ComparatorGreater: public ComparatorBase{
    public:
    ComparatorGreater(){}
    virtual ~ComparatorGreater(){}

    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
    bool is_matching(int64_t p1, int64_t p2);
};


class ComparatorGreaterEqual: public ComparatorBase{
    public:
    ComparatorGreaterEqual(){}
    virtual ~ComparatorGreaterEqual(){}

    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
    bool is_matching(int64_t p1, int64_t p2);
};


class ComparatorEqual: public ComparatorBase{
    public:
    ComparatorEqual(){}
    virtual ~ComparatorEqual(){}

    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
    bool is_matching(int64_t p1, int64_t p2);
};


class ComparatorLowerEqual: public ComparatorBase{
    public:
    ComparatorLowerEqual(){}
    virtual ~ComparatorLowerEqual(){}

    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
    bool is_matching(int64_t p1, int64_t p2);
};


class ComparatorLower: public ComparatorBase{
    public:
    ComparatorLower(){}
    virtual ~ComparatorLower(){}

    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
    bool is_matching(int64_t p1, int64_t p2);
};



class ComparatorNotEqual: public ComparatorBase{
    public:
    ComparatorNotEqual(){}
    virtual ~ComparatorNotEqual(){}

    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
    bool is_matching(int64_t p1, int64_t p2);
};


class ComparatorRegexp: public ComparatorBase{
    public:
    ComparatorRegexp(){}
    virtual ~ComparatorRegexp(){}

    bool is_matching(const char **p1, uint32_t p1_len,
                     const char **p2, uint32_t p2_len);
    private:
    std::shared_ptr<RE2> regex;
};

}



#endif