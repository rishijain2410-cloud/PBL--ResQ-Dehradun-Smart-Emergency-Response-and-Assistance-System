# JEEVAN MARG — MASTER SHOWCASE & PRESENTATION GUIDE
**B.Tech Computer Science & Engineering • Graphic Era University, Dehradun**  
*Live Netlify URL:* **[https://jeevan-marg.netlify.app](https://jeevan-marg.netlify.app)**  
*Direct Login:* **[https://jeevan-marg.netlify.app/login.html](https://jeevan-marg.netlify.app/login.html)**

---

## 1. QUICK-START LAUNCHERS (LOCAL C++ SERVER)

Agar evaluator ya mentor laptop par C++ backend chalte hue dekhna chahe:
1. **Full System Start:** Double click `run_server.bat`  
   *(Auto-builds with g++, boots SQLite persistence on port 8080, and opens browser automatically)*
2. **Automated Smoke Test Suite (13 Scenarios):** Double click `smoke_test.bat`  
   *(Runs all 13 test scenarios on port 8089 with 100% GREEN PASS)*
3. **Graph & Dijkstra Terminal Demo:** Double click `run_demo.bat`
4. **All Unit Tests:** Double click `run_tests.bat`

---

## 2. PRE-SEEDED DEMONSTRATION ACCOUNTS

| Role | Username | Password | Linked Resource / Purpose |
| :--- | :--- | :--- | :--- |
| **👑 System Administrator** | `admin` | `Admin@JeevanMarg2026!` *(ya `admin123`)* | Global city fleet map, user management, one-click demo data reset |
| **🚑 Ambulance Driver** | `driver_amb01` | `Driver@123` | Linked to ALS Ambulance `AMB-01` (Clock Tower Station) |
| **🚑 Ambulance Driver (Alt)**| `driver_amb02` | `Driver@123` | Linked to BLS Ambulance `AMB-02` (ISBT Base Depot) |
| **🏥 Hospital Facility** | `hosp_max` | `Hospital@123` | Linked to Max Super Speciality Hospital (`LOC-004`) |
| **🏥 Govt Hospital** | `hosp_doon` | `Hospital@123` | Linked to Doon Govt Hospital (`LOC-011`) |
| **🏥 Super Speciality** | `hosp_synergy` | `Hospital@123` | Linked to Synergy Super Speciality (`LOC-006`) |
| **👤 Citizen / Patient** | `patient_demo` | `Patient@123` | Citizen account (ya direct anonymous SOS bhejo bina login ke) |

---

## 3. PROJECT ARCHITECTURE & 4 MODULES BREAKDOWN

| Module | Directory | Core Responsibilities | C++ Data Structures & OOP Concepts |
| :--- | :--- | :--- | :--- |
| **M1** | `m2_healthcare` | Hospital directory, general & ICU bed capacity tracking, facility admissions | `std::unordered_map` ($O(1)$ fast lookup), Encapsulation & Mutex concurrency control |
| **M2** | `m1_dispatch` | Emergency incident intake, Clinical Triage Rule Engine, priority scheduling | `std::priority_queue` (Min-Heap urgency scheduling), Finite State Machine (FSM) |
| **M3** | `m3_navigation` | Urban road graph (20 nodes, 31 edges), Dijkstra routing, Haversine search | Sparse Adjacency List (`vector<vector<Edge>>`), Min-Heap Dijkstra ($O((V+E)\log V)$), SQLite RAII |
| **M4** | `m4_frontend` | 4 Role-based portals, Leaflet animated maps, live siren markers, audio synthesizer | Thin-client model, Web Audio API chime synthesis, CSS dashFlow shader keyframes |

---

## 4. 5-STEP LIVE PRESENTATION SCRIPT (MENTOR KE SAAMNE KAISE PRESENT KAREIN)

### 🎙️ Opening Pitch:
> *"Good morning Sir! Hamare project ka naam hai JEEVAN MARG — Smart Emergency Response & Healthcare Navigation System. Dehradun ke context mein emergency delay aur ICU bed availability ko solve karne ke liye humne C++ aur Advanced Data Structures (Min-Heap Dijkstra, Adjacency List graph, Hash Maps, aur FSM) ka use karke ek synchronized dispatch engine banaya hai."*

---

### Step 1: Patient SOS Bhejna (Citizen Perspective)
* **URL:** `https://jeevan-marg.netlify.app/patient.html`
* **Action:**
  1. Niche Dehradun hospitals (Max, Doon, Synergy) ki live ICU aur Oxygen beds list dikhao.
  2. Quick Symptom Pill mein **"Severe Chest Pain"** click karo.
  3. Bada red button **`DISPATCH EMERGENCY SOS`** dabao.
* **Mentor ko batao:**
  > *"Sir, backend C++ ke Triage Engine ne symptoms evaluate karke isse CRITICAL (Urgency 1) mark kiya, closest Max Hospital allocate kiya aur Clock Tower se nearest ALS Ambulance AMB-01 dispatch kar di. Map par dekhiye glowing siren beacon aur flowing route live render ho gaya!"*

---

### Step 2: Driver Cockpit & Route Simulation (Driver Perspective)
* **URL:** `https://jeevan-marg.netlify.app/login.html` (Login: `driver_amb01` / `Driver@123`)
* **Action:**
  1. Cockpit mein active mission Aman Negi ka card dikhega.
  2. Button dabao: **`Simulate Route Drive`** &rarr; Ambulance Leaflet map par patient ki taraf smoothly move karegi.
  3. State Machine aage badhao:
     - Click **`1. Confirm Arrival At Scene (ON_SCENE)`**
     - Click **`2. Patient Boarded • Begin Hospital Transport (TRANSPORTING)`**
     - Click **`3. Arrived At Hospital Emergency Wing (ARRIVED)`**

---

### Step 3: Hospital Live Radar & Admission (Hospital Perspective)
* **URL:** Bottom-right floating switcher se **"Hospital"** dabao (ya login: `hosp_max` / `Hospital@123`)
* **Action:**
  1. Right-side column mein **Live Hospital Radar Map** dikhao jahan incoming ambulance AMB-01 live blink kar rahi hai.
  2. Left-side queue mein patient card par:
     - Click **`Accept & Prep`** (Trauma team alerted).
     - Click **`Admit Patient`** (Patient admitted into ICU).
* **Mentor ko batao:**
  > *"Sir, admit karte hi hospital ka ICU bed deduct ho gaya, emergency COMPLETED ho gayi, aur ambulance wapas agle emergency ke liye READY status mein release ho gayi!"*

---

### Step 4: Navigation & Dijkstra Algorithm Demonstration (M3 Engine)
* **URL:** Admin page par **"NAVIGATION & LIVE TRACKING (M3)"** tab par jao.
* **Action:**
  1. Origin: **Clock Tower (LOC-001)** $\rightarrow$ Destination: **Doon Hospital (LOC-011)** &rarr; Click **`Calculate Shortest Path (Dijkstra)`** &rarr; **0.45 km** via Court Road!
  2. Origin: **Clock Tower (LOC-001)** $\rightarrow$ Destination: **Synergy Hospital (LOC-006)** &rarr; **3.20 km** via Chakrata Road & Canal Road!
  3. Origin: **Clock Tower (LOC-001)** $\rightarrow$ Destination: **Max Hospital (LOC-004)** &rarr; **7.30 km** via Rajpur Road & Mussoorie Diversion!
* **Mentor ko batao:**
  > *"Sir, 20 landmarks aur 31 connected roads par Min-Heap Dijkstra algorithm real-time shortest path, turn-by-turn steps, aur multi-modal ETAs (Ambulance, Bike, Bus, Walk) calculate karta hai with $O((V+E)\log V)$ efficiency."*

---

### Step 5: Admin Console, Fleet Management & Account Provisioning
* **URL:** Admin page par **"ADMIN CONSOLE"** tab par jao.
* **Action:**
  1. **Emergency Fleet Table:** Chaaron ambulances (`AMB-01` to `AMB-04`), live coordinates, aur maintenance toggle dikhao.
  2. **Live Provisioning:** "Create Driver Account" form mein naya driver banao &rarr; Turant green success toast aayega aur User Management Directory table mein live add ho kar dikhega!
  3. **Dispatches Audit Log:** City-wide emergency archive dikhao.
  4. **Reset Demo Data:** Ek click mein fresh start ke liye baseline reset kar sakte ho.

---

## 5. TOP VIVA DEFENSE QUESTIONS & ANSWERS

* **Q: Backend mein C++ kyun use kiya?**
  > *"Emergency dispatch ek life-critical system hai jisme garbage collection pause aur high latency unacceptable hai. C++ direct memory control aur deterministic sub-millisecond execution deta hai."*

* **Q: OOPs principles kahan use hue?**
  > *"Encapsulation (thread-safe private members), Abstraction (service aur persistence layer separation), Polymorphism/Strategy Pattern (multi-modal travel estimators), aur RAII (leak-free database connections)."*

* **Q: Kaunse Data Structures implement kiye?**
  > *"Sparse Adjacency List (Road graph), Min-Heap Priority Queue (Dijkstra shortest path relaxation & emergency triage scheduling), Unordered Hash Maps ($O(1)$ facility query), aur Finite State Machine (emergency lifecycle)."*
