#include "utils.h"

long long Utils::pow(long long base, int exp, long long mod) {
  long long result = 1;
  for (; exp; exp >>= 1, base = (base * base) % mod) {
    if (exp & 1) {
      result = (result * base) % mod;
    }
  }
  return result;
}

int Utils::generateVerificationCode(long long secret, int timestamp) {
  constexpr long long MOD = 998244353; // A large prime number for modulus
  constexpr int timeStep =
      30; // Time step in seconds for verification code generation
  return pow(secret, timestamp / timeStep, MOD) %
         1000000; // Generate a 6-digit code
}
