/**
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <mutex>
#include <condition_variable>

#include "swcdb/core/QueueSafe.h"


static const size_t WORK_LOAD = 100000;


namespace SWC {

void write(int nthread, QueueSafe<size_t>* queue) {
  size_t sz;
  bool chk;
  //size_t item;
  for(size_t i=0;i<WORK_LOAD; ++i) {
    queue->push(i);
    sz = queue->size();
    chk = queue->empty();
    //item = queue->front();
    //if(i % 1000 == 0)
    //  std::cout << "thread=" << nthread << " at=" << i << "\n";
  }
}


void run(int threads) {
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> completing = threads;
  QueueSafe<size_t> queue;
  
  for(auto n=0; n<threads; ++n) {
      std::thread t(
        [n, &completing, &cv, q=&queue]() { 
          write(n, q); 
          --completing;
          cv.notify_all();
        });
      t.detach();
  }

  std::unique_lock lock(m);
  cv.wait(lock, [&completing]() { return !completing; });
}

}

int main(int argc, char** argv) {
  uint64_t total_ts = 0;
  uint64_t total = 0;
  for(int chk = 1; chk <= 50; ++chk) {
  for(int t = 1; t <= 8; ++t) {
    auto start = std::chrono::high_resolution_clock::now();
    SWC::run(t);
    auto took = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now() - start).count();
    total_ts += took;
    total += WORK_LOAD*t;
    std::cout << " chk="<< chk 
              << " threads " << t 
              << " took=" <<  took
              << " avg=" <<  took/(WORK_LOAD*t)
              << "\n";  
  }
  }
  
  std::cout << " total took="<< total_ts 
            << " avg=" <<  total_ts/total
            << "\n";
  std::cout << "SWC::Mutex sz=" << sizeof(SWC::Mutex) << "\n";
  std::cout << "SWC::Mutex::scope sz=" << sizeof(SWC::Mutex::scope) << "\n";
}