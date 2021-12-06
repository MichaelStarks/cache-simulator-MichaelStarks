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
    // Gets the row from the address
    int tag = (addr & cache->tagMask) >> (cache->offsetLength);
    // Gets the tag from the address
    int block = (addr & cache->blockMask) >> (cache->offsetLength + cache->tagLength);
    // Loops through the cache row to find the tag
    for (int numassoc = 0; numassoc < cache->assoc; numassoc++)
    {
        // If the block is empty cache miss and set dirty
        if ((cache->tags[tag].associations[numassoc].block == -1))
        {
            cache->tags[tag].associations[numassoc].block = block;
            // I already dirty, add 1 to dirty evictions
            if (cache->tags[tag].associations[numassoc].dirty == 1)
            {
                *dirtyEvic = *dirtyEvic + 1;
            }
            else
            {
                cache->tags[tag].associations[numassoc].dirty = 1;
            }

            *storeMiss = *(storeMiss) + 1;
            // Update the LRU
            updateLRU(&cache->tags[tag], cache->assoc, numassoc);

            return 0;
        }
        // If th tag is found, add one to store hit and update the lru
        else if (cache->tags[tag].associations[numassoc].block == block)
        {
            *storeHit = *(storeHit) + 1;
            cache->tags[tag].associations[numassoc].dirty = 1;
            updateLRU(&cache->tags[tag], cache->assoc, numassoc);
            return 0;
        }
    }
    // If the tag cannot be found, look at the lru for an index
    int lruIndex = cache->tags[tag].LRU[cache->assoc - 1];
    cache->tags[tag].associations[lruIndex].block = block;
    // if index is dirty add one to dirty evictions
    if (cache->tags[tag].associations[lruIndex].dirty == 1)
    {
        *dirtyEvic = *dirtyEvic + 1;
    }
    cache->tags[tag].associations[lruIndex].dirty = 1;
    // Add one to store miss
    *storeMiss = *storeMiss + 1;
    updateLRU(&cache->tags[tag], cache->assoc, lruIndex);

    return 0;
}

int load(struct cachestructure *cache, int addr, int *loadMiss, int *loadHit, int *dirtyEvic, long int *lruUses)
{
    // Find the row from the address
    int tag = (addr & cache->tagMask) >> (cache->offsetLength);
    // Find the tag from the address
    int block = (addr & cache->blockMask) >> (cache->offsetLength + cache->tagLength);
    bool foundEmpty = 0;
    int emptyIndex = 0;
    // Loops through the cache row
    for (int numassoc = 0; numassoc < cache->assoc; numassoc++)
    {
        // if the tag is found add 1 to hit
        if ((cache->tags[tag].associations[numassoc].block == block))
        {
            *loadHit = *(loadHit) + 1;
            // update lru
            updateLRU(&cache->tags[tag], cache->assoc, numassoc);
            return 0;
        }
        // if not found find the first empty address to put it in
        else if ((cache->tags[tag].associations[numassoc].block == -1))
        {
            foundEmpty = 1;
            emptyIndex = numassoc;
            break;
        }
    }
    // if an empty location is found
    if (foundEmpty == 1)
    {
        // if dirty add one to dirty evic
        if (cache->tags[tag].associations[emptyIndex].dirty == 1)
        {
            *dirtyEvic = *dirtyEvic + 1;
            cache->tags[tag].associations[emptyIndex].dirty = 0;
        }
        // add one to load miss
        *loadMiss = *loadMiss + 1;
        // set the tag
        cache->tags[tag].associations[emptyIndex].block = block;
        // Update lru
        updateLRU(&cache->tags[tag], cache->assoc, emptyIndex);
        return 0;
    }
    // if not found
    else if (foundEmpty == 0)
    {
        //get index from lru
        int lruIndex = cache->tags[tag].LRU[cache->assoc - 1];
        // if index is dirty add one to dirty evic
        if (cache->tags[tag].associations[lruIndex].dirty == 1)
        {
            *dirtyEvic = *dirtyEvic + 1;
            cache->tags[tag].associations[lruIndex].dirty = 0;
        }
        // add one to load miss
        *loadMiss = *loadMiss + 1;
        // set tag
        cache->tags[tag].associations[lruIndex].block = block;
        // update lru
        updateLRU(&cache->tags[tag], cache->assoc, lruIndex);
        return 0;
    }

    return 1;
}