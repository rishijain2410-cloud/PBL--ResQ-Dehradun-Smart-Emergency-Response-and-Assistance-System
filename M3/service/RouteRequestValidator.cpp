#include "service/RouteRequestValidator.h"

#include <algorithm>
#include <cctype>

using namespace std;

string RouteRequestValidator::trim(const string& str) {
    string s = str;
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
    return s;
}

bool RouteRequestValidator::isValidLocationId(const string& id) {
    if (id.length() < 5) {
        return false;
    }
    if (id.rfind("LOC-", 0) != 0) {
        return false;
    }
    for (size_t i = 4; i < id.length(); ++i) {
        if (!isdigit(static_cast<unsigned char>(id[i]))) {
            return false;
        }
    }
    return true;
}

RouteRequestValidator::RouteRequestValidator(const ILocationRepository& repo)
    : repo(repo) {}

ApiResponse<RouteRequest> RouteRequestValidator::validate(
    const string& sourceId,
    const string& destinationId,
    const string& modeText) const {

    string src = trim(sourceId);
    string dest = trim(destinationId);
    string mText = trim(modeText);

    if (src.empty()) {
        return ApiResponse<RouteRequest>::fail(
            "Validation Error: Source location ID cannot be empty.",
            ErrorCode::INVALID_ID
        );
    }
    if (dest.empty()) {
        return ApiResponse<RouteRequest>::fail(
            "Validation Error: Destination location ID cannot be empty.",
            ErrorCode::INVALID_ID
        );
    }

    if (!isValidLocationId(src)) {
        return ApiResponse<RouteRequest>::fail(
            "Validation Error: Source location ID '" + src + "' does not match required pattern 'LOC-<digits>'.",
            ErrorCode::INVALID_ID
        );
    }
    if (!isValidLocationId(dest)) {
        return ApiResponse<RouteRequest>::fail(
            "Validation Error: Destination location ID '" + dest + "' does not match required pattern 'LOC-<digits>'.",
            ErrorCode::INVALID_ID
        );
    }

    if (!repo.findById(src).has_value()) {
        return ApiResponse<RouteRequest>::fail(
            "Source location '" + src + "' was not found in repository.",
            ErrorCode::NOT_FOUND
        );
    }
    if (!repo.findById(dest).has_value()) {
        return ApiResponse<RouteRequest>::fail(
            "Destination location '" + dest + "' was not found in repository.",
            ErrorCode::NOT_FOUND
        );
    }

    if (src == dest) {
        return ApiResponse<RouteRequest>::fail(
            "Validation Error: source and destination must be different.",
            ErrorCode::VALIDATION_ERROR
        );
    }

    auto modeOpt = parseTransportMode(mText);
    if (!modeOpt.has_value()) {
        return ApiResponse<RouteRequest>::fail(
            "Validation Error: Unknown or empty transport mode '" + modeText + "'. Supported modes: AMBULANCE, CAR.",
            ErrorCode::VALIDATION_ERROR
        );
    }

    RouteRequest request(src, dest, modeOpt.value());
    return ApiResponse<RouteRequest>::ok(
        request,
        "Route request validated successfully."
    );
}
