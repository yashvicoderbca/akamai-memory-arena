#Akamai - inspired low-latency memory arena optimizer
a high-performance, deterministic customer memory allocation implemented in c++17**, designed to minimize kernel-overhead and eliminate memory fragmentation issues common in multi-threaded edge systems.
---
## dynamic logic and core purpose
standard operating system heap allocation(`new`/`malloc`) introduces non-deterministic latencies caused by context switches,page fault,and complex system calls. engine acts as a **pool allocator (arena)**. it pre-allocates a contiguous block of bytes up-front during bootstrap. when the application requests memory, this custom engine intercepts the request and slies segments internally in **0(1) time complexity**, enshuring deterministic throughput ideal for real-time application like content delivery networks (CDNs) and high-frequency trading platforms
------------
## core Architecture and Implemented subsystems
the systems architecture seamlessly bridges physicial allocation with low-overhead logical tracking through these fully build subsystems:
 ## Physicial allocation subsystems:** leverages a base 'char*' buffer to prevent dynamic pointer multiplication andpreverses raw byte offset layouts in a single continuous block.
 ## logical allocation mapping:** uses dynamic metadata tracking descriptors via `struct block`continious allocation capacities, availability states(`is_free`) and physicial memory address mappings 
 ## smart coalescing engine (adjacency merging):** automically checks and merges both forwardward and backward adjacent free memory blocks upon deallocation (`free`) to eliminate fragmentation in real-time
 ## fragmentation analysis subsystem:** calculates real-time metrics including scattered vacant segments, continuous allocation windows, and cumulative residual capacity.
 ## high-precision performance becnchmarking:** A HARDWARE clock telemetry engine utilizing `std::chrono` to stress-test allocator strategies with latency tracked precisely in nanoseconds (`ns`).
-------
## implemented allocation policies
the manager supports runtime policy switching between two industry-standard strategies:
* **FIRST-FIT:** Scans the memory map register and returns the very first vacant block that satisfies the request size. optimized for ultra-low allocation latency.
* **BEST-FIT:** Iterates through the entire tracking table to allocate the chunk providing the absolute tighest residual window, drastically reducing internal memory leftovers.
  -----
  ## Codebase structure and Components
  the complete logic is encapsulated cleanly inside `main.cpp` using the following production-grade interfaces:
  *`CustomMemoryManager(size_t size)`- bootstrapes the continuous arena block
  *` void* alloc(size_t size)`-routes allocation via the currently active strategy token
  *` void* free(void* ptr)`- releases memory segments and traggers the structure merging pipeline.
  *` void* dump_arena()`- visualizes the current state layout of the memory map register.
  *` void* analyze_fragmentation()`-outputs structural health data continuous windows.
  *` void* run_performance_benchmark()`- automatically testes workload under different policies and computes average `ns/op`.
  ----
  ## Prerequisites and Compilation
  ### Requirements:
  * GCC/G++ compiler with full c++17 support.
  ### Run Instructions:
  execute the following commands in your terminal/console window:
  ```bash
  g++ -std=c++17 main.cpp -o memory_arena
  ./memory_arena
  ## expected production telemetry log
  when executed the system outputs deterministic runtime tracking sequences, including the bechmatking telemetry data:
  ```text
  [SUCCESS][INIT]: manager arena of 1024 bytes initiated
  -> operational policy: defaulting to first-fit stategy.
  ---testing part 2 allocation ---
  [FIRST-FIT ALLOC]: 100 bytes mapped at adress:0x.....
  [FIRST-FIT ALLOC]: 200 bytes mapped at adress:0x.....
  ---testing part 3 deallocation and merging ----
  [FREED]: block of size 100 bytes released at address:0x...
  [FREED]: block of size 200 bytes released at address:0x...
  -->[MERGE]: coalesced with the adjacent next block.
  [METRICS][FRAGMENTATION REPORT]:
  -> Total segment counts: 3 blocks metadata mapped.
  --> scattered vacant segments: 1 discreate free blocks.
  -> arena is prefectly contiguous. zero fragmentation overhead detected.
  ======================================================================
  [STARTING PART 7: STRATEGY PERFORMANCE BENCHMARK REPORT]
  ====================================================================
  [STRATEGY CHANGED]: Allocation policy successfully switched to: FIRST-FIT
  [TELEMETRY RESULTS- FIRST-FIT]:
  -> total workload latency : 4500 ns
  ->allocation operational delay :450 ns/op
  ->allocation success rate :10/10
  [STRATEGY CHANGED]: allocate policy successfully switched to: BEST-FIT]:
  [TELEMETRY RESULTS- BEST-FIT]:
  -> total workload latency : 5200 ns
  ->allocation operational delay :520 ns/op
  ->allocation success rate :10/10
  ============================================================
  [BENCHMARK COMPLETE: RESTORED ORGINAL OPERATIONAL STATES]
  
  
  
 
