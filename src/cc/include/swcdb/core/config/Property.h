/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_Property_h
#define swc_core_config_Property_h

#include <mutex>
#include <atomic>
#include <functional>
#include <vector>
#include "swcdb/core/Error.h"

namespace SWC {

typedef std::vector<std::string>  Strings;
typedef std::vector<int64_t>      Int64s;
typedef std::vector<double>       Doubles;

const uint64_t K = 1000;
const uint64_t KiB = 1024;
const uint64_t M = K * 1000;
const uint64_t MiB = KiB * 1024;
const uint64_t G = M * 1000;
const uint64_t GiB = MiB * 1024;


namespace Property {


/**
 * Convertors & Validators from std::string
*/

void from_string(const std::string& s, double* value);

void from_string(const std::string& s, int64_t* value);

void from_string(const std::string& s, uint8_t* value);

void from_string(const std::string& s, uint16_t* value);

void from_string(const std::string& s, int32_t* value);


class Value {
  public:

  enum Type {
    BOOL,
    UINT8,
    UINT16,
    INT32,
    INT64,
    DOUBLE,

    STRING,
    ENUM,

    STRINGS,
    INT64S,
    DOUBLES,
  
    G_BOOL,
    G_UINT8,
    G_INT32,
    G_ENUM,
    G_STRINGS
  };

  typedef Value* Ptr;

  Value(bool skippable=false, bool guarded=false);
  
  Value(Value* ptr);

  virtual ~Value();

  virtual Ptr make_new(const Strings& values = Strings()) = 0;
  
  virtual const Type type() const = 0;

  virtual void set_from(Ptr ptr) = 0;

  virtual void set_from(const Strings& values) = 0;

  virtual const std::string to_string() const = 0;

  std::ostream& operator<<(std::ostream& ostream);
  
  const bool is_skippable() const;

  const bool is_guarded() const;

  void guarded(bool guarded);

  Ptr default_value(bool defaulted);

  const bool is_default() const;
  
  Ptr zero_token();

  const bool is_zero_token() const;

  private:
  std::atomic<bool> m_skippable = false;
  std::atomic<bool> m_guarded   = false;
  std::atomic<bool> m_defaulted = false;
  std::atomic<bool> m_no_token  = false;

};



class V_BOOL : public Value {
  public:
  static const Type value_type = BOOL;
  
  V_BOOL(const bool& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }
  
  V_BOOL(V_BOOL* ptr) : Value(ptr), value(ptr->get()) { }
  
  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_BOOL(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }

  void set_from(Ptr ptr) override {
    auto from = ((V_BOOL*)ptr);
    value = from->value;
  }
  
  void set_from(const Strings& values) override {
    auto& str = values.back();
    value = str.compare("1") == 0 ||
            strncasecmp(str.data(), "true", 4) == 0 ||
            strncasecmp(str.data(), "yes", 3) == 0;
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return value ? "true" : "false";
  }

  bool get() const {
    return value;
  }

  bool value;
};


class V_UINT8 : public Value {
  public:
  static const Type value_type = UINT8;

  V_UINT8(const uint8_t& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }
  
  V_UINT8(V_UINT8* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_UINT8(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_UINT8*)ptr);
    value = from->value;
  }
  
  void set_from(const Strings& values) override {
    from_string(values.back(), &value);
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return std::to_string((int16_t)value);
  }

  uint8_t get() const {
    return value;
  }

  uint8_t value;
};


class V_UINT16 : public Value {
  public:
  static const Type value_type = UINT16;
  
  V_UINT16(const uint16_t& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }
  
  V_UINT16(V_UINT16* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_UINT16(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_UINT16*)ptr);
    value = from->value;
  }

  void set_from(const Strings& values) override {
    from_string(values.back(), &value);
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return std::to_string(value);
  }

  uint16_t get() const {
    return value;
  }

  uint16_t value;
};


class V_INT32 : public Value {
  public:
  static const Type value_type = INT32;
  
  V_INT32(const int32_t& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }

  V_INT32(V_INT32* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_INT32(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_INT32*)ptr);
    value = from->value;
  }

  void set_from(const Strings& values) override {
    from_string(values.back(), &value);
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return std::to_string(value);
  }

  int32_t get() const {
    return value;
  }

  int32_t value;
};


class V_INT64 : public Value {
  public:
  static const Type value_type = INT64;
  
  V_INT64(const int64_t& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }

  V_INT64(V_INT64* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_INT64(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_INT64*)ptr);
    value = from->value;
  }

  void set_from(const Strings& values) override {
    from_string(values.back(), &value);
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return std::to_string(value);
  }

  int64_t get() const {
    return value;
  }

  int64_t value;
};


class V_DOUBLE : public Value {
  public:
  static const Type value_type = DOUBLE;

  V_DOUBLE(const double& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }
  
  V_DOUBLE(V_DOUBLE* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_DOUBLE(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_DOUBLE*)ptr);
    value = from->value;
  }

  void set_from(const Strings& values) override {
    from_string(values.back(), &value);
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return format("%g", value);
  }

  double get() const {
    return value;
  }

  double value;
};


class V_STRING : public Value {
  public:
  static const Type value_type = STRING;

  V_STRING(const std::string& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }
  
  V_STRING(V_STRING* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_STRING(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_STRING*)ptr);
    value = from->value;
  }

  void set_from(const Strings& values) override {
    value = values.back();
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return value;
  }

  std::string get() const {
    return value;
  }

  std::string value;
};


class V_ENUM : public Value {
  public:
  static const Type value_type = ENUM;

  typedef std::function<int(const std::string&)>  FromString_t; 
  typedef std::function<std::string(int)>         Repr_t; 

  V_ENUM(const int32_t& v, 
         const FromString_t& from_string, 
         const Repr_t& repr,
         bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), 
           value(v), call_from_string(from_string), call_repr(repr) {
  }

  V_ENUM(V_ENUM* ptr) 
        : Value(ptr), value(ptr->get()),
          call_from_string(ptr->call_from_string), call_repr(ptr->call_repr) { 
  }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_ENUM(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_ENUM*)ptr);
    value = from->value;
    call_from_string = from->call_from_string;
    call_repr = from->call_repr;
  }

  void set_from(const Strings& values) override {
    if(!call_from_string)
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s, no from_string cb set", values.back().c_str());
    
    int nv = call_from_string(values.back());
    if(nv < 0) 
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s, no corresponding enum", values.back().c_str());
    value = nv;
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return format(
      "%s  # (%d)", 
      (call_repr ? call_repr(get()).c_str() : "repr not defined"), get());
  }

  int32_t get() const {
    return value;
  }

  int32_t       value;
  FromString_t  call_from_string = 0;
  Repr_t        call_repr = 0;
};

// lists
class V_STRINGS : public Value {
  public:
  static const Type value_type = STRINGS;

  V_STRINGS(const Strings& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }
  
  V_STRINGS(V_STRINGS* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_STRINGS(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_STRINGS*)ptr);
    value = from->value;
  }

  void set_from(const Strings& values) override {
    value = values;
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return format_list(value);
  }

  Strings get() const {
    return value;
  }

  Strings value;
};


class V_INT64S : public Value {
  public:
  static const Type value_type = INT64S;

  V_INT64S(const Int64s& v, bool skippable=false, bool guarded=false)
         : Value(skippable, guarded), value(v) {
  }
  
  V_INT64S(V_INT64S* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_INT64S(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_INT64S*)ptr);
    value = from->value;
  }

  void set_from(const Strings& values) override {
    value.clear();
    int64_t v;
    for(const std::string& s: values) {
      from_string(s, &v);
      value.push_back(v);
    }
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return format_list(value);
  }

  Int64s get() const {
    return value;
  }

  Int64s value;
};


class V_DOUBLES : public Value {
  public:
  static const Type value_type = DOUBLES;

  V_DOUBLES(const Doubles& v, bool skippable=false, bool guarded=false)
            : Value(skippable, guarded), value(v) {
  }
  
  V_DOUBLES(V_DOUBLES* ptr) : Value(ptr), value(ptr->get()) { }

  Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_DOUBLES(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Ptr ptr) override {
    auto from = ((V_DOUBLES*)ptr);
    value = from->get();
  }

  void set_from(const Strings& values) override {
    value.clear();
    double v;
    for(const std::string& s: values) {
      from_string(s, &v);
      value.push_back(v);
    }
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return format_list(value);
  }

  Doubles get() const {
    return value;
  }

  Doubles value;
};


// Guarded Atomic
class V_GBOOL : public Value {
  public:
  static const Type value_type = G_BOOL;

  typedef V_GBOOL*                   Ptr;
  typedef std::function<void(bool)>  OnChg_t;
  
  V_GBOOL(const bool& v, const OnChg_t& cb, bool skippable=false, bool guarded=true)
         : Value(skippable, guarded), value(v), on_chg_cb(cb) {
  }

  V_GBOOL(V_GBOOL* ptr) 
          : Value(ptr), 
            value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
  }
  
  Value::Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_GBOOL(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }

  void set_from(Value::Ptr ptr) override {
    auto from = ((V_GBOOL*)ptr);
    value.store(from->value.load());
    on_chg_cb = from->on_chg_cb;
  }
  
  void set_from(const Strings& values) override {
    auto& str = values.back();
    bool nv = str.compare("1") == 0 ||
              strncasecmp(str.data(), "true", 4) == 0 ||
              strncasecmp(str.data(), "yes", 3) == 0;
    bool chg = nv != value;
    value = nv;
    if(chg) 
      on_change();
  }

  const Type type() const {
    return value_type;
  }

  bool get() const {
    return value;
  }

  void set(bool v) {
    value.store(v);
  }

  const std::string to_string() const override {
    return value ? "true" : "false";
  }


  void on_change() const {
    if(on_chg_cb)
      on_chg_cb(get());
  }

  void set_cb_on_chg(const OnChg_t& cb) {
    on_chg_cb = cb;
  }

  std::atomic<bool> value;
  OnChg_t           on_chg_cb;
};


class V_GUINT8 : public Value {
  public:
  static const Type value_type = G_UINT8;
  
  typedef V_GUINT8*                     Ptr;
  typedef std::function<void(uint8_t)>  OnChg_t;

  V_GUINT8(const uint8_t& v, const OnChg_t& cb, bool skippable=false, bool guarded=true)
          : Value(skippable, guarded), value(v), on_chg_cb(cb) {
  }

  V_GUINT8(V_GUINT8* ptr) 
          : Value(ptr), 
            value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
  }

  Value::Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_GUINT8(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Value::Ptr ptr) override {
    auto from = ((V_GUINT8*)ptr);
    value.store(from->value.load());
    on_chg_cb = from->on_chg_cb;
  }
  
  void set_from(const Strings& values) override {
    uint8_t nv;
    from_string(values.back(), &nv);
    bool chg = nv != value;
    value = nv;
    if(chg) 
      on_change();
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return std::to_string((int16_t)value);
  }

  uint8_t get() const {
    return value;
  }

  void on_change() const {
    if(on_chg_cb)
      on_chg_cb(get());
  }

  void set_cb_on_chg(const OnChg_t& cb) {
    on_chg_cb = cb;
  }

  std::atomic<uint8_t>  value;
  OnChg_t               on_chg_cb;
};


class V_GINT32 : public Value {
  public:
  static const Type value_type = G_INT32;
  
  typedef V_GINT32*                     Ptr;
  typedef std::function<void(int32_t)>  OnChg_t;

  V_GINT32(const int32_t& v, const OnChg_t& cb, bool skippable=false, bool guarded=true)
          : Value(skippable, guarded), value(v), on_chg_cb(cb) {
  }

  V_GINT32(V_GINT32* ptr) 
          : Value(ptr), 
            value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
  }


  Value::Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_GINT32(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Value::Ptr ptr) override {
    auto from = ((V_GINT32*)ptr);
    value.store(from->value.load());
    on_chg_cb = from->on_chg_cb;
  }

  void set_from(const Strings& values) override {
    int32_t nv;
    from_string(values.back(), &nv);
    bool chg = nv != value;
    value = nv;
    if(chg) 
      on_change();
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return std::to_string(value);
  }

  int32_t get() const {
    return value;
  }

  void on_change() const {
    if(on_chg_cb)
      on_chg_cb(get());
  }

  void set_cb_on_chg(const OnChg_t& cb) {
    on_chg_cb = cb;
  }

  std::atomic<int32_t>  value;
  OnChg_t               on_chg_cb;

};


class V_GENUM : public Value {
  public:
  static const Type value_type = G_ENUM;
  
  typedef V_GENUM*                      Ptr;
  typedef std::function<void(int32_t)>  OnChg_t;
  typedef V_ENUM::FromString_t          FromString_t; 
  typedef V_ENUM::Repr_t                Repr_t; 

  V_GENUM(const int32_t& v, 
          const OnChg_t& cb, 
          const FromString_t& from_string, 
          const Repr_t& repr,
          bool skippable=false, bool guarded=false)
          : Value(skippable, guarded), value(v), on_chg_cb(cb), 
            call_from_string(from_string), call_repr(repr) {
  }

  V_GENUM(V_GENUM* ptr) 
        : Value(ptr), value(ptr->get()), on_chg_cb(ptr->on_chg_cb),
          call_from_string(ptr->call_from_string), call_repr(ptr->call_repr) { 
  }

  Value::Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_GENUM(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Value::Ptr ptr) override {
    auto from = ((V_GENUM*)ptr);
    value.store(from->get());
    on_chg_cb = from->on_chg_cb;
    call_from_string = from->call_from_string;
    call_repr = from->call_repr;
  }

  void set_from(const Strings& values) override {
    if(!call_from_string)
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s, no from_string cb set", values.back().c_str());
    
    int nv = call_from_string(values.back());
    if(nv < 0) 
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s, no corresponding enum", values.back().c_str());

    bool chg = nv != get();
    value.store(nv);
    if(chg) 
      on_change();
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    return format(
      "%s  # (%d)", 
      (call_repr ? call_repr(get()).c_str() : "repr not defined"), get());
  }

  int32_t get() const {
    return value;
  }

  void set(int32_t nv) {
    bool chg = nv != get();
    value.store(nv);
    if(chg) 
      on_change();
  }

  void on_change() const {
    if(on_chg_cb)
      on_chg_cb(get());
  }

  void set_cb_on_chg(const OnChg_t& cb) {
    on_chg_cb = cb;
  }

  std::atomic<int32_t>  value;
  OnChg_t               on_chg_cb;
  FromString_t          call_from_string = 0;
  Repr_t                call_repr = 0;
};


// Guarded Mutex
class V_GSTRINGS : public Value {
  public:
  static const Type value_type = G_STRINGS;
  
  typedef V_GSTRINGS*            Ptr;
  typedef std::function<void()>  OnChg_t;

  V_GSTRINGS(const Strings& v, const OnChg_t& cb, bool skippable=false, bool guarded=true)
          : Value(skippable, guarded), value(v), on_chg_cb(cb) {
  }

  V_GSTRINGS(V_GSTRINGS* ptr) 
          : Value(ptr), 
            value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
  }


  Value::Ptr make_new(const Strings& values = Strings()) override {
    auto ptr = new V_GSTRINGS(this);
    if(!values.empty())
      ptr->set_from(values);
    return ptr;
  }
  
  void set_from(Value::Ptr ptr) override {
    auto from = ((V_GSTRINGS*)ptr);
    
    std::scoped_lock lock(mutex);
    value = from->get();
    on_chg_cb = from->on_chg_cb;
  }

  void set_from(const Strings& values) override {
    bool chg;
    {
      std::scoped_lock lock(mutex);
      chg = values != value;
      value = values;
    }
    if(chg) 
      on_change();
  }

  const Type type() const override {
    return value_type;
  }

  const std::string to_string() const override {
    std::scoped_lock lock(mutex);
    return format_list(value);
  }

  Strings get() const {
    std::scoped_lock lock(mutex);
    return value;
  }

  size_t size() {
    std::scoped_lock lock(mutex);	
    return value.size();
  }

  std::string get_item(size_t n) {
    std::scoped_lock lock(mutex);
    return value[n];
  }

  void on_change() const {
    if(on_chg_cb)
      on_chg_cb();
  }

  void set_cb_on_chg(const OnChg_t& cb) {
    std::scoped_lock lock(mutex);
    on_chg_cb = cb;
  }

  mutable std::mutex mutex;
  Strings            value;
  OnChg_t            on_chg_cb;

};



}} // namespace SWC::Property


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Property.cc"
#endif 

#endif // swc_core_config_Property_h
