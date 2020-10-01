/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_config_Property_h
#define swcdb_core_config_Property_h

#include "swcdb/core/LockAtomicUnique.h"
#include <functional>
#include <vector>

namespace SWC { namespace Config {

typedef std::vector<std::string>  Strings;
typedef std::vector<int64_t>      Int64s;
typedef std::vector<double>       Doubles;

const uint64_t K = 1000;
const uint64_t KiB = 1024;
const uint64_t M = K * 1000;
const uint64_t MiB = KiB * 1024;
const uint64_t G = M * 1000;
const uint64_t GiB = MiB * 1024;
const uint64_t T = G * 1000;
const uint64_t TiB = GiB * 1024;

namespace Property {


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

  static const uint8_t SKIPPABLE   = 0x01;
  static const uint8_t GUARDED     = 0x02;
  static const uint8_t DEFAULT     = 0x04;
  static const uint8_t NO_TOKEN    = 0x08;

  typedef Value* Ptr;

  Value(uint8_t flags=0);
  
  Value(Ptr ptr);

  virtual ~Value();

  virtual Ptr make_new(const Strings& values = Strings()) = 0;
  
  virtual Type type() const = 0;

  virtual void set_from(Ptr ptr) = 0;

  virtual void set_from(const Strings& values) = 0;

  virtual std::string to_string() const = 0;

  std::ostream& operator<<(std::ostream& ostream);
  
  bool is_skippable() const;

  bool is_guarded() const;

  Ptr default_value(bool defaulted);

  bool is_default() const;
  
  Ptr zero_token();

  bool is_zero_token() const;

  std::atomic<uint8_t> flags;
};


/**
 * Convertors & Validators from std::string
*/

void from_string(const std::string& s, double* value);

void from_string(const std::string& s, int64_t* value);

void from_string(const std::string& s, uint8_t* value);

void from_string(const std::string& s, uint16_t* value);

void from_string(const std::string& s, int32_t* value);



class V_BOOL final : public Value {
  public:
  static const Type value_type = BOOL;
  
  V_BOOL(const bool& v, uint8_t flags=0);
  
  V_BOOL(V_BOOL* ptr);
  
  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;
  
  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  bool get() const;

  bool value;
};


class V_UINT8 final : public Value {
  public:
  static const Type value_type = UINT8;

  V_UINT8(const uint8_t& v, uint8_t flags=0);
  
  V_UINT8(V_UINT8* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;
  
  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  uint8_t get() const;

  uint8_t value;
};


class V_UINT16 final : public Value {
  public:
  static const Type value_type = UINT16;
  
  V_UINT16(const uint16_t& v, uint8_t flags=0);
  
  V_UINT16(V_UINT16* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  uint16_t get() const;

  uint16_t value;
};


class V_INT32 final : public Value {
  public:
  static const Type value_type = INT32;
  
  V_INT32(const int32_t& v, uint8_t flags=0);

  V_INT32(V_INT32* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  int32_t get() const;

  int32_t value;
};


class V_INT64 final : public Value {
  public:
  static const Type value_type = INT64;
  
  V_INT64(const int64_t& v, uint8_t flags=0);

  V_INT64(V_INT64* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  int64_t get() const;

  int64_t value;
};


class V_DOUBLE final : public Value {
  public:
  static const Type value_type = DOUBLE;

  V_DOUBLE(const double& v, uint8_t flags=0);
  
  V_DOUBLE(V_DOUBLE* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  double get() const;

  double value;
};


class V_STRING final : public Value {
  public:
  static const Type value_type = STRING;

  V_STRING(const std::string& v, uint8_t flags=0);
  
  V_STRING(V_STRING* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  std::string get() const;

  std::string value;
};


class V_ENUM final : public Value {
  public:
  static const Type value_type = ENUM;

  typedef std::function<int(const std::string&)>  FromString_t; 
  typedef std::function<std::string(int)>         Repr_t; 

  V_ENUM(const int32_t& v, 
         const FromString_t& from_string, 
         const Repr_t& repr,
         uint8_t flags=0);

  V_ENUM(V_ENUM* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  int32_t get() const;

  int32_t       value;
  FromString_t  call_from_string = 0;
  Repr_t        call_repr = 0;
};

// lists
class V_STRINGS final : public Value {
  public:
  static const Type value_type = STRINGS;

  V_STRINGS(const Strings& v, uint8_t flags=0);
  
  V_STRINGS(V_STRINGS* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  Strings get() const;

  Strings value;
};


class V_INT64S final : public Value {
  public:
  static const Type value_type = INT64S;

  V_INT64S(const Int64s& v, uint8_t flags=0);
  
  V_INT64S(V_INT64S* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  Int64s get() const;

  Int64s value;
};


class V_DOUBLES final : public Value {
  public:
  static const Type value_type = DOUBLES;

  V_DOUBLES(const Doubles& v, uint8_t flags=0);
  
  V_DOUBLES(V_DOUBLES* ptr);

  Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  Doubles get() const;

  Doubles value;
};



// Guarded Atomic
class V_GBOOL final : public Value {
  public:
  static const Type value_type = G_BOOL;

  typedef V_GBOOL*                   Ptr;
  typedef std::function<void(bool)>  OnChg_t;
  
  V_GBOOL(const bool& v, const OnChg_t& cb, uint8_t flags=0);

  V_GBOOL(V_GBOOL* ptr);
  
  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;
  
  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  bool get() const;

  void set(bool v);

  void on_change() const;

  void set_cb_on_chg(const OnChg_t& cb);

  std::atomic<bool> value;
  OnChg_t           on_chg_cb;
};


class V_GUINT8 final : public Value {
  public:
  static const Type value_type = G_UINT8;
  
  typedef V_GUINT8*                     Ptr;
  typedef std::function<void(uint8_t)>  OnChg_t;

  V_GUINT8(const uint8_t& v, const OnChg_t& cb, uint8_t flags=0);

  V_GUINT8(V_GUINT8* ptr);

  Value::Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Value::Ptr ptr) override;
  
  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  uint8_t get() const;

  void on_change() const;

  void set_cb_on_chg(const OnChg_t& cb);

  std::atomic<uint8_t>  value;
  OnChg_t               on_chg_cb;
};


class V_GINT32 final : public Value {
  public:
  static const Type value_type = G_INT32;
  
  typedef V_GINT32*                     Ptr;
  typedef std::function<void(int32_t)>  OnChg_t;

  V_GINT32(const int32_t& v, const OnChg_t& cb, uint8_t flags=0);

  V_GINT32(V_GINT32* ptr);


  Value::Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  int32_t get() const;

  void on_change() const;

  void set_cb_on_chg(const OnChg_t& cb);

  std::atomic<int32_t>  value;
  OnChg_t               on_chg_cb;

};


class V_GENUM final : public Value {
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
          uint8_t flags=0);

  V_GENUM(V_GENUM* ptr);

  Value::Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  int32_t get() const;

  void set(int32_t nv);

  void on_change() const;

  void set_cb_on_chg(const OnChg_t& cb);

  std::atomic<int32_t>  value;
  OnChg_t               on_chg_cb;
  FromString_t          call_from_string = 0;
  Repr_t                call_repr = 0;
};



// Guarded Mutex
class V_GSTRINGS final : public Value {
  public:
  static const Type value_type = G_STRINGS;
  
  typedef V_GSTRINGS*            Ptr;
  typedef std::function<void()>  OnChg_t;

  V_GSTRINGS(const Strings& v, const OnChg_t& cb, uint8_t flags=0);

  V_GSTRINGS(V_GSTRINGS* ptr);

  Value::Ptr make_new(const Strings& values = Strings()) override;
  
  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const override;

  std::string to_string() const override;

  Strings get() const;

  size_t size();

  std::string get_item(size_t n);

  void on_change() const;

  void set_cb_on_chg(const OnChg_t& cb);

  mutable LockAtomic::Unique  mutex;
  Strings                     value;
  OnChg_t                     on_chg_cb;

};





}}} // namespace SWC::Config::Property


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Property.cc"
#endif 

#endif // swcdb_core_config_Property_h
