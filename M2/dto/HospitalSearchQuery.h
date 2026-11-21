#ifndef M2_DTO_HOSPITAL_SEARCH_QUERY_H
#define M2_DTO_HOSPITAL_SEARCH_QUERY_H

#include <string>

enum class HospitalSortBy {
    DISTANCE,
    AVAILABLE_BEDS,
    AVAILABLE_ICU,
    RATING
};

struct HospitalSearchQuery {
    std::string textQuery;
    std::string specialty;
    std::string department;
    bool requireIcu;
    HospitalSortBy sortBy;

    HospitalSearchQuery()
        : textQuery(""), specialty(""), department(""),
          requireIcu(false), sortBy(HospitalSortBy::RATING) {}
};

#endif // M2_DTO_HOSPITAL_SEARCH_QUERY_H
