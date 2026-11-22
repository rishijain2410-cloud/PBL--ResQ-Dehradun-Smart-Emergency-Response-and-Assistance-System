#ifndef M3_DTO_API_RESPONSE_H
#define M3_DTO_API_RESPONSE_H

#include <string>
#include <sstream>

template <typename T>
class ApiResponse {
private:
    bool success;
    std::string status;
    std::string message;
    std::string errorCode;
    T data;

public:
    ApiResponse(bool ok, const std::string& stat, const std::string& msg, const std::string& err, const T& d)
        : success(ok), status(stat), message(msg), errorCode(err), data(d) {}

    static ApiResponse<T> ok(const T& d, const std::string& msg = "Success") {
        return ApiResponse<T>(true, "OK", msg, "", d);
    }

    static ApiResponse<T> error(const std::string& errCode, const std::string& msg, const T& fallback = T()) {
        return ApiResponse<T>(false, "ERROR", msg, errCode, fallback);
    }

    bool isSuccess() const { return success; }
    const std::string& getStatus() const { return status; }
    const std::string& getMessage() const { return message; }
    const std::string& getErrorCode() const { return errorCode; }
    const T& getData() const { return data; }

    std::string toJson() const {
        std::ostringstream ss;
        ss << "{"
           << "\"success\":" << (success ? "true" : "false") << ","
           << "\"status\":\"" << status << "\","
           << "\"message\":\"" << message << "\","
           << "\"errorCode\":\"" << errorCode << "\","
           << "\"data\":" << data.toJson()
           << "}";
        return ss.str();
    }
};

// Specialization for raw string data
template <>
inline std::string ApiResponse<std::string>::toJson() const {
    std::ostringstream ss;
    ss << "{"
       << "\"success\":" << (success ? "true" : "false") << ","
       << "\"status\":\"" << status << "\","
       << "\"message\":\"" << message << "\","
       << "\"errorCode\":\"" << errorCode << "\","
       << "\"data\":" << data
       << "}";
    return ss.str();
}

#endif // M3_DTO_API_RESPONSE_H
