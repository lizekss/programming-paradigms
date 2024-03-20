#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0 && numBuckets > 0 && hashfn != NULL && comparefn != NULL);
	h->elemSize = elemSize;
	h->numBuckets = numBuckets;
	h->hashfn = hashfn;
	h->comparefn = comparefn;
	h->freefn = freefn;
	h->buckets = malloc(numBuckets * sizeof(vector));

	assert(h->buckets != NULL);

	for (int i = 0; i < h->numBuckets; i++) 
		VectorNew(h->buckets + i, h->elemSize, h->freefn, 10);
	
}

void HashSetDispose(hashset *h)
{
	for (int i = 0; i < h->numBuckets; i++)
		VectorDispose(h->buckets + i);

	free(h->buckets);
}

int HashSetCount(const hashset *h)
{ 
	int result = 0;
	for (int i = 0; i < h->numBuckets; i++) {
		result += VectorLength(h->buckets + i);
	}
	return result;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL);
	for (int i = 0; i < h->numBuckets; i++)
		VectorMap(h->buckets + i, mapfn, auxData);
}

vector *getBucketAddr(const hashset *h, const void *elemAddr) {
	assert(elemAddr != NULL);
	int hashIndex = h->hashfn(elemAddr, h->numBuckets);
	assert(hashIndex >= 0 && hashIndex < h->numBuckets);

	return h->buckets + hashIndex;
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	vector *bucketAddr = getBucketAddr(h, elemAddr);
	int index = VectorSearch(bucketAddr, elemAddr, h->comparefn, 0, 0);
	if (index > -1)
		VectorReplace(bucketAddr, elemAddr, index);
	else VectorAppend(bucketAddr, elemAddr);
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
	vector *bucketAddr = getBucketAddr(h, elemAddr);

	int idx = VectorSearch(bucketAddr, elemAddr, h->comparefn, 0, 0);
	if (idx > -1)
		return VectorNth(bucketAddr, idx);
	else return NULL;
}
