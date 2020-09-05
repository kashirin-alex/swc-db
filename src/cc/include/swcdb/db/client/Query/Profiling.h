
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_client_Query_Profiling_h
#define swc_db_client_Query_Profiling_h



namespace SWC { namespace client { namespace Query {

struct Profiling {
  const int64_t ts_start = Time::now_ns();
  int64_t       ts_finish = ts_start;

  struct Component {

    std::atomic<uint64_t> count = 0;
    std::atomic<uint64_t> time = 0;
    std::atomic<uint64_t> error = 0;

    struct Start {
      Component&    _m;
      const int64_t ts;

      Start(Component& m)
            : _m(m), ts(Time::now_ns()) {
      }

      void add(bool err) const {
        _m.add(ts, err);
      }
    };

    Start start() {
      return Start(*this);
    }

    void add(uint64_t ts, bool err) {
      ++count;
      time += Time::now_ns() - ts;
      if(err)
        ++error;
    }

    void print(std::ostream& out) const {
      out << "(count=" << count 
          << " time=" << time << "ns errors=" << error << ')';
    }
    
    void display(std::ostream& out) const { 
      out << time << "ns" << "/" << count << "(" << error << ")\n";
    }
  };

  Component _mngr_locate;
  Component _mngr_res;
  Component _rgr_locate_master;
  Component _rgr_locate_meta;
  Component _rgr_data;

  void finished() {
    ts_finish = Time::now_ns();
  }

  Component::Start mngr_locate() {
    return Component::Start(_mngr_locate);
  }

  Component::Start mngr_res() {
    return Component::Start(_mngr_res);
  }

  Component::Start rgr_locate(Types::Range type) {    
    switch(type) {
      case Types::Range::MASTER: 
        return Component::Start(_rgr_locate_master);
      default:
        return Component::Start(_rgr_locate_meta);
    }
  }

  Component::Start rgr_data() {
    return Component::Start(_rgr_data);
  }

  void display(std::ostream& out) const {
    _mngr_locate.display(
      out << " Mngr Locate:            ");
    _mngr_res.display(
      out << " Mngr Resolve:           ");
    _rgr_locate_master.display(
      out << " Rgr Locate Master:      ");
    _rgr_locate_meta.display(
      out << " Rgr Locate Meta:        ");
    _rgr_data.display(
      out << " Rgr Data:               ");
    out  << std::flush;
  }

  void print(std::ostream& out) const {
    out << "Profile(took=" << (ts_finish - ts_start) << "ns";
    _mngr_locate.print(out << " mngr[locate");
    _mngr_res.print(out << " res");
    _rgr_locate_master.print(out << "] rgr[locate-master");
    _rgr_locate_meta.print(out << " locate-meta");
    _rgr_data.print(out << " data");
    out << "])";
  }

  std::string to_string() const {
    std::stringstream ss;
    print(ss);
    return ss.str();
  }

};


}}}

#endif // swc_db_client_Query_Profiling_h