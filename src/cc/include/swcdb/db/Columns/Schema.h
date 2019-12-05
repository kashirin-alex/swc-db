/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Schema_h
#define swcdb_lib_db_Columns_Schema_h

#include "swcdb/core/Serialization.h"
#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Types/Encoding.h"

#include <shared_mutex>

namespace SWC { namespace DB {


class Schema final {

  public:
  
  typedef std::shared_ptr<Schema> Ptr;

  static const int64_t NO_CID = 0;

  inline static Ptr make(
         std::string col_name, 
         Types::Column col_type=Types::Column::PLAIN,
         int32_t cell_versions=1, uint32_t cell_ttl=0,
         uint8_t blk_replication=0, 
         Types::Encoding blk_encoding=Types::Encoding::DEFAULT,
         uint32_t blk_size=0,
         int64_t revision=0){
    return std::make_shared<Schema>(
      NO_CID, col_name, col_type, 
      cell_versions, cell_ttl, 
      blk_replication, blk_encoding, blk_size, revision);
  }

  inline static Ptr make(
         int64_t cid, std::string col_name, 
         Types::Column col_type=Types::Column::PLAIN,
         int32_t cell_versions=1, uint32_t cell_ttl=0,
         uint8_t blk_replication=0, 
         Types::Encoding blk_encoding=Types::Encoding::DEFAULT,
         uint32_t blk_size=0,
         int64_t revision=0){
    return std::make_shared<Schema>(
      cid, col_name, col_type, 
      cell_versions, cell_ttl, 
      blk_replication, blk_encoding, blk_size, revision);
  }
  
  inline static Ptr make(int64_t cid, Ptr other, int64_t revision){
    return std::make_shared<Schema>(
      cid, other->col_name, other->col_type, 
      other->cell_versions, other->cell_ttl, 
      other->blk_replication, other->blk_encoding, other->blk_size,
      revision);
  }
  
  inline static Ptr make(std::string col_name, Ptr other, 
                               int64_t revision){
    return std::make_shared<Schema>(
      other->cid, col_name, other->col_type, 
      other->cell_versions, other->cell_ttl, 
      other->blk_replication, other->blk_encoding, other->blk_size,
      revision);
  }

  Schema(int64_t cid, std::string col_name, Types::Column col_type,
         int32_t cell_versions, uint32_t cell_ttl,
         uint8_t blk_replication, Types::Encoding blk_encoding, 
         uint32_t blk_size, int64_t revision)
        : cid(cid), col_name(col_name), col_type(col_type),
          cell_versions(cell_versions), cell_ttl(cell_ttl),
          blk_replication(blk_replication),
          blk_encoding(blk_encoding), blk_size(blk_size), 
          revision(revision) {
  }
  
  Schema(const uint8_t **bufp, size_t *remainp)
    : cid(Serialization::decode_vi64(bufp, remainp)),
      col_name(Serialization::decode_vstr(bufp, remainp)),
      col_type((Types::Column)Serialization::decode_i8(bufp, remainp)),

      cell_versions(Serialization::decode_vi32(bufp, remainp)),
      cell_ttl(Serialization::decode_vi32(bufp, remainp)),

      blk_replication(Serialization::decode_i8(bufp, remainp)),
      blk_encoding((Types::Encoding)Serialization::decode_i8(bufp, remainp)),
      blk_size(Serialization::decode_vi32(bufp, remainp)),
      revision(Serialization::decode_vi64(bufp, remainp)) {
  }

  ~Schema() {}

  bool equal(const Ptr &other, bool with_rev=true) {
    return   cid == other->cid
          && col_type == other->col_type
          && cell_versions == other->cell_versions
          && cell_ttl == other->cell_ttl
          && blk_replication == other->blk_replication
          && blk_encoding == other->blk_encoding
          && blk_size == other->blk_size
          && col_name.compare(other->col_name) == 0
          && (!with_rev || revision == other->revision)
    ;
  }
  const size_t encoded_length() const {
    return Serialization::encoded_length_vi64(cid)
         + Serialization::encoded_length_vstr(col_name.length())
         + 1
         + Serialization::encoded_length_vi32(cell_versions)
         + Serialization::encoded_length_vi32(cell_ttl)
         + 1 
         + 1
         + Serialization::encoded_length_vi32(blk_size)
         + Serialization::encoded_length_vi64(revision);
  } 
 
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vstr(bufp, col_name.data(), col_name.length());
    Serialization::encode_i8(bufp, (uint8_t)col_type);

    Serialization::encode_vi32(bufp, cell_versions);
    Serialization::encode_vi32(bufp, cell_ttl);

    Serialization::encode_i8(bufp, blk_replication);
    Serialization::encode_i8(bufp, (uint8_t)blk_encoding);
    Serialization::encode_vi32(bufp, blk_size);
    Serialization::encode_vi64(bufp, revision);
  }

  const std::string to_string(){
    std::stringstream ss;
    ss << "Schema(" 
        << "cid=" << std::to_string(cid)
        << ", col_name=" << col_name
        << ", col_type=" << Types::to_string(col_type)
        << ", cell_versions=" << std::to_string(cell_versions)
        << ", cell_ttl=" << std::to_string(cell_ttl)
        << ", blk_replication=" << std::to_string(blk_replication)
        << ", blk_encoding=" << Types::to_string(blk_encoding)
        << ", blk_size=" << std::to_string(blk_size)
        << ", revision=" << std::to_string(revision)
        << ")"
       ;
    return ss.str();
  }


	const int64_t 		    cid;
	const std::string 		col_name;
	const Types::Column   col_type;

	const int32_t 		    cell_versions;
	const uint32_t 		    cell_ttl;

	const uint8_t 		    blk_replication;
	const Types::Encoding blk_encoding;
	const uint32_t        blk_size;

	const int64_t         revision;
};



class Schemas final {
  public:
  
  typedef Schemas* Ptr;

  Schemas(){}

  ~Schemas(){}
  
  void add(int &err, Schema::Ptr schema){
    std::lock_guard lock(m_mutex);
    if(!m_map.insert(
        std::pair<int64_t, Schema::Ptr>(schema->cid, schema)).second) {
      SWC_LOGF(LOG_WARN, "Unable to add column %s, remove first", 
                schema->to_string().c_str());
      err = Error::COLUMN_SCHEMA_NAME_EXISTS;
    }
  }

  void remove(int64_t cid){
    std::lock_guard lock(m_mutex);

    auto it = m_map.find(cid);
    if(it != m_map.end())
      m_map.erase(it);
  }

  void replace(Schema::Ptr schema){
    std::lock_guard lock(m_mutex);

    auto it = m_map.find(schema->cid);
    if(it == m_map.end())
       m_map.insert(std::pair<int64_t, Schema::Ptr>(schema->cid, schema));
    else
      it->second = schema;
  }

  Schema::Ptr get(int64_t cid){
    std::shared_lock lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end())
      return nullptr;
    return  it->second;
  }
  
  Schema::Ptr get(const std::string &name){
    std::shared_lock lock(m_mutex);
    
    for( const auto& it : m_map ) {
      if(name.compare(it.second->col_name) == 0)
        return it.second;
    }
    return nullptr;
  }

  void all(std::vector<Schema::Ptr> &entries){
    std::shared_lock lock(m_mutex);

    for( const auto& it : m_map) 
      entries.push_back(it.second);
  }

  private:
  std::shared_mutex                         m_mutex;
  std::unordered_map<int64_t, Schema::Ptr>  m_map;
};



} // DB namespace

namespace Env {
class Schemas final {
  public:

  static void init() {
    m_env = std::make_shared<Schemas>();
  }

  static DB::Schemas::Ptr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_schemas;
  }

  Schemas() : m_schemas(new DB::Schemas()) {}
  
  ~Schemas(){
    if(m_schemas != nullptr)
      delete m_schemas;
  }

  private:
  DB::Schemas::Ptr                       m_schemas = nullptr;
  inline static std::shared_ptr<Schemas> m_env = nullptr;
};
}

}
#endif