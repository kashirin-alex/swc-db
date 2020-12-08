/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Compat.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Semaphore.h"
#include <thread>

const size_t num_threads = 5;
SWC::Core::Semaphore sem(num_threads, num_threads);


void run_thread(size_t t) {
  printf(" starting t=%lu \n", t);

  sem.release();
  sem.wait_all();

  size_t total_bytes = 0;
  size_t total_took = 0;
  size_t probes = 1;
  size_t bytes_start = 0;
  size_t bytes_end = 32; //1024 * 1024 * 256;
  size_t checks = 30000;
  size_t total_checks = probes*checks;

  for(size_t byte = bytes_start; byte <= bytes_end; ++byte) {
    for(size_t probe=1; probe <= probes; ++probe) {

      auto ns = SWC::Time::now_ns();
      for(size_t chk=0; chk < checks; ++chk) {

        uint8_t* ptr = new uint8_t[byte];
        //delete [] ptr;
        //printf("ptr = %lu \n", size_t(ptr));
        (void)ptr;

      }
      size_t took = SWC::Time::now_ns() - ns;
      total_took += took;
      //printf("probe=%lu byte=%lu took=%lu avg=%lu \n", 
      //        probe, byte, took, (took ? took/checks : 0));
    }
    total_bytes += total_checks*byte;
  }

  printf("total: bytes=%lu took=%lu byte-avg=%lu avg=%lu \n", 
    total_bytes, total_took, 
    (total_took ? total_took/total_bytes : 0),
    (total_took ? total_took/total_checks : 0)
  );

  printf(" stopping t=%lu \n", t);
}


int main() {
  printf(" START num_threads=%lu\n", num_threads);
  std::cout << "--1--\n";

  std::thread threads[num_threads];
  for(size_t t=0; t < num_threads; ++t)
    threads[t] = std::thread(&run_thread, t+1);
    
  sem.wait_all();
  std::cout << "--2--\n";
  for(auto& t : threads)
    t.join();
  
  std::cout << " OK! num_threads=" << num_threads << " \n";
  return 0;
}
