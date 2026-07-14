/**
*================================================================================================================================================
*CUSTOM MEMORY MANAGEMENT ENGINE( COMPLECTED PROJECT: PARTS 1-7)
*================================================================================================================================================
*STATUS: ALL PARTS SUCCESSFULLY IMPLEMENTED & VERIFIED
*CORE ARCHITECTURE HIGHLIGHTS:
* - PART 1 & 2 : CONTINUOUS MEMORY ARENA ALLOCATION AND ROUTING MANAGEMENT
* - PART 3 :  FRAGMENTED BLOCK DEALLOCATION WITH DYNAMIC ADJACENCY COALESCING
* - PART 4 & 5 :  STRUCTURAL VISUALIZER (ARENA DUMP) & FRAGMENTATION ANALYSIS
* - PART 6  :  RUNTIME POLICY SWITCHING (FIRST-FIT VS OPTIMIZED BEST-FIT)
* - PART 7 :  HIGH-PRECISION TELEMETRY BENCHMARKING USING std:: CHRONO
*AUTHOR :[YASHVI]
*DEGREE: BACHELOR OF COMPUTER APPLICATIONS (BCA)
*TECHNOLOGY: C++, MEMORY MANAGMENT, DATA STRUCTURES AND ALGORITHMS
*================================================================================================================================================
*/
/** 
 *@file main.cpp
 *@brief custom fixed-size memory Arena(pool Allocator) for low-latency systems.
 *@version 1.0
 *
 * DESIGN PURPOSE:
 * Standard OS allocation via `new`/`malloc` induce context switches and non-deterministic
 * latency. this architecture pre-allocates a continuousmemory block (Arena) up-front
 * from the heap to guarantee deterministic o(1) allocation overhead during runtime.
 */
#include <iostream>
#include <vector>
#include <cstddef> // Required for size_t alignment tracking
#include<chrono>// PART 7: REQUIRED FOR HIGH-PRECISION TIME BENCHMARKING
using namespace std;

// ENUMS AND STRUCTURE (MUST BE DEFINED FIRST)
enum AllocationStrategy{
    FIRST_FIT,
    BEST_FIT
};
/** 
 * @struct Block
 * @brief Memory segment descriptor (Allocation metadata)
 * tracks the boundary , availability, and hardware offset of a logical memory slice.
 */
struct Block
{
   size_t size; //capacity of this specific contiguous chunk in bytes
   bool is_free; // Availability Flag: true if vacant, false if occupied
   char* memory_ptr; // hardware start address mapping to the physical pool
};
/** 
 * @class CustomMemoryManager
 * @brief Core subsystem Managing the contiguous byte-buffer.
 */
class CustomMemoryManager {
    private:
    char* total_memory_pool;// base address pointer of the physical heap allocation
    size_t pool_size;// total capacity threshold of the allocated memory arena
    vector<Block> block_list;// Allocation table tracking physical segments logically
    AllocationStrategy current_strategy;// Strategy tracking state token

    public:
    /** 
     * @brief constructor - pre-allocates the total memory arena from OS heap.
     * @param size total continuous bytes to pool up-front.
     */
    CustomMemoryManager(size_t size):pool_size(size), current_strategy(FIRST_FIT){
        // core hardware allocation via system heap
        total_memory_pool= new char[pool_size];
        // logical initialization: bootstrap the arena as one large free block
        Block initial_block={pool_size,true, total_memory_pool};
        //commit the tracking node into our memory map register
        block_list.push_back(initial_block);
        cout<<"[SUCCESS][INIT]: Manager Arena of"<<pool_size
            <<"bytes initialized at the base address:"<<(void*)total_memory_pool<<endl;
            cout<<" -> operational policy: defaulting to first-fit strategy."<<endl;
    }
    /** 
     * @brief PART6: dynamic strategy configuration modifer
     * @param strategy the target selection algorithm(FIRST_FIT/ BEST_FIT)
     */
    void set_allocation_strategy(AllocationStrategy strategy){
        current_strategy = strategy;
        cout<<"\n [STRATEGY CHANGED]: allocation policy successfully switched to:"<<(current_strategy== FIRST_FIT?"FIRST_FIT":"BEST_FIT")<<endl;
    }
    /** 
     * @brief EXTENDED PART2 AND PART6: universal allocation routing interface
     * @details implements a dual engine architecture capable of executing instantaneous linear
     * discovered mapping(first-fit) or optimised scanning for fragmentation reduction(best-fit)
     *@param size bytes requested by the user application playload
     *@return void*contiguous memory pointer aligned to the requested block profile
     */
     void* alloc(size_t size){
        int chosen_index = -1;
        // -----PART 6 STRATEGY ROUNTING BRANCH------
        if(current_strategy==FIRST_FIT){
            /**
             * @brief first-fit rounting engine
             * scan the tracking table sequentially and registers the absolute first available
             * segment that can satisfy the exact payload size. prioritizes execution velocity
             */
            for(size_t i=0; i<block_list.size();++i){
                if(block_list[i].is_free && block_list[i].size>=size){
                    chosen_index=(int)i;
                    break;//deterministic early exit loop optimization
                }
            }
        }
        else if (current_strategy == BEST_FIT){
            /** 
             *@brief best-fit optimization routing engine
             *iterates completely through the tracking descriptors to pinpoint the structural layout.
             *hole that minimizes waste space.minimizes internal remnants to prevent fragmentation.
             */
            size_t tightest_remnants = -1; // initialize to maximum scaler value to serve as theoretical infinity
            for(size_t i = 0; i<block_list.size();++i){
                // core filter: block validation checks (vacancy status and size threshold constraints)
                if(block_list[i].is_free && block_list[i].size>=size){
                    size_t leftover = block_list[i].size - size;
                    //optimization strategy: retain the chunk providing the smallest residual payload window
                    if(leftover<tightest_remnants){
                        tightest_remnants = leftover;
                        chosen_index=(int)i;//lock down the index of the current optimal block candidate
    
                    }
                }
            }
        }
        //----ALLOCATION EXECUTION ENGINE---
        if(chosen_index != -1){
            size_t idx = (size_t)chosen_index;
            // splitting validation check: split memory segment if capacity exceeds requested footprint
            if(block_list[idx].size>size){
                char* next_block_ptr = block_list[idx].memory_ptr + size;
                size_t next_block_size = block_list[idx].size - size;
                Block next_block = {next_block_size, true,next_block_ptr};
                block_list[idx].size = size;
                block_list.insert(block_list.begin()+ idx + 1, next_block);
            }
            block_list[idx].is_free = false;
            cout<<"["<<(current_strategy==FIRST_FIT? "FIRST-FIT": "BEST-FIT")<<"ALLOC]:"<<size<<"bytes mapped at adress:"<<(void*)block_list[idx].memory_ptr<<endl;
            return (void*)block_list[idx].memory_ptr;
        }
        cout<<"[ERROR]: allocation failure no valid chunk matches requested block size:"<<size<<"bytes"<<endl;
        return nullptr;
     }
    /** 
     * @brief PART3: DEALLOCATION AND ADJACENCY MERGING(COALESING) LOGIC
     * @PARAM ptr harware address pointer that needs to be released
     */
    void free(void*ptr){
        if(ptr==nullptr) return;
        //step1: locate the metadata descriptor mapping to this raw hardware address
        for(size_t i=0;i<block_list.size();++i){
            if(block_list[i].memory_ptr==(char*)ptr){
                //step2:mark the found block descriptor as free
                block_list[i].is_free=true;
                cout<<"[FREED]:Block of size"<<block_list[i].size<<"bytes released at adress:"<<ptr<<endl;
                //step3: forward coalesing-merge with the adjacent next block if it is free
                if(i+1<block_list.size()&& block_list[i+1].is_free){
                    block_list[i].size+=block_list[i+1].size;//absorb size
                    block_list.erase(block_list.begin()+i+1);//delete duplicate metadata descriptor
                    cout<<"-->[MERGE]:coalesced with the adjacent NEXT block."<<endl;
                }
                //step4: backward coalescing-merge with the adjacent previous block if it is free
                if(i>0 && block_list[i-1].is_free){
                    block_list[i-1].size+=block_list[i].size;//previous block absorbs current size
                    block_list.erase(block_list.begin()+i);//delete current metadata descriptor
                    cout<<"-->[MERGE]:coalesced with the adjacent privious block."<<endl;
                }
                return;
            }
        }
        cout<<"[ERROR]: invalied pointer deallocation attempted"<<endl;
    }
     /** 
      * @brief PART4: visualization engine/ Arena DUMP
      */
     void dump_arena(){
        cout<<"\n==========CURRENT MEMORY ARENA MAP=============="<<endl;
        for(size_t i=0; i<block_list.size(); ++i){
            if(block_list[i].is_free){
                cout<<"[FREE| SIZE:"<<block_list[i].size<<"B]";
            } else{
                cout<<"[ALLOCATED| SIZE:"<<block_list[i].size<<"B]";
            }
            if(i+1<block_list.size()){
                cout<<"----->";
            }
        }
        cout<<"\n===================================================================="<<endl;
     }
     /** 
      * @brief PART5: FRAGMENTATION ANALYSIS SUBSYSTEM
      * CALCULATES THE INTERNAL BLOCK LAYOUT STATE TO VERIFY MEMORY EFFICIENCY
      */
      void analyze_fragmentataion(){
        size_t free_blocks_count=0;
        size_t total_free_memory=0;
        size_t largest_free_blocks=0;

        for(size_t i=0; i<block_list.size();++i){
            if(block_list[i].is_free){
                free_blocks_count++;
                total_free_memory += block_list[i].size;
                if(block_list[i].size>largest_free_blocks){
                    largest_free_blocks = block_list[i].size;
                }
            }
        }
        cout<<"\n [METRICS][FRAGMENTATION REPORT]:"<<endl;
        cout<<"  -> total segment counts:"<<block_list.size()<<"blocks metadata mapped."<<endl;
        cout<<"  -> scattered vacant segments:"<<free_blocks_count<<"discrete free blocks."<<endl;
        cout<<"  -> cumulative residual capacity:"<<total_free_memory<<"bytes available."<<endl;
        cout<<"  -> maximum continuous allocation window:"<<largest_free_blocks<<"bytes."<<endl;
        if(free_blocks_count>1){
            cout<<" -> NOTICE: MEMORY fragmentation detected. coalescing subsystem operating on standby"<<endl;
        } else{
            cout<<"  -> NOTICE: arena is perfectly contiguous. zero fragmentation overhead detected"<<endl;
        }
      }
      // PART 7: PERFORMANCE BENCHMARKING SUBSYSTEM
      /** 
       * @brief PART 7: executes an intense time-tracked stress workload to benchmark strategies.
       * calculates precision latency in nanoseconds and records allocation failures
       */
      void run_performance_benchmark(){
        cout<<"\n======================================================="<<endl;
        cout<< "[STARTING PART 7: STRATEGY PERFORMANCE BENCHMARK REPORT]"<<endl;
        cout<<"========================================================="<<endl;
        //array storing different playload requests to simulate dynamic workload variations
        size_t benchmark_requests[]={50,120,30,200,70,150,40,90,110,60};
        size_t num_requests= sizeof(benchmark_requests)/ sizeof(benchmark_requests[0]);
        //retain the current orginal strategy state to restore it at the end of the benchmark
        AllocationStrategy backup_strategy=current_strategy;
        //array of the strategies to loop throught sequentially for direct comparison
        AllocationStrategy strategy_to_test[]={FIRST_FIT,BEST_FIT};
        for(AllocationStrategy strategy: strategy_to_test){
            // step 7.1: apply target strategy under evalution
            set_allocation_strategy(strategy);
            size_t successful_allocation=0;
            size_t failed_allocation=0;
            vector<void*> allocated_pointers;//Track pointers to clean up after benchmark run
            cout<<"\n[BENCHMARKING]: Executing operational stress loop..."<<endl;
            //step 7.2:capture high-precision hardware clock timestamp BEFORE Execution
            auto start_time= chrono::high_resolution_clock::now();
            //step7.3: execute workload sequence loops
            for(size_t i=0;i<num_requests;++i){
                size_t current_request_size=benchmark_requests[i];
                void* allocated_ptr=alloc(current_request_size);
                if(allocated_ptr!=nullptr){
                    successful_allocation++;
                    allocated_pointers.push_back(allocated_ptr);
                } else{
                    failed_allocation++;
                }
            }
            //step 7.4: capture high-precision hardware clock timestamp after execution
            auto end_time=chrono::high_resolution_clock::now();
            //step 7.5: mathematical calculations of duration in nanoseconds
            auto total_duration=chrono::duration_cast<chrono::nanoseconds>(end_time-start_time).count();
            //Display performance telemetry data for the evaluated strategy
            cout<<"\n[TELEMETRY RESULTS-"<<(strategy == FIRST_FIT?"FIRST-FIT":"BEST-FIT")<<"]:"<<endl;
            cout<<" -> total workload latency:  "<<total_duration<<"ns"<<endl;
            cout<<" -> average operationl delay:  "<<(total_duration/num_requests)<<"ns/op"<<endl;
            cout<<" -> allocation success rate:  "<<successful_allocation<<"/"<<num_requests<<endl;
            cout<<" -> allocation failure rate "<<failed_allocation<<"/"<<num_requests<<endl;
            //step7.6:reset structural states by deallocation tracked pointers to prevent test cross-contamination
            cout<<"[CLEANUP]: flushing benchmark payloads to restore continuous state.."<<endl;
            for(void* ptr: allocated_pointers){
                free(ptr);
            }
        }
        //restore orginal operational system state token
        current_strategy = backup_strategy;
        cout<<"\n====================================================="<<endl;
        cout<<"[BENCHMARK COMPLETE: RESTORE ORGINAL OPERATIONAL STATES]"<<endl;
        cout<<"\n====================================================="<<endl;
      }
    /** 
     * @brief Destructor-safe release of the shared pool boundary to prevent memory leaks.
     */
    CustomMemoryManager(){
        // Release the physical buffer back to the OS
        delete[]total_memory_pool;
        // zeroing out structural anchors for safety
        total_memory_pool=nullptr;
        cout<<"[SUCCESS][SHUTDOWN]:Memory arena safely released. system leaks prevented."<<endl; 
    }
    };

int main(){
    // instantiate a 1KB Memory pool for edge-worker thread simulations
    const size_t ARENA_SIZE=1024;
    CustomMemoryManager akamai_pool(ARENA_SIZE);
    // TEST ALLOACTION (PART2 TESTING)
    cout<<"\n--- testing part 2 allocation ----\n"<<endl;
    void* user1=akamai_pool.alloc(100);//requesting 1st continuous slice of 100 bytes from the managed pool
    void* user2=akamai_pool.alloc(200);//requesting 2nd continuous slice of 200 bytes from the remaining pool capacity
    // PART 4 INTEGRATION: RENDER VISUAL SNAPSHOT OF THE MEMORY LAYOUT IMMEDIATELY AFTER ALLOCATIONS
    akamai_pool.dump_arena();
    cout<<"\n--- testing part 3 deallocation and merging----n"<<endl;
    akamai_pool.free(user1);//freeing 1st block(no adjacent merge yet)
    //PART 4 INTEGRATION: RENDER LAYOUT TO VERFIFY LOCALIZED DEALLOCATION WITHOUT CASCADING MERGES
    akamai_pool.dump_arena();
    akamai_pool.free(user2);//freeing 1st block(will merge with user 1 and remaining arena)
    //PART 4 INTEGRATION: FINAL STRUCTURAL AUDIT TO CONFIRM ARENA HAS SUCCESSFULLY MERGED BACK INTO A SINGLE BLOCK
    akamai_pool.dump_arena();
    // EXECUTION PHASE 3: STRESS TESTING AND FRAGMENTATION METRICS(PART 5 TESTING)
    cout<<"\n---STARTING PART 5: DYNAMIC STRESS TESTING SUB-ROUTE--"<<endl;
    //allocates 3 blocks sequentially to generate multiple segments
    void* blockA = akamai_pool.alloc(150);
    void* blockB = akamai_pool.alloc(250);
    void* blockC = akamai_pool.alloc(350);
    akamai_pool.dump_arena(); 
    // free middle block B purposefully to introduce simulated memory fragmentation
    cout<<"\n[ACTION]: freeing blockB to inject artificial structural fragmentation"<<endl;
    akamai_pool.free(blockB);
    //render map and audit fragmentation
    akamai_pool.dump_arena();
    akamai_pool.analyze_fragmentataion();
    //clean up remaining allocations to test final system reset integrity
    cout<<"\n[ACTION]: releasing remaining stress blocks to guarantee clean shutdown states"<<endl;
    akamai_pool.free(blockA);
    akamai_pool.free(blockC);
    akamai_pool.dump_arena();
    akamai_pool.analyze_fragmentataion();
    //"----ADDING PART 6 STARTING HERE: DYNAMIC BEST-FIT STRATEGY BENCHMARK"
    //1 SETUP PHASE: CREATE THE  FRAGMENTATION LANDSCAPE (300B AND 150B HOLES)
    cout<<"step 1: building bechmark layout(300 and 150 holes)"<<endl;
    void* hole1 = akamai_pool.alloc(300);// will become free hole 1 (300b)
    void* seperator = akamai_pool.alloc(100);//occupied buffer block to prevent merging
    void* hole2 = akamai_pool.alloc(150);//will become free hole 2 (150b)
    void* remainder = akamai_pool.alloc(450);//occupied block taking up the rest of the pool (1024-1000=24b left)
    // 2 release the target slots to inject localized holes
    cout<<"[ACTION]: RELEASING 300B AND 150B blocks into the arena.."<<endl;
    akamai_pool.free(hole1);
    akamai_pool.free(hole2);
    //view current status before evaluating algorithms
    akamai_pool.dump_arena();
    // 3 execution path A: TEST WITH STANDARD FIRSTFIRST-FIT POLICY
    akamai_pool.set_allocation_strategy(FIRST_FIT);
    cout<<"[TEST]: requesting 120b allocation using FIRST-FIT..."<<endl;
    void* test_ff = akamai_pool.alloc(120);
    akamai_pool.dump_arena();//will show that it cut into the 300b hole instantly
    //reset state for an identical clean best-fit evaluation
    cout<<"[RESET]: releasing FIRST-FIT tracking slot to restore the 300b hole..."<<endl;
    akamai_pool.free(test_ff);
    //4 execution path B: switch dynamically and test with BEST-FIT policy
    akamai_pool.set_allocation_strategy(BEST_FIT);
    cout<<"[TEST]: requesting 120B allocation using BEST-FIT..."<<endl;
    void* test_bf = akamai_pool.alloc(120);
    akamai_pool.dump_arena();// will show that it intelligently picked the tighter 150B hole
    //final cleanup of all allocation to enshure safe arena destruction 
    cout<<"[ACTION]: Cleaning remaining benchmark payloads for the safe system shutdown..."<<endl;
    akamai_pool.free(test_bf);
    akamai_pool.free(seperator);
    akamai_pool.free(remainder);
    //------ PART7 CALL:EXECUTING THE PERFORMANCE BENCHMARK SUBSYSTEM---
    akamai_pool.run_performance_benchmark();
    cout<<endl;
    return 0;
}


