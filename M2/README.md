# M2 Healthcare module (C++17 starter)

This is an independent, in-memory starter for the Healthcare vertical slice. It follows the new plan's `model -> service -> repository` boundaries and uses stable string IDs. It deliberately does not own routing or emergency-request logic.

## Implemented foundation
- Hospital, department, doctor, medical-service, and bed-availability models.
- In-memory hospital repository indexed by ID (`itemMapById`).
- Search/filter by hospital name, department, medical service, and availability.
- Deterministic sorting by name, rating, or available beds.

## Build
From this directory, compile the `.cpp` files with your team's eventual controller/API and UI. Example:

```sh
g++ -std=c++17 -Wall -Wextra -pedantic -I. -c repository/HospitalRepository.cpp service/HospitalSearchService.cpp
```

Distance sorting is intentionally left for integration with M3's route/location result. Use simulated Dehradun data only; do not put real patient data in the demo.
