#ifndef UTILS_H
#define UTILS_H

namespace Utils {
/**
 * @brief calculate pow of a number
 * @param base: the base number
 * @param exp: the exponent
 * @param mod: the modulus (optional)
 * @return the result of base^exp
 */
long long pow(long long base, int exp, long long mod);
/**
 * @brief Generate a verification code based on the secret and timestamp
 * @param secret: Secret key for the verification code generation
 * @param timestamp: Current timestamp
 * @return Generated verification code as an integer
 */
int generateVerificationCode(long long secret, int timestamp);
} // namespace Utils

#endif // UTILS_H
