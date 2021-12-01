#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include "inc/headers/cachestructure.h"

int associativity = 2;    // Associativity of cache
int blocksize_bytes = 32; // Cache Block size in bytes
int cachesize_kb = 64;    // Cache size in KB
int clockRate = 0;        //in giga hertz
int miss_penalty = 30;
int extraPenalty = 0;
float associativityPen = 0;
float cachePen = 0;

long int executTime = 0;
long int instructions = 0;
long int memAccess = 0;
float totalMissRate = 0;
float readMissRate = 0;
float memCPI = 0;
float totalCPI = 0;
float avgMemTime = 0;
int dirtyEvic = 0;
int loadMiss = 0;
int storeMiss = 0;
int loadHits = 0;
int storeHits = 0;
long int lruUses = 0;

struct cachestructure cache;

void print_usage()
{
  printf("Usage: gunzip2 -c <tracefile> | ./cache -a <assoc> -l <blksz> -s <size> -mp <mispen>\n");
  printf("  <tracefile>: The memory trace file\n");
  printf("  -a <assoc>: The associativity of the cache\n");
  printf("  -l <blksz>: The blocksize (in bytes) of the cache\n");
  printf("  -s <size>: The size (in KB) of the cache\n");
  printf("  -mp <mispen>: The miss penalty (in cycles) of a miss\n");
  exit(0);
}

void getPenalty(int associativity, float *associativityPen, int cachesize_kb, float *cachePen, int blocksize_bytes, int *blockPen)
{
  switch (associativity)
  {
  case 1:
    *associativityPen = 0;
    break;
  case 2:
    *associativityPen = 0.05;
    break;
  case 4:
    *associativityPen = 0.075;
    break;
  case 8:
    *associativityPen = 0.1;
    break;
  default:
    break;
  }

  switch (cachesize_kb)
  {
  case 16:
    *cachePen = 0;
    break;
  case 32:
    *cachePen = 0.05;
    break;
  case 64:
    *cachePen = 0.1;
    break;
  case 128:
    *cachePen = 0.15;
    break;
  default:
    break;
  }

  switch (blocksize_bytes)
  {
  case 16:
    *blockPen = 0;
    break;
  case 32:
    *blockPen = 2;
    break;
  case 64:
    *blockPen = 6;
    break;
  case 128:
    *blockPen = 14;
    break;
  default:
    break;
  }
}

int main(int argc, char *argv[])
{

  long address;
  int loadstore, icount;
  char marker;

  int j = 1;
  // Process the command line arguments
  // Process the command line arguments
  while (j < argc)
  {
    if (strcmp("-a", argv[j]) == 0)
    {
      j++;
      if (j >= argc)
        print_usage();
      associativity = atoi(argv[j]);
      j++;
    }
    else if (strcmp("-l", argv[j]) == 0)
    {
      j++;
      if (j >= argc)
        print_usage();
      blocksize_bytes = atoi(argv[j]);
      j++;
    }
    else if (strcmp("-s", argv[j]) == 0)
    {
      j++;
      if (j >= argc)
        print_usage();
      cachesize_kb = atoi(argv[j]);
      j++;
    }
    else if (strcmp("-mp", argv[j]) == 0)
    {
      j++;
      if (j >= argc)
        print_usage();
      miss_penalty = atoi(argv[j]);
      j++;
    }
    else
    {
      print_usage();
    }
  }

  getPenalty(associativity, &associativityPen, cachesize_kb, &cachePen, blocksize_bytes, &extraPenalty);

  // print out cache configuration
  printf("Cache parameters:\n");
  printf("Cache Size (KB)\t\t\t%d\n", cachesize_kb);
  printf("Cache Associativity\t\t%d\n", associativity);
  printf("Cache Block Size (bytes)\t%d\n", blocksize_bytes);
  printf("Miss penalty (cyc)\t\t%d\n", miss_penalty);
  printf("\n");

  init(&cache, associativity, blocksize_bytes, cachesize_kb, miss_penalty);
  while (scanf("%c %d %lx %d\n", &marker, &loadstore, &address, &icount) != EOF)
  {
    // Code to print out just the first 10 addresses.  You'll want to delete
    // this part once you get things going.
    // printf("%ld\n",address);
    instructions = instructions + icount;
    if (memAccess < 10)
    {
      // printf("\t%c %d %lx %d\n", marker, loadstore, address, icount);
    }
    memAccess++;
    if (loadstore == 0)
    {
      load(&cache, address, &loadMiss, &loadHits, &dirtyEvic, &lruUses);
    }
    else
    {
      store(&cache, address, &dirtyEvic, &storeMiss, &storeHits, &lruUses);
    }

    //here is where you will want to process your memory accesses
  }
  // Here is where you want to print out stats
  printf("Lines found = %ld \n", memAccess);
  printf("Simulation results:\n");
  //  Use your simulator to output the following statistics.  The
  //  print statements are provided, just replace the question marks with
  //  your calcuations.

  readMissRate = (float)(loadMiss) / (float)(loadMiss + loadHits);
  totalMissRate = ((float)loadMiss + (float)storeMiss) / (float)memAccess;
  executTime = (instructions + (dirtyEvic * (miss_penalty + extraPenalty + 2)) +
                ((loadMiss + storeMiss - dirtyEvic) * miss_penalty + extraPenalty)) *
               (1 + (associativityPen + cachePen));
  totalCPI = (float)executTime / (float)instructions;

  printf("\texecution time %ld cycles\n", executTime);
  printf("\tinstructions %ld\n", instructions);
  printf("\tmemory accesses %ld\n", memAccess);
  printf("\toverall miss rate %.2f\n", totalMissRate);
  printf("\tread miss rate %.2f\n", readMissRate);
  printf("\tmemory CPI %.2f\n", totalCPI - 1);
  printf("\ttotal CPI %.2f\n", totalCPI);
  printf("\taverage memory access time %.2f cycles\n", ((miss_penalty + extraPenalty) * totalMissRate) + (1 - totalMissRate));
  printf("dirty evictions %d\n", dirtyEvic);
  printf("load_misses %d\n", loadMiss);
  printf("store_misses %d\n", storeMiss);
  printf("load_hits %d\n", loadHits);
  printf("store_hits %d\n", storeHits);
  // printf("LRU Uses %ld\n", lruUses);
}
