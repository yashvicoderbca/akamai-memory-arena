// PROJECT STATUS:DAY3(PART 1 + PART 2 + PART 3 COMPLECTED)
//CURRENT STATE: CORE STRUCTURE + INITIALIZATION + SPLITTING + COALESCING FREE LOGIC DONE
//NEXT STEP:PART 4 (VISUALIZATION ENGINE AND DUMP FUNCTION) WILL COME SOON
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
using namespace std;
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

    public:
    /** 
     * @brief constructor - pre-allocates the total memory arena from OS heap.
     * @param size total continuous bytes to pool up-front.
     */
    CustomMemoryManager(size_t size):pool_size(size){
        // core hardware allocation via system heap
        total_memory_pool= new char[pool_size];
        // logical initialization: bootstrap the arena as one large free block
        Block initial_block={pool_size,true, total_memory_pool};
        //commit the tracking node into our memory map register
        block_list.push_back(initial_block);
        cout<<"[SUCCESS][INIT]: Manager Arena of"<<pool_size
            <<"bytes initialized at the base address:"<<(void*)total_memory_pool<<endl;
    }
    /** 
     * @brief PART2: Block splitting and allocation logic
     * @param size bytes requested by the user
     * @return void* pointer to the alloaction memory area
     */
     void* alloc(size_t size){
        //Line 1 and 2 : iterate through the vector tracking register(metadata table)
        for(size_t i=0; i<block_list.size();++i){
            // line3: first-fit strategy-check if the block is free and has sufficient capacity 
            if(block_list[i].is_free && block_list[i].size>=size){
                //line4: splliting decision- trigger split if the block size strictly exceeds the requested size
                if(block_list[i].size>size){
                    // line5:calculate the hardware memory adress and the remaining size for the new residual block
                    //new address= base address of current block + bytes requested by user
                    char* next_block_ptr=block_list[i].memory_ptr+size;
                    size_t next_block_size=block_list[i].size-size;

                    // line6: instantiate metadata descriptior for the new residual free block
                    Block next_block={next_block_size,true,next_block_ptr};
                    //line7: shrink the current block logical size to match exact user demand
                    block_list[i].size=size;
                    //dynamically insert the new free block description immediately after the current block in the vector register
                    block_list.insert(block_list.begin()+i+1,next_block);
                }
                //line8: mark the current block as occupied/allocated
                block_list[i].is_free=false;
                cout<<"[ALLOCATED]:"<<size<<"bytes given at adress:"
                <<(void*)block_list[i].memory_ptr<<endl;
                //line9: return the direct hardware address to the user for safe data writing
                //(no header skip layout adjustement needed since metadata is off-loaded to std::vector)
                return(void*)block_list[i].memory_ptr;
            }
        }
        cout<<"[ERROR]: OUT OF MEMORY OR NOT SUITABLE block FOUND"<<endl;
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

    cout<<"\n--- testing part 3 deallocation and merging----n"<<endl;
    akamai_pool.free(user1);//freeing 1st block(no adjacent merge yet)
    akamai_pool.free(user2);//freeing 1st block(will merge with user 1 and remaining arena)
    cout<<endl;
    return 0;
}



