#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include "CryptoUtil.h"

#include <windows.h>
#include <bcrypt.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace {
// Helper for constant-time comparison
bool constantTimeMemcmp(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned char result = 0;
    for (size_t i = 0; i < len; ++i) {
        result |= (a[i] ^ b[i]);
    }
    return result == 0;
}
} // anonymous namespace

std::string CryptoUtil::toHex(const unsigned char* data, size_t len) {
    std::ostringstream ss;
    for (size_t i = 0; i < len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return ss.str();
}

std::vector<unsigned char> CryptoUtil::fromHex(const std::string& hex) {
    std::vector<unsigned char> bytes;
    if (hex.size() % 2 != 0) return bytes;
    bytes.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        try {
            unsigned char b = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
            bytes.push_back(b);
        } catch (...) {
            return {};
        }
    }
    return bytes;
}

std::vector<unsigned char> CryptoUtil::generateRandomBytes(size_t numBytes) {
    std::vector<unsigned char> buffer(numBytes, 0);
    NTSTATUS status = BCryptGenRandom(
        NULL,
        buffer.data(),
        static_cast<ULONG>(numBytes),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    if (!BCRYPT_SUCCESS(status)) {
        throw std::runtime_error("BCryptGenRandom failed with status: " + std::to_string(status));
    }
    return buffer;
}

std::string CryptoUtil::generateToken(size_t numBytes) {
    auto bytes = generateRandomBytes(numBytes);
    return toHex(bytes.data(), bytes.size());
}

std::string CryptoUtil::sha256Hex(const std::string& input) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
    if (!BCRYPT_SUCCESS(status)) {
        throw std::runtime_error("BCryptOpenAlgorithmProvider for SHA256 failed");
    }

    DWORD hashObjSize = 0;
    DWORD dataSize = 0;
    status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjSize, sizeof(DWORD), &dataSize, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw std::runtime_error("BCryptGetProperty BCRYPT_OBJECT_LENGTH failed");
    }

    std::vector<unsigned char> hashObj(hashObjSize);
    BCRYPT_HASH_HANDLE hHash = NULL;

    status = BCryptCreateHash(hAlg, &hHash, hashObj.data(), hashObjSize, NULL, 0, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw std::runtime_error("BCryptCreateHash failed");
    }

    status = BCryptHashData(hHash, (PUCHAR)input.data(), static_cast<ULONG>(input.size()), 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw std::runtime_error("BCryptHashData failed");
    }

    unsigned char hashBytes[32] = {0};
    status = BCryptFinishHash(hHash, hashBytes, sizeof(hashBytes), 0);

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (!BCRYPT_SUCCESS(status)) {
        throw std::runtime_error("BCryptFinishHash failed");
    }

    return toHex(hashBytes, sizeof(hashBytes));
}

std::string CryptoUtil::hashPassword(const std::string& password) {
    constexpr ULONG ITERATIONS = 100000;
    constexpr size_t SALT_LEN = 16;
    constexpr size_t KEY_LEN = 32;

    auto salt = generateRandomBytes(SALT_LEN);

    BCRYPT_ALG_HANDLE hAlg = NULL;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_SHA256_ALGORITHM,
        NULL,
        BCRYPT_ALG_HANDLE_HMAC_FLAG);

    if (!BCRYPT_SUCCESS(status)) {
        throw std::runtime_error("BCryptOpenAlgorithmProvider for HMAC-SHA256 failed");
    }

    unsigned char derivedKey[KEY_LEN] = {0};
    status = BCryptDeriveKeyPBKDF2(
        hAlg,
        (PUCHAR)password.data(),
        static_cast<ULONG>(password.size()),
        salt.data(),
        static_cast<ULONG>(salt.size()),
        ITERATIONS,
        derivedKey,
        sizeof(derivedKey),
        0);

    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (!BCRYPT_SUCCESS(status)) {
        throw std::runtime_error("BCryptDeriveKeyPBKDF2 failed with status: " + std::to_string(status));
    }

    std::string saltHex = toHex(salt.data(), salt.size());
    std::string keyHex = toHex(derivedKey, sizeof(derivedKey));

    std::ostringstream ss;
    ss << "pbkdf2_sha256$" << ITERATIONS << "$" << saltHex << "$" << keyHex;
    return ss.str();
}

bool CryptoUtil::verifyPassword(const std::string& password, const std::string& storedHash) {
    // Format: pbkdf2_sha256$<iterations>$<salt_hex>$<hash_hex>
    if (storedHash.rfind("pbkdf2_sha256$", 0) != 0) {
        return false;
    }

    std::istringstream ss(storedHash);
    std::string token;
    std::vector<std::string> parts;
    while (std::getline(ss, token, '$')) {
        parts.push_back(token);
    }

    if (parts.size() != 4) {
        return false;
    }

    ULONG iterations = 0;
    try {
        iterations = std::stoul(parts[1]);
    } catch (...) {
        return false;
    }

    std::string saltHex = parts[2];
    std::string storedKeyHex = parts[3];

    auto salt = fromHex(saltHex);
    auto storedKey = fromHex(storedKeyHex);

    if (salt.empty() || storedKey.empty() || storedKey.size() != 32) {
        return false;
    }

    BCRYPT_ALG_HANDLE hAlg = NULL;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_SHA256_ALGORITHM,
        NULL,
        BCRYPT_ALG_HANDLE_HMAC_FLAG);

    if (!BCRYPT_SUCCESS(status)) {
        return false;
    }

    unsigned char computedKey[32] = {0};
    status = BCryptDeriveKeyPBKDF2(
        hAlg,
        (PUCHAR)password.data(),
        static_cast<ULONG>(password.size()),
        salt.data(),
        static_cast<ULONG>(salt.size()),
        iterations,
        computedKey,
        sizeof(computedKey),
        0);

    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (!BCRYPT_SUCCESS(status)) {
        return false;
    }

    return constantTimeMemcmp(computedKey, storedKey.data(), sizeof(computedKey));
}

bool CryptoUtil::constantTimeCompare(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    return constantTimeMemcmp(
        reinterpret_cast<const unsigned char*>(a.data()),
        reinterpret_cast<const unsigned char*>(b.data()),
        a.size());
}
