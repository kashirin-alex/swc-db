/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

/** @file
 * A Property Value Guarded used with Properties 
 */

#ifndef swc_core_config_PropertyValueGuarded_h
#define swc_core_config_PropertyValueGuarded_h


namespace SWC {


typedef std::vector<std::string>  Strings;
typedef std::vector<int64_t>      Int64s;
typedef std::vector<double>       Doubles;


namespace Property {

template <class T>
class ValueGuardedAtomic {
  
  public:

  ValueGuardedAtomic () noexcept {}
 
  ValueGuardedAtomic (T nv) noexcept {
    store(nv);
  }
 
  ValueGuardedAtomic (ValueGuardedAtomic &other) noexcept {
    store(other.get());
  }
 
  void store(T nv){
    if(nv == vg.load()) 
      return;
    vg.store(nv);
    if(on_chg_cb)
      on_chg_cb();
  }

  ~ValueGuardedAtomic () noexcept {};
    
  operator ValueGuardedAtomic*() { 
    return this;    
  }
    
  ValueGuardedAtomic* operator =(T nv) {
    store(nv);
    return *this;
  }

  ValueGuardedAtomic* operator =(ValueGuardedAtomic &other) {
    store(other.get());
    return *this;
  }
    
  operator T() {
    return vg.load(); 
  }

  T get() {
    return vg.load(); 
  }

  operator std::atomic<T>*() {
    return &vg; 
  }

  void set_cb_on_chg(std::function<void()> cb) {
    on_chg_cb = cb;
  }

  private:
  std::atomic<T> vg;
  std::function<void()> on_chg_cb;
};

}

typedef Property::ValueGuardedAtomic<bool>      gBool;
typedef Property::ValueGuardedAtomic<int32_t>   gInt32t;
typedef gBool*    gBoolPtr;
typedef gInt32t*  gInt32tPtr;



namespace Property {

template <class T>
class ValueGuardedVector {

  private:
  std::mutex mutex;	
  T v;
  using T_of = typename std::decay<decltype(*v.begin())>::type;
  std::function<void()> on_chg_cb;

  public:

  ValueGuardedVector () noexcept {}
 
  ValueGuardedVector (T nv) noexcept {
    set(nv);
  }
 
  ValueGuardedVector (ValueGuardedVector &other) noexcept {
    set(other.get());
  }
 
  ~ValueGuardedVector () noexcept {};
    
  operator ValueGuardedVector*() {
    return this;    
  }
    
  ValueGuardedVector* operator =(T nv){
    set(nv);
      
    if(on_chg_cb)
      on_chg_cb();
    return *this;
  }

  ValueGuardedVector* operator =(ValueGuardedVector &other){
    set(other.get());
    return *this;
  } 
    
  void set(T nv)  {
    std::lock_guard<std::mutex> lock(mutex);	
    v.swap(nv);
  }
    
  operator T(){
    return get(); 
  }

  T get(){
    std::lock_guard<std::mutex> lock(mutex);	
    return v;      
  }

  size_t size()  {
    std::lock_guard<std::mutex> lock(mutex);	
    return v.size();
  }

  T_of get_item(size_t n){
    std::lock_guard<std::mutex> lock(mutex);	
    return v[n];
  }

  void set_cb_on_chg(std::function<void()> cb) {
    on_chg_cb = cb;
  }

};

} // namespace Property


typedef Property::ValueGuardedVector<Strings> gStrings;
typedef gStrings*                             gStringsPtr;


} // namespace SWC

#endif // swc_core_config_PropertyValueGuarded_h
