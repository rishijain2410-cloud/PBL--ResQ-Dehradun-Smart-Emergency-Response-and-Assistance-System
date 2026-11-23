/**
 * JEEVAN MARG - Showcase Cloud Mock & Universal Resilience Engine
 * Provides realistic Dehradun hospital inventory, Dijkstra routes, emergency lifecycle simulation,
 * and fail-safe API mocking when running on cloud static hosting (Netlify) or when local C++ server is offline.
 */
(function() {
    'use strict';

    const isCloud = window.location.hostname.includes('netlify.app') || 
                    (window.location.protocol.startsWith('http') && 
                     window.location.hostname !== 'localhost' && 
                     window.location.hostname !== '127.0.0.1');

    console.info(`[JEEVAN MARG] Cloud Mock Engine active (Environment: ${isCloud ? 'Netlify Showcase Cloud' : 'Local Host'})`);

    // -------------------------------------------------------------------------
    // Seed Data: Dehradun Hospitals Directory
    // -------------------------------------------------------------------------
    const SEED_HOSPITALS = [
        {
            id: "LOC-004",
            name: "Max Super Speciality Hospital",
            address: "Mussoorie Diversion Road, Malsi, Dehradun",
            latitude: 30.3812,
            longitude: 78.0772,
            totalBeds: 280,
            icuBeds: 45,
            availableIcu: 14,
            admittedCount: 0,
            oxygenBeds: 95,
            specialties: ["Cardiology", "Trauma", "Neurology", "Critical Care", "Pediatrics"],
            rating: 4.8,
            phone: "+91-135-7193000"
        },
        {
            id: "LOC-011",
            name: "Doon Government Hospital",
            address: "Parade Ground, Court Road, Dehradun",
            latitude: 30.3256,
            longitude: 78.0437,
            totalBeds: 450,
            icuBeds: 50,
            availableIcu: 8,
            oxygenBeds: 180,
            specialties: ["General Medicine", "Emergency & Trauma", "General Surgery", "Pediatrics"],
            rating: 4.3,
            phone: "+91-135-2659847"
        },
        {
            id: "LOC-006",
            name: "Synergy Super Speciality Hospital",
            address: "Canal Road, Ballupur, Dehradun",
            latitude: 30.3398,
            longitude: 78.0124,
            totalBeds: 180,
            icuBeds: 32,
            availableIcu: 9,
            oxygenBeds: 60,
            specialties: ["Cardiology", "Nephrology", "Critical Care", "Gastroenterology"],
            rating: 4.6,
            phone: "+91-135-2226000"
        },
        {
            id: "LOC-014",
            name: "Graphic Era Hospital & Medical College",
            address: "Subhash Nagar, Clement Town, Dehradun",
            latitude: 30.2715,
            longitude: 78.0084,
            totalBeds: 320,
            icuBeds: 40,
            availableIcu: 18,
            oxygenBeds: 110,
            specialties: ["Cardiology", "Emergency Medicine", "Orthopedics", "Pulmonology"],
            rating: 4.9,
            phone: "+91-135-2643070"
        },
        {
            id: "LOC-018",
            name: "Shri Mahant Indiresh Hospital",
            address: "Patel Nagar, Dehradun",
            latitude: 30.3112,
            longitude: 78.0189,
            totalBeds: 750,
            icuBeds: 80,
            availableIcu: 21,
            oxygenBeds: 220,
            specialties: ["Trauma Surgery", "Neurosurgery", "Cardiology", "Dialysis"],
            rating: 4.4,
            phone: "+91-135-6672400"
        },
        {
            id: "LOC-020",
            name: "AIIMS Rishikesh (Apex Trauma Centre)",
            address: "Virbhadra Road, Rishikesh, Dehradun District",
            latitude: 30.0758,
            longitude: 78.2880,
            totalBeds: 960,
            icuBeds: 120,
            availableIcu: 26,
            oxygenBeds: 350,
            specialties: ["Apex Trauma", "Cardiothoracic Surgery", "Neurosurgery", "Organ Transplant"],
            rating: 4.9,
            phone: "+91-135-2462929"
        }
    ];

    // -------------------------------------------------------------------------
    // Seed Data: Ambulances Fleet
    // -------------------------------------------------------------------------
    const SEED_AMBULANCES = [
        {
            id: "AMB-01",
            ambulanceId: "AMB-01",
            unitCode: "AMB-01 (ALS)",
            type: "ALS",
            vehicleType: "ALS",
            equipmentLevel: "ADVANCED_LIFE_SUPPORT",
            status: "READY",
            stationName: "Clock Tower Station",
            driverName: "Vikram Negi",
            driver: "Vikram Negi",
            driverPhone: "+91 98970 11001",
            phone: "+91 98970 11001",
            latitude: 30.3244,
            longitude: 78.0418,
            currentLat: 30.3244,
            currentLng: 78.0418,
            assignedEmergencyId: null
        },
        {
            id: "AMB-02",
            ambulanceId: "AMB-02",
            unitCode: "AMB-02 (BLS)",
            type: "BLS",
            vehicleType: "BLS",
            equipmentLevel: "BASIC_LIFE_SUPPORT",
            status: "READY",
            stationName: "ISBT Base Depot",
            driverName: "Sanjay Rawat",
            driver: "Sanjay Rawat",
            driverPhone: "+91 98970 11002",
            phone: "+91 98970 11002",
            latitude: 30.2872,
            longitude: 78.0069,
            currentLat: 30.2872,
            currentLng: 78.0069,
            assignedEmergencyId: null
        },
        {
            id: "AMB-03",
            ambulanceId: "AMB-03",
            unitCode: "AMB-03 (ALS)",
            type: "ALS",
            vehicleType: "ALS",
            equipmentLevel: "ADVANCED_LIFE_SUPPORT",
            status: "READY",
            stationName: "Ballupur Substation",
            driverName: "Ramesh Joshi",
            driver: "Ramesh Joshi",
            driverPhone: "+91 98970 11003",
            phone: "+91 98970 11003",
            latitude: 30.3340,
            longitude: 78.0180,
            currentLat: 30.3340,
            currentLng: 78.0180,
            assignedEmergencyId: null
        },
        {
            id: "AMB-04",
            ambulanceId: "AMB-04",
            unitCode: "AMB-04 (ALS)",
            type: "ALS",
            vehicleType: "ALS",
            equipmentLevel: "ADVANCED_LIFE_SUPPORT",
            status: "READY",
            stationName: "Railway Station Post",
            driverName: "Anil Chauhan",
            driver: "Anil Chauhan",
            driverPhone: "+91 98970 11004",
            phone: "+91 98970 11004",
            latitude: 30.3155,
            longitude: 78.0331,
            currentLat: 30.3155,
            currentLng: 78.0331,
            assignedEmergencyId: null
        }
    ];

    // -------------------------------------------------------------------------
    // Seed Data: Admin Users
    // -------------------------------------------------------------------------
    const SEED_USERS = [
        { userId: 1, username: "admin", role: "ADMIN", displayName: "System Administrator", phone: "+91-135-2651000", isActive: true },
        { userId: 2, username: "driver_amb01", role: "DRIVER", displayName: "Vikram Negi (AMB-01)", phone: "+91-98970-11001", linkedAmbulanceId: "AMB-01", isActive: true },
        { userId: 3, username: "driver_amb02", role: "DRIVER", displayName: "Sanjay Rawat (AMB-02)", phone: "+91-98970-11002", linkedAmbulanceId: "AMB-02", isActive: true },
        { userId: 4, username: "hosp_max", role: "HOSPITAL", displayName: "Max Super Speciality Desk", phone: "+91-135-7193000", linkedHospitalId: "LOC-004", isActive: true },
        { userId: 5, username: "hosp_doon", role: "HOSPITAL", displayName: "Doon Govt Hospital Desk", phone: "+91-135-2659847", linkedHospitalId: "LOC-011", isActive: true },
        { userId: 6, username: "hosp_synergy", role: "HOSPITAL", displayName: "Synergy Hospital Desk", phone: "+91-135-2226000", linkedHospitalId: "LOC-006", isActive: true },
        { userId: 7, username: "patient_demo", role: "PATIENT", displayName: "Aman Negi", phone: "+91-98765-43210", isActive: true }
    ];

    // -------------------------------------------------------------------------
    // Seed Data: City-Wide Emergencies Archive
    // -------------------------------------------------------------------------
    const SEED_EMERGENCIES = [
        {
            emergencyId: "EMG-2026-9104",
            status: "COMPLETED",
            severity: "CRITICAL",
            triageCategory: "CRITICAL",
            urgencyScore: 1,
            callerName: "Aman Negi",
            callerPhone: "+91 98765 43210",
            patientName: "Aman Negi",
            condition: "Acute severe chest pain & dyspnea",
            conditionText: "Acute severe chest pain & dyspnea",
            pickupLocation: "Clock Tower Chowk, Dehradun",
            location: "Clock Tower Chowk, Dehradun",
            assignedAmbulanceId: "AMB-01",
            assignedHospitalId: "LOC-004",
            latitude: 30.3244,
            longitude: 78.0418,
            currentLat: 30.3421,
            currentLng: 78.0532,
            createdAt: "Today 11:42 PM",
            timestamp: "Today 11:42 PM",
            liveRoute: {
                active: true,
                phase: "TO_HOSPITAL",
                etaMinutes: 4.2,
                distanceKm: 2.8,
                coordinates: [
                    { latitude: 30.3244, longitude: 78.0418 },
                    { latitude: 30.3340, longitude: 78.0480 },
                    { latitude: 30.3500, longitude: 78.0570 },
                    { latitude: 30.3650, longitude: 78.0680 },
                    { latitude: 30.3812, longitude: 78.0772 }
                ]
            }
        },
        {
            emergencyId: "EMG-2026-8742",
            status: "COMPLETED",
            severity: "CRITICAL",
            triageCategory: "CRITICAL",
            urgencyScore: 1,
            callerName: "Pooja Sharma",
            callerPhone: "+91 98970 44221",
            patientName: "Pooja Sharma",
            condition: "Road traffic accident with lower limb fracture",
            conditionText: "Road traffic accident with lower limb fracture",
            pickupLocation: "Rajpur Road Jakhan, Dehradun",
            location: "Rajpur Road Jakhan, Dehradun",
            assignedAmbulanceId: "AMB-04",
            assignedHospitalId: "LOC-004",
            latitude: 30.3580,
            longitude: 78.0645,
            createdAt: "Today 09:15 PM",
            timestamp: "Today 09:15 PM"
        },
        {
            emergencyId: "EMG-2026-8319",
            status: "COMPLETED",
            severity: "URGENT",
            triageCategory: "URGENT",
            urgencyScore: 2,
            callerName: "Rajesh Rawat",
            callerPhone: "+91 94120 77103",
            patientName: "Rajesh Rawat",
            condition: "High grade pyrexia with acute abdominal distress",
            conditionText: "High grade pyrexia with acute abdominal distress",
            pickupLocation: "Patel Nagar, Dehradun",
            location: "Patel Nagar, Dehradun",
            assignedAmbulanceId: "AMB-02",
            assignedHospitalId: "LOC-011",
            latitude: 30.3112,
            longitude: 78.0189,
            createdAt: "Today 07:30 PM",
            timestamp: "Today 07:30 PM"
        }
    ];

    // -------------------------------------------------------------------------
    // LocalStorage State Management
    // -------------------------------------------------------------------------
    function getStore(key, defaultVal) {
        try {
            const raw = localStorage.getItem('jm_mock_' + key);
            return raw ? JSON.parse(raw) : defaultVal;
        } catch (e) {
            return defaultVal;
        }
    }

    function setStore(key, val) {
        try {
            localStorage.setItem('jm_mock_' + key, JSON.stringify(val));
        } catch (e) {}
    }

    if (!localStorage.getItem('jm_mock_hospitals')) {
        setStore('hospitals', SEED_HOSPITALS);
    }
    if (!localStorage.getItem('jm_mock_ambulances')) {
        setStore('ambulances', SEED_AMBULANCES);
    }
    if (!localStorage.getItem('jm_mock_users')) {
        setStore('users', SEED_USERS);
    }
    if (!localStorage.getItem('jm_mock_emergencies')) {
        setStore('emergencies', SEED_EMERGENCIES);
    } else {
        // Sanitize any existing cached emergencies so old seed records don't stay in active state
        try {
            const stored = getStore('emergencies', []);
            let changed = false;
            stored.forEach(e => {
                if (e.emergencyId === 'EMG-2026-9104' && e.status !== 'COMPLETED') {
                    e.status = 'COMPLETED';
                    changed = true;
                }
            });
            if (changed) setStore('emergencies', stored);
        } catch (e) {}
    }

    // -------------------------------------------------------------------------
    // Synthetic Dijkstra Road Coordinates for Dehradun
    // -------------------------------------------------------------------------
    function generateDijkstraRoute(pLat, pLng, hLat, hLng) {
        return [
            { latitude: pLat, longitude: pLng },
            { latitude: pLat + (hLat - pLat) * 0.25 + 0.002, longitude: pLng + (hLng - pLng) * 0.25 - 0.001 },
            { latitude: pLat + (hLat - pLat) * 0.50 + 0.004, longitude: pLng + (hLng - pLng) * 0.50 + 0.002 },
            { latitude: pLat + (hLat - pLat) * 0.75 + 0.001, longitude: pLng + (hLng - pLng) * 0.75 + 0.003 },
            { latitude: hLat, longitude: hLng }
        ];
    }

    // -------------------------------------------------------------------------
    // Mock API Dispatcher
    // -------------------------------------------------------------------------
    function handleMockApi(urlStr, method, headers, body) {
        const url = new URL(urlStr, window.location.origin);
        const path = url.pathname;
        const normMethod = (method || 'GET').toUpperCase();

        let parsedBody = {};
        if (body && typeof body === 'string') {
            try { parsedBody = JSON.parse(body); } catch (e) {}
        }

        // 0a. Authentication: POST /api/auth/login
        if (normMethod === 'POST' && path.includes('/api/auth/login')) {
            const uLower = (parsedBody.username || '').toLowerCase().trim();
            const users = getStore('users', SEED_USERS);
            let user = users.find(u => u.username.toLowerCase() === uLower);

            if (!user) {
                let role = 'PATIENT';
                if (uLower.includes('admin')) role = 'ADMIN';
                else if (uLower.includes('driver') || uLower.includes('amb')) role = 'DRIVER';
                else if (uLower.includes('hosp')) role = 'HOSPITAL';

                user = {
                    userId: users.length + 1,
                    role: role,
                    username: parsedBody.username || 'user',
                    displayName: parsedBody.username || 'Authenticated User',
                    linkedAmbulanceId: role === 'DRIVER' ? 'AMB-01' : undefined,
                    linkedHospitalId: role === 'HOSPITAL' ? 'LOC-004' : undefined,
                    isActive: true
                };
            }

            const token = 'showcase-jwt-token-' + user.username;
            try {
                localStorage.setItem('jeevan_marg_auth_token', token);
                localStorage.setItem('jeevan_marg_auth_user', JSON.stringify(user));
            } catch (e) {}

            return {
                success: true,
                status: "OK",
                token: token,
                user: user
            };
        }

        // 0b. Authentication: POST /api/auth/register
        if (normMethod === 'POST' && path.includes('/api/auth/register')) {
            const users = getStore('users', SEED_USERS);
            const u = {
                userId: users.length + 1,
                role: 'PATIENT',
                username: parsedBody.username || 'new_patient',
                displayName: parsedBody.displayName || parsedBody.username || 'Registered Patient',
                phone: parsedBody.phone || '+91-9876543210',
                isActive: true
            };
            users.push(u);
            setStore('users', users);
            const token = 'showcase-jwt-token-' + u.username;
            try {
                localStorage.setItem('jeevan_marg_auth_token', token);
                localStorage.setItem('jeevan_marg_auth_user', JSON.stringify(u));
            } catch (e) {}
            return { success: true, token: token, user: u };
        }

        // 0c. Authentication: GET /api/auth/me
        if (path.includes('/api/auth/me')) {
            let u = { role: 'ADMIN', username: 'admin', displayName: 'System Administrator' };
            try {
                const stored = localStorage.getItem('jeevan_marg_auth_user');
                if (stored) u = JSON.parse(stored);
            } catch (e) {}
            return { success: true, user: u };
        }

        // 0d. Authentication: POST /api/auth/logout
        if (path.includes('/api/auth/logout')) {
            try {
                localStorage.removeItem('jeevan_marg_auth_token');
                localStorage.removeItem('jeevan_marg_auth_user');
            } catch (e) {}
            return { success: true, message: "Logged out successfully" };
        }

        // 1. GET /api/hospitals or /hospitals
        if (path === '/api/hospitals' || path === '/hospitals' || path.endsWith('/hospitals')) {
            const hospitals = getStore('hospitals', SEED_HOSPITALS);
            return {
                success: true,
                status: "OK",
                count: hospitals.length,
                data: hospitals
            };
        }

        // 2. GET /api/hospitals/search
        if (path.includes('/hospitals/search')) {
            const hospitals = getStore('hospitals', SEED_HOSPITALS);
            const q = (url.searchParams.get('query') || '').toLowerCase();
            const spec = (url.searchParams.get('specialty') || '').toLowerCase();
            const filtered = hospitals.filter(h => {
                const matchQ = !q || h.name.toLowerCase().includes(q) || h.address.toLowerCase().includes(q);
                const matchS = !spec || (h.specialties || []).some(s => s.toLowerCase().includes(spec));
                return matchQ && matchS;
            });
            return { success: true, status: "OK", count: filtered.length, data: filtered };
        }

        // 3a. Patient Portal: GET /api/patient/emergencies (Strict Patient Isolation)
        if (normMethod === 'GET' && path.includes('/patient/emergencies')) {
            const list = getStore('emergencies', SEED_EMERGENCIES);

            let currUser = null;
            try {
                const rawUser = localStorage.getItem('jeevan_marg_auth_user');
                if (rawUser) currUser = JSON.parse(rawUser);
            } catch (e) {}

            const callerQuery = (url.searchParams.get('patient') || url.searchParams.get('caller') || currUser?.displayName || currUser?.username || '').toLowerCase().trim();

            let patientEmergencies = [];
            if (callerQuery && !callerQuery.includes('admin')) {
                patientEmergencies = list.filter(e => {
                    const pName = (e.patientName || e.callerName || '').toLowerCase().trim();
                    return pName.includes(callerQuery) || callerQuery.includes(pName);
                });
            } else {
                // If patient name not specified, check for emergencies for 'Aryan'
                patientEmergencies = list.filter(e => {
                    const pName = (e.patientName || e.callerName || '').toLowerCase().trim();
                    return pName.includes('aryan');
                });
            }

            return {
                success: true,
                status: "OK",
                count: patientEmergencies.length,
                data: patientEmergencies,
                emergencies: patientEmergencies
            };
        }

        // 3b. Central Control & Audit: GET /api/emergency-requests or GET /api/emergencies
        if (normMethod === 'GET' && (path.includes('/emergency-requests') || path === '/api/emergencies')) {
            const list = getStore('emergencies', SEED_EMERGENCIES);
            return {
                success: true,
                status: "OK",
                count: list.length,
                data: list,
                emergencies: list
            };
        }

        // 4. POST /api/emergency-requests or /api/patient/emergency or /api/emergencies
        if (normMethod === 'POST' && (path.includes('/emergency-requests') || path.includes('/patient/emergency') || path === '/api/emergencies')) {
            const emergencies = getStore('emergencies', SEED_EMERGENCIES);
            const newId = 'EMG-' + new Date().getFullYear() + '-' + Math.floor(1000 + Math.random() * 9000);
            
            const cond = (parsedBody.condition || parsedBody.conditionText || 'Emergency Dispatch').toLowerCase();
            let cat = 'CRITICAL';
            let urg = 1;
            if (cond.includes('fever') || cond.includes('minor') || cond.includes('headache')) {
                cat = 'MODERATE';
                urg = 3;
            } else if (cond.includes('fracture') || cond.includes('bleeding')) {
                cat = 'URGENT';
                urg = 2;
            }

            const pLat = parsedBody.latitude || 30.3244;
            const pLng = parsedBody.longitude || 78.0418;
            const hosp = SEED_HOSPITALS[0]; // Max Super Speciality
            const amb = SEED_AMBULANCES[0]; // AMB-01

            const routeCoords = generateDijkstraRoute(pLat, pLng, hosp.latitude, hosp.longitude);

            const newEm = {
                emergencyId: newId,
                status: "DISPATCHED",
                severity: cat,
                triageCategory: cat,
                urgencyScore: urg,
                callerName: parsedBody.patientName || parsedBody.callerName || "Aryan",
                callerPhone: parsedBody.patientPhone || parsedBody.callerPhone || "+91-9876543210",
                patientName: parsedBody.patientName || parsedBody.callerName || "Aryan",
                patientPhone: parsedBody.patientPhone || parsedBody.callerPhone || "+91-9876543210",
                relativeName: parsedBody.relativeName || "",
                relativePhone: parsedBody.relativePhone || "",
                relativeRelation: parsedBody.relativeRelation || "",
                condition: parsedBody.condition || parsedBody.conditionText || "Severe Acute Emergency",
                conditionText: parsedBody.condition || parsedBody.conditionText || "Severe Acute Emergency",
                pickupLocation: parsedBody.pickupLocation || parsedBody.location || "Clock Tower, Dehradun",
                location: parsedBody.pickupLocation || parsedBody.location || "Clock Tower, Dehradun",
                assignedAmbulanceId: amb.unitCode,
                assignedHospitalId: hosp.id,
                hospitalName: hosp.name,
                latitude: pLat,
                longitude: pLng,
                createdAt: "Just now",
                timestamp: "Just now",
                liveRoute: {
                    active: true,
                    phase: "TO_PICKUP",
                    etaMinutes: 3.8,
                    distanceKm: 2.4,
                    coordinates: routeCoords
                }
            };

            emergencies.unshift(newEm);
            setStore('emergencies', emergencies);

            // Also assign to driver
            const ambulances = getStore('ambulances', SEED_AMBULANCES);
            const aIdx = ambulances.findIndex(a => a.id === amb.id || a.unitCode === amb.unitCode);
            if (aIdx >= 0) {
                ambulances[aIdx].status = 'ASSIGNED';
                ambulances[aIdx].assignedEmergencyId = newId;
                setStore('ambulances', ambulances);
            }

            return {
                success: true,
                status: "DISPATCHED",
                emergencyId: newId,
                token: "showcase-token-" + newId,
                data: newEm,
                triage: {
                    category: cat,
                    urgencyScore: urg,
                    recommendedAmbulance: amb.unitCode,
                    rationale: "Automated Clinical Rule Engine: Classified as " + cat + " with ICU priority."
                },
                assignedAmbulanceId: amb.unitCode,
                assignedHospitalId: hosp.id,
                liveRoute: newEm.liveRoute
            };
        }

        // 5. GET /api/emergencies/:id or tracking
        if (normMethod === 'GET' && path.includes('/api/emergencies/')) {
            const parts = path.split('/');
            const emId = parts[parts.length - 1].replace('?.*', '');
            const emergencies = getStore('emergencies', SEED_EMERGENCIES);
            const em = emergencies.find(e => e.emergencyId === emId || path.includes(e.emergencyId)) || emergencies[0];
            const hospitals = getStore('hospitals', SEED_HOSPITALS);
            const hosp = hospitals.find(h => h.id === em.assignedHospitalId) || hospitals[0];

            let ambLat = em.latitude + 0.003;
            let ambLng = em.longitude - 0.002;
            if (em.status === 'ON_SCENE') {
                ambLat = em.latitude;
                ambLng = em.longitude;
            } else if (em.status === 'TRANSPORTING') {
                ambLat = (em.latitude + hosp.latitude) / 2;
                ambLng = (em.longitude + hosp.longitude) / 2;
            } else if (em.status === 'ARRIVED' || em.status === 'COMPLETED') {
                ambLat = hosp.latitude;
                ambLng = hosp.longitude;
            }

            return {
                success: true,
                data: {
                    ...em,
                    emergency: em,
                    id: em.emergencyId,
                    status: em.status,
                    severity: em.severity,
                    callerName: em.callerName || em.patientName,
                    callerPhone: em.callerPhone || em.patientPhone,
                    relativeName: em.relativeName || "",
                    relativePhone: em.relativePhone || "",
                    relativeRelation: em.relativeRelation || "",
                    ambulance: {
                        unitCode: em.assignedAmbulanceId || "AMB-01",
                        status: em.status === 'COMPLETED' ? 'READY' : (em.status || 'EN_ROUTE'),
                        currentLat: ambLat,
                        currentLng: ambLng,
                        driverName: "Vikram Negi",
                        driverPhone: "+91-98765-43210"
                    },
                    hospital: hosp
                }
            };
        }

        // 6. Driver Portal: GET /api/driver/ambulance
        if (path === '/api/driver/ambulance') {
            const ambulances = getStore('ambulances', SEED_AMBULANCES);
            return { success: true, data: ambulances[0] };
        }

        // 7. Driver Portal: GET /api/driver/assigned-emergency
        if (path.includes('/driver/assigned-emergency')) {
            const ambulances = getStore('ambulances', SEED_AMBULANCES);
            const emergencies = getStore('emergencies', SEED_EMERGENCIES);

            // Get logged-in driver's ambulance
            let driverAmbId = "AMB-01";
            try {
                const u = JSON.parse(localStorage.getItem('jeevan_marg_auth_user') || '{}');
                if (u.linkedAmbulanceId) driverAmbId = u.linkedAmbulanceId;
            } catch (e) {}

            const amb = ambulances.find(a => a.id === driverAmbId || a.ambulanceId === driverAmbId || a.unitCode?.includes(driverAmbId)) || ambulances[0];

            let active = null;
            if (amb && amb.assignedEmergencyId) {
                active = emergencies.find(e => e.emergencyId === amb.assignedEmergencyId && e.status !== 'COMPLETED' && e.status !== 'CANCELLED');
            }
            if (!active && amb && amb.status !== 'READY') {
                active = emergencies.find(e => (e.assignedAmbulanceId === amb.id || e.assignedAmbulanceId === amb.unitCode || e.assignedAmbulanceId?.includes(driverAmbId)) && e.status !== 'COMPLETED' && e.status !== 'CANCELLED');
            }

            return {
                success: true,
                data: active || null
            };
        }

        // 8. Driver Portal: Status Transitions (PATCH /api/driver/ambulance/status or POST /driver/progress-status)
        if ((normMethod === 'PATCH' || normMethod === 'POST') && (path.includes('/driver/ambulance/status') || path.includes('/driver/progress-status') || path.includes('/driver/emergencies'))) {
            const newStatus = parsedBody.status || 'ON_SCENE';
            const emId = parsedBody.emergencyId;
            const action = parsedBody.action;
            const emergencies = getStore('emergencies', SEED_EMERGENCIES);
            const ambulances = getStore('ambulances', SEED_AMBULANCES);

            // Locate matching emergency
            let targetEm = null;
            if (emId) {
                targetEm = emergencies.find(e => e.emergencyId === emId);
            }
            if (!targetEm) {
                targetEm = emergencies.find(e => e.status !== 'COMPLETED' && e.status !== 'CANCELLED') || emergencies[0];
            }

            // Locate driver's ambulance
            let driverAmbId = "AMB-01";
            try {
                const u = JSON.parse(localStorage.getItem('jeevan_marg_auth_user') || '{}');
                if (u.linkedAmbulanceId) driverAmbId = u.linkedAmbulanceId;
            } catch (e) {}
            const amb = ambulances.find(a => a.id === driverAmbId || a.ambulanceId === driverAmbId || a.unitCode?.includes(driverAmbId)) || ambulances[0];

            if (action === 'DECLINE' || newStatus === 'READY') {
                if (targetEm) {
                    targetEm.status = 'CANCELLED';
                    targetEm.assignedAmbulanceId = null;
                    if (targetEm.liveRoute) targetEm.liveRoute.active = false;
                }
                if (amb) {
                    amb.status = 'READY';
                    amb.assignedEmergencyId = null;
                }
            } else if (newStatus === 'COMPLETED') {
                if (targetEm) {
                    targetEm.status = 'COMPLETED';
                    if (targetEm.liveRoute) targetEm.liveRoute.active = false;
                }
                if (amb) {
                    amb.status = 'READY';
                    amb.assignedEmergencyId = null;
                }
            } else {
                if (targetEm) {
                    targetEm.status = newStatus;
                }
                if (amb) {
                    amb.status = 'BUSY';
                    if (targetEm) amb.assignedEmergencyId = targetEm.emergencyId;
                }
            }

            setStore('emergencies', emergencies);
            setStore('ambulances', ambulances);

            return {
                success: true,
                message: `Ambulance status updated to ${newStatus}`,
                status: newStatus,
                ambulance: amb,
                emergency: targetEm
            };
        }

        // 9. Driver Portal: POST /api/driver/update-location
        if (normMethod === 'POST' && path.includes('/driver/update-location')) {
            return { success: true, message: "GPS position recorded" };
        }

        // 10. Hospital Portal: GET /api/hospital/profile
        if (path === '/api/hospital/profile') {
            const hospitals = getStore('hospitals', SEED_HOSPITALS);
            return { success: true, data: hospitals[0] || SEED_HOSPITALS[0] };
        }

        // 11. Hospital Portal: GET /api/hospital/emergencies
        if (path === '/api/hospital/emergencies') {
            const emergencies = getStore('emergencies', SEED_EMERGENCIES);
            return { success: true, count: emergencies.length, data: emergencies, emergencies };
        }

        // 12. Hospital Portal: PATCH /api/hospital/beds
        if (normMethod === 'PATCH' && path.includes('/hospital/beds')) {
            const hospitals = getStore('hospitals', SEED_HOSPITALS);
            if (hospitals.length > 0) {
                if (parsedBody.totalBeds !== undefined) hospitals[0].totalBeds = parsedBody.totalBeds;
                if (parsedBody.icuTotal !== undefined) hospitals[0].icuBeds = parsedBody.icuTotal;
                if (parsedBody.icuAvailable !== undefined) hospitals[0].availableIcu = parsedBody.icuAvailable;
                if (parsedBody.oxygenBeds !== undefined) hospitals[0].oxygenBeds = parsedBody.oxygenBeds;
                setStore('hospitals', hospitals);
            }
            return { success: true, message: "Bed capacity updated successfully", data: hospitals[0] };
        }

        // 13. Hospital Portal: POST /api/hospital/accept or /api/hospital/admit
        if (normMethod === 'POST' && (path.includes('/hospital/accept') || path.includes('/hospital/admit') || path.includes('/admit') || path.includes('/accept'))) {
            const emergencies = getStore('emergencies', SEED_EMERGENCIES);
            const hospitals = getStore('hospitals', SEED_HOSPITALS);
            const ambulances = getStore('ambulances', SEED_AMBULANCES);

            // Locate matching emergency
            const emId = parsedBody.emergencyId || path.split('/').find(p => p.startsWith('EMG-'));
            let targetEm = null;
            if (emId) {
                targetEm = emergencies.find(e => e.emergencyId === emId);
            }
            if (!targetEm) {
                targetEm = emergencies.find(e => e.status !== 'COMPLETED' && e.status !== 'CANCELLED') || emergencies[0];
            }

            const hospId = targetEm?.assignedHospitalId || 'LOC-004';
            const hosp = hospitals.find(h => h.id === hospId) || hospitals[0];

            if (path.includes('admit')) {
                // Precondition check: Ambulance must have confirmed at least ON_SCENE arrival!
                if (targetEm && targetEm.status !== 'ON_SCENE' && targetEm.status !== 'TRANSPORTING' && targetEm.status !== 'ARRIVED') {
                    return {
                        success: false,
                        status: "PRECONDITION_FAILED",
                        message: "Cannot admit patient yet: Driver must reach on-scene and confirm arrival first."
                    };
                }

                if (targetEm) {
                    targetEm.status = 'COMPLETED';
                    targetEm.admitted = true;
                    targetEm.admittedAt = new Date().toLocaleTimeString();
                    if (targetEm.liveRoute) targetEm.liveRoute.active = false;
                }

                // Decrement Available ICU Bed count by 1 (-1)
                if (hosp.availableIcu > 0) {
                    hosp.availableIcu -= 1;
                }
                // Increment Admitted Patients count by 1 (+1)
                hosp.admittedCount = (hosp.admittedCount || 0) + 1;
                setStore('hospitals', hospitals);

                // Release assigned ambulance to READY
                const ambCode = targetEm?.assignedAmbulanceId;
                const amb = ambulances.find(a => a.id === ambCode || a.unitCode === ambCode || a.ambulanceId === ambCode) || ambulances[0];
                if (amb) {
                    amb.status = 'READY';
                    amb.assignedEmergencyId = null;
                    setStore('ambulances', ambulances);
                }

                setStore('emergencies', emergencies);

                return {
                    success: true,
                    status: "OK",
                    message: `Patient ${targetEm?.patientName || 'Emergency'} admitted to ICU. Bed allocated (-1 bed, +1 patient). Ambulance released to READY.`,
                    availableIcu: hosp.availableIcu,
                    admittedCount: hosp.admittedCount,
                    hospital: hosp,
                    emergency: targetEm
                };
            } else {
                // Accept & Prep ICU Ward (Staff acknowledges and prepares trauma bay in advance)
                if (targetEm) {
                    targetEm.prepped = true;
                    targetEm.preppedAt = new Date().toLocaleTimeString();
                    // Do NOT jump to ARRIVED prematurely if still en route!
                }
                setStore('emergencies', emergencies);
                return {
                    success: true,
                    status: "OK",
                    message: `Emergency dispatch ${targetEm?.emergencyId} accepted and ICU team prepared.`,
                    emergency: targetEm,
                    hospital: hosp
                };
            }
        }

        // 14. Admin Portal: GET /api/ambulances
        if (path === '/api/ambulances') {
            const ambulances = getStore('ambulances', SEED_AMBULANCES);
            return { success: true, count: ambulances.length, data: ambulances };
        }

        // 15. Admin Portal: GET /api/admin/users
        if (path.includes('/admin/users') && normMethod === 'GET') {
            const users = getStore('users', SEED_USERS);
            return {
                success: true,
                count: users.length,
                data: users,
                users: users
            };
        }

        // 15b. Admin Portal: POST /api/admin/drivers (Provision Driver Account)
        if (normMethod === 'POST' && path.includes('/admin/drivers')) {
            const users = getStore('users', SEED_USERS);
            const username = (parsedBody.username || 'new_driver').trim();
            const newUser = {
                userId: users.length + 1,
                username: username,
                role: 'DRIVER',
                displayName: parsedBody.displayName || username,
                phone: parsedBody.phone || '+91-98970-00000',
                linkedAmbulanceId: parsedBody.ambulanceId || 'AMB-01',
                isActive: true
            };
            users.push(newUser);
            setStore('users', users);
            return { success: true, message: `Driver account '${username}' provisioned and saved!`, user: newUser };
        }

        // 15c. Admin Portal: POST /api/admin/hospitals (Provision Hospital Account)
        if (normMethod === 'POST' && path.includes('/admin/hospitals')) {
            const users = getStore('users', SEED_USERS);
            const username = (parsedBody.username || 'new_hosp').trim();
            const newUser = {
                userId: users.length + 1,
                username: username,
                role: 'HOSPITAL',
                displayName: parsedBody.displayName || username,
                phone: parsedBody.phone || '+91-135-0000000',
                linkedHospitalId: parsedBody.hospitalId || 'LOC-004',
                isActive: true
            };
            users.push(newUser);
            setStore('users', users);
            return { success: true, message: `Hospital account '${username}' provisioned and saved!`, user: newUser };
        }

        // 15d. Admin Portal: PATCH /api/admin/users/:id/status (Toggle user active status)
        if (normMethod === 'PATCH' && path.includes('/admin/users/')) {
            const parts = path.split('/');
            const uid = parseInt(parts[4] || parts[3], 10);
            const users = getStore('users', SEED_USERS);
            const user = users.find(u => u.userId === uid);
            if (user) {
                user.isActive = !user.isActive;
                setStore('users', users);
            }
            return { success: true, message: "User status updated", user: user };
        }

        // 16. Admin Portal: POST /api/admin/reset-demo
        if (path.includes('/reset-demo') || path.includes('/demo/reset')) {
            setStore('hospitals', SEED_HOSPITALS);
            setStore('ambulances', SEED_AMBULANCES);
            setStore('users', SEED_USERS);
            setStore('emergencies', SEED_EMERGENCIES);
            return { success: true, message: "Demo data reset to clean baseline." };
        }

        // 17. GET /route or /api/route (Dijkstra Shortest Path across 20 nodes)
        if (path === '/route' || path.endsWith('/route') || path.includes('/api/route')) {
            const fromId = url.searchParams.get('from') || 'LOC-001';
            const toId = url.searchParams.get('to') || 'LOC-004';
            if (typeof M3_API !== 'undefined' && typeof M3_API.calculateDijkstra === 'function') {
                const routeData = M3_API.calculateDijkstra(fromId, toId);
                return {
                    success: true,
                    status: "OK",
                    data: routeData
                };
            }
            return {
                success: true,
                status: "OK",
                data: {
                    startNodeId: fromId,
                    endNodeId: toId,
                    totalDistanceKm: 7.3,
                    algorithm: "Dijkstra (Min-Heap Adjacency List)",
                    pathNodeIds: [fromId, "LOC-017", toId],
                    segments: [
                        { from: fromId, to: "LOC-017", roadName: "Rajpur Road", roadType: "MAIN_ROAD", distanceKm: 4.1 },
                        { from: "LOC-017", to: toId, roadName: "Mussoorie Diversion Hill Road", roadType: "HILL_ROAD", distanceKm: 3.2 }
                    ],
                    travelEstimates: {
                        "Ambulance": { durationMinutes: 8.5, estimatedCostRupees: 180 },
                        "Two-Wheeler": { durationMinutes: 11.0, estimatedCostRupees: 45 },
                        "Public Transit (Bus)": { durationMinutes: 18.0, estimatedCostRupees: 30 },
                        "Walking": { durationMinutes: 95.0, estimatedCostRupees: 0 }
                    }
                }
            };
        }

        // 18. GET /nearby
        if (path === '/nearby' || path.endsWith('/nearby')) {
            return {
                success: true,
                count: SEED_HOSPITALS.length,
                data: SEED_HOSPITALS
            };
        }

        // 19. Default fallback for unhandled /api calls
        return { success: true, status: "OK", message: "Processed in Showcase Cloud Mode" };
    }

    // -------------------------------------------------------------------------
    // Monkey-patch window.fetch
    // -------------------------------------------------------------------------
    const nativeFetch = window.fetch.bind(window);

    window.fetch = async function(input, init) {
        const urlStr = typeof input === 'string' ? input : (input instanceof Request ? input.url : String(input));
        const method = (init && init.method) ? init.method : (input instanceof Request ? input.method : 'GET');
        const headers = (init && init.headers) ? init.headers : (input instanceof Request ? input.headers : {});
        const body = (init && init.body) ? init.body : null;

        const isApiCall = urlStr.includes('/api/') || 
                          urlStr.endsWith('/nearby') || 
                          urlStr.includes('/nearby?') ||
                          urlStr.endsWith('/route') || 
                          urlStr.includes('/route?') ||
                          urlStr.endsWith('/hospitals');

        // On Netlify: ALWAYS use client mock engine for API endpoints so zero network latency or HTML redirects occur
        if (isCloud && isApiCall) {
            try {
                const mockRes = handleMockApi(urlStr, method, headers, body);
                const isFail = mockRes && (mockRes.success === false || mockRes.status === "PRECONDITION_FAILED");
                return new Response(JSON.stringify(mockRes), {
                    status: isFail ? 400 : 200,
                    statusText: isFail ? "Bad Request" : "OK",
                    headers: {
                        'Content-Type': 'application/json',
                        'X-Powered-By': 'JEEVAN-MARG-SHOWCASE-CLOUD'
                    }
                });
            } catch (err) {
                console.error('[Cloud Mock] Processing error:', err);
            }
        }

        // If local or non-API: attempt native fetch first
        try {
            const res = await nativeFetch(input, init);
            
            // If response returned HTML on an API endpoint (e.g. Netlify 200 redirect to index.html or 404 HTML), intercept!
            if (isApiCall) {
                const contentType = res.headers.get('content-type') || '';
                if (contentType.includes('text/html') || res.status === 404 || res.status === 502) {
                    console.warn(`[Cloud Mock] Caught non-JSON response (${res.status} ${contentType}) for ${urlStr}. Returning mock fallback.`);
                    const mockRes = handleMockApi(urlStr, method, headers, body);
                    return new Response(JSON.stringify(mockRes), {
                        status: 200,
                        statusText: "OK",
                        headers: {
                            'Content-Type': 'application/json',
                            'X-Powered-By': 'JEEVAN-MARG-FAILSAFE'
                        }
                    });
                }
            }

            return res;
        } catch (netErr) {
            // Local C++ server is offline or unreachable: fall back to mock engine
            if (isApiCall) {
                console.info(`[Cloud Mock] Local server offline for ${urlStr}. Serving showcase mock data.`);
                const mockRes = handleMockApi(urlStr, method, headers, body);
                return new Response(JSON.stringify(mockRes), {
                    status: 200,
                    statusText: "OK",
                    headers: {
                        'Content-Type': 'application/json',
                        'X-Powered-By': 'JEEVAN-MARG-OFFLINE-FALLBACK'
                    }
                });
            }
            throw netErr;
        }
    };

    console.info('[JEEVAN MARG] Universal fetch interceptor installed successfully.');
})();
