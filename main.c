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

#define BITS      (6)                        //current max is 8 because path is made of u8, but should become u32
#define NODES_MAX (1ULL << BITS)             //number of nodes
#define PATHS_MAX (1ULL << (NODES_MAX >> 1)) //total number of unique paths taken from 0 to 1 in the graph (conjectured)

/* MACROS */

#define ROR(x, n) (((x) >> (n)) | ((x) << (NODES_MAX - (n)))) //NODES_MAX bit rotate right
#define ELCNT(array) (sizeof(array) / sizeof(array[0])) //number of elements in an array

/* GLOBALS */

u32 paths[256][NODES_MAX] = { 0 }; //256 because I don't care about generating more

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

u64 make_seq(u8* path) {
    /**/
    u64 seq = 0;
    for (u32 i = 0; i < NODES_MAX; i++)
    {
        seq |= (u64)path[i] << i;
    }
    return seq;
}

void gen(RORNODE* graph) {
    /* Generate sequences with the nodes approach */

    //you must traverse the graph from 0 back to itself and visit all nodes in exactly NODES_MAX steps
    //nb: the only node that can come back to 0 is 1
    //nb: always starting by checking the next "one" node will NEVER make you backtrack
    //nb: graph[j].self == j

    u8 h[NODES_MAX] = { 0 }; //collision checker, init zero node as visited
    u8 path[NODES_MAX] = { 0 }; //path traveled through the graph
    u32 i = 1; //path index
    u32 j = 1; //graph index

    memset(h, NODE_FREE, sizeof(h));
    h[0] = NODE_TOOK_ONE;

    while (i && i < NODES_MAX)
    {
        //todo: generate multiple seq, not just one
        //todo: manage priority, check ZERO or ONE first?
        //todo: clean up duplicate code

        if (h[j] != NODE_FORB_ZERO && h[graph[j].zero] == NODE_FREE) //if branch zero of current node is free
        {
            path[i] = graph[j].zero; //set path
            j = path[i]; //new node
            h[j] = NODE_TOOK_ZERO; //mark as visited
            i++; //advance
            continue;
        }

        if (h[j] != NODE_FORB_ONE && h[graph[j].one] == NODE_FREE) //else if branch one of current node is free
        {
            path[i] = graph[j].one; //set path
            j = path[i]; //new node
            h[j] = NODE_TOOK_ONE; //set as visited
            i++; //advance
            continue;
        }

        /* BACKTRACK */
        u32 branch = h[j]; //took one or zero?
        h[j] = NODE_FREE; //set current node as free
        i--; //go back once in the path
        j = path[i - 1]; //go back twice in the graph
        h[j] = branch | NODE_FORBIDDEN; //set one or zero branch as forbidden
    }

    print_seq(path);
    print_seq_bits(path);
    //print_path(path);

    //seq = make_seq(path);

    /* test all permutations of this seq */
    //for (u32 k = 0; k < NODES_MAX; k++)
    //{
        //printf("%016llx (%u)\n", seq, test_period(seq));
        //print_bits(seq, sizeof(seq));
    //    seq = ROR(seq, 1);
    //}
}

int rorng_main() {

	clock_t start = clock();


    RORNODE graph[NODES_MAX] = { 0 };
	printf("Generating %u-bit sequence (%u-bit number):\n\n", BITS, NODES_MAX);
    make_graph(graph);
    gen(graph);
	

	printf("Time: %u ms.\n", clock() - start);
    return 0;
}
