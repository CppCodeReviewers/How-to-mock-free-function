#include "Decoder.h"

#include "ResourceSystem.h"

namespace Decoder
{

	namespace details
	{
		int decode(const char* msg, size_t length, char* output)
		{
			if (length == 0 || msg[0] != '#') 
				return 1;

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
		union {
			void* void_ptr;
			char* char_ptr;
		} p = { Resource_Reserve(size) };

		if (!p.void_ptr)
			return std::make_pair(std::string(), -1);

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