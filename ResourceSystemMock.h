#ifndef RESOURCESYSTEMMOCK_H
#define RESOURCESYSTEMMOCK_H

#include <gmock/gmock.h>
#include "ResourceSystem.h"

struct ResourceSystemMock
{
	ResourceSystemMock();
	~ResourceSystemMock();
	MOCK_CONST_METHOD1(Resource_Reserve, void*(size_t));
	MOCK_CONST_METHOD1(Resource_Free, void(void*));
};


#endif // RESOURCESYSTEMMOCK_H
