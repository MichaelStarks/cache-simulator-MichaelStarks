#include <stdbool.h>

struct association
{
    char dirty;
    int block;
}; //association

struct tag
{
    int *LRU;
    int end;
    int newElements;
    struct association *associations;

}; //tag

struct cachestructure
{
    int assoc;
    int blksz;
    int cacheSize;
    int misspen;
    int offsetLength;
    int tagLength;
    unsigned long blockMask;
    unsigned long tagMask;
    unsigned long offsetMask;

    struct tag *tags;

}; //cachestructure

void init(struct cachestructure *, int, int, int, int);
int updateLRU(struct tag *, int, int);
int store(struct cachestructure *, int, int *, int *, int *, long int *);
int load(struct cachestructure *, int, int *, int *, int *, long int *);