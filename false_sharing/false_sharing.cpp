// This program shows off the sever implications of false sharing in
// C++ using std::atomic and std::thread

#include <benchmark/benchmark.h>
#include <atomic>
#include <thread>
#include <iostream>
#include <unistd.h>
using namespace std;

// Simple function for incrememnting an atomic int
void work(std::atomic<int>& a) {
  for (int i = 0; i < 100000; i++) {
    a++;
  }
}

// Simple single-threaded function that calls work 4 times
void single_thread() {
  std::atomic<int> a;
  a = 0;

  work(a);
  work(a);
  work(a);
  work(a);
}

// A simple benchmark that runs our single-threaded implementation
static void singleThread(benchmark::State& s) {
  while (s.KeepRunning()) {
    single_thread();
  }
}
BENCHMARK(singleThread)->Unit(benchmark::kMillisecond);

// Tries to parallelize the work across multiple threads
// However, each core invalidates the other cores copies on a write
// This is an EXTREME example of poorly thought out code
void same_var() {
  std::atomic<int> a;
  a = 0;

  // Create four threads and use a lambda to launch work
  std::thread t1([&]() { work(a); });
  std::thread t2([&]() { work(a); });
  std::thread t3([&]() { work(a); });
  std::thread t4([&]() { work(a); });

  // Join the threads
  t1.join();
  t2.join();
  t3.join();
  t4.join();
}

// A simple benchmark that runs our single-threaded implementation
static void directSharing(benchmark::State& s) {
  while (s.KeepRunning()) {
    same_var();
  }
}
BENCHMARK(directSharing)->UseRealTime()->Unit(benchmark::kMillisecond);


void diff_var() {

  int num_cores = sysconf(_SC_NPROCESSORS_ONLN) + 1;
  long cache_lsize = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
  unsigned int num_threads = std::thread::hardware_concurrency();
  //std::cout << "Number of Cores: " << num_cores << std::endl;
  //std::cout << "Cache Line Size: " << cache_lsize << std::endl;
  std::cout << "Number of threads supported: " << num_threads << std::endl;

  std::vector<std::atomic<int>> intVec(num_cores);
  std::vector<std::thread> threadVec;
  


  for (int i = 0; i < num_cores; i++) {
      //std::atomic<int> a{0};
      intVec[i] = 0;
  }

  // Create four threads and use lambda to launch work
  for (int i = 0; i < num_cores; i++) {
      //std::cout << "Address of atomic<int> a - " << &intVec[i] << '\n';
      threadVec.emplace_back([&]() { work(intVec[i]); });
  }

  // Join the threads
  for (std::thread& thread : threadVec) {
    thread.join();
  }
}

// A simple benchmark that runs our single-threaded implementation
static void falseSharing(benchmark::State& s) {
  while (s.KeepRunning()) {
    diff_var();
  }
}
BENCHMARK(falseSharing)->UseRealTime()->Unit(benchmark::kMillisecond);

// We can align the struct to 64 bytes
// Now each struct will be on a different cache line
struct alignas(64) AlignedType {
  AlignedType() { val = 0; }
  std::atomic<int> val;
};

// No more invalidations, so our code should be roughly the same as the
void diff_line() {
  AlignedType a{};
  AlignedType b{};
  AlignedType c{};
  AlignedType d{};
/*

  std::cout << "Address of atomic<int> a - " << &a << '\n';
  std::cout << "Address of atomic<int> b - " << &b << '\n';
  std::cout << "Address of atomic<int> c - " << &c << '\n';
  std::cout << "Address of atomic<int> d - " << &d << '\n';
*/
  // Launch the four threads now using our aligned data
  std::thread t1([&]() { work(a.val); });
  std::thread t2([&]() { work(b.val); });
  std::thread t3([&]() { work(c.val); });
  std::thread t4([&]() { work(d.val); });

  // Join the threads
  t1.join();
  t2.join();
  t3.join();
  t4.join();
}

// A simple benchmark that runs our single-threaded implementation
static void noSharing(benchmark::State& s) {
  while (s.KeepRunning()) {
    diff_line();
  }
}
BENCHMARK(noSharing)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();