/* INCLUDES */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* TYPE DEFINITIONS */

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef struct {
    u32 self; //your own value
    u32 zero; // self >> 1
    u32 one;  // self >> 1, top bit becomes 1
}RORNODE;

/* CONSTANTS */

#define BITS      (3)                        //current max is 8 because path is made of u8, but should become 63 (implying u64 NODES_MAX)
#define NODES_MAX (1ULL << BITS)             //number of nodes
#define PATHS_MAX (1ULL << (NODES_MAX >> 1)) //number of unique paths taken from 0 to 1 in the graph (conjectured)
#define FOUND_MAX (256)                      //number of unique sequences found (arbitrary)

//bitmasks for node branch and status
#define NODE_BRANCH     (0x0f)
#define NODE_ZERO       (0x00)
#define NODE_ONE        (0x01)

#define NODE_STATUS     (0xf0)
//0x10 is reserved for FORBIDDEN and RESTRICTED branch indication
#define NODE_TAKEN      (0x20)
#define NODE_FORBIDDEN  (0x40)
#define NODE_FREE       (0x80)
#define NODE_RESTRICTED (NODE_FREE|NODE_FORBIDDEN) //can be selected but can't branch to either 0 or 1
#define NODE_TOOK_ZERO  (NODE_TAKEN|NODE_ZERO)
#define NODE_TOOK_ONE   (NODE_TAKEN|NODE_ONE)
#define NODE_FORB_ZERO  (NODE_FORBIDDEN|(NODE_ZERO<<4))
#define NODE_FORB_ONE   (NODE_FORBIDDEN|(NODE_ONE<<4))
#define NODE_ALLW_ZERO  (~NODE_FORB_ZERO)
#define NODE_ALLW_ONE   (~NODE_FORB_ONE)
#define NODE_REST_ZERO  (NODE_RESTRICTED|(NODE_ZERO<<4))
#define NODE_REST_ONE   (NODE_RESTRICTED|(NODE_ONE<<4))

/* MACROS */

#define ROR(x, n) (((x) >> (n)) | ((x) << (NODES_MAX - (n)))) //NODES_MAX bit rotate right
#define ELCNT(array) (sizeof(array) / sizeof(array[0])) //number of elements in an array

/* GLOBALS */

u32 seqs[FOUND_MAX][NODES_MAX] = { 0 };

/* FUNCTIONS */

void print_path(u32* path) {
    /* Print the path taken */
    for (u32 i = 0; i < NODES_MAX; i++)
    {
        printf("%x ", path[i]);
    }
    printf("\n");
}

void print_seq(u32* path) {
    /* Print one byte at a time (assuming 3 <= NODES_MAX <= 8) */
    for (u32 i = 0; i < NODES_MAX; i += 8)
    {
        u8 byte = 0;
        for (u32 j = 0; j < 8; j++)
        {
            byte |= path[NODES_MAX - i - j - 1] << (8 - j - 1);
        }
        printf("%02x", byte);
    }
    printf("\n\n");
}

void print_bits(u8 byte) {
    /* Print the bits of a byte */
    for (u32 i = 0; i < 8; i++)
    {
        printf("%u", (byte >> (8 - i - 1)) & 1);
    }
}

void print_seq_bits(u32* path) {
    /* Print one byte at a time (assuming 3 <= NODES_MAX <= 8) */
    for (u32 i = 0; i < NODES_MAX; i += 8)
    {
        u8 byte = 0;
        for (u32 j = 0; j < 8; j++)
        {
            byte |= path[NODES_MAX - i - j - 1] << (8 - j - 1);
        }
        print_bits(byte);
    }
    printf("\n\n");
}

u64 prng(u64 seq, u32 state) {
    //should compile to ROR
    //todo: rewrite for values above 64 bits (treat as u64 or u32 buffer)
    return ROR(seq, state) & (NODES_MAX - 1);
}

bool test_period(u32* path) {
    /* Test if you can complete a full period */

}

void make_graph(RORNODE* graph) {
    /* Generate a digraph */
    for (u32 i = 0; i < NODES_MAX; i++)
    {
        graph[i].self = i;
        graph[i].zero = graph[i].self >> 1;
        graph[i].one = graph[i].zero | (1 << (BITS - 1));
        /* Debug, print for https://csacademy.com/app/graph_editor/ */
        //printf("%u %u\n", graph[i].self, graph[i].zero);
        //printf("%u %u\n", graph[i].self, graph[i].one);
    }
}

bool is_dupe(u32* path, u32 found) {
    /* Check if path has already been found */
    for (u32 i = 0; i < found; i++)
    {
        u32 matches = 0;
        for (u32 j = 0; j < NODES_MAX; j++)
        {
            if (path[j] == seqs[i][j]) matches++;
        }
        if (matches == NODES_MAX) return true;
    }
    return false;
}

bool set_hits(u32* seq, u8* hit, u32 found) {
    /* Set prev as a forbidden version of cur */

    if (++found >= NODES_MAX) return false; //prevent seq array oob

    u32 branch = (hit[seq[found]] & NODE_BRANCH) << 4; //set branch in status

    memset(hit, NODE_FREE, NODES_MAX);
    hit[0] = NODE_TOOK_ONE;
    hit[seq[found]] = branch | NODE_RESTRICTED; //can't take this starting path
    return true;
}

void make_seqs(RORNODE* graph) {
    /* Generate sequences with the nodes approach */

    //you must traverse the graph from 0 back to itself and visit all nodes in exactly NODES_MAX steps
    //nb: the only node that can come back to 0 is 1
    //nb: always starting by checking the next "one" node will NEVER make you backtrack
    //nb: graph[j].self == j

    u32 found = 0;
    u8 hit[NODES_MAX] = { 0 }; //previous collision checker

    /* Init collisions: all nodes are free except 0 which always takes its ONE branch */
    memset(hit, NODE_FREE, sizeof(hit));
    hit[0] = NODE_TOOK_ONE;

    while (found < FOUND_MAX)
    {

        u32 seq[NODES_MAX] = { 0 }; //path traveled through the graph
        u32 p = 1; //path index
        u32 g = 1; //graph index

        //seq[p] = graph[g].one; //0 always takes its ONE branch

        while (p < NODES_MAX)
        {
            //todo: generate multiple seq, not just one
            //todo: manage priority, check ZERO or ONE first?
            //todo: clean up duplicate code

            /* if current node is allowed to take ZERO and its ZERO branch is free */
            if (((hit[g] & NODE_FORB_ZERO) != NODE_FORB_ZERO) && (hit[graph[g].zero] & NODE_FREE)) //todo: fix NODE_FREE check, doesn't have to ==
            {
                seq[p] = graph[g].zero; //set path
                g = seq[p]; //new node
                hit[g] = (hit[g] ^ NODE_FREE) | NODE_TOOK_ZERO; //mark as visited
                p++; //advance
                continue;
            }

            /* if current node is allowed to take ONE and its ONE branch is free */
            if (((hit[g] & NODE_FORB_ONE) != NODE_FORB_ONE) && (hit[graph[g].one] & NODE_FREE))
            {
                seq[p] = graph[g].one; //set path
                g = seq[p]; //new node
                hit[g] = (hit[g] ^ NODE_FREE) | NODE_TOOK_ONE; //set as visited
                p++; //advance
                continue;
            }

            /* BACKTRACK */
            u32 branch = (hit[g] & NODE_BRANCH) << 4; //took one or zero, set in status
            hit[g] = NODE_FREE; //set current node as free
            if (--p == 0) goto GEN_END; //go back once in the path, abort if 0
            g = seq[p - 1]; //go back twice in the graph
            hit[g] = branch | NODE_FORBIDDEN; //set taken branch as forbidden
            seq[p] = 0; //debug
        }

        if (is_dupe(seq, found)) break; //found a duplicate, abort
        if (!set_hits(seq, hit, found)) break; //couldn't init hit array, abort

        memcpy(&seqs[found++], seq, sizeof(seq)); //add to list of found sequences
        print_seq(seq); //print seq value
        //print_path(seq); //print path taken
        //print_seq_bits(seq);
    }

GEN_END:
    printf("Found: %u\n", found);
}

int main(void) {

    clock_t start = clock();


    RORNODE graph[NODES_MAX] = { 0 };
    printf("Generating %u-bit period (%llu-bit ROR value):\n\n", BITS, NODES_MAX);
    make_graph(graph);
    make_seqs(graph);


    printf("Time: %u ms.\n", clock() - start);
    return 0;
}