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
 * }
 */
typedef struct main_memory_line_
{
    int addr;
    one_word *data;
} main_memory_line;

// main_memory_line* [] -> that's a main memory;
// !MAIN MEMORY
main_memory_line *main_memory;

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
    int set_num, i;

    set_num = cache_size / (block_size * associative_size);
    // initialize with 0 -> we have to use calloc
    cache = (cache_line *)calloc(set_num * associative_size, sizeof(cache_line));
    for (i = 0; i < set_num * associative_size; i++)
        cache[i].data = (one_word *)calloc(block_size / ONE_WORD_SIZE, sizeof(one_word));

    // initialize size 512 with main_memory_line struct
    main_memory = (main_memory_line *)calloc(INITIAL_BUFFER_SIZE, sizeof(main_memory_line));
    for (i = 0; i < INITIAL_BUFFER_SIZE; i++)
        main_memory[i].data = (one_word *)calloc(block_size / ONE_WORD_SIZE, sizeof(one_word));

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

            int index, associative_offset;
            cache_line *cache_line_ptr;
            index = ((addr / (block_size / ONE_WORD_SIZE)) / ONE_WORD_SIZE) % set_num;

            // go to cache[index] and add data
            for (associative_offset = 0; associative_offset < associative_size; associative_offset++)
            {
                cache_line_ptr = cache + (index * associative_size + associative_offset);
            }
        }
        else
        {
            printf("%08X %c \n", addr, memory_access_type);
        }
    }
    return 0;
}