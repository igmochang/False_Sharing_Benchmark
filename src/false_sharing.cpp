// This program shows off the sever implications of false sharing in
// C++ using std::atomic and std::thread

#include <benchmark/benchmark.h>
#include <atomic>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <perfmon/perf_event.h>

int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
long cache_lsize = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

unsigned int num_threads = std::thread::hardware_concurrency();

const int int_per_line = cache_lsize / sizeof(std::atomic<int>);
const int int_per_thread = int_per_line / num_threads;

// Simple function for incrementing an atomic int
void work(std::atomic<int>& a) {
  for (int i = 0; i < 100000; i++) {
    a++;
  }
}


void single_thread() {
  std::atomic<int> a;
  a = 0;

  for (int i = 0; i < int_per_line; i++){
    work(a);
  }
}

static void singleThread(benchmark::State& s) {
  while (s.KeepRunning()) {
    single_thread();
  }
}
BENCHMARK(singleThread)->Unit(benchmark::kMillisecond);


void shared_var() {
  
  std::vector<std::thread> threadVec;

  std::atomic<int> a;
  a = 0;

  for (int i = 0; i < num_threads; i++) {
    threadVec.emplace_back([&]() { 
        for (int j = 0; j < int_per_thread; j++){ 
          work(a); 
        }
      });
  }  

  // Join the threads
  for (std::thread& thread : threadVec) {
    thread.join();
  }

}

static void directSharing(benchmark::State& s) {
  while (s.KeepRunning()) {
    shared_var();
  }
}
BENCHMARK(directSharing)->UseRealTime()->Unit(benchmark::kMillisecond);


void diff_var() {

  std::vector<std::atomic<int>> intVec(int_per_line);
  std::vector<std::thread> threadVec;
  
  for (int i = 0; i < int_per_line; i++) {
      intVec[i] = 0;
  }

  for (int i = 0; i < num_threads; i++) {
    threadVec.emplace_back([&]() { 
      for(int j = 0; j < int_per_thread; j++){
        work(intVec[i + j]); 
      }
    });
  }  

  // Join the threads
  for (std::thread& thread : threadVec) {
    thread.join();
  }
}

static void falseSharing(benchmark::State& s) {
  while (s.KeepRunning()) {
    diff_var();
  }
}
BENCHMARK(falseSharing)->UseRealTime()->Unit(benchmark::kMillisecond);

struct alignas(64) AlignedType {
  AlignedType() { val = 0; }
  std::atomic<int> val;
};

void diff_line() {
  
  std::vector<AlignedType> intVec(num_cores);
  std::vector<std::thread> threadVec;


  for (int i = 0; i < num_threads; i++) {
    threadVec.emplace_back([&intVec, i]() { 
      for (int j = 0; j < int_per_thread; j++){
        work(intVec[i].val); 
      }
    });
  }

  // Join the threads
  for (std::thread& thread : threadVec) {
    thread.join();
  }
}

static void padding(benchmark::State& s) {
  while (s.KeepRunning()) {
    diff_line();
  }
}
BENCHMARK(padding)->UseRealTime()->Unit(benchmark::kMillisecond);

void diff_var_local() {

  std::vector<std::atomic<int>> intVec(int_per_line);
  std::vector<std::thread> threadVec;
  thread_local std::atomic<int> a;

  for (int i = 0; i < int_per_line; i++) {
    intVec[i] = 0;
  }

  for (int i = 0; i < num_threads; i++) {
    
    threadVec.emplace_back([&]() { 
      for(int j = 0; j < int_per_thread; j++){
        a = intVec[i+j].load();
        work(a); 
      }
    });
  }

  // Join the threads
  for (std::thread& thread : threadVec) {
    thread.join();
  }

}

static void noSharingLocal(benchmark::State& s) {
  while (s.KeepRunning()) {
    diff_var_local();
  }
}
BENCHMARK(noSharingLocal)->UseRealTime()->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();