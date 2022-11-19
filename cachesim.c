/**
 * @file cachesim.c
 * @author hyunseo (heart20021010@gmail.com)
 * @brief cache simulator for computer assignment 2022
 * @version 0.1
 * @date 2022-11-18
 *
 * @copyright writer: hyunseo lee
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#define INITIAL_BUFFER_SIZE 512
#define ONE_WORD_SIZE 4
#define ONE_BYTE_SIZE 4

typedef int one_word;

// parameter info
int cache_size;
int block_size;
int associative_size;
FILE *tracefile_fp;

/**
 * @struct cache_line_ -> cache_line
 * @brief {
 *  for cache line element
 *  1. valid
 *  2. dirty
 *  3. tag
 *  4. data
 * }
 *
 */
typedef struct cache_line_
{
    int valid;
    int dirty;
    int tag;
    // data have to dynamic -> change with block size
    // block size 8B -> 2word -> sizeof(int) = 4byte = a word
    one_word *data;
    int lru_seq;
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
 *  3. check_sum
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
    int set_num, word_num, i;

    set_num = cache_size / (block_size * associative_size);
    word_num = (block_size / ONE_BYTE_SIZE);

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

    // for W/R trace file -> read sample.trc file
    /**
     * @brief read tracefile accurate format
     * 00010008 W 33
     * 0001000C W 4
     * 00010000 R
     * -> 8(hex) R|W W?data
     */

    // addr is 32bit -> 4byte -> int is 4byte
    int addr, temp_write_data;
    char memory_access_type;
    while (fscanf(tracefile_fp, "%8x %c", &addr, &memory_access_type) != EOF)
    {
        // if memory access type is Write
        if (memory_access_type == 'W')
        {
            fscanf(tracefile_fp, "%d", &temp_write_data);
            // printf("%08X %c %d \n", addr, memory_access_type, temp_write_data);

            // we have to write data to cache
            write_cache(addr, temp_write_data);
        }
        else
        {
            printf("%08X %c \n", addr, memory_access_type);
        }
    }
    return 0;
}

void write_cache(int addr, one_word data)
{
    int index, associative_offset, entry_set_offset;
    cache_line *cache_line_ptr;

    index = (addr / block_size) % set_num;
    entry_set_offset = associative_size;

    for (associative_offset = 0; associative_offset < associative_size; associative_offset++)
    {
        cache_line_ptr = cache + (index * associative_size + associative_offset);

        // CASE1: hit
        if (cache_line_ptr->valid == 1 && cache_line_ptr->tag == ((addr / block_size) / set_num))
        {
            // hit -> (update or write) data to cache
            // 1. dirty bit to 1
            cache_line_ptr->dirty = 1;
            cache_line_ptr->data[((addr / ONE_BYTE_SIZE) % word_num)] = temp_write_data;
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
    cache_line_ptr = cache + (index * associative_size + entry_set_offset);

    // finally write data to cache
    cache_line_ptr->dirty = 1;
    cache_line_ptr->valid = 1;
    cache_line_ptr->tag = ((addr / block_size) / set_num);
    (cache_line_ptr->data)[((addr / ONE_BYTE_SIZE) % word_num)] = temp_write_data;
}

void read_cache(int addr)
{
    int index, associative_offset, entry_set_offset;

    index = (addr / block_size) % set_num;

    for (associative_offset = 0; associative_offset < associative_size; associative_offset++)
    {
        cache_line_ptr = cache + (index * associative_size + associative_offset);

        // CASE 1: hit
        if (cache_line_ptr->valid == 1 && cache_line_ptr->tag == ((addr / block_size) / set_num))
        {
            // TODO: LRU POLICY applicate
            return;
        } // CASE 2: miss
        else if (cache_line_ptr->valid == 0 && entry_set_offset > associative_offset)
        {
            entry_set_offset = associative_offset;
        }
    }

    // CASE 2: miss
    if (entry_set_offset == associative_size)
    {
        // if miss -> we have to read data from main memory
        // and cache the data -> but we don't have some entry
        // so we have to kick the entry by lRU POLICY
    }
    else
    {
        // we have entry
        cache_line_ptr = cache + (index * associative_size + entry_set_offset);
        // if read -> dirty bit is 0
        // because data in cache isn't changed
        cache_line_ptr->dirty = 0;
        cache_line_ptr->valid = 1;
        cache_line_ptr->tag = ((addr / block_size) / set_num);
    }
}