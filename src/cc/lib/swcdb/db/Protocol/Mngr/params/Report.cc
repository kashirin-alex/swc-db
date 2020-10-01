
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/Report.h"
#include "swcdb/core/Serialization.h"



namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params { 
namespace Report {



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
  state = (DB::Types::MngrRange::State)Serialization::decode_i8(bufp, remainp);
  rid = Serialization::decode_vi64(bufp, remainp);
  rgr_id = Serialization::decode_vi64(bufp, remainp);
}

void RspColumnStatus::RangeStatus::display(std::ostream& out, 
                                           const std::string& offset) const {
  out << offset << "state=" << DB::Types::to_string(state) 
                << " rid=" << rid
                << " rgr_id=" << rgr_id;
}



RspColumnStatus::RspColumnStatus()
                  : state(DB::Types::MngrColumn::State::NOTSET) { 
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
  state = (DB::Types::MngrColumn::State)Serialization::decode_i8(bufp, remainp);
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
  out << offset << "column-status=" << DB::Types::to_string(state) 
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
  state = (DB::Types::MngrRanger::State)Serialization::decode_i8(bufp, remainp);
  rgr_id = Serialization::decode_vi64(bufp, remainp);
  failures = Serialization::decode_vi32(bufp, remainp);
  interm_ranges = Serialization::decode_vi64(bufp, remainp);
  load_scale = Serialization::decode_i16(bufp, remainp);
  Common::Params::HostEndPoints::internal_decode(bufp, remainp);
}

void RspRangersStatus::Ranger::display(std::ostream& out, 
                                       const std::string& offset) const {
  out << offset << "state=" << DB::Types::to_string(state) 
                << " rgr_id=" << rgr_id
                << " failures=" << failures
                << " interm_ranges=" << interm_ranges
                << " load_scale=" << load_scale;
  Common::Params::HostEndPoints::print(out << ' ');
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





size_t RspManagersStatus::Manager::encoded_length() const { 
  return  Serialization::encoded_length_vi32(priority)
        + 2
        + Serialization::encoded_length_vi64(cid_begin)
        + Serialization::encoded_length_vi64(cid_end)
        + Serialization::encoded_length_vi32(failures)
        + Common::Params::HostEndPoints::internal_encoded_length();
}

void RspManagersStatus::Manager::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, priority);
  Serialization::encode_i8(bufp, (uint8_t)state);
  Serialization::encode_i8(bufp, role);
  Serialization::encode_vi64(bufp, cid_begin);
  Serialization::encode_vi64(bufp, cid_end);
  Serialization::encode_vi32(bufp, failures);
  Common::Params::HostEndPoints::internal_encode(bufp);
}

void RspManagersStatus::Manager::decode(const uint8_t** bufp, size_t* remainp) {
  priority = Serialization::decode_vi32(bufp, remainp);
  state = (DB::Types::MngrState)Serialization::decode_i8(bufp, remainp);
  role = Serialization::decode_i8(bufp, remainp);
  cid_begin = Serialization::decode_vi64(bufp, remainp);
  cid_end = Serialization::decode_vi64(bufp, remainp);
  failures = Serialization::decode_vi32(bufp, remainp);
  Common::Params::HostEndPoints::internal_decode(bufp, remainp);
}

void RspManagersStatus::Manager::display(std::ostream& out, 
                                       const std::string& offset) const {
  out << offset << "role=" << DB::Types::MngrRole::to_string(role);
  if(role & DB::Types::MngrRole::COLUMNS)
    out << " " << (cid_begin ? std::to_string(cid_begin) : "any") 
               << "<=CID<=" 
               << (cid_end ? std::to_string(cid_end) : "any");
  out  << " priority=" << priority
        << " state=" << DB::Types::to_string(state);
  Common::Params::HostEndPoints::print(out << ' ');
}


RspManagersStatus::RspManagersStatus() { }

RspManagersStatus::~RspManagersStatus() { }

size_t RspManagersStatus::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi64(managers.size());
  for(auto& m : managers)
    sz += m.encoded_length();
  sz += Serialization::encoded_length(inchain);
  return sz;
}
  
void RspManagersStatus::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, managers.size());
  for(auto& m : managers)
    m.encode(bufp);
  Serialization::encode(inchain, bufp);
}
  
void RspManagersStatus::internal_decode(const uint8_t** bufp, size_t* remainp) {
  managers.resize(Serialization::decode_vi64(bufp, remainp));
  for(auto& m : managers)
    m.decode(bufp, remainp);
  std::sort(
    managers.begin(), managers.end(), 
    [](const Manager& l, const Manager& r) {
      return l.role == r.role && l.cid_begin == r.cid_begin 
              ? l.priority < r.priority
              : l.role > r.role && l.cid_begin < r.cid_begin;
    } 
  );
  inchain = Serialization::decode(bufp, remainp);
}

void RspManagersStatus::display(std::ostream& out, 
                               const std::string& offset) const {
  out << offset << "managers=" << managers.size() 
                << " inchain=" << inchain;
  for(auto& m : managers) 
    m.display(out << '\n', offset + "  ");
}





}
}}}}}
