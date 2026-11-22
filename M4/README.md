# M4 — Frontend Dashboard and Portals

**Module Lead:** Aryan Pundir (Member 4) • `aryanpundir193@gmail.com`  
**Live Deployment:** [https://jeevan-marg.netlify.app/](https://jeevan-marg.netlify.app/)

M4 provides the web interface and integrated user portals for the JEEVAN MARG project: a control-room dashboard and dedicated patient, ambulance-driver, and hospital portals.

---

## Portals & Capabilities

- **Admin Control Room (`index.html`):** City-wide emergency overview, fleet telemetry, hospital directory & live bed census, Dijkstra route testing, and response analytics.
- **Patient Portal (`patient.html`):** Emergency SOS request dispatch, clinical triage symptoms (Cardiac Arrest, Trauma, Stroke, etc.), interactive Leaflet pin-drop GPS, real-time 5-stage tracking (`[Dispatched] -> [At Scene] -> [In Transit] -> [Hospital Gate] -> [Admitted]`), and private patient mission isolation.
- **Driver Cockpit (`driver.html`):** Live siren dispatch notification, mission Accept/Decline autonomy, 4-stage mission lifecycle progression (`ON_SCENE` ➔ `TRANSPORTING` ➔ `ARRIVED` ➔ `COMPLETED`), Dijkstra route map navigation, and vehicle release to standby.
- **Hospital ER Desk (`hospital.html`):** Real-time inpatient and ICU bed inventory tracking, incoming patient queue, one-click patient admission protocol with dynamic ICU bed deduction (-1) and admitted count increment (+1), and admission history archiving.
- **Shared Authentication & Utilities:** Role-based sign-in (`login.html`), citizen registration (`register.html`), Leaflet maps, audio siren alerts, and REST API adapters for M1–M3 services.

---

## Academic & Presentation Documentation (`M4/docs/`)

- [`M4/docs/FINAL_DEMO_GUIDE.md`](docs/FINAL_DEMO_GUIDE.md): Step-by-step evaluator presentation guide and pre-flight checklist.
- [`M4/docs/MENTOR_PRESENTATION_SCRIPT.md`](docs/MENTOR_PRESENTATION_SCRIPT.md): Complete word-to-word script (Hinglish/English) for presenting to project mentors.
- [`M4/docs/VIVA_CHEATSHEET.md`](docs/VIVA_CHEATSHEET.md): Viva-voce questions and technical defense answers covering DSA, data structures, and architecture.
- [`M4/docs/PHASE4_PROGRESS.md`](docs/PHASE4_PROGRESS.md): Detailed development progress documentation for Phase 4 frontend integration.
- [`M4/docs/JEEVAN_MARG_MENTOR_SCRIPT.pdf`](docs/JEEVAN_MARG_MENTOR_SCRIPT.pdf): Exported presentation and defense guide in PDF format.

---

## Run Locally

Requires Python 3 and an internet connection for CDN-hosted UI libraries, fonts, icons, and map tiles. From the repository root, run:

```powershell
py -m http.server 8000 --directory M4
```

Open [http://localhost:8000/login.html](http://localhost:8000/login.html). Serve the pages over HTTP rather than opening them directly from the file system.

---

## Backend Connection

The frontend expects the integrated JEEVAN MARG backend at `http://localhost:8080` by default. To use another API base URL, define `window.JEEVAN_MARG_API_URL` before the frontend scripts load, or set `localStorage.jeevan_marg_api_url` in the browser.

When running independently or in cloud preview (e.g. on Netlify), the built-in browser-side showcase mock (`cloud-mock.js`) provides realistic simulation data and state management for academic demonstration.

---

## Directory Structure

```text
M4/
├── index.html              # Admin / control-room dashboard
├── login.html              # Role-based sign-in (Admin, Driver, Hospital, Patient)
├── register.html           # Citizen / Patient registration
├── patient.html            # Patient portal & live emergency tracker
├── driver.html             # Ambulance driver cockpit & telemetry
├── hospital.html           # Hospital ER capacity & admission desk
├── netlify.toml            # Netlify deployment headers & security rules
├── css/                    # Shared and portal-specific styles
│   ├── common.css
│   ├── driver.css
│   ├── hospital.css
│   ├── patient.css
│   └── style.css
├── js/                     # API adapters, maps, authentication, and state management
│   ├── analytics.js
│   ├── auth.js
│   ├── cloud-mock.js
│   ├── common.js
│   ├── driver.js
│   ├── file-guard.js
│   ├── hospital.js
│   ├── m1-api.js
│   ├── m2-api.js
│   ├── m3-api.js
│   ├── main.js
│   ├── map.js
│   ├── patient.js
│   └── tracking.js
└── docs/                   # Evaluation guides, mentor presentation scripts & PDF
    ├── FINAL_DEMO_GUIDE.md
    ├── MENTOR_PRESENTATION_SCRIPT.md
    ├── VIVA_CHEATSHEET.md
    ├── PHASE4_PROGRESS.md
    └── JEEVAN_MARG_MENTOR_SCRIPT.pdf
```

---

## Safety Note

This is an academic project demonstration developed for Graphic Era University (GEU) Project-Based Learning (PBL). It is not a real-world emergency dispatch service.
