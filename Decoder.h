#ifndef DECODER_H
#define DECODER_H

#include <string>
#include <utility>

namespace Decoder
{
	std::pair<std::string, int> decode(const std::string& msg);
}

#endif