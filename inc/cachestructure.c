#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/cachestructure.h"

void init(struct cachestructure *cache, int assoc, int blksz, int cacheSize, int misspen)
{
    // Initializes variables to build cache structure
    int numAssoc = (cacheSize * 1024) / blksz;
    int numRows = numAssoc / assoc;
    int offsetLength = (int)(log10(blksz) / log10(2));
    int tagLength = (int)(log10(numRows) / log10(2));
    int blockLength = 32 - (offsetLength + tagLength);
    cache->assoc = assoc;
    cache->blksz = blksz;
    cache->cacheSize = cacheSize * 1024;
    cache->misspen = misspen;
    cache->tagLength = tagLength;
    cache->offsetLength = offsetLength;
    // Finds the mask needed to parse the informtion from the address
    cache->tagMask = ((unsigned long)pow(2, tagLength) - 1) << offsetLength;
    cache->blockMask = ((unsigned long)pow(2, blockLength) - 1) << (tagLength + offsetLength);
    cache->offsetMask = ((unsigned long)pow(2, offsetLength));

    // Generates the cache structure 
    cache->tags = malloc(numRows * sizeof(struct tag)); //Generates the rows

    for (int index = 0; index < numRows; index++)
    {
        (cache->tags)[index].associations = malloc(assoc * sizeof(struct association)); // Generates each row's associations
        for (int num = 0; num < assoc; num++)
        {   
            // Initializes each variable in association
            cache->tags[index].associations[num].block = -1;
            cache->tags[index].associations[num].dirty = 0;
        }
        (cache->tags)[index].LRU = malloc(assoc * sizeof(int)); // Generates the rows LRU
        for (int i = 0; i < assoc; i++)
        {
            // initializes the LRU index to -1
            (cache->tags)[index].LRU[i] = -1;
        }
        // Counter for LRU array
        (cache->tags)[index].end = assoc;
    }
}

int updateLRU(struct tag *tag, int numassoc, int associndex)
{
    // Loops through the LRU
    for (int index = numassoc - 1; index >= tag->end; index--)
    {
        // Finds the index of the MRU association in the array
        if (tag->LRU[index] == associndex)
        {
            // Moves all the values after the MRU up on position
            for (int i = index - 1; i >= tag->end; i--)
            {
                tag->LRU[i + 1] = tag->LRU[i];
            }
            // Moves the MRU to the end of the array
            tag->LRU[tag->end] = associndex;
            return 0;
        }
    }
    // If the index is not in the LRU, add it and decrement the end pointer
    tag->end = tag->end - 1;
    tag->LRU[tag->end] = associndex;
    return 0;
}

int store(struct cachestructure *cache, int addr, int *dirtyEvic, int *storeMiss, int *storeHit, long int *lruUses)
{
    int tag = (addr & cache->tagMask) >> (cache->offsetLength);
    int block = (addr & cache->blockMask) >> (cache->offsetLength + cache->tagLength);

    for (int numassoc = 0; numassoc < cache->assoc; numassoc++)
    {
        if ((cache->tags[tag].associations[numassoc].block == -1))
        {
            cache->tags[tag].associations[numassoc].block = block;
            if (cache->tags[tag].associations[numassoc].dirty == 1)
            {
                *dirtyEvic = *dirtyEvic + 1;
            }
            else
            {
                cache->tags[tag].associations[numassoc].dirty = 1;
            }

            *storeMiss = *(storeMiss) + 1;
            updateLRU(&cache->tags[tag], cache->assoc, numassoc);

            return 0;
        }
        else if (cache->tags[tag].associations[numassoc].block == block)
        {
            *storeHit = *(storeHit) + 1;
            cache->tags[tag].associations[numassoc].dirty = 1;
            updateLRU(&cache->tags[tag], cache->assoc, numassoc);
            return 0;
        }
    }

    int lruIndex = cache->tags[tag].LRU[cache->assoc - 1];
    cache->tags[tag].associations[lruIndex].block = block;
    if (cache->tags[tag].associations[lruIndex].dirty == 1)
    {
        *dirtyEvic = *dirtyEvic + 1;
    }
    cache->tags[tag].associations[lruIndex].dirty = 1;
    *storeMiss = *storeMiss + 1;
    updateLRU(&cache->tags[tag], cache->assoc, lruIndex);

    return 0;
}

int load(struct cachestructure *cache, int addr, int *loadMiss, int *loadHit, int *dirtyEvic, long int *lruUses)
{
    int tag = (addr & cache->tagMask) >> (cache->offsetLength);
    int block = (addr & cache->blockMask) >> (cache->offsetLength + cache->tagLength);
    bool foundEmpty = 0;
    int emptyIndex = 0;
    for (int numassoc = 0; numassoc < cache->assoc; numassoc++)
    {
        if ((cache->tags[tag].associations[numassoc].block == block))
        {
            *loadHit = *(loadHit) + 1;
            updateLRU(&cache->tags[tag], cache->assoc, numassoc);
            return 0;
        }
        else if ((cache->tags[tag].associations[numassoc].block == -1))
        {
            foundEmpty = 1;
            emptyIndex = numassoc;
            break;
        }
    }

    if (foundEmpty == 1)
    {
        if (cache->tags[tag].associations[emptyIndex].dirty == 1)
        {
            *dirtyEvic = *dirtyEvic + 1;
            cache->tags[tag].associations[emptyIndex].dirty = 0;
        }
        *loadMiss = *loadMiss + 1;
        cache->tags[tag].associations[emptyIndex].block = block;
        updateLRU(&cache->tags[tag], cache->assoc, emptyIndex);
        return 0;
    }
    else if (foundEmpty == 0)
    {
        int lruIndex = cache->tags[tag].LRU[cache->assoc - 1];
        if (cache->tags[tag].associations[lruIndex].dirty == 1)
        {
            *dirtyEvic = *dirtyEvic + 1;
            cache->tags[tag].associations[lruIndex].dirty = 0;
        }
        *loadMiss = *loadMiss + 1;
        cache->tags[tag].associations[lruIndex].block = block;
        updateLRU(&cache->tags[tag], cache->assoc, lruIndex);
        return 0;
    }

    return 1;
}