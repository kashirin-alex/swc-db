/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_config_Property_h
#define swcdb_core_config_Property_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/MutexAtomic.h"


namespace SWC { namespace Config {

typedef Core::Vector<std::string>  Strings;
typedef Core::Vector<int64_t>      Int64s;
typedef Core::Vector<double>       Doubles;

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
    TYPE_BOOL,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_DOUBLE,

    TYPE_STRING,
    TYPE_ENUM,

    TYPE_STRINGS,
    TYPE_INT64S,
    TYPE_DOUBLES,

    TYPE_BOOL_G,
    TYPE_UINT8_G,
    TYPE_UINT16_G,
    TYPE_INT32_G,
    TYPE_UINT64_G,
    TYPE_ENUM_G,
    TYPE_STRINGS_G
  };
  static const char* SWC_CONST_FUNC to_string(Type type) noexcept;

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

  Value(Value&&) = delete;
  Value(const Value&) = delete;
  Value& operator=(Value&&) = delete;
  Value& operator=(const Value&) = delete;

  virtual ~Value() noexcept;

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

void from_string(const char* s, double* value);

void from_string(const char* s, int64_t* value);

void from_string(const char* s, uint64_t* value);

void from_string(const char* s, uint8_t* value);

void from_string(const char* s, uint16_t* value);

void from_string(const char* s, int32_t* value);

template<typename T>
SWC_CAN_INLINE
void from_string(const std::string& s, T value) {
  from_string(s.c_str(), value);
}



class Value_bool final : public Value {
  public:
  static const Type value_type = TYPE_BOOL;

  Value_bool(const bool& v, uint8_t flags=0) noexcept;

  Value_bool(Value_bool* ptr) noexcept;

  virtual ~Value_bool() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  bool get() const noexcept {
    return value;
  }

  bool value;
};


class Value_uint8 final : public Value {
  public:
  static const Type value_type = TYPE_UINT8;

  Value_uint8(const uint8_t& v, uint8_t flags=0) noexcept;

  Value_uint8(Value_uint8* ptr) noexcept;

  virtual ~Value_uint8() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  uint8_t get() const noexcept {
    return value;
  }

  uint8_t value;
};


class Value_uint16 final : public Value {
  public:
  static const Type value_type = TYPE_UINT16;

  Value_uint16(const uint16_t& v, uint8_t flags=0) noexcept;

  Value_uint16(Value_uint16* ptr) noexcept;

  virtual ~Value_uint16() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  uint16_t get() const noexcept {
    return value;
  }

  uint16_t value;
};


class Value_int32 final : public Value {
  public:
  static const Type value_type = TYPE_INT32;

  Value_int32(const int32_t& v, uint8_t flags=0) noexcept;

  Value_int32(Value_int32* ptr) noexcept;

  virtual ~Value_int32() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  int32_t get() const noexcept {
    return value;
  }

  int32_t value;
};


class Value_int64 final : public Value {
  public:
  static const Type value_type = TYPE_INT64;

  Value_int64(const int64_t& v, uint8_t flags=0) noexcept;

  Value_int64(Value_int64* ptr) noexcept;

  virtual ~Value_int64() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  int64_t get() const noexcept {
    return value;
  }

  int64_t value;
};


class Value_double final : public Value {
  public:
  static const Type value_type = TYPE_DOUBLE;

  Value_double(const double& v, uint8_t flags=0) noexcept;

  Value_double(Value_double* ptr) noexcept;

  virtual ~Value_double() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  double get() const noexcept {
    return value;
  }

  double value;
};


class Value_string final : public Value {
  public:
  static const Type value_type = TYPE_STRING;

  Value_string(std::string&& v, uint8_t flags=0) noexcept;

  Value_string(Value_string* ptr);

  virtual ~Value_string() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  std::string get() const {
    return value;
  }

  std::string value;
};


class Value_enum final : public Value {
  public:
  static const Type value_type = TYPE_ENUM;

  typedef std::function<int(const std::string&)>  FromString_t;
  typedef std::function<std::string(int)>         Repr_t;

  Value_enum(const int32_t& v,
         FromString_t&& from_string,
         Repr_t&& repr,
         uint8_t flags=0);

  Value_enum(Value_enum* ptr);

  virtual ~Value_enum() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  int32_t get() const noexcept {
    return value;
  }

  int32_t       value;
  FromString_t  call_from_string = nullptr;
  Repr_t        call_repr = nullptr;
};

// lists
class Value_strings final : public Value {
  public:
  static const Type value_type = TYPE_STRINGS;

  Value_strings(Strings&& v, uint8_t flags=0) noexcept;

  Value_strings(Value_strings* ptr);

  virtual ~Value_strings() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  Strings get() const {
    return value;
  }

  Strings value;
};


class Value_int64s final : public Value {
  public:
  static const Type value_type = TYPE_INT64S;

  Value_int64s(Int64s&& v, uint8_t flags=0) noexcept;

  Value_int64s(Value_int64s* ptr);

  virtual ~Value_int64s() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  Int64s get() const {
    return value;
  }

  Int64s value;
};


class Value_doubles final : public Value {
  public:
  static const Type value_type = TYPE_DOUBLES;

  Value_doubles(Doubles&& v, uint8_t flags=0) noexcept;

  Value_doubles(Value_doubles* ptr);

  virtual ~Value_doubles() noexcept;

  Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  Doubles get() const {
    return value;
  }

  Doubles value;
};



// Guarded Atomic
class Value_bool_g final : public Value {
  public:
  static const Type value_type = TYPE_BOOL_G;

  typedef Value_bool_g*              Ptr;
  typedef std::function<void(bool)>  OnChg_t;

  Value_bool_g(const bool& v, OnChg_t&& cb, uint8_t flags=0);

  Value_bool_g(Value_bool_g* ptr);

  virtual ~Value_bool_g() noexcept;

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  bool get() const noexcept {
    return value;
  }

  void set(bool v) noexcept;

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::AtomicBool  value;
  OnChg_t           on_chg_cb;
};


class Value_uint8_g final : public Value {
  public:
  static const Type value_type = TYPE_UINT8_G;

  typedef Value_uint8_g*                     Ptr;
  typedef std::function<void(uint8_t)>  OnChg_t;

  Value_uint8_g(const uint8_t& v, OnChg_t&& cb, uint8_t flags=0);

  Value_uint8_g(Value_uint8_g* ptr);

  virtual ~Value_uint8_g() noexcept;

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  uint8_t get() const noexcept {
    return value;
  }

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<uint8_t> value;
  OnChg_t               on_chg_cb;
};


class Value_uint16_g final : public Value {
  public:
  static const Type value_type = TYPE_UINT16_G;

  typedef Value_uint16_g*                     Ptr;
  typedef std::function<void(uint16_t)>  OnChg_t;

  Value_uint16_g(const uint16_t& v, OnChg_t&& cb, uint8_t flags=0);

  Value_uint16_g(Value_uint16_g* ptr);

  virtual ~Value_uint16_g() noexcept;

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  uint16_t get() const noexcept {
    return value;
  }

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<uint16_t> value;
  OnChg_t                on_chg_cb;

};


class Value_int32_g final : public Value {
  public:
  static const Type value_type = TYPE_INT32_G;

  typedef Value_int32_g*                     Ptr;
  typedef std::function<void(int32_t)>  OnChg_t;

  Value_int32_g(const int32_t& v, OnChg_t&& cb, uint8_t flags=0);

  Value_int32_g(Value_int32_g* ptr);

  virtual ~Value_int32_g() noexcept;

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  int32_t get() const noexcept {
    return value;
  }

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<int32_t> value;
  OnChg_t               on_chg_cb;

};


class Value_uint64_g final : public Value {
  public:
  static const Type value_type = TYPE_UINT64_G;

  typedef Value_uint64_g*                     Ptr;
  typedef std::function<void(uint64_t)>  OnChg_t;

  Value_uint64_g(const uint64_t& v, OnChg_t&& cb, uint8_t flags=0);

  Value_uint64_g(Value_uint64_g* ptr);

  virtual ~Value_uint64_g() noexcept;

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  uint64_t get() const noexcept {
    return value;
  }

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<uint64_t> value;
  OnChg_t                on_chg_cb;

};


class Value_enum_g final : public Value {
  public:
  static const Type value_type = TYPE_ENUM_G;

  typedef Value_enum_g*                      Ptr;
  typedef std::function<void(int32_t)>  OnChg_t;
  typedef Value_enum::FromString_t          FromString_t;
  typedef Value_enum::Repr_t                Repr_t;

  Value_enum_g(const int32_t& v,
          OnChg_t&& cb,
          FromString_t&& from_string,
          Repr_t&& repr,
          uint8_t flags=0);

  Value_enum_g(Value_enum_g* ptr);

  virtual ~Value_enum_g() noexcept;

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

  std::string to_string() const override;

  SWC_CAN_INLINE
  int32_t get() const noexcept {
    return value;
  }

  void set(int32_t nv);

  void on_change() const;

  void set_cb_on_chg(OnChg_t&& cb);

  Core::Atomic<int32_t> value;
  OnChg_t               on_chg_cb;
  FromString_t          call_from_string = nullptr;
  Repr_t                call_repr = nullptr;
};



// Guarded Mutex
class Value_strings_g final : public Value {
  public:
  static const Type value_type = TYPE_STRINGS_G;

  typedef Value_strings_g*       Ptr;
  typedef std::function<void()>  OnChg_t;

  Value_strings_g(Strings&& v, OnChg_t&& cb, uint8_t flags=0) noexcept;

  Value_strings_g(Value_strings_g* ptr);

  virtual ~Value_strings_g() noexcept;

  Value::Ptr make_new(const Strings& values = Strings()) override;

  void set_from(Value::Ptr ptr) override;

  void set_from(const Strings& values) override;

  Type SWC_CONST_FUNC type() const noexcept override;

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
