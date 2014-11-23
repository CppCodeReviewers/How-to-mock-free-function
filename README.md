How-to-mock-free-function
=========================

# Problem

There are situations where we need to change or refactor some part of code where dependencies are impossible to replace in runtime for testing (ie. using dependency injection technique). Sometimes we have only header files of such entities with libraries available but only for different platform then we are working on.

Such situation happens when code is using free functions - ie. for memory management. How can we proceed with such situation to be able to follow rule - "cover and change" instead of "change and pray"?

Lets assume that we have piece of code which looks like this:

```
#ifndef DECODER_H
#define DECODER_H
#include <string>
#include <utility>  

namespace Decoder { 
	std::pair<std::string, int> decode(const std::string& msg); 
}  
#endif
```

```
#include "Decoder.h"
#include "ResourceSystem.h"

namespace Decoder 
{
	namespace details 
	{ 
		int decode(const char* msg, size_t length, char* output) 
		{ 
			if (length == 0 || msg[0] != '#')  return 1;  
			output[0] = '!';  
			for (size_t i = 1; i < length; ++i) 
			{ 
				output[i] = msg[i] + 1; 
			}
			return 0; 
		} 
	}
	
	std::pair<std::string, int> decode(const std::string& msg) 
	{ 
		size_t size = msg.size() + 1; 
		union 
		{ 
			void* void_ptr; 
			char* char_ptr; 
		} p = { Resource_Reserve(size) };  
		
		if (!p.void_ptr) return std::make_pair(std::string(), -1);  
		
		int errorCode = 0;  
		if ((errorCode = details::decode(msg.data(), size, p.char_ptr)) != 0) 
		{ 
			Resource_Free(p.void_ptr); 
			return std::make_pair(std::string(), errorCode); 
		}  
		std::string result(p.char_ptr, size-1);  
		Resource_Free(p.void_ptr);  
		return { result, errorCode }; 
	} 
}
```

As you can see code is using external free functions called Resource_Reserve  and Resource_Free  for memory management.

```
#ifndef RESOURCE_SYSTEM_H 
#define RESOURCE_SYSTEM_H  

void* Resource_Reserve(size_t size);  
void Resource_Free(void* resource);  

#endif
```

We don't have access to ResourceSystem implementation - it is delivered for us as library for embedded platform.

# Possible solutions
To think about possible solution lets think about when/how we can replace one entity with its test double.

1. Run-time - dependency injection - most common use for mocking purposes (require dynamic polymorphic types!)
2. Compile-time - use of templates to replace one type to another (require static polymorphic types - duck-typing)
3. Preprocessing - using preprocessor to replace one entity with another
4. Linking-time - replace one implementation of given interface to another

I would like to present one solution which use linking time replacement and Google Mock framework.

# Linking-time solution
Google Mock framework gives us tools for mocking polymorphic types (dynamic and static way). Unfortunatelly there are no support for dealing with free function mocking and non polymorphic types. Nevertheless there is one solution which can handle some of the cases and which is worth mentioning - lets see example.

Lets introduce ResourceSystemMock:

```
#ifndef RESOURCESYSTEMMOCK_H
#define RESOURCESYSTEMMOCK_H  
#include <gmock/gmock.h> 
#include "ResourceSystem.h"  

struct ResourceSystemMock 
{ 
	ResourceSystemMock(); 
	MOCK_CONST_METHOD1(Resource_Reserve, void*(size_t)); MOCK_CONST_METHOD1(Resource_Free, void(void*)); 
};  
#endif // RESOURCESYSTEMMOCK_H
```

This is nothing new - common mock done using Google Mock framework, but it is not mocking free functions, right? At least for now it is not, but...

Lets look on cpp file:

```
#include "ResourceSystemMock.h"
#include <functional>

static std::function<void*(size_t)> _reserve; 
static std::function<void(void*)> _free;

ResourceSystemMock::ResourceSystemMock() 
{ 
	assert(!_reserve && !_free); 
	_reserve = [this](size_t s){ return Resource_Reserve(s); }; 
	_free = [this](void* p){ Resource_Free(p); }; 
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
```

What is happening here? Most important thing to notice is implementation of free functions `void* Resource_Reserve(size_t size)`  and `void Resource_Free(void* resource)` . The only thing they are doing is passing work to `std::function` static objects (`_reserve`  and `_free` ) which are initialized inside `ResourceSystemMock` constructor (line 10 and 11). Which means that after creation of `ResourceSystemMock`  free function `Resource_Reserve`  will delegate its work to `ResourceSystemMock::Resource_Reserve`  method and free function `Resource_Free`  will delegate its work to `ResourceSystemMock::Resource_Free`  method - which as we all know are mocked by our Google Mock framework. That means that we can make expectations on `Resource_Reserve`  and `Resource_Free`  free functions via expectations on `ResourceSystemMock::Resource_Reserve`  and `ResourceSystemMock::Resource_Free`  methods.

Lets see some test code which will probable make things more clear.

```
// ... 
using ValueWithError = std::pair < std::string, int >;
ResourceSystemMock rs;
const char msg[] = "#abc";
char out[sizeof(msg)];
EXPECT_CALL(rs, Resource_Reserve(sizeof(msg))).WillOnce(Return(out));
EXPECT_CALL(rs, Resource_Free(out));
ASSERT_THAT(Decoder::decode(msg), Eq(ValueWithError("!bcd", 0)));
// ...
```

At line 3 we create `ResourceSystemMock`  (called `rs`) and then at lines 7-8 we make expectations on `Resource_Reserve`  and `Resource_Free`  functions - later they will be called inside `Decoder::decode`  function.

Lets see few tests which use such approach:

```
#include <gmock/gmock.h>
#include "ResourceSystemMock.h"
#include "Decoder.h"

using namespace ::testing;

struct DecoderTestsFixture : Test 
{ 
	ResourceSystemMock rs;
	
	using ValueWithError = std::pair < std::string, int >;
};  

TEST_F(DecoderTestsFixture, failed_reserve_resources) 
{ 
	EXPECT_CALL(rs, Resource_Reserve(_)).WillRepeatedly(Return(nullptr));
	ASSERT_THAT(Decoder::decode("message which needs too much resources"), Eq(ValueWithError("", -1))); 
}

TEST_F(DecoderTestsFixture, unsuccessful_decode) 
{ 
	const char msg[] = "wrong message"; 
	char out[sizeof(msg)];  
	
	EXPECT_CALL(rs, Resource_Reserve(sizeof(msg))).WillOnce(Return(out));
	EXPECT_CALL(rs, Resource_Free(out));  
	ASSERT_THAT(Decoder::decode(msg), Eq(ValueWithError("", 1))); 
}  

TEST_F(DecoderTestsFixture, successful_decode) 
{ 
	const char msg[] = "#abc"; 
	char out[sizeof(msg)];  
	EXPECT_CALL(rs, Resource_Reserve(sizeof(msg))).WillOnce(Return(out));
	EXPECT_CALL(rs, Resource_Free(out));  ASSERT_THAT(Decoder::decode(msg), Eq(ValueWithError("!bcd", 0)));
}
```

At line 9 we create `ResourceSystemMock` as `rs` object (it is created inside test fixture - `DecoderTestsFixture` - which will be used in every test).

And that's it - works like a dream! Unfortunately this solution has its own limitation that we cannot have more then one `ResourceSystemMock` object created at a time (every creation of `ResourceSystemMock` override `_reserve` and `_free` static variables - that's why we introduce assert in constructor that these variables has to be empty).

# Expert question
Can we solve the problem with only one ResourceSystemMock  at time? I am waiting for proposals!
