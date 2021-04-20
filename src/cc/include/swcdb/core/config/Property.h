/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_config_Property_h
#define swcdb_core_config_Property_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/MutexAtomic.h"


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


//! The SWC-DB Property C++ namespace 'SWC::Config::Property'
namespace Property {


class Value {
  public:

  enum Type : uint8_t {
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
  static const char* to_string(Type type) noexcept;

  static const uint8_t SKIPPABLE   = 0x01;
  static const uint8_t GUARDED     = 0x02;
  static const uint8_t DEFAULT     = 0x04;
  static const uint8_t NO_TOKEN    = 0x08;

  typedef Value* Ptr;

  template <typename T>
  SWC_CAN_INLINE
  static T* get_pointer(Ptr ptr) {
    assure_match(ptr->type(), T::value_type);
    return static_cast<T*>(ptr);
  }

  static void assure_match(Type t1, Type t2);

  Value(uint8_t flags=0) noexcept;

  Value(Ptr ptr) noexcept;

  virtual ~Value() { }

  virtual Ptr make_new(const Strings& values = Strings()) = 0;

  virtual Type type() const noexcept = 0;

  virtual void set_from(Ptr ptr) = 0;

  virtual void set_from(const Strings& values) = 0;

  virtual std::string to_string() const = 0;

  std::ostream& operator<<(std::ostream& ostream);

  bool is_skippable() const noexcept;

  bool is_guarded() const noexcept;

  Ptr default_value(bool defaulted) noexcept;

  bool is_default() const noexcept;

  Ptr zero_token() noexcept;

  bool is_zero_token() const noexcept;

  Core::Atomic<uint8_t> flags;

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

  V_BOOL(const bool& v, uint8_t flags=0) noexcept;

  V_BOOL(V_BOOL* ptr) noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  bool get() const noexcept;

  bool value;
};


class V_UINT8 final : public Value {
  public:
  static const Type value_type = UINT8;

  V_UINT8(const uint8_t& v, uint8_t flags=0) noexcept;

  V_UINT8(V_UINT8* ptr) noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  uint8_t get() const noexcept;

  uint8_t value;
};


class V_UINT16 final : public Value {
  public:
  static const Type value_type = UINT16;

  V_UINT16(const uint16_t& v, uint8_t flags=0) noexcept;

  V_UINT16(V_UINT16* ptr) noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  uint16_t get() const noexcept;

  uint16_t value;
};


class V_INT32 final : public Value {
  public:
  static const Type value_type = INT32;

  V_INT32(const int32_t& v, uint8_t flags=0) noexcept;

  V_INT32(V_INT32* ptr) noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  int32_t get() const noexcept;

  int32_t value;
};


class V_INT64 final : public Value {
  public:
  static const Type value_type = INT64;

  V_INT64(const int64_t& v, uint8_t flags=0) noexcept;

  V_INT64(V_INT64* ptr) noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  int64_t get() const noexcept;

  int64_t value;
};


class V_DOUBLE final : public Value {
  public:
  static const Type value_type = DOUBLE;

  V_DOUBLE(const double& v, uint8_t flags=0) noexcept;

  V_DOUBLE(V_DOUBLE* ptr) noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  double get() const noexcept;

  double value;
};


class V_STRING final : public Value {
  public:
  static const Type value_type = STRING;

  V_STRING(std::string&& v, uint8_t flags=0) noexcept;

  V_STRING(V_STRING* ptr);

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

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
         FromString_t&& from_string,
         Repr_t&& repr,
         uint8_t flags=0);

  V_ENUM(V_ENUM* ptr);

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  int32_t get() const noexcept;

  int32_t       value;
  FromString_t  call_from_string = nullptr;
  Repr_t        call_repr = nullptr;
};

// lists
class V_STRINGS final : public Value {
  public:
  static const Type value_type = STRINGS;

  V_STRINGS(Strings&& v, uint8_t flags=0) noexcept;

  V_STRINGS(V_STRINGS* ptr);

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  Strings get() const;

  Strings value;
};


class V_INT64S final : public Value {
  public:
  static const Type value_type = INT64S;

  V_INT64S(Int64s&& v, uint8_t flags=0) noexcept;

  V_INT64S(V_INT64S* ptr);

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  Int64s get() const;

  Int64s value;
};


class V_DOUBLES final : public Value {
  public:
  static const Type value_type = DOUBLES;

  V_DOUBLES(Doubles&& v, uint8_t flags=0) noexcept;

  V_DOUBLES(V_DOUBLES* ptr);

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

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

  V_GBOOL(const bool& v, OnChg_t&& cb, uint8_t flags=0);

  V_GBOOL(V_GBOOL* ptr);

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  bool get() const noexcept;

  void set(bool v) noexcept;

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::AtomicBool  value;
  OnChg_t           on_chg_cb;
};


class V_GUINT8 final : public Value {
  public:
  static const Type value_type = G_UINT8;

  typedef V_GUINT8*                     Ptr;
  typedef std::function<void(uint8_t)>  OnChg_t;

  V_GUINT8(const uint8_t& v, OnChg_t&& cb, uint8_t flags=0);

  V_GUINT8(V_GUINT8* ptr);

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  uint8_t get() const noexcept;

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<uint8_t> value;
  OnChg_t               on_chg_cb;
};


class V_GINT32 final : public Value {
  public:
  static const Type value_type = G_INT32;

  typedef V_GINT32*                     Ptr;
  typedef std::function<void(int32_t)>  OnChg_t;

  V_GINT32(const int32_t& v, OnChg_t&& cb, uint8_t flags=0);

  V_GINT32(V_GINT32* ptr);


  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  int32_t get() const noexcept;

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<int32_t> value;
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
          OnChg_t&& cb,
          FromString_t&& from_string,
          Repr_t&& repr,
          uint8_t flags=0);

  V_GENUM(V_GENUM* ptr);

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  int32_t get() const noexcept;

  void set(int32_t nv);

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<int32_t> value;
  OnChg_t               on_chg_cb;
  FromString_t          call_from_string = nullptr;
  Repr_t                call_repr = nullptr;
};



// Guarded Mutex
class V_GSTRINGS final : public Value {
  public:
  static const Type value_type = G_STRINGS;

  typedef V_GSTRINGS*            Ptr;
  typedef std::function<void()>  OnChg_t;

  V_GSTRINGS(Strings&& v, OnChg_t&& cb, uint8_t flags=0) noexcept;

  V_GSTRINGS(V_GSTRINGS* ptr);

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type type() const noexcept override;

  std::string to_string() const override;

  Strings get() const;

  size_t size() noexcept;

  std::string get_item(size_t n);

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  mutable Core::MutexAtomic   mutex;
  Strings                     value;
  OnChg_t                     on_chg_cb;

};





}}} // namespace SWC::Config::Property


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Property.cc"
#endif

#endif // swcdb_core_config_Property_h
