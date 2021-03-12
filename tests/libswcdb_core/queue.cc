/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Semaphore.h"
#include "swcdb/core/QueueSafe.h"
#include "swcdb/core/QueuePointer.h"
#include <thread>

  const size_t num_threads = 2;
  const size_t WORK_LOAD = 10000000; // X producers
  const size_t TOTAL_WORKLOAD = num_threads * WORK_LOAD;


namespace SWC {


struct A1 {
  static A1* make(size_t n) {
    return new A1(n);
  }
  A1(size_t n) : n(n) { }
  void done() {
    delete this;
  }
  size_t n;
};


struct A2 : Core::QueuePointer<A2*>::Pointer {
  static A2* make(size_t n) {
    return new A2(n);
  }
  A2(size_t n) : n(n) { }
  void done() {
    delete this;
  }
  size_t n;
};


struct A3 : Core::QueuePointer<std::shared_ptr<A3>>::Pointer {
  static std::shared_ptr<A3> make(size_t n) {
    return std::shared_ptr<A3>(new A3(n));
  }
  A3(size_t n) : n(n) { }
  void done() {}
  size_t n;
};


struct A4  {
  static std::shared_ptr<A4> make(size_t n) {
    return std::shared_ptr<A4>(new A4(n));
  }
  A4(size_t n) : n(n) { }
  void done() {}
  size_t n;
};


template<class QueueT, typename ItemT>
struct Test {

  QueueT queue;

  Core::Semaphore sem_producer;
  Core::Semaphore sem_consumer;

  Core::Atomic<size_t> counted;
  Core::Atomic<size_t> time_producer;
  Core::Atomic<size_t> time_consumer;

  Test() : sem_producer(num_threads, num_threads),
           sem_consumer(num_threads, num_threads),
           counted(0), time_producer(0), time_consumer(0) {
  }

  void print() {
    std::cout
      << " workload=" << TOTAL_WORKLOAD << " counted=" << counted << "\n"
      << " time_producer=" << time_producer.load()
        << " avg=" << time_producer/TOTAL_WORKLOAD <<  "\n"
      << " time_consumer=" << time_consumer.load()
        << " avg=" << time_consumer/TOTAL_WORKLOAD <<  "\n";
    SWC_ASSERT(counted == TOTAL_WORKLOAD);
  }

  void run_producer(size_t t) {
    printf(" starting run_producer=%lu \n", t);

    sem_producer.release();
    sem_producer.wait_all();

    size_t start = t * WORK_LOAD;
    size_t end = start + WORK_LOAD;
    for(size_t i=start;i<end; ++i) {
      auto v = ItemT::make(i);
      auto ts = Time::now_ns();
      queue.push(v);
      time_producer.fetch_add(Time::now_ns() - ts);
    }
    printf(" stopping run_producer=%lu \n", t);
  }


  void run_consumer(size_t t) {
    printf(" starting run_consumer=%lu \n", t);

    sem_consumer.release();
    sem_consumer.wait_all();

    typename QueueT::value_type v;
    for(;;) {
      auto ts = Time::now_ns();
      if(!queue.pop(&v))
        break;
      time_consumer.fetch_add(Time::now_ns() - ts);
      counted.fetch_add(1);
      v->done();
    }

    printf(" stopping run_consumer=%lu \n", t);
  }

  void run() {

    std::cout << "--1--\n";
    std::thread threads_producer[num_threads];
    for(size_t t=0; t < num_threads; ++t)
      threads_producer[t] = std::thread([this, t](){ run_producer(t+1);} );
    sem_producer.wait_all();

    std::cout << "--2--\n";
    for(auto& t : threads_producer)
      t.join();

    std::cout << "--3--\n";
    std::thread threads_consumer[num_threads];
    for(size_t t=0; t < num_threads; ++t)
      threads_consumer[t] = std::thread([this, t](){ run_consumer(t+1);} );
    sem_consumer.wait_all();

    std::cout << "--4--\n";
    for(auto& t : threads_consumer)
      t.join();

    print();
  }

};

void run() {

  printf("\n START Test<Core::QueuePointer<std::shared_ptr<A3>>, A3>\n");
  Test<Core::QueuePointer<std::shared_ptr<A3>>, A3> test3;
  test3.run();

  printf("\n START Test<Core::QueuePointer<A2*>, A2>\n");
  Test<Core::QueuePointer<A2*>, A2> test2;
  test2.run();

  printf("\n START Test<Core::QueueSafe<A1*>, A1>\n");
  Test<Core::QueueSafe<A1*>, A1>    test1;
  test1.run();

  printf("\n START Test<Core::QueueSafe<std::shared_ptr<A4>>, A4>\n");
  Test<Core::QueueSafe<std::shared_ptr<A4>>, A4>    test4;
  test4.run();
}

}

int main() {
  printf(" START num_threads=%lu\n", num_threads);

  SWC::run();

  std::cout << " OK! num_threads=" << num_threads << " \n";
  return 0;
}
