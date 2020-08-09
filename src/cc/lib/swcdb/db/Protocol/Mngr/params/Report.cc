
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/Report.h"
#include "swcdb/core/Serialization.h"



namespace SWC { namespace Protocol { namespace Mngr { namespace Params { namespace Report {



RspClusterStatus::RspClusterStatus(uint8_t status) 
                                  : status(status) {
}

RspClusterStatus::~RspClusterStatus() { }

std::string RspClusterStatus::to_string() const {
  std::string s("Cluster(status=");
  s.append(std::to_string((int)status));
  s.append(")");
  return s;
}

size_t RspClusterStatus::internal_encoded_length() const {
  return 1;
}
  
void RspClusterStatus::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, status);
}
  
void RspClusterStatus::internal_decode(const uint8_t** bufp, size_t* remainp) {
  status = Serialization::decode_i8(bufp, remainp);
}




ReqColumnStatus::ReqColumnStatus(cid_t cid)
                                : cid(cid) { }

ReqColumnStatus::~ReqColumnStatus() { }

size_t ReqColumnStatus::internal_encoded_length() const {
  return Serialization::encoded_length_vi64(cid);
}
  
void ReqColumnStatus::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, cid);
}
  
void ReqColumnStatus::internal_decode(const uint8_t** bufp, size_t* remainp) {
  cid = Serialization::decode_vi64(bufp, remainp);
}





size_t RspColumnStatus::RangeStatus::encoded_length() const { 
  return  1
        + Serialization::encoded_length_vi64(rid)
        + Serialization::encoded_length_vi64(rgr_id);
}

void RspColumnStatus::RangeStatus::encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)state);
  Serialization::encode_vi64(bufp, rid);
  Serialization::encode_vi64(bufp, rgr_id);
}

void RspColumnStatus::RangeStatus::decode(const uint8_t** bufp, 
                                          size_t* remainp) {
  state = (Types::MngrRange::State)Serialization::decode_i8(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  rgr_id = Serialization::decode_vi64(bufp, remainp);
}

void RspColumnStatus::RangeStatus::display(std::ostream& out, 
                                           const std::string& offset) const {
  out << offset << "state=" << Types::to_string(state) 
                << " rid=" << rid
                << " rgr_id=" << rgr_id;
}



RspColumnStatus::RspColumnStatus()
                  : state(Types::MngrColumn::State::NOTSET) { 
}

RspColumnStatus::~RspColumnStatus() { }

size_t RspColumnStatus::internal_encoded_length() const {
  size_t sz = 1;
  sz += Serialization::encoded_length_vi64(ranges.size());
  for(auto& r : ranges)
    sz += r.encoded_length();
  return sz;
}
  
void RspColumnStatus::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)state);
  Serialization::encode_vi64(bufp, ranges.size());
  for(auto& r : ranges)
    r.encode(bufp);
}
  
void RspColumnStatus::internal_decode(const uint8_t** bufp, size_t* remainp) {
  state = (Types::MngrColumn::State)Serialization::decode_i8(bufp, remainp);
  ranges.resize(Serialization::decode_vi64(bufp, remainp));
  for(auto& r : ranges)
    r.decode(bufp, remainp);
  std::sort(
    ranges.begin(), ranges.end(), 
    [](const RangeStatus& l, const RangeStatus& r) { return l.rid < r.rid; } 
  );
}

void RspColumnStatus::display(std::ostream& out, 
                              const std::string& offset) const {
  out << offset << "column-status=" << Types::to_string(state) 
                << " ranges=" << ranges.size();
  for(auto& r : ranges) 
    r.display(out << '\n', offset + "  ");
}





size_t RspRangersStatus::Ranger::encoded_length() const { 
  return  1
        + Serialization::encoded_length_vi64(rgr_id)
        + Serialization::encoded_length_vi32(failures)
        + Serialization::encoded_length_vi64(interm_ranges)
        + 2
        + Common::Params::HostEndPoints::internal_encoded_length();
}

void RspRangersStatus::Ranger::encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)state);
  Serialization::encode_vi64(bufp, rgr_id);
  Serialization::encode_vi32(bufp, failures);
  Serialization::encode_vi64(bufp, interm_ranges);
  Serialization::encode_i16(bufp, load_scale);
  Common::Params::HostEndPoints::internal_encode(bufp);
}

void RspRangersStatus::Ranger::decode(const uint8_t** bufp, size_t* remainp) {
  state = (Types::MngrRanger::State)Serialization::decode_i8(bufp, remainp);
  rgr_id = Serialization::decode_vi64(bufp, remainp);
  failures = Serialization::decode_vi32(bufp, remainp);
  interm_ranges = Serialization::decode_vi64(bufp, remainp);
  load_scale = Serialization::decode_i16(bufp, remainp);
  Common::Params::HostEndPoints::internal_decode(bufp, remainp);
}

void RspRangersStatus::Ranger::display(std::ostream& out, 
                                       const std::string& offset) const {
  out << offset << "state=" << Types::to_string(state) 
                << " rgr_id=" << rgr_id
                << " failures=" << failures
                << " interm_ranges=" << interm_ranges
                << " load_scale=" << load_scale
                << " " << Common::Params::HostEndPoints::to_string();
}


RspRangersStatus::RspRangersStatus() { }

RspRangersStatus::~RspRangersStatus() { }

size_t RspRangersStatus::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi64(rangers.size());
  for(auto& r : rangers)
    sz += r.encoded_length();
  return sz;
}
  
void RspRangersStatus::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, rangers.size());
  for(auto& r : rangers)
    r.encode(bufp);
}
  
void RspRangersStatus::internal_decode(const uint8_t** bufp, size_t* remainp) {
  rangers.resize(Serialization::decode_vi64(bufp, remainp));
  for(auto& r : rangers)
    r.decode(bufp, remainp);
  std::sort(
    rangers.begin(), rangers.end(), 
    [](const Ranger& l, const Ranger& r) { return l.rgr_id < r.rgr_id; } 
  );
}

void RspRangersStatus::display(std::ostream& out, 
                               const std::string& offset) const {
  out << offset << "rangers=" << rangers.size();
  for(auto& r : rangers) 
    r.display(out << '\n', offset + "  ");
}



}}}}}
