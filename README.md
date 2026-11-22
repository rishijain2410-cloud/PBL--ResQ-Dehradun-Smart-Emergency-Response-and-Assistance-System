# 🚑 JEEVAN MARG &mdash; Smart Emergency Response & Healthcare Navigation System
### 🎓 Project-Based Learning (PBL) &bull; Department of Computer Science & Engineering &bull; Graphic Era (Deemed to be University), Dehradun

[![Live Demo on Netlify](https://img.shields.io/badge/🌐_Live_Demo-Netlify_Cloud-00C7B7?style=for-the-badge&logo=netlify&logoColor=white)](https://jeevan-marg.netlify.app/)
[![Language: C++17](https://img.shields.io/badge/Backend-C%2B%2B17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://github.com/rishijain2410-cloud/PBL--ResQ-Dehradun-Smart-Emergency-Response-and-Assistance-System)
[![Frontend: JavaScript & Leaflet](https://img.shields.io/badge/Frontend-Vanilla_JS_|_Leaflet-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)](https://github.com/rishijain2410-cloud/PBL--ResQ-Dehradun-Smart-Emergency-Response-and-Assistance-System/tree/main/M4)
[![Database: SQLite](https://img.shields.io/badge/Database-SQLite3-003B57?style=for-the-badge&logo=sqlite&logoColor=white)](https://github.com/rishijain2410-cloud/PBL--ResQ-Dehradun-Smart-Emergency-Response-and-Assistance-System/tree/main/M3)
[![Status: Evaluation Ready](https://img.shields.io/badge/Status-Evaluation_Ready_💯-success?style=for-the-badge)](https://github.com/rishijain2410-cloud/PBL--ResQ-Dehradun-Smart-Emergency-Response-and-Assistance-System)

---

## 🚀 Live Cloud Deployment

Experience the complete multi-portal emergency ecosystem live in your browser:
### 🌐 **[https://jeevan-marg.netlify.app/](https://jeevan-marg.netlify.app/)**

| Portal | Direct Access Link | Target Role / Primary Functionality | Pre-configured Credentials |
| :--- | :--- | :--- | :--- |
| 🚨 **Citizen SOS & Tracking** | [`/patient.html`](https://jeevan-marg.netlify.app/patient.html) | Emergency SOS trigger, symptom intake, live 5-stage ambulance tracker | `patient_demo` / `Patient@123` *(or register new)* |
| 🚑 **Ambulance Driver Cockpit** | [`/driver.html`](https://jeevan-marg.netlify.app/driver.html) | Real-time siren dispatch alerts, Accept/Decline, 4-stage mission lifecycle | `driver_amb01` / `Driver@123` |
| 🏥 **Hospital ER Capacity Desk**| [`/hospital.html`](https://jeevan-marg.netlify.app/hospital.html) | Live ICU & inpatient bed inventory, incoming trauma queue, atomic admission | `hosp_max` / `Hospital@123` |
| 🎛️ **Central Command Room** | [`/index.html`](https://jeevan-marg.netlify.app/index.html) | City-wide fleet telemetry, hospital directory, Dijkstra route calculation | `admin` / `Admin@JeevanMarg2026!` |

---

## 📌 Executive Summary & Problem Statement

In critical medical emergencies, the **"Golden Hour"** &mdash; the initial 60-minute window following acute trauma or cardiac arrest &mdash; determines patient survival. In traditional urban emergency workflows:
1. **Prolonged Telephone Intake:** Emergency callers spend 6–10 minutes verbally describing landmarks over 108 helpline calls.
2. **Blind Hospital Transfers:** Ambulances arrive at casualty departments only to discover zero ICU beds or ventilators available, necessitating secondary transfers.
3. **Suboptimal Urban Routing:** Emergency vehicles navigate congestion without dynamic algorithmic shortest-path optimization.

**JEEVAN MARG** solves this through a distributed **4-Tier Smart Emergency Response System** tailored for the urban geography of **Dehradun, Uttarakhand**. It integrates high-performance **C++17 graph data structures**, **Dijkstra's Shortest Path algorithm**, **clinical triage heuristics**, **SQLite persistence**, and a **modern reactive frontend** across four synchronized portals.

---

## 👥 Project Team & Module Distribution

| Module | Module Name | Team Member | University ID | Core Responsibilities & Deliverables |
| :--- | :--- | :--- | :--- | :--- |
| **M1** | **Emergency Dispatch & Triage** | **Rajat Saxena** (Lead) | `2510011033` | Incident reporting, urgency scoring (1-4), clinical triage rule engine, 5-stage emergency lifecycle (`PENDING` &rarr; `ASSIGNED` &rarr; `ON_SCENE` &rarr; `TRANSPORTING` &rarr; `ARRIVED` &rarr; `COMPLETED`). |
| **M2** | **Healthcare Discovery** | **Rishi Jain** | `2510010332` | Hospital registry, live bed & ICU inventory tracking, multi-criteria specialty query, facility admission workflows, and repository persistence. |
| **M3** | **Navigation & Graph Engine** | **Anubhav Garg** | `2510011405` | Dehradun road graph (20 nodes, 31 bidirectional edges), Min-Heap Dijkstra ($O((V+E)\log V)$), Bellman-Ford validation (380 pairs), Haversine nearby search, multi-modal transport speeds, SQLite schema, and embedded REST API server. |
| **M4** | **Frontend Dashboard & Portals**| **Aryan Pundir** | `2510010816` | 4 role-based web portals (Admin, Patient, Driver, Hospital), Leaflet.js interactive maps, real-time GPS telemetry animation, audio siren alerts, client-side cloud mock, Netlify CI/CD, and presentation guides. |

---

## 🏗️ System Architecture & Workflow

```text
 ┌──────────────────────────────────────────────────────────────────────────────────────────┐
 │                                 JEEVAN MARG ECOSYSTEM                                    │
 └──────────────────────────────────────────────────────────────────────────────────────────┘
                                              │
                      ┌───────────────────────┴───────────────────────┐
                      │                                               │
             [ Citizen / Patient ]                                [ Driver / EMT ]
             • Instant GPS SOS                                    • Audio Siren Alert
             • Clinical Symptom Select                            • Mission Accept/Decline
             • 5-Stage Live Tracker                               • 4-Step Lifecycle Update
                      │                                               │
                      ▼                                               ▼
     ┌─────────────────────────────────────────────────────────────────────────────────┐
     │                     M4: FRONTEND DASHBOARD & INTEGRATION                        │
     │   HTML5 / CSS3 • Responsive Portals • Leaflet OpenStreetMap • REST Client / Mock │
     └─────────────────────────────────────────────────────────────────────────────────┘
                                              ▲
                                              │ REST API / JSON
                                              ▼
     ┌─────────────────────────────────────────────────────────────────────────────────┐
     │                      C++17 REST API & DISPATCH ENGINE                           │
     │      Embedded Server • Multi-threaded Request Handler • Session Management      │
     └─────────────────────────────────────────────────────────────────────────────────┘
                 │                                    │                       │
                 ▼                                    ▼                       ▼
 ┌──────────────────────────────┐ ┌───────────────────────┐ ┌─────────────────────────────────┐
 │   M1: EMERGENCY DISPATCH     │ │   M2: HEALTHCARE      │ │   M3: NAVIGATION & GRAPH        │
 │ • Clinical Urgency Scoring   │ │ • Hospital Registry   │ │ • 20 Urban Nodes, 31 Road Edges │
 │ • Triage Matrix (Scores 1-4) │ │ • Bed & ICU Inventory │ │ • Min-Heap Dijkstra Shortest Path│
 │ • Lifecycle State Machine    │ │ • Admission Protocol  │ │ • Spherical Haversine ($R=6371$)│
 └──────────────────────────────┘ └───────────────────────┘ └─────────────────────────────────┘
                 │                                    │                       │
                 └────────────────────────────────────┴───────────────────────┘
                                              │
                                              ▼
                             ┌──────────────────────────────────┐
                             │       SQLITE PERSISTENCE         │
                             │   Schema v2 • Foreign Keys       │
                             │   CHECK Constraints • SHA-256    │
                             └──────────────────────────────────┘
```

---

## 🔬 In-Depth Module Specifications

### 🚨 Module 1: Emergency Dispatch & Clinical Triage (`M1/`)
- **Deterministic Urgency Scoring:** Maps symptoms and condition descriptors to priority tiers:
  - `Score 1 (CRITICAL):` Cardiac arrest, unconsciousness, severe hemorrhage, respiratory cessation &rarr; *Immediate Advanced Life Support (ALS) dispatch & ICU reservation.*
  - `Score 2 (HIGH):` Compound trauma, acute respiratory distress, severe burns &rarr; *ALS / BLS with oxygen delivery.*
  - `Score 3 (MEDIUM):` Moderate trauma, persistent high fever, controlled fractures &rarr; *Basic Life Support (BLS).*
  - `Score 4 (LOW):` Minor injury, non-urgent consultation &rarr; *Standard medical transport.*
- **Lifecycle Integrity:** State transitions are strictly validated. Reversals or illegal status jumps (e.g., `PENDING` directly to `ARRIVED`) are rejected.

### 🏥 Module 2: Healthcare Discovery & Bed Tracking (`M2/`)
- **Facility Directory:** Comprehensive models for Dehradun healthcare centers (Max Super Speciality, Doon Hospital, Synergy Hospital, Graphic Era Hospital, AIIMS Rishikesh, etc.).
- **Dynamic Bed & ICU Inventory:** Real-time inventory tracking for Total Inpatient Beds, Total ICU Beds, Available ICU Beds, and Oxygen Beds.
- **Atomic Admission Protocol:** On patient intake confirmation, available ICU beds decrement (-1), admitted count increments (+1), and ambulance is released back to `READY` status.

### 🗺️ Module 3: Navigation, Dijkstra & SQLite Persistence (`M3/`)
- **Dehradun Road Graph:** Adjacency list representation of 20 key intersections (Clock Tower, ISBT, Rajpur Road, Ballupur, Prem Nagar, Rispana, etc.) and 31 bidirectional weighted segments.
- **Min-Heap Dijkstra Algorithm:** Single-source shortest path calculation executing in $O((V + E) \log V)$ time complexity.
- **Algorithm Cross-Validation:** Every ordered pair of locations ($20 \times 19 = 380$ pairs) mathematically verified against a reference Bellman-Ford implementation with **0 discrepancies**.
- **Haversine Proximity Search:** Spherical great-circle formula calculating nearest facilities based on latitude and longitude ($R = 6,371\text{ km}$).
- **SQLite Database Persistence:** Full relational persistence (`data/jeevanmarg.db`) with tables for users, hospitals, emergencies, ambulances, locations, and audit logs.

### 💻 Module 4: Frontend Dashboard & Portals (`M4/`)
- **4 Autonomous User Portals:** Tailored responsive views for Patients, Drivers, Hospitals, and System Administrators.
- **Leaflet OpenStreetMap Integration:** Visualizes patient coordinates, hospital destinations, animated ambulance GPS markers, and computed shortest routes.
- **Audio Siren Telemetry:** Web Audio API sound alerts notifying drivers instantly of incoming emergency dispatches.
- **Strict Patient Privacy Isolation:** Guarantees that citizen users only view their own live emergency status and tracking data without data leaks.
- **Academic Presentation Materials:** Located in [`M4/docs/`](M4/docs/) including demo guides, viva cheatsheets, and defense PDF scripts.

---

## 🧮 Data Structures & Algorithmic Complexity

| Algorithm / Data Structure | Implementation Location | Time Complexity | Engineering Purpose |
| :--- | :--- | :--- | :--- |
| **Min-Heap Priority Queue** | `m3_navigation/service/ShortestPathFinder.cpp` | $O(\log V)$ insertion / extract-min | Optimal Dijkstra frontier exploration |
| **Dijkstra's Shortest Path**| `m3_navigation/service/ShortestPathFinder.cpp` | $O((V + E) \log V)$ | Computes fastest road path through Dehradun graph |
| **Haversine Spherical Distance** | `m3_navigation/util/GeoUtil.cpp` | $O(1)$ | Real-time nearest emergency resource discovery |
| **Adjacency List Graph** | `m3_navigation/structure/RoadGraph.cpp` | $O(V + E)$ space | Memory-efficient sparse urban road representation |
| **Clinical Triage Rule Matrix** | `m1_dispatch/service/TriageRuleEngine.cpp` | $O(1)$ deterministic lookup | High-speed clinical categorization without latency |
| **PBKDF2-HMAC-SHA256** | `m3_navigation/util/CryptoUtil.cpp` | Key stretching (10,000 iterations) | Cryptographic password hashing and bearer token generation |

---

## 📁 Repository Directory Structure

```text
.
├── netlify.toml                        # Netlify continuous deployment configuration
├── README.md                           # Main project documentation & technical guide
├── M1/                                 # Module 1: Emergency Dispatch & Triage Engine
│   └── model/                          # Emergency contact, manager, request, and service models
├── M2/                                 # Module 2: Healthcare Discovery & Hospital Directory
│   ├── data/                           # Seed hospital & facility data
│   ├── dto/                            # Data transfer objects
│   ├── model/                          # Hospital, department, and doctor models
│   ├── repository/                     # Facility persistence interfaces
│   ├── service/                        # Search, filtering, and bed capacity logic
│   └── test/                           # M2 component tests
├── M3/                                 # Module 3: Navigation, Dijkstra & C++ Server
│   ├── data/                           # Seed CSV coordinates & road segment definitions
│   ├── datasource/                     # CSV data ingestion parsers
│   ├── model/                          # Location, road, and transport models
│   ├── persistence/                    # SQLite repositories (Hospitals, Users, Emergencies)
│   ├── service/                        # Dijkstra pathfinder, Haversine search, REST server
│   ├── structure/                      # Adjacency list graph & min-heap priority queue
│   ├── test/                           # Dijkstra vs Bellman-Ford validation (380 pairs)
│   └── util/                           # GeoUtil, CryptoUtil, and JSON escaping helpers
└── M4/                                 # Module 4: Frontend Dashboard & Portals
    ├── index.html                      # Central Command Room Console
    ├── login.html                      # Multi-role authentication entrypoint
    ├── register.html                   # Citizen registration portal
    ├── patient.html                    # Citizen SOS & 5-stage live tracker
    ├── driver.html                     # Driver cockpit & siren telemetry
    ├── hospital.html                   # Hospital ER capacity & admission desk
    ├── netlify.toml                    # Frontend security headers
    ├── README.md                       # Comprehensive M4 guide
    ├── css/                            # Common, portal, and custom styling
    ├── js/                             # APIs, Leaflet map engine, cloud-mock, and state
    └── docs/                           # Evaluation & presentation documentation
        ├── FINAL_DEMO_GUIDE.md         # Evaluator demonstration walkthrough
        ├── MENTOR_PRESENTATION_SCRIPT.md # Complete mentor presentation script
        ├── VIVA_CHEATSHEET.md          # Technical viva questions & answers
        ├── PHASE4_PROGRESS.md          # Frontend development progress logs
        └── JEEVAN_MARG_MENTOR_SCRIPT.pdf # Exported presentation PDF guide
```

---

## ⚙️ How to Run Locally

### Option 1: Live Cloud Preview (Recommended & Instant)
No local compilation or dependencies required:
👉 Visit **[https://jeevan-marg.netlify.app/](https://jeevan-marg.netlify.app/)**

### Option 2: Run Frontend via Local HTTP Server
Requires Python 3. From the repository root:
```powershell
python -m http.server 8000 --directory M4
```
Then open [http://localhost:8000/login.html](http://localhost:8000/login.html) in your browser.

### Option 3: Full C++ Backend Build (Windows with MSYS2 / GCC)
Prerequisites: MSYS2 UCRT64 toolchain with `g++` and `make`.
```cmd
cd M3
make
./m3_nav.exe
```
The REST API server will bind to `http://localhost:8080`, initialize `data/jeevanmarg.db`, and serve real-time endpoints.

---

## 🔒 Security & Access Control (RBAC Matrix)

| Endpoint | Method | Permitted Roles | Description |
| :--- | :--- | :--- | :--- |
| `/api/emergency-requests` | POST | **Public / Patient** | Anonymous or authenticated emergency SOS dispatch |
| `/api/emergencies/:id/tracking` | GET | **Scoped Token** | Secure tracking restricted to emergency owner or SHA-256 token bearer |
| `/api/driver/ambulance/status` | PATCH | **DRIVER** | Ambulance crew advances mission status (`ON_SCENE`, `TRANSPORTING`, `ARRIVED`, `COMPLETED`) |
| `/api/hospital/beds` | PATCH | **HOSPITAL** | Strictly scoped: hospital users may only update bed inventory for their assigned facility |
| `/api/hospitals/:id/admit` | POST | **HOSPITAL** | Confirms emergency patient admission, allocates ICU bed, releases ambulance |
| `/api/admin/fleet` | GET | **ADMIN** | City-wide ambulance fleet GPS coordinates and availability status |

---

## 📜 Academic Attribution & License

- **Course:** Project-Based Learning (PBL) &bull; B.Tech CSE
- **Institution:** Graphic Era (Deemed to be University), Dehradun, Uttarakhand, India
- **Year of Submission:** 2026
- **License:** Educational & Academic Demonstration License. Developed strictly for academic evaluation and research purposes.
