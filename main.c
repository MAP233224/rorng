/* INCLUDES */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

enum NODE_STATE {
    NODE_FREE = 0x00,
    NODE_TOOK_ZERO = 0x20,
    NODE_TOOK_ONE = 0x21,
    NODE_FORBIDDEN = 0xF0, //OR mask on top of TOOK to create FORB
    NODE_FORB_ZERO = 0xF0,
    NODE_FORB_ONE = 0xF1
};

/* CONSTANTS */

#define BITS      (6)                        //current max is 8 because path is made of u8, but should become 63 (implying u64 NODES_MAX)
#define NODES_MAX (1ULL << BITS)             //number of nodes
#define PATHS_MAX (1ULL << (NODES_MAX >> 1)) //number of unique paths taken from 0 to 1 in the graph (conjectured)
#define FOUND_MAX (256)                      //number of unique sequences found (arbitrary)

/* MACROS */

#define ROR(x, n) (((x) >> (n)) | ((x) << (NODES_MAX - (n)))) //NODES_MAX bit rotate right
#define ELCNT(array) (sizeof(array) / sizeof(array[0])) //number of elements in an array

/* GLOBALS */

u32 seqs[FOUND_MAX][NODES_MAX] = { 0 };

/* FUNCTIONS */

void print_path(u8* path) {
    /*  */
    printf("%u", path[0]);
    for (u32 i = 1; i < NODES_MAX; i++)
    {
        printf(":%u", path[i]);
    }
    printf("\n");
}

void print_seq(u8* path) {
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

void print_seq_bits(u8* path) {
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

void make_seqs(RORNODE* graph) {
    /* Generate sequences with the nodes approach */

    //you must traverse the graph from 0 back to itself and visit all nodes in exactly NODES_MAX steps
    //nb: the only node that can come back to 0 is 1
    //nb: always starting by checking the next "one" node will NEVER make you backtrack
    //nb: graph[j].self == j

    u8 hit[NODES_MAX] = { 0 }; //collision checker
    u8 seq[NODES_MAX] = { 0 }; //path traveled through the graph
    u32 p = 1; //path index
    u32 g = 1; //graph index

    /* Init collisions: all nodes are free except 0 which always takes its ONE branch */
    memset(hit, NODE_FREE, sizeof(hit));
    hit[0] = NODE_TOOK_ONE;

    while (p && p < NODES_MAX)
    {
        //todo: generate multiple seq, not just one
        //todo: manage priority, check ZERO or ONE first?
        //todo: clean up duplicate code

        if (hit[g] != NODE_FORB_ZERO && hit[graph[g].zero] == NODE_FREE) //if branch zero of current node is free
        {
            seq[p] = graph[g].zero; //set path
            g = seq[p]; //new node
            hit[g] = NODE_TOOK_ZERO; //mark as visited
            p++; //advance
            continue;
        }

        if (hit[g] != NODE_FORB_ONE && hit[graph[g].one] == NODE_FREE) //else if branch one of current node is free
        {
            seq[p] = graph[g].one; //set path
            g = seq[p]; //new node
            hit[g] = NODE_TOOK_ONE; //set as visited
            p++; //advance
            continue;
        }

        /* BACKTRACK */
        u32 branch = hit[g]; //took one or zero?
        hit[g] = NODE_FREE; //set current node as free
        p--; //go back once in the path
        g = seq[p - 1]; //go back twice in the graph
        hit[g] = branch | NODE_FORBIDDEN; //set taken branch as forbidden
    }

    print_seq(seq);
    print_seq_bits(seq);
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