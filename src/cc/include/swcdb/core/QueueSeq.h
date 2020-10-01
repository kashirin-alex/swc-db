/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_QueueSeq_h
#define swcdb_core_QueueSeq_h
  
namespace SWC {


template <class ItemT>
class QueueSeq {
  
  struct Seq final {
    Seq(const ItemT& item) 
        : value(std::move(item)), next(nullptr) { 
    }
    ~Seq() { }
    ItemT value;
    Seq*  next;
  };

  size_t  m_size;
  Seq*    m_front;
  Seq*    m_back;

  public:
  
  QueueSeq() : m_size(0), m_front(nullptr), m_back(nullptr) { }

  ~QueueSeq() {
    clear();
  }

  void clear() {
    for(;!empty();pop());
  }

  bool empty() const {
    return !m_size;
  }

  size_t size() const {
    return m_size;
  }

  void push(const ItemT& item) {
    m_back = (m_front ? m_back->next : m_front) = new Seq(item);
    ++m_size;
  }

  ItemT& front() const {
    if(!m_front)
      throw std::runtime_error("Empty QueueSeq");
    return m_front->value; 
  }
  
  void pop() {
    auto seq = m_front->next;
    if(m_front == m_back)
      m_back = seq;
    delete m_front;
    m_front = seq;
    --m_size;
  }

};


}

#endif // swcdb_core_QueueSeq_h
