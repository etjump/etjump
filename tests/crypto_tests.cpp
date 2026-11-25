#include <gtest/gtest.h>

#include "../src/game/etj_crypto.h"

using ETJump::Crypto;

TEST(CryptoTests, sha2_IsValidSHA2) {
  EXPECT_TRUE(Crypto::isValidSHA2(Crypto::sha2("foo")));
}

TEST(CryptoTests, sha1_IsValidSHA1) {
  EXPECT_TRUE(Crypto::isValidSHA1(Crypto::sha1("foo")));
}

TEST(CryptoTests, isValidSHA2_RejectsWrongLength) {
  EXPECT_FALSE(Crypto::isValidSHA2(Crypto::sha1("foo")));
  EXPECT_FALSE(Crypto::isValidSHA2(""));
}

TEST(CryptoTests, isValidSHA2_RejectsLowercaseHex) {
  EXPECT_FALSE(Crypto::isValidSHA2(std::string(64, 'a')));
}

// ':' to '@' sit between '9' and 'A' in ASCII and are not hex digits
TEST(CryptoTests, isValidSHA2_RejectsCharactersBetweenDigitsAndLetters) {
  for (const char c : std::string(":;<=>?@")) {
    EXPECT_FALSE(Crypto::isValidSHA2(std::string(64, c))) << c;
  }
}
