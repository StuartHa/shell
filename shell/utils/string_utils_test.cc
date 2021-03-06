#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "string_utils.h"

using std::string;
using std::vector;
using utils::Split;
using utils::Trim;

TEST(Split, Basic) {
	vector<string> expected {"how", "now", "brown", "cow"};
	EXPECT_THAT(expected, Split("how now brown cow", ' '));
}

TEST(Split, MultipleSpaces) {
	vector<string> expected {"hi", "there"};
	EXPECT_THAT(expected, Split("hi    there", ' '));
}

TEST(Split, BeginningAndEndingSpaces) {
	vector<string> expected {"hi", "there"};
	EXPECT_THAT(expected, Split("       hi    there     ", ' '));
}

TEST(Split, NonSpaceDelimiter) {
	vector<string> expected {"h e", "o "};
	EXPECT_THAT(expected, Split("h ello ", 'l'));
}

TEST(Trim, Basic) {
	EXPECT_EQ("hi there", Trim("  hi there   "));
	EXPECT_EQ("hi there", Trim("  hi there"));
	EXPECT_EQ("hi there", Trim("hi there   "));
}
