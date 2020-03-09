/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_ranger_db_CellStoreBlock_h
#define swc_ranger_db_CellStoreBlock_h



namespace SWC { namespace Ranger { namespace CellStore {


namespace Block {

/* file-format: 
      header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
      data:   [cell]
*/

static const uint8_t HEADER_SIZE=17;



class Read final {  
  public:
  typedef Read* Ptr;

  enum State {
    NONE,
    LOADING,
    LOADED
  };

  static const std::string to_string(const State state);
  
  static Ptr make(const size_t offset, 
                  const DB::Cells::Interval& interval, 
                  uint32_t cell_revs);

  const size_t               offset;
  const DB::Cells::Interval  interval;
  const uint32_t             cell_revs;

  explicit Read(const size_t offset, const DB::Cells::Interval& interval, 
                uint32_t cell_revs);
  
  Ptr ptr();

  ~Read();
  
  bool load(const std::function<void()>& cb);

  void load(FS::SmartFd::Ptr smartfd, const std::function<void()>& cb);

  void load_cells(int& err, Ranger::Block::Ptr cells_block);

  void processing_decrement();

  const size_t release();

  const bool processing();

  const int error();

  const bool loaded();
  
  const bool loaded(int& err);

  const size_t size_bytes(bool only_loaded=false);

  const std::string to_string();

  
  private:
  
  void load(int& err, FS::SmartFd::Ptr smartfd);

  void run_queued();

  void _run_queued();

  std::shared_mutex                  m_mutex;
  State                              m_state;
  size_t                             m_processing;
  bool                               m_loaded_header;
  Types::Encoding                    m_encoder;
  size_t                             m_size;
  size_t                             m_sz_enc;
  uint32_t                           m_cells_count;
  StaticBuffer                       m_buffer;
  bool                               m_q_runs = false;
  std::queue<std::function<void()>>  m_queue;
  std::atomic<uint32_t>              m_cells_remain;
  int                                m_err;
};



class Write final {  
  public:
  typedef std::shared_ptr<Write> Ptr;

  Write(const size_t offset, const DB::Cells::Interval& interval, 
        const uint32_t cell_count);

  ~Write();

  void write(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
             DynamicBuffer& output);

  const std::string to_string();

  const size_t              offset;
  const DB::Cells::Interval interval;
  const uint32_t            cell_count;

};



}}}} // namespace SWC::Ranger::CellStore::Block

#endif // swc_ranger_db_CellStoreBlock_h