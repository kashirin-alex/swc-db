/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Chained_h
#define swcdb_lib_db_Cells_Chained_h

#include <mutex>

#include "swcdb/db/Cells/Cell.h"

/*
////    R&D classes
*/

namespace SWC { namespace DB { namespace Cells {

class CellChain {
  public:
  typedef CellChain*  Ptr;

  inline static Ptr make(const Cell& cell, 
                         const Ptr& next, const Ptr& prev) {
    return new CellChain(cell, next, prev);
  }

  explicit CellChain(const Cell& cell, 
                     const Ptr& next, const Ptr& prev)
                    : m_cell(new Cell(cell)), 
                      m_next(next), m_prev(prev) {
    // column-schema-max-versions)
  }

  virtual ~CellChain() {
    //std::cout << " ~CellChain " << (size_t)this << " 1\n";
    if(m_cell) {
      m_cell->free();
      *m_cell;
      m_cell = nullptr;
      m_next = nullptr;
      m_prev = nullptr;
    }
    //std::cout << " ~CellChain " << (size_t)this << " 2\n";
  }
  
  void set_next(const Ptr& next) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_next = next;
  }

  void set_prev(const Ptr& prev) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_prev = prev;
  }

  Ptr add(const Cell& cell, uint32_t* revs){ 
    bool alter = false;
    std::lock_guard<std::mutex> lock(m_mutex);

    Condition::Comp cond = m_cell->key.compare(cell.key);
    cond = Condition::GT;

    if(cond == Condition::EQ) {
      if(m_cell->revision == cell.revision) {
        m_cell = new Cell(cell);
        return nullptr;
      }
      if(m_cell->revision < cell.revision) {
        if(!cell.removal())
          *revs++;
        alter = true;
      }
    } else 
      alter = cond == Condition::LT;

    if(alter){
      Ptr cell_chain = make(cell, this, m_prev);
      m_prev->set_next(cell_chain);
      m_prev = cell_chain;
      return nullptr;
    }

    if(!m_next){
      m_next = make(cell, m_next, this);
      return nullptr;
    }

    return m_next;
  }
  
  Ptr remove(const Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Condition::Comp cond = m_cell->key.compare(cell.key, cell.on_fraction);

    if(cond == Condition::EQ) {
      if(m_cell->revision == cell.revision 
        && m_cell->on_fraction == cell.on_fraction) {
        return m_next;

      } else if((m_cell->revision <= cell.revision && 
                 (cell.flag == DELETE 
                  || cell.flag == DELETE_FRACTION ))
             || (m_cell->revision == cell.revision && 
                 (cell.flag == DELETE_VERSION 
                  || cell.flag == DELETE_FRACTION_VERSION)
                 )) {
        m_prev->set_next(m_next);
        if(m_next)
          m_next->set_prev(m_prev);
        Ptr current = m_next;
        free();
        return current;
      }
    }
    
    return cond == Condition::GT ? nullptr : m_next;
  }

  void remove(){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_next) {
      m_prev->set_next(m_next);
      m_next->set_prev(m_prev);
    }
    free();
  }

  Ptr next() { 
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_next;
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cell->to_string();
  }
  
  private:

  void free() {
    if(m_cell) {
      m_cell->free();
      delete m_cell;
      m_cell = nullptr;
      m_next = nullptr;
      m_prev = nullptr;
    }
  }

  std::mutex        m_mutex;
  Cell*  m_cell;
  Ptr               m_next;
  Ptr               m_prev;
};


class Chain {
  public:
  typedef std::shared_ptr<Chain>  Ptr;


  Chain(uint32_t max_revs=1) 
            : m_base(CellChain::make(Cell(), nullptr, nullptr)),
              m_max_revs(max_revs) { 
  }

  virtual ~Chain() {
    //std::cout << " ~Chain " << (size_t)this << " 1\n";
    free();
    //std::cout << " ~Chain " << (size_t)this << " 1\n";
  }

  void add(const Cell& cell){
    CellChain::Ptr current = m_base->next();
    if(!current) {
      m_base->set_next(CellChain::make(cell, current, m_base));
      return;
    }

    uint32_t revs = 0;
    while((current = current->add(cell, &revs)) && revs < m_max_revs);

    if(cell.removal()){
      current = m_base->next();
      while(current = current->remove(cell));
    }
  }

  const std::string to_string() {
    std::string s("Chain([");

    int64_t count = 0;
    for(CellChain::Ptr current=m_base; current=current->next(); count++){
      s.append(current->to_string());
      s.append(",");
    }

    s.append("] count=");
    s.append(std::to_string(count));
    
    s.append(")");
    return s;
  }

  const size_t count() {
    int64_t count = 0;
    for(CellChain::Ptr current=m_base; current=current->next(); count++);
    return count;
  }

  void free() {
    CellChain::Ptr next = m_base;
    CellChain::Ptr current;
    if(!(next = next->next()))
      return;
      
    for(;;){
      current = next->next();
      //std::cout << " remove, delete " << (size_t)next << " 1\n";
      next->remove();
      delete next;
      //std::cout << " remove, delete " << (size_t)next << " 2\n";
      if(!current)
        break;
      next = current;
    }
    m_base->set_next(nullptr);
  }

  private:
  
  CellChain::Ptr  m_base;
  uint32_t        m_max_revs;
};


}}}
#endif