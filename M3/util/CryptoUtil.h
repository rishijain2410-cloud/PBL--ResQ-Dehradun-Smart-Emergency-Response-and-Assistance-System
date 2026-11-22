#ifndef M3_UTIL_CRYPTO_UTIL_H
#define M3_UTIL_CRYPTO_UTIL_H

#include <string>
#include <vector>

class CryptoUtil {
public:
    // Hashes password using PBKDF2-HMAC-SHA256 with 100,000 iterations and a cryptographically secure 16-byte salt.
    // Format: "pbkdf2_sha256$100000$<salt_hex>$<hash_hex>"
    static std::string hashPassword(const std::string& password);

    // Verifies password against stored formatted hash using constant-time comparison
    static bool verifyPassword(const std::string& password, const std::string& storedHash);

    // Generates a cryptographically secure random session token (32 random bytes -> 64 hex characters)
    static std::string generateToken(size_t numBytes = 32);

    // Computes SHA-256 hash of a string, returning 64 lowercase hex characters
    static std::string sha256Hex(const std::string& input);

    // Generates N cryptographically secure random bytes
    static std::vector<unsigned char> generateRandomBytes(size_t numBytes);

    // Converts bytes to lowercase hex string
    static std::string toHex(const unsigned char* data, size_t len);

    // Converts hex string to bytes
    static std::vector<unsigned char> fromHex(const std::string& hex);

    // Constant-time comparison between two strings to prevent timing attacks
    static bool constantTimeCompare(const std::string& a, const std::string& b);
};

#endif // M3_UTIL_CRYPTO_UTIL_H
