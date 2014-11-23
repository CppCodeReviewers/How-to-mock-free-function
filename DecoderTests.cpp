#include <gmock/gmock.h>
#include "ResourceSystemMock.h"
#include "Decoder.h"

using namespace ::testing;

struct DecoderTestsFixture : Test
{
	ResourceSystemMock rs;

	using ValueWithError = std::pair < std::string, int > ;
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
	EXPECT_CALL(rs, Resource_Free(out));

	ASSERT_THAT(Decoder::decode(msg), Eq(ValueWithError("!bcd", 0)));
}
