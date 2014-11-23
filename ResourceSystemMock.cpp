#include "ResourceSystemMock.h"
#include <functional>

static std::function<void*(size_t)> _reserve;
static std::function<void(void*)> _free;

ResourceSystemMock::ResourceSystemMock()
{
	assert(!_reserve && !_free);
	_reserve = [this](size_t s){ return Resource_Reserve(s); };
	_free =    [this](void*  p){ Resource_Free(p); };
}

ResourceSystemMock::~ResourceSystemMock()
{
	_reserve = {};
	_free = {};
}

void* Resource_Reserve(size_t size)
{
	return _reserve(size);
}

void Resource_Free(void* resource)
{
	_free(resource);
}
