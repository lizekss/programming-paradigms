#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int DEFAULT_INITIAL_ALLOC = 10;


void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(v != NULL && elemSize > 0 && initialAllocation >= 0);
    if (initialAllocation == 0) initialAllocation = DEFAULT_INITIAL_ALLOC;
    v->allocLen = initialAllocation;
    v->elemSize = elemSize;
    v->elems = malloc(v->allocLen * v->elemSize);
    v->logLen = 0;
    v->freefn = freeFn;

    assert(v->elems != NULL);
}

void *VectorNth(const vector *v, int position)
{ 
    assert(position >= 0 && position < v->logLen);
    void *addr = (char*)v->elems + position * v->elemSize;
    return addr;

}

void VectorDispose(vector *v)
{
    if (v->freefn != NULL) {
        for (int i = 0; i < v->logLen; i++) 
            v->freefn(VectorNth(v, i));
    }
    free(v->elems);

}

int VectorLength(const vector *v)
{ 
    return v->logLen; 
}



void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position < v->logLen);
    void *addr = VectorNth(v, position);
    if (v->freefn != NULL) v->freefn(addr);
    memcpy(addr, elemAddr, v->elemSize);
}

void manageMemoryReallocation(vector *v) {
    if (v->logLen == v->allocLen) {
        v->allocLen *= 2;
        v->elems = realloc(v->elems, v->allocLen * v->elemSize);
        assert(v->elems != NULL);
    }
} 

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position <= v->logLen);
    manageMemoryReallocation(v);
    v->logLen++;
    
    void *addr = VectorNth(v, position);
    void *nextAddr = (char*)addr + v->elemSize;
    memmove(nextAddr, addr, (v->logLen - position - 1) * v->elemSize);
    memcpy(addr, elemAddr, v->elemSize);
    
}


void VectorAppend(vector *v, const void *elemAddr)
{
    manageMemoryReallocation(v);
    
    v->logLen++;
    void *addr = VectorNth(v, v->logLen - 1);
    memcpy(addr, elemAddr, v->elemSize);
}

void VectorDelete(vector *v, int position)
{
    void *addr = VectorNth(v, position);
    v->logLen--;
    if (v->freefn != NULL) v->freefn(addr);
    void *nextAddr = (char*)addr + v->elemSize;
    memmove(addr, nextAddr, (v->logLen - position) * v->elemSize);
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->logLen, v->elemSize, compare);

}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for (int i = 0; i < v->logLen; i++) {
        mapFn(VectorNth(v, i), auxData);
    }

}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
    assert(key != NULL && searchFn != NULL);
    assert(startIndex >= 0 && startIndex <= v->logLen);
    if (isSorted) {
        void *addr = bsearch(key, VectorNth(v, startIndex), v->logLen - startIndex, v->elemSize, searchFn);
        if (addr == NULL) return kNotFound;
        int idx = ((char*)addr - (char*)v->elems) / v->elemSize;
        return idx;
    } else {
        char *ptr = (char *)v->elems;
        for (int i = startIndex; i < v->logLen; i++) {
            if (searchFn(key, ptr) == 0)
                return i;
            ptr += v->elemSize;
        }
        
    }

    return kNotFound;

} 
