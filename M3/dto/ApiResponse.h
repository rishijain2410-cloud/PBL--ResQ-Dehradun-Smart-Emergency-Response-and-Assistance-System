#pragma once

#include <string>

enum class ErrorCode {
    NONE,
    VALIDATION_ERROR,
    NOT_FOUND,
    NO_RESULTS,
    INVALID_ID,
    SERVICE_UNAVAILABLE,
    INTERNAL_ERROR
};

inline std::string errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::NONE:                return "";
        case ErrorCode::VALIDATION_ERROR:    return "VALIDATION_ERROR";
        case ErrorCode::NOT_FOUND:           return "NOT_FOUND";
        case ErrorCode::NO_RESULTS:          return "NO_RESULTS";
        case ErrorCode::INVALID_ID:          return "INVALID_ID";
        case ErrorCode::SERVICE_UNAVAILABLE: return "SERVICE_UNAVAILABLE";
        case ErrorCode::INTERNAL_ERROR:      return "INTERNAL_ERROR";
        default:                             return "UNKNOWN_ERROR";
    }
}

template <typename T>
struct ApiResponse {
    bool success;
    std::string message;
    T data;
    std::string errorCode;

    ApiResponse()
        : success(false), message(""), data(T{}), errorCode("") {}

    ApiResponse(bool success, const std::string& message, const T& data, const std::string& errorCode = "")
        : success(success), message(message), data(data), errorCode(errorCode) {}

    static ApiResponse<T> ok(const T& data, const std::string& message = "Operation completed successfully") {
        return ApiResponse<T>(true, message, data, "");
    }

    static ApiResponse<T> fail(const std::string& message, ErrorCode code = ErrorCode::INTERNAL_ERROR) {
        return ApiResponse<T>(false, message, T{}, errorCodeToString(code));
    }

    static ApiResponse<T> fail(const std::string& message, const std::string& customErrorCode) {
        return ApiResponse<T>(false, message, T{}, customErrorCode);
    }
};
