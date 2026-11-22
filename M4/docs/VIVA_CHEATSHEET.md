# JEEVAN MARG &mdash; VIVA VOCE &amp; TECHNICAL CHEATSHEET
### Quick-Reference Technical Guide for Project Examiners &amp; Defense  
*Department of Computer Science &amp; Engineering &bull; Graphic Era University, Dehradun*

---

## 1. MODULE-BY-MODULE ARCHITECTURE SUMMARY

### Module 1: Emergency Dispatch &amp; Clinical Triage (Rajat Saxena &bull; 2510011033)
- **Core Files:** `m1_dispatch/model/EmergencyRequest.*`, `m1_dispatch/service/TriageRuleEngine.*`, `m3_navigation/service/EmergencyDispatcher.*`
- **Clinical Urgency Score &amp; Severity Hierarchy:**
  - **Score 1 &rarr; `CRITICAL`:** Keywords: *severe chest pain, unconscious, not breathing, no pulse, cardiac arrest, severe bleeding, stroke, seizure, choking*. Action: Dispatches ALS unit, automatically reserves 1 ICU bed at target hospital.
  - **Score 2 &rarr; `HIGH`:** Keywords: *trauma, accident, fracture, asthma, breathlessness*. Action: ALS / BLS with oxygen, specialty hospital routing.
  - **Score 3 &rarr; `MEDIUM`:** Keywords: *fever, headache, general injury*. Action: BLS ambulance.
  - **Score 4 &rarr; `LOW`:** Keywords: *cold, cough*. Action: Outpatient care / BLS.
- **Triage Source of Truth Rule:** The clinical engine outcome cannot be overridden by callers. Caller-supplied severity may only **escalate** (e.g. caller says "CRITICAL" for mild symptom), never downgrade.
- **Mission Lifecycle:** `PENDING` &rarr; `ASSIGNED` &rarr; `ON_SCENE` &rarr; `TRANSPORTING` &rarr; `ARRIVED` &rarr; `COMPLETED` (or `CANCELLED`).
- **Resource Integrity:** Transitioning a `CRITICAL` mission to `CANCELLED` atomically increments `available_icu` beds back to original capacity in SQLite.

---

### Module 2: Healthcare Discovery &amp; Bed Capacity (Rishi Jain &bull; 2510010332)
- **Core Files:** `m2_healthcare/model/Hospital.*`, `m2_healthcare/service/HealthcareService.*`, `m3_navigation/persistence/SqliteHospitalRepository.*`
- **Tracked Facilities in Dehradun:** 6 major hospitals (Max Super Speciality `LOC-004`, Shri Mahant Indiresh `LOC-005`, Synergy Hospital `LOC-006`, Graphic Era Hospital `LOC-007`, CMI Hospital `LOC-009`, Doon Hospital `LOC-011`).
- **Inventory Metrics:** Total Bed Capacity, ICU Beds Total, Available ICU Beds, Oxygen-Supported Beds.
- **Specialty Mapping:** Cardiology, Neurology, Orthopedics &amp; Trauma, Pulmonology, Pediatrics, Emergency Medicine.
- **Facility Access Isolation:** Hospital accounts (`hosp_max`, `hosp_doon`, etc.) are cryptographically bound to their `hospital_id`. `PATCH /api/hospitals/:id/beds` enforces that hospital A cannot modify hospital B's bed numbers (HTTP 403 Forbidden).

---

### Module 3: Navigation, Routing Engine &amp; Persistence (Anubhav Garg &bull; 2510011405)
- **Core Files:** `structure/RoadGraph.*`, `service/ShortestPathFinder.*`, `service/NearbyServicesFinder.*`, `service/TransportEstimator.*`, `persistence/Database.*`, `service/RestApiServer.*`
- **Graph Topology:** 20 landmark vertices, 31 bidirectional corridors, 62 directed edges. Connected urban road network covering Clock Tower, ISBT, Graphic Era University, Rajpur Road, Rispana, etc.
- **Algorithm 1 (Dijkstra's Shortest Path):**
  - **Implementation:** Priority Queue Min-Heap (`std::priority_queue<pair<double, string>, vector<...>, greater<...>>`).
  - **Time Complexity:** $O((V + E) \log V)$ where $|V|=20$, $|E|=62$.
  - **Space Complexity:** $O(V + E)$ adjacency list.
- **Algorithm 2 (Bellman-Ford Cross-Validation):**
  - **Purpose:** Dynamic programming algorithm with $O(V \cdot E)$ complexity used to mathematically cross-verify Dijkstra across all $20 \times 19 = 380$ location pairs (100% agreement, 0 error).
- **Algorithm 3 (Haversine Formula):**
  - **Purpose:** Spherical great-circle distance between two GPS coordinates $(\phi_1, \lambda_1)$ and $(\phi_2, \lambda_2)$:
    $$d = 2R \arcsin \left(\sqrt{\sin^2\left(\frac{\Delta\phi}{2}\right) + \cos\phi_1\cos\phi_2\sin^2\left(\frac{\Delta\lambda}{2}\right)}\right)$$
    where Earth radius $R = 6371.0\text{ km}$.
- **Multi-Modal Transport Estimation:** Polymorphic Strategy pattern:
  - Ambulance (75 km/h Highway, 45 km/h City, 30 km/h Hill Road)
  - Two-Wheeler (50 km/h Highway, 35 km/h City, 25 km/h Hill Road)
  - Public Transit (40 km/h Highway, 20 km/h City, 15 km/h Hill Road)
  - Walking (5 km/h flat, unsuitable above 5 km)
- **Database &amp; Storage Architecture:** SQLite 3 with WAL mode, foreign keys (`PRAGMA foreign_keys = ON`), schema version 2 migration, ACID transactions.
- **Authentication &amp; Security:** PBKDF2-HMAC-SHA256 (100,000 iterations), 32-hex bearer tokens, SHA-256 hashed anonymous tracking tokens, CORS whitelist (`localhost:8080`, `127.0.0.1:8080`, `localhost:8089`, `127.0.0.1:8089`), sliding-window SOS rate limiter (5/10 min anon, 20/10 min user).

---

### Module 4: Responsive Frontend &amp; Portals (Aryan Pundir &bull; 2510010816)
- **Core Files:** `m4_frontend/index.html`, `login.html`, `patient.html`, `driver.html`, `hospital.html`, `js/file-guard.js`, `js/map.js`, `js/tracking.js`, `js/main.js`
- **4 Role-Based Portals:**
  - **Administrator (`index.html`):** Global fleet map, metric cards, user provisioning, one-click demo data reset.
  - **Citizen / Patient (`patient.html`):** 1-Click SOS dispatch, anonymous tracking token persistence in `sessionStorage`, live Leaflet map tracking approaching ambulance.
  - **Driver Cockpit (`driver.html`):** Mission queue, live GPS broadcasting, lifecycle buttons (`ON_SCENE`, `TRANSPORTING`, `ARRIVED`).
  - **Hospital Facility (`hospital.html`):** Incoming patient admission queue, real-time ICU bed capacity gauge, bed inventory editor.
- **Open-From-File Guard:** `file-guard.js` detects `location.protocol === 'file:'` and blocks execution, prompting users to launch `run_server.bat` and open `http://localhost:8080/`.

---

## 2. HIGH-FREQUENCY EXAMINER QUESTIONS &amp; MODEL ANSWERS

### Q1: Why use an Adjacency List instead of an Adjacency Matrix?
> **Model Answer:** Urban road networks are sparse graphs where average vertex degree is only 3 to 4. For $V=20$, an adjacency matrix requires $V^2 = 400$ entries, and for a real city with $V=10,000$, it would require $10^8$ entries ($100\text{ MB}$), mostly storing zeroes. An adjacency list uses only $O(V + E)$ memory and enables $O(\text{deg}(u))$ edge iteration in Dijkstra's inner loop, avoiding scanning non-adjacent nodes.

### Q2: Why use a Min-Heap priority queue in Dijkstra?
> **Model Answer:** A naive array-based Dijkstra searches through all vertices to find the minimum distance node, giving $O(V^2)$ time complexity. By maintaining distances in a binary Min-Heap (`std::priority_queue`), extracting the minimum distance node takes $O(\log V)$ and edge relaxation takes $O(E \log V)$, bringing the total time complexity down to $O((V + E) \log V)$.

### Q3: Why is Euclidean distance inaccurate for GPS coordinates?
> **Model Answer:** Euclidean distance $\sqrt{(\Delta x)^2 + (\Delta y)^2}$ assumes a flat Cartesian coordinate system. Earth is an oblate spheroid where lines of longitude converge at the poles (1 degree of longitude is $\approx 111\text{ km}$ at the equator but shrinks by $\cos(\text{latitude})$ elsewhere). The **Haversine formula** applies spherical trigonometry on great-circle arcs to compute accurate geographic surface distances in kilometers.

### Q4: How does JEEVAN MARG secure anonymous emergency tracking?
> **Model Answer:** To prevent attackers from enumerating sequential emergency IDs and reading patients' private locations and medical symptoms, anonymous requests generate a 32-hex random tracking token. The server stores only its **SHA-256 hash** in SQLite. The plain token is returned once to the caller's browser session. Future tracking calls must transmit the token via header `X-Tracking-Token` or `?token=`. Unauthorized requests receive `401 Unauthorized`; mismatched tokens receive `403 Forbidden`.

### Q5: What happens if two critical patients request the last available ICU bed simultaneously?
> **Model Answer:** SQLite operations are serialized through atomic transactions (`Database::Transaction` with `BEGIN IMMEDIATE` and mutex locks). When the first dispatch arrives, it reserves the final ICU bed and updates `icu_available = 0`. The second dispatch immediately reads `icu_available == 0`, identifies that no ICU beds remain at that facility, and gracefully diverts the patient to the next nearest hospital with available ICU capacity or returns a capacity alert.

### Q6: How does the system implement C++ OOP Concepts and Data Structures?
> **Model Answer:** 
> 1. **Data Structures in C++:**
>    - **Graph via Adjacency List:** `structure/RoadGraph` stores vertices in `std::unordered_map<std::string, std::vector<RoadEdge>>`, allowing $O(1)$ vertex lookup and $O(\text{deg}(u))$ edge iteration.
>    - **Min-Heap Binary Priority Queue:** `service/ShortestPathFinder` utilizes `std::priority_queue<std::pair<double, std::string>, std::vector<...>, std::greater<...>>` for Dijkstra's shortest path routing in $O((V+E)\log V)$ time.
>    - **Hash Tables / Hash Maps:** `std::unordered_map` for location dictionaries, session caching, and rate limiting.
>    - **Priority Queue & State Machine:** `m1_dispatch` implements clinical urgency scores (Score 1 `CRITICAL` down to Score 4 `LOW`) to prioritize life-threatening incidents.
> 2. **C++ Object-Oriented Programming (OOP):**
>    - **Encapsulation:** All core models (`Hospital`, `LocationNode`, `RoadEdge`, `BedAvailability`, `EmergencyRequest`) encapsulate private state with validated getters and invariant checks.
>    - **Abstraction:** Database persistence is decoupled behind C++ repository classes (`SqliteHospitalRepository`, `SqliteAmbulanceRepository`, `SqliteEmergencyRepository`), isolating raw SQL operations from application services.
>    - **Polymorphism & Strategy Pattern:** `TransportEstimator` calculates travel times dynamically for multiple transport modalities (Ambulance, Two-Wheeler, Public Transit, Walking).
>    - **RAII & Thread Safety:** `std::lock_guard<std::mutex>` guarantees exception-safe locking across multi-threaded HTTP server requests.
> 3. **C++ Backend as the Single Source of Truth:**
>    - The backend runs entirely in compiled native C++ (`m3_nav.exe`), exposing REST endpoints. The frontend (JavaScript/Leaflet) is purely a thin visualization client that fetches JSON data calculated by the C++ data structures and routing algorithms.

---

## 3. KEY METRICS AT A GLANCE
- **Programming Language:** C++17 (MSYS2 UCRT64 GCC 14+), Modern JavaScript (ES6+), SQLite 3.
- **Dehradun Topology:** 20 Locations, 31 Bidirectional Roads, 62 Directed Graph Edges.
- **Shortest Path Pairs Verified:** 380 location pairs (Dijkstra vs Bellman-Ford: 100% agreement).
- **Automated Test Assertions:** 451 C++ assertions (7 suites) + 13 automated smoke tests = **464 Total Assertions (100% Green)**.
- **Password Cryptography:** PBKDF2-HMAC-SHA256 with 100,000 iterations and 16-byte cryptographic salt.
- **Server Ports:** Default port `8080`, Isolated test/smoke port `8089`.
