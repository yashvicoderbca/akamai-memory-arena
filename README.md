#Akamai-inspired low-latency memory Arena optimizer
A high-performance, deterministic custom memory allocator implemented in c++17. designed to minimize karnel-overhead and eliminate memory fragmentation issues common in multi-threaded edge system.
## dynamic logic and core purpose
Standard operating system heap allocation (`new`/`malloc`) introduce non-deterministic latencies caused by context switches,page faults, and complex system call
this engine acts as a **pool Allocator (Arena)**. It pre-alloactes a contiguous block of bytes up-front during bootstrap . When the application requests memory ,this curstomer engine intercepts the request and slices segments internally in $0(1)$ time complexity, ensuring deterministic throughput ideal for real-time application like content delivery networks (CDNs) and high -frequency trading platforms.
## architecture setup 
The architecture bridges physical allocation with low_overhead logical tracking :
-**physical allocation subsystem:**Leverages a base `char*` buffer to prevent dynamic pointer multiplication and preserve raw byte offset layouts.
-**Logical allocation mapping:** uses inline metadata tracking descriptors('struct Block`) containing allocation states and offset configurations, storing segment limits securely inside private class encapulates.
### prerequisites
-GCC/G++ compiler with c++17 support 
### compilation
```bash
g++ -std=c++17 main.cpp -o memory_arena
./memory_arena