#ifndef RESOURCE_SYSTEM_H
#define RESOURCE_SYSTEM_H

void* Resource_Reserve(size_t size);

void Resource_Free(void* resource);

#endif