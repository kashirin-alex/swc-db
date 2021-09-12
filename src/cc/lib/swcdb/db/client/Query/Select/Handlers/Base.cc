/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/client/Query/Select/Handlers/Base.h"
#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Select/BrokerScanner.h"


namespace SWC { namespace client { namespace Query { namespace Select {
namespace Handlers {



void Base::default_executor(DB::Types::KeySeq key_seq, cid_t cid,
                            const DB::Specs::Interval& intval) {
  completion.increment();
  switch(executor) {
    case Clients::DEFAULT:
      return Select::Scanner::execute(
        shared_from_this(), key_seq, cid, intval);
    case Clients::BROKER:
      return Select::BrokerScanner::execute(
        shared_from_this(), cid, intval);
    default:
      break;
  }
  SWC_THROWF(Error::INVALID_ARGUMENT, "Bad Executor=%d", int(executor));
}

void Base::default_executor(DB::Types::KeySeq key_seq, cid_t cid,
                            DB::Specs::Interval&& intval) {
  completion.increment();
  switch(executor) {
    case Clients::DEFAULT:
      return Select::Scanner::execute(
        shared_from_this(), key_seq, cid, std::move(intval));
    case Clients::BROKER:
      return Select::BrokerScanner::execute(
        shared_from_this(), cid, std::move(intval));
    default:
      break;
  }
  SWC_THROWF(Error::INVALID_ARGUMENT, "Bad Executor=%d", int(executor));
}

void Base::default_executor(int& err, const DB::Specs::Scan& specs) {
  auto hdlr = shared_from_this();
  switch(executor) {

    case Clients::DEFAULT: {
      DB::SchemasVec schemas(specs.columns.size());
      auto it_seq = schemas.begin();
      size_t count = 0;
      for(auto& col : specs.columns) {
        auto schema = clients->get_schema(err, col->cid);
        if(err)
          return;
        *it_seq++ = schema;
        count += col->size();
      }
      completion.increment(count);

      it_seq = schemas.begin();
      for(auto& col : specs.columns) {
        for(auto& intval : *col.get()) {
          if(!intval->flags.max_buffer)
            intval->flags.max_buffer = buff_sz;
          if(!intval->values.empty())
            intval->values.col_type = (*it_seq)->col_type;
          Select::Scanner::execute(
            hdlr, (*it_seq)->col_seq, col->cid, *intval.get());
        }
        ++it_seq;
      }
      return;
    }

    case Clients::BROKER: {
      size_t count = 0;
      for(auto& col : specs.columns)
        count += col->size();
      completion.increment(count);

      for(auto& col : specs.columns) {
        for(auto& intval : *col.get()) {
          if(!intval->flags.max_buffer)
            intval->flags.max_buffer = buff_sz;
          Select::BrokerScanner::execute(
            hdlr, col->cid, *intval.get());
        }
      }
      return;
    }

    default:
      break;
  }
  SWC_THROWF(Error::INVALID_ARGUMENT, "Bad Executor=%d", int(executor));
}

void Base::default_executor(int& err, DB::Specs::Scan&& specs) {
  auto hdlr = shared_from_this();
  switch(executor) {

    case Clients::DEFAULT: {
      DB::SchemasVec schemas(specs.columns.size());
      auto it_seq = schemas.begin();
      size_t count = 0;
      for(auto& col : specs.columns) {
        auto schema = clients->get_schema(err, col->cid);
        if(err)
          return;
        *it_seq++ = schema;
        count += col->size();
      }
      completion.increment(count);

      it_seq = schemas.begin();
      for(auto& col : specs.columns) {
        for(auto& intval : *col.get()) {
          if(!intval->flags.max_buffer)
            intval->flags.max_buffer = buff_sz;
          if(!intval->values.empty())
            intval->values.col_type = (*it_seq)->col_type;
          Select::Scanner::execute(
            hdlr, (*it_seq)->col_seq, col->cid, std::move(*intval.get()));
        }
        ++it_seq;
      }
      return;
    }

    case Clients::BROKER: {
      size_t count = 0;
      for(auto& col : specs.columns)
        count += col->size();
      completion.increment(count);

      for(auto& col : specs.columns) {
        for(auto& intval : *col.get()) {
          if(!intval->flags.max_buffer)
            intval->flags.max_buffer = buff_sz;
          Select::BrokerScanner::execute(
            hdlr, col->cid, std::move(*intval.get()));
        }
      }
      return;
    }

    default:
      break;
  }
  SWC_THROWF(Error::INVALID_ARGUMENT, "Bad Executor=%d", int(executor));
}


}}}}}
