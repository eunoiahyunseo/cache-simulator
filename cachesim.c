/**
 * @file cachesim.c
 * @author hyunseo (heart20021010@gmail.com)
 * @brief cache simulator for computer assignment 2022
 * @version 0.1
 * @date 2022-11-18
 * @copyright writer: hyunseo lee
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#define INITIAL_BUFFER_SIZE 512
#define ONE_WORD_SIZE 4
#define ONE_BYTE_SIZE 4

typedef int one_word;

// parameter info
int cache_size;
int block_size;
int associative_size;
FILE *tracefile_fp;

int set_num, word_num;

int extern_lru_value;

/**
 * @struct cache_line_ -> cache_line
 * @brief {
 *  for cache line element
 *  1. valid -> check not a trash value
 *  2. dirty -> R/W check ( for write back )
 *  3. tag
 *  4. data
 *  5. lru_val -> lru_val is high is evict
 *  6. addr -> for evict data ( LRU )
 * }
 */
typedef struct cache_line_
{
    int valid;
    int dirty;
    int tag;
    // data have to dynamic -> change with block size
    // block size 8B -> 2word -> sizeof(int) = 4byte = a word
    one_word *data;
    int lru_val;
    int addr;
} cache_line;

// cache_line* [] -> that's a cache
// !CACHE
cache_line *cache;

/**
 * @struct main_memory_line_ -> main_memory
 * @brief {
 *  for main memory initializer
 *  we can't init 4G MM -> so save the data which have value
 *  and other address data value is assume with 0 value
 *  1. addr
 *  2. data
 *  3. check_sum -> for save to MM ( 1 means valid value in MM )
 * }
 */
typedef struct main_memory_line_
{
    int addr;
    int check_sum;
    one_word *data;
} main_memory_line;

// main_memory_line* [] -> that's a main memory;
// !MAIN MEMORY
main_memory_line *main_memory;

void write_cache(int addr, one_word data);
void read_cache(int addr);

// LRU linked-list
// typedef struct lru_struct_
// {
//     int lru_data;
//     struct lru_struct_ *prev;
//     struct lru_struct_ *next;
// } lru_struct, *lru_struct_ptr;

// lru_struct **associative_head_lru;
// lru_struct **associative_tail_lru;

// void init_lru(int set_num);
// void update_lru(int cache_index, int data);
// void add_lru(int cache_index, int data);

void save_MM(cache_line *cache_line_ptr);
void read_MM(int addr, cache_line *cache_line_ptr);

void writeProcess(int addr, int data, cache_line *cache_line_ptr);
void readProcess(int addr, cache_line *cache_line_ptr);

int main(int ac, char *av[])
{
    int parameter_return;
    // parameter parsing
    /**
     * @brief
     * -1 means getopt() parse all options
     */
    while ((parameter_return = getopt(ac, av, "s:b:a:f:")) != -1)
    {
        switch (parameter_return)
        {
        case 's':
            // optarg include '=' so we have to add 1
            cache_size = atoi((char *)++optarg);
            break;
        case 'b':
            block_size = atoi((char *)++optarg);
            break;
        case 'a':
            associative_size = atoi((char *)++optarg);
            break;
        case 'f':
            tracefile_fp = fopen(++optarg, "r+");
            break;
        case '?':
            printf("error occured in input parameter");
            break;
        }
    }

    // start init cache and MM(Main Memory)
    int i;

    set_num = cache_size / (block_size * associative_size);
    word_num = (block_size / ONE_BYTE_SIZE);
    extern_lru_value = 0;

    set_num = cache_size / (block_size * associative_size);
    // initialize with 0 -> we have to use calloc
    cache = (cache_line *)calloc(set_num * associative_size, sizeof(cache_line));
    for (i = 0; i < set_num * associative_size; i++)
        cache[i].data = (one_word *)calloc(block_size / ONE_WORD_SIZE, sizeof(one_word));

    // initialize size 512 with main_memory_line struct
    main_memory = (main_memory_line *)calloc(INITIAL_BUFFER_SIZE, sizeof(main_memory_line));
    for (i = 0; i < INITIAL_BUFFER_SIZE; i++)
    {
        main_memory[i].data = (one_word *)calloc(block_size / ONE_WORD_SIZE, sizeof(one_word));
        // for memory is valid
        main_memory[i].check_sum = 0;
    }

    // put set_num
    // init_lru(set_num);

    // for W/R trace file -> read sample.trc file
    /**
     * @brief read tracefile accurate format
     * 00010008 W 33
     * 0001000C W 4
     * 00010000 R
     * -> 8(hex)   R|W   W?data
     */

    // addr is 32bit -> 4byte -> int is 4byte
    int addr, temp_write_data;
    char memory_access_type;
    while (fscanf(tracefile_fp, "%8x %c", &addr, &memory_access_type) != EOF)
    {
        // if memory access type is Write

        // plus lru value for time goes on
        extern_lru_value++;
        if (memory_access_type == 'W')
        {
            fscanf(tracefile_fp, "%d", &temp_write_data);
            printf("%08X %c %d \n", addr, memory_access_type, temp_write_data);

            // we have to write data to cache
            write_cache(addr, temp_write_data);
        }
        else if (memory_access_type == 'R')
        {
            printf("%08X %c \n", addr, memory_access_type);
            read_cache(addr);
        }
        else
            exit(1);
    }
    return 0;
}

// void init_lru(int set_num)
// {
//     int cache_index;
//     lru_struct_ptr temp;
//     associative_head_lru = (lru_struct_ptr *)calloc(set_num, sizeof(lru_struct_ptr));
//     associative_tail_lru = (lru_struct_ptr *)calloc(set_num, sizeof(lru_struct_ptr));
//     for (cache_index = 0; cache_index < set_num; cache_index++)
//     {
//         associative_head_lru[cache_index] = (lru_struct_ptr)calloc(1, sizeof(lru_struct));
//         associative_tail_lru[cache_index] = (lru_struct_ptr)calloc(1, sizeof(lru_struct));

//         associative_head_lru[cache_index]->lru_data = 0;
//         associative_tail_lru[cache_index]->lru_data = 0;

//         associative_head_lru[cache_index]->next = associative_tail_lru[i];
//         associative_tail_lru[cache_index]->prev = associative_head_lru[i];
//     }
// }

// void update_lru(int cache_index, int data)
// {
//     // find data
// }

// void add_lru(int cache_index, int data)
// {
//     lru_struct_ptr temp;
//     temp = (lru_struct_ptr)calloc(1, sizeof(lru_struct));

//     temp->lru_data = data;
//     temp->next = associative_head_lru[cache_index]->next;
//     associative_head_lru[cache_index]->next->prev = temp;

//     temp->prev = associative_head_lru[cache_index];
//     associative_head_lru[cache_index]->next = temp;
// }

void write_cache(int addr, one_word data)
{
    int index, associative_offset, entry_set_offset, temp_write_data;
    cache_line *cache_line_ptr;

    index = (addr / block_size) % set_num;
    entry_set_offset = associative_size;
    temp_write_data = data;

    for (associative_offset = 0; associative_offset < associative_size; associative_offset++)
    {
        cache_line_ptr = cache + (index * associative_size + associative_offset);

        // CASE1: hit
        if (cache_line_ptr->valid == 1 && cache_line_ptr->tag == ((addr / block_size) / set_num))
        {
            // hit -> (update or write) data to cache
            printf("[%08X] hit \n", addr);
            // update lru
            // update_lru(index, cache_line_ptr->data[((addr / ONE_BYTE_SIZE) % word_num)], temp_write_data);

            cache_line_ptr->dirty = 1;
            cache_line_ptr->data[((addr / ONE_BYTE_SIZE) % word_num)] = temp_write_data;
            cache_line_ptr->addr = addr;
            cache_line_ptr->lru_val = extern_lru_value;
            return;

            // valid bit is 0 so we have to write data to cache
            // first find cache entry set to add data
        }
        else if (cache_line_ptr->valid == 0 && entry_set_offset > associative_offset)
        {
            // while associative_offset is in loop while 0 ~ associative_size
            // find available entry set -> this is typeof find algorithm
            /**
             * @brief
             * if entry_set_off find some entry -> then it could not be updated
             * ! if we break this when update entry_set_offset -> there is some exception when finding hit data
             */
            entry_set_offset = associative_offset;
        }
    }

    // CASE2: miss
    printf("[%08X] miss \n", addr);
    // CASE2-1: miss and cache entry is full
    // -> we can't find entry to put temp_write_data
    if (entry_set_offset == associative_size)
    {
        int max_sizeof_int = INT_MAX;
        // we have to go loop and search the oldest entry(cache->lru_val is smallest) in the target cache index
        for (associative_offset = 0; associative_offset < associative_size; associative_offset++)
        {
            cache_line_ptr = cache + (index * associative_size + associative_offset);
            if (cache_line_ptr->lru_val < max_sizeof_int)
            {
                max_sizeof_int = cache_line_ptr->lru_val;
                entry_set_offset = associative_offset;
            }
        }
        cache_line_ptr = cache + (index * associative_size + entry_set_offset);

        // and if dirty bit is 1 -> save to the main memory
        // dirty bit is 0 -> continue
        if (cache_line_ptr->dirty == 1)
        {
            // evict
            // !we cannot do that because we have to know address of evict entry
            // save_MM(addr, cache_line_ptr);
            save_MM(cache_line_ptr);
        }

        writeProcess(addr, temp_write_data, cache_line_ptr);
    }
    else
    {
        cache_line_ptr = cache + (index * associative_size + entry_set_offset);
        writeProcess(addr, temp_write_data, cache_line_ptr);
    }
}

void read_cache(int addr)
{
    int index, associative_offset, entry_set_offset;
    cache_line *cache_line_ptr;

    index = (addr / block_size) % set_num;

    for (associative_offset = 0; associative_offset < associative_size; associative_offset++)
    {
        cache_line_ptr = cache + (index * associative_size + associative_offset);

        // CASE 1: hit
        if (cache_line_ptr->valid == 1 && cache_line_ptr->tag == ((addr / block_size) / set_num))
        {
            // hit -> update the lru value ( LRU evict policy )
            cache_line_ptr->lru_val = extern_lru_value;
            return;
        } // CASE 2: miss
        else if (cache_line_ptr->valid == 0 && entry_set_offset > associative_offset)
        {
            entry_set_offset = associative_offset;
        }
    }

    // CASE 2: miss
    // same with write cache -> evict
    if (entry_set_offset == associative_size)
    {
        int max_sizeof_int = INT_MAX;
        // we have to go loop and search the oldest entry(cache->lru_val is smallest) in the target cache index
        for (associative_offset = 0; associative_offset < associative_size; associative_offset++)
        {
            cache_line_ptr = cache + (index * associative_size + associative_offset);
            if (cache_line_ptr->lru_val < max_sizeof_int)
            {
                max_sizeof_int = cache_line_ptr->lru_val;
                entry_set_offset = associative_offset;
            }
        }
        cache_line_ptr = cache + (index * associative_size + entry_set_offset);

        // and if dirty bit is 1 -> save to the main memory
        // dirty bit is 0 -> continue
        if (cache_line_ptr->dirty == 1)
        {
            // evict
            // !we cannot do that because we have to know address of evict entry
            // save_MM(addr, cache_line_ptr);
            save_MM(cache_line_ptr);
        }

        readProcess(addr, cache_line_ptr);
    }
    else
    {
        // we have entry
        cache_line_ptr = cache + (index * associative_size + entry_set_offset);
        // if read -> dirty bit is 0
        // because data in cache isn't changed
        readProcess(addr, cache_line_ptr);
    }
}

void save_MM(cache_line *cache_line_ptr)
{
    // we have to know address
    int memory_line, evict_addr;
    evict_addr = (cache_line_ptr->addr / block_size);

    for (memory_line = 0; memory_line < INITIAL_BUFFER_SIZE; memory_line++)
    {
        // CASE1: if checksum is ok -> only copy data to mainmemory[memory_line]
        if (main_memory[memory_line].check_sum == 1 && main_memory[memory_line].addr == evict_addr)
        {
            memcpy(main_memory[memory_line].data, cache_line_ptr->data, sizeof(one_word) * word_num);
            return;
            // CASE2: if checksum is not ok -> set check_sum to 1 and put some data
        }
        else if (main_memory[memory_line].check_sum == 1 && main_memory[memory_line].addr != evict_addr)
            continue;
        else if (main_memory[memory_line].check_sum == 0)
        {
            main_memory[memory_line].check_sum = 1;
            main_memory[memory_line].addr = evict_addr;
            memcpy(main_memory[memory_line].data, cache_line_ptr->data, sizeof(one_word) * word_num);
            return;
        }
    }
}

void read_MM(int addr, cache_line *cache_line_ptr)
{
    int memory_line, target_addr;
    target_addr = addr / block_size;

    for (memory_line = 0; memory_line < INITIAL_BUFFER_SIZE; memory_line++)
    {
        if (main_memory[memory_line].check_sum == 1 && main_memory[memory_line].addr == target_addr)
        {
            // cache_line_ptr->data is int* type
            memcpy(cache_line_ptr->data, main_memory[memory_line].data, sizeof(one_word) * word_num);
            return;
        }
    }
}

void writeProcess(int addr, int data, cache_line *cache_line_ptr)
{
    if (block_size == ONE_WORD_SIZE)
    {
        // finally write data to cache
        cache_line_ptr->dirty = 1;
        cache_line_ptr->valid = 1;
        cache_line_ptr->tag = ((addr / block_size) / set_num);
        (cache_line_ptr->data)[((addr / ONE_BYTE_SIZE) % word_num)] = data;
        cache_line_ptr->addr = addr;
        cache_line_ptr->lru_val = extern_lru_value;
    }
    else
        read_MM(addr, cache_line_ptr);
}

void readProcess(int addr, cache_line *cache_line_ptr)
{
    cache_line_ptr->dirty = 0;
    cache_line_ptr->valid = 1;
    cache_line_ptr->addr = (addr / block_size);
    cache_line_ptr->tag = ((addr / block_size) / set_num);
    cache_line_ptr->lru_val = extern_lru_value;
    read_MM(addr, cache_line_ptr);
}