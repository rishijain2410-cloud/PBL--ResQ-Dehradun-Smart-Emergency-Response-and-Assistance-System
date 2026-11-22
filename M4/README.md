# M4 — Frontend Dashboard and Portals

M4 provides the web interface for the JEEVAN MARG project: a control-room dashboard and dedicated patient, ambulance-driver, and hospital portals.

## Features

- **Admin dashboard:** emergency overview, fleet and hospital information, analytics, and maps.
- **Patient portal:** SOS request flow, emergency status and tracking, and hospital discovery.
- **Driver portal:** assigned mission details, status updates, and location telemetry.
- **Hospital portal:** facility information, incoming cases, bed capacity, and admissions.
- **Shared frontend:** sign-in and registration pages, responsive layouts, Leaflet maps, and API adapters for M1–M3 services.

The browser includes a showcase mock for trying the interface without a backend. Mock data and interactions are for academic demonstration only.

## Run locally

Requires Python 3 and an internet connection for CDN-hosted UI libraries, fonts, icons, and map tiles. From the repository root, run:

```powershell
py -m http.server 8000 --directory M4
```

Open [http://localhost:8000/login.html](http://localhost:8000/login.html). Serve the pages over HTTP rather than opening them directly from the file system.

## Backend connection

The frontend expects the integrated JEEVAN MARG backend at `http://localhost:8080` by default. To use another API base URL, define `window.JEEVAN_MARG_API_URL` before the frontend scripts load, or set `localStorage.jeevan_marg_api_url` in the browser.

Without a compatible backend, the showcase mock may provide sample responses; it does not provide persistent or real emergency-dispatch service.

## Main files

```text
M4/
├── index.html              # Admin / control-room dashboard
├── login.html              # Sign-in
├── register.html           # Patient registration
├── patient.html            # Patient portal
├── driver.html              # Ambulance driver portal
├── hospital.html            # Hospital portal
├── css/                    # Shared and portal-specific styles
└── js/                     # API adapters, maps, authentication, and portal behavior
```

## Safety note

This is an academic project demonstration, not an emergency service. Do not use it to request real medical assistance or enter real patient information.
