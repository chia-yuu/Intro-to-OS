/*
Student No.: 111550108
Student Name: Chia-Yu Wu
Email: amberwu0411@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include<stdlib.h>
#include<stdio.h>
#include<sys/mman.h>
#include<string.h>
#include<unistd.h>

#define n_level 11
#define header_size 32
#define mem_pool_size 20000

typedef struct block{
    int size;   // 4 bytes, size of the data this block can store (no header)
    int free;   // 4 bytes, whether this block is free
    struct block *mem_next; // 8 bytes, next block in mem_pool
    struct block *mem_pre;  // 8 bytes, previous block in mem_pool
    struct block *list_next;// 8 bytes, next block in the free list
} block;

int is_first = 1;
block *mem_pool = NULL;
block *free_list[n_level] = {NULL};     // start at the header, size of the data

char debug[100] = {'\0'};

int get_level(int size){
    int level = 0, top = 32;
    while(level < n_level && size >= top){
        level++;
        top *= 2;
    }
    return level;
}

void *malloc(size_t size){
    // sprintf(debug, "\nmalloc %ld\n", size);
    // write(1, debug, strlen(debug));

    // print mem_pool
    // block *tp = mem_pool;
    // sprintf(debug, "mem_pool: ");
    // write(1, debug, strlen(debug));
    // while(tp){
    //     sprintf(debug, "%d, ", tp->size);
    //     write(1, debug, strlen(debug));
    //     tp = tp->mem_next;
    // }
    // sprintf(debug, "\n");
    // write(1, debug, strlen(debug));

    // last call, print size and release mem_pool
    if(size == 0){
        // find mx size
        block *cur = mem_pool;
        int mx = 0;
        while(cur){
            if(cur->free && cur->size > mx){
                mx = cur->size;
            }
            cur = cur->mem_next;
        }

        // print mx size
        char str[50];
        memset(str, 0, 50);
        sprintf(str, "Max Free Chunk Size = %d\n", mx);
        write(1, str, strlen(str));

        // release mem_pool
        munmap(mem_pool, mem_pool_size);
        return 0;
    }
    else{
        // allocation size round to a multiple of 32
        size = ((size+31)/32) * 32;
        // sprintf(debug, "A data = %ld, add header = %ld, ", size, size+32);
        // write(1, debug, strlen(debug));

        // first call, allocate 20000 bytes mem
        if(!mem_pool){
            // allocate mem_pool and init
            mem_pool = mmap(NULL, mem_pool_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            mem_pool->size = mem_pool_size - header_size;
            mem_pool->free = 1;
            mem_pool->mem_next = NULL;
            mem_pool->mem_pre = NULL;
            mem_pool->list_next = NULL;

            // push mem_pool (the block) into free list
            free_list[10] = mem_pool;
        }

        // find the best fit block
        int level = get_level(size);
        for(int i=level;i<n_level;i++){
            block *cur = free_list[i], *bf = NULL;
            int mx = 999999;
            while(cur){
                // cur == required size, don't need to split
                if(cur->size < mx && cur->size == size){
                    bf = cur;
                    mx = cur->size;
                }
                // cur > required size, have to split, new block after split at least = header_size (32)
                else if(cur->size < mx && cur->size >= size+header_size){
                    bf = cur;
                    mx = cur->size;
                }
                cur = cur->list_next;
            }

            // find bf in this level, allocate and return
            // else go to next level to search
            if(bf){
                // sprintf(debug, "bf = %d\n", bf->size);
                // write(1, debug, strlen(debug));
                // size in bf is larger than required size, split
                if(bf->size > size){
                    // create new block for addition mem
                    block *new_block = bf + 1 + (size / 32);
                    new_block->size = bf->size - size - header_size;    // bf size - required size - new_block header
                    new_block->free = 1;
                    new_block->mem_pre = bf;
                    new_block->mem_next = bf->mem_next;
                    new_block->list_next = NULL;    // add in the end of the list

                    // sprintf(debug, "space size = %d, require = %ld, new_block size = %d\n", bf->size, size, new_block->size);
                    // write(1, debug, strlen(debug));

                    // new_block push into free_list and set its list_next
                    int new_block_level = get_level(new_block->size);
                    if(free_list[new_block_level]){
                        block *tmp = free_list[new_block_level];
                        while(tmp->list_next){tmp = tmp->list_next;}
                        tmp->list_next = new_block; // add in the end of the list
                    }
                    else{
                        // null, no block in this level (new_block is the first one)
                        free_list[new_block_level] = new_block;
                    }

                    // modify bf (in mem_pool)
                    if(bf->mem_next){bf->mem_next->mem_pre = new_block;}
                    bf->mem_next = new_block;
                    bf->size = size;
                }
                // remove bf from free_list
                bf->free = 0;
                block *t = free_list[i];
                if(t == bf){
                    free_list[i] = bf->list_next;
                }
                else{
                    while(t){
                        if(t->list_next == bf){
                            t->list_next = bf->list_next;
                        }
                        t = t->list_next;
                    }
                }

                // return allocate address

                // print free_list
                // for(int lv = 0;lv<11;lv++){
                //     // sprintf(debug, "\nfreelist[%d]: ", lv);
	            //     // write(1, debug, strlen(debug));
                //     block *t = free_list[lv];
                //     while(t){
                //         sprintf(debug, "%d, ", t->size);
	            //         write(1, debug, strlen(debug));
                //         t = t->list_next;
                //     }
                // }

                // print mem_pool
                // t = mem_pool;
                // sprintf(debug, "mem_pool: ");
                // write(1, debug, strlen(debug));
                // while(t){
                //     sprintf(debug, "%p (%d), ", t, t->size);
                //     write(1, debug, strlen(debug));
                //     t = t->mem_next;
                // }
                // sprintf(debug, "\n");
                // write(1, debug, strlen(debug));
                return bf + 1;  // the begining address of the data
            }
        }
    }
    return 0;   // there's no space?
}

void free(void *pos){
    // sprintf(debug, "free\n");
    // write(1, debug, strlen(debug));
    block *cur = pos;   // pos is the begining of the data
    cur -= 1;   // go to the begining of the block (begining of the header)
    cur->free = 1;

    // sprintf(debug, "D data = %d, add header = %d, ", cur->size, cur->size+32);
    // write(1, debug, strlen(debug));

    // modify mem_pool, merge free block and modify free_list
    if(cur->mem_next && cur->mem_next->free){
        block *cur_next = cur->mem_next;
        // merge cur and cur->next in mem_pool
        cur->size += cur->mem_next->size + header_size; // header of cur->next
        if(cur->mem_next->mem_next){cur->mem_next->mem_next->mem_pre = cur;}
        cur->mem_next = cur->mem_next->mem_next;
        
        // remove cur_next from free_list
        int level = get_level(cur_next->size);
        block *tmp = free_list[level];
        if(tmp == cur_next){
            // cur_next is the first block in this level
            free_list[level] = cur_next->list_next;
        }
        else{
            while(tmp->list_next != cur_next){tmp = tmp->list_next;}
            tmp->list_next = cur_next->list_next;
        }
    }
    if(cur->mem_pre && cur->mem_pre->free){
        block *cur_pre = cur->mem_pre;

        // remove cur_pre from free_list
        int level = get_level(cur_pre->size);
        block *tmp = free_list[level];
        if(tmp == cur_pre){
            // cur is the first block in this level
            free_list[level] = cur_pre->list_next;
        }
        else{
            while(tmp && tmp->list_next != cur_pre){tmp = tmp->list_next;}
            tmp->list_next = cur_pre->list_next;
        }

        // merge cur and cur->pre in mem_pool
        cur->mem_pre->size += cur->size + header_size; // header of cur
        if(cur->mem_next){cur->mem_next->mem_pre = cur->mem_pre;}
        cur->mem_pre->mem_next = cur->mem_next;

        cur = cur_pre;
    }

    // push cur to free_list
    int level = get_level(cur->size);
    block *tmp = free_list[level];
    if(!tmp){
        // nothing in this level (NULL), cur is the first block
        free_list[level] = cur;
    }
    else{
        while(tmp->list_next){tmp = tmp->list_next;}
        tmp->list_next = cur;
    }
    cur->list_next = NULL;

    // sprintf(debug, "after merge data = %d, add header = %d\n", cur->size, cur->size+32);
    // write(1, debug, strlen(debug));

    // print free_list
    // for(int lv = 0;lv<11;lv++){
    //     sprintf(debug, "freelist[%d]: ", lv);
    //     write(1, debug, strlen(debug));
    //     block *t = free_list[lv];
    //     while(t){
    //         sprintf(debug, "%d, ", t->size);
    //         write(1, debug, strlen(debug));
    //         t = t->list_next;
    //     }
    //     sprintf(debug, "\n");
    //     write(1, debug, strlen(debug));
    // }
    // sprintf(debug, "\n");
    // write(1, debug, strlen(debug));
    
    // print mem_pool
    // block *t = mem_pool;
    // sprintf(debug, "mem_pool: ");
    // write(1, debug, strlen(debug));
    // while(t){
    //     sprintf(debug, "%p (%d), ", t, t->size);
    //     write(1, debug, strlen(debug));
    //     t = t->mem_next;
    // }
    // sprintf(debug, "\n");
    // write(1, debug, strlen(debug));
}

/*
gcc -shared -fPIC multilevelBF.c -o multilevelBF.so
LD_PRELOAD=./multilevelBF.so ./main

all malloc
0x7f37368b5000 (2016), 0x7f37368b5800 (960), 0x7f37368b5be0 (960), 0x7f37368b5fc0 (960), 0x7f37368b63a0 (960), 0x7f37368b6780 (960), 0x7f37368b6b60 (960), 0x7f37368b6f40 (960), 0x7f37368b7320 (480), 0x7f37368b7520 (448), 0x7f37368b7700 (9984),

free
0x7f37368b5000 (2016), 0x7f37368b5800 (960), 0x7f37368b5be0 (960), 0x7f37368b5fc0 (960), 0x7f37368b63a0 (960), 0x7f37368b6780 (960), 0x7f37368b6b60 (960), 0x7f37368b6f40 (960), 0x7f37368b7520 (960), 0x7f37368b7700 (9984),

first malloc after free
0x7f37368b5000 (2016), 0x7f37368b5800 (960), 0x7f37368b5be0 (512), 0x7f37368b5e00 (416), 0x7f37368b5fc0 (960), 0x7f37368b63a0 (960), 0x7f37368b6780 (960), 0x7f37368b6b60 (960), 0x7f37368b6f40 (960), 0x7f37368b7520 (960), 0x7f37368b7700 (9984),

before error
0x7f37368b5000 (2016), 0x7f37368b5800 (960), 0x7f37368b5be0 (512), 0x7f37368b5e00 (416), 0x7f37368b5fc0 (960), 0x7f37368b63a0 (512), 0x7f37368b65c0 (416), 0x7f37368b6780 (960), 0x7f37368b6b60 (512), 0x7f37368b6d80 (416), 0x7f37368b6f40 (960), 0x7f37368b7520 (512), 0x7f37368b7740 (416), 0x7f37368b7700 (9984),

/*
0, 0, 32
1, 32, 64
2, 64, 128
3, 128, 256
4, 256, 512
5, 512, 1024
6, 1024, 2048
7, 2048, 4096
8, 4096, 8192
9, 8192, 16384
10, 16384, 32768 (20000)
*/