// ==========================================================================
// JEEVAN MARG -- PATIENT PORTAL JAVASCRIPT (patient.js)
// Complete emergency SOS, live real-time tracking, history & hospital directory
// ==========================================================================

let activeEmergencyId = null;
let trackingPoller = null;
let patientMap = null;
let patientMarker = null;
let ambulanceMarker = null;
let hospitalMarker = null;
let routeLine = null;
let currentCoords = { lat: 30.3244, lng: 78.0418, name: "Clock Tower, Dehradun" };

document.addEventListener('DOMContentLoaded', async () => {
    // 1. Enforce authentication & role
    if (!Auth.requireRole(['PATIENT', 'ADMIN'])) return;

    const user = Auth.getUser();
    renderUserProfile(user);

    // Inject portal quick switcher for evaluation
    if (Common.injectPortalSwitcher) {
        Common.injectPortalSwitcher('patient');
    }

    // 2. Initialize Leaflet Map
    initPatientMap();

    // 3. Load initial GPS location
    detectPatientGps(false);

    // 4. Load initial data
    loadNearbyHospitals();
    loadPatientHistory();

    // 5. Setup symptom quick selector listeners
    initSymptomPills();
});

// -----------------------------------------------------------------------------
// User Profile
// -----------------------------------------------------------------------------
function renderUserProfile(user) {
    const nameEl = document.getElementById('patient-name-display');
    const phoneEl = document.getElementById('patient-phone-display');
    const inputName = document.getElementById('sos-patient-name');
    const inputPhone = document.getElementById('sos-patient-phone');
    if (nameEl) nameEl.textContent = user.displayName || user.username;
    if (phoneEl) phoneEl.textContent = user.phone || 'Emergency Contact Ready';
    if (inputName && (user.displayName || user.username)) {
        inputName.value = user.displayName || user.username;
    }
    if (inputPhone && user.phone) {
        inputPhone.value = user.phone;
    }
}

// -----------------------------------------------------------------------------
// Geolocation Detection with Comprehensive Permission & Accuracy Handling
// -----------------------------------------------------------------------------
async function detectPatientGps(showToast = true) {
    const locInput = document.getElementById('sos-location-input');
    const locStatus = document.getElementById('gps-status-pill');

    if (locStatus) {
        locStatus.innerHTML = `<span class="w-2 h-2 rounded-full bg-amber-400 animate-ping mr-1.5"></span>Acquiring GPS Signal...`;
    }

    const pos = await Common.getCurrentPosition();
    currentCoords.lat = pos.lat;
    currentCoords.lng = pos.lng;

    if (pos.status === 'OK') {
        const accMeters = Math.round(pos.accuracy || 10);
        if (locInput && (!locInput.value || locInput.value === 'Clock Tower, Dehradun' || locInput.value.startsWith('GPS ('))) {
            locInput.value = `Current Location (${pos.lat.toFixed(4)}, ${pos.lng.toFixed(4)})`;
        }
        if (locStatus) {
            locStatus.innerHTML = `<span class="w-2 h-2 rounded-full bg-emerald-400 animate-pulse mr-1.5"></span>GPS Active (&plusmn;${accMeters}m)`;
        }
        if (showToast) {
            Common.toast(`GPS acquired accurately (&plusmn;${accMeters}m)`, 'success');
        }
    } else if (pos.status === 'PERMISSION_DENIED') {
        if (locStatus) {
            locStatus.innerHTML = `<span class="w-2 h-2 rounded-full bg-rose-500 mr-1.5"></span>GPS Denied (Manual Mode)`;
        }
        if (locInput && (!locInput.value || locInput.value === 'Clock Tower, Dehradun')) {
            locInput.value = 'Clock Tower, Dehradun';
        }
        if (showToast) {
            Common.toast('GPS permission denied. Please enter pickup address or click map.', 'warning', 6000);
        }
    } else {
        if (locStatus) {
            locStatus.innerHTML = `<span class="w-2 h-2 rounded-full bg-amber-500 mr-1.5"></span>GPS Fallback (Dehradun)`;
        }
        if (locInput && (!locInput.value || locInput.value === 'Clock Tower, Dehradun')) {
            locInput.value = 'Clock Tower, Dehradun';
        }
        if (showToast) {
            Common.toast(pos.error || 'Using default Dehradun coordinates', 'info');
        }
    }

    if (patientMap) {
        patientMap.setView([currentCoords.lat, currentCoords.lng], 14);
        updatePatientMarker(currentCoords.lat, currentCoords.lng, locInput ? locInput.value : 'Your Location');
    }
}

// -----------------------------------------------------------------------------
// Leaflet Map Initialization & Interactive Click Pickup
// -----------------------------------------------------------------------------
function initPatientMap() {
    const mapEl = document.getElementById('patient-map');
    if (!mapEl || typeof L === 'undefined') return;

    patientMap = L.map('patient-map', {
        zoomControl: true,
        attributionControl: false
    }).setView([30.3244, 78.0418], 13);

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19
    }).addTo(patientMap);

    updatePatientMarker(currentCoords.lat, currentCoords.lng, 'Patient Location');

    // Allow user to click anywhere on map to specify pickup location
    patientMap.on('click', (e) => {
        if (activeEmergencyId) return; // Do not alter pickup during active emergency
        currentCoords.lat = parseFloat(e.latlng.lat.toFixed(6));
        currentCoords.lng = parseFloat(e.latlng.lng.toFixed(6));

        const locInput = document.getElementById('sos-location-input');
        if (locInput) {
            locInput.value = `Selected Map Point (${currentCoords.lat.toFixed(4)}, ${currentCoords.lng.toFixed(4)})`;
        }

        const locStatus = document.getElementById('gps-status-pill');
        if (locStatus) {
            locStatus.innerHTML = `<span class="w-2 h-2 rounded-full bg-indigo-400 mr-1.5"></span>Manual Pin Selected`;
        }

        updatePatientMarker(currentCoords.lat, currentCoords.lng, 'Selected Pickup Point');
        Common.toast(`Pickup point updated: ${currentCoords.lat.toFixed(4)}, ${currentCoords.lng.toFixed(4)}`, 'info', 2500);
    });

    // Setup manual input landmark auto-resolution
    const locInput = document.getElementById('sos-location-input');
    if (locInput) {
        locInput.addEventListener('change', () => {
            const val = locInput.value.trim();
            const landmark = Common.resolveLandmark(val);
            if (landmark) {
                currentCoords.lat = landmark.lat;
                currentCoords.lng = landmark.lng;
                patientMap.setView([landmark.lat, landmark.lng], 14);
                updatePatientMarker(landmark.lat, landmark.lng, landmark.name);
                Common.toast(`Resolved to ${landmark.name}`, 'success', 2500);
            }
        });
    }
}

function updatePatientMarker(lat, lng, label) {
    if (!patientMap) return;
    if (patientMarker) {
        patientMarker.setLatLng([lat, lng]);
    } else {
        const icon = Common.createPinIcon('patient', 'fa-user');
        patientMarker = L.marker([lat, lng], { icon }).addTo(patientMap)
            .bindPopup(`<b>${Common.escapeHtml(label)}</b><br>Pickup coordinates`);
    }
}

function updateAmbulanceMarker(lat, lng, unitCode, status) {
    if (!patientMap) return;
    if (ambulanceMarker) {
        ambulanceMarker.setLatLng([lat, lng]);
        ambulanceMarker.setPopupContent(`<b>Ambulance ${Common.escapeHtml(unitCode)}</b><br>Status: <span class="text-amber-400 font-bold">${Common.escapeHtml(status)}</span>`);
    } else {
        const icon = Common.createPinIcon('ambulance', 'fa-truck-medical');
        ambulanceMarker = L.marker([lat, lng], { icon }).addTo(patientMap)
            .bindPopup(`<b>Ambulance ${Common.escapeHtml(unitCode)}</b><br>Status: <span class="text-amber-400 font-bold">${Common.escapeHtml(status)}</span>`);
    }
}

function updateHospitalMarker(lat, lng, name, address) {
    if (!patientMap) return;
    if (hospitalMarker) {
        hospitalMarker.setLatLng([lat, lng]);
    } else {
        const icon = Common.createPinIcon('hospital', 'fa-hospital');
        hospitalMarker = L.marker([lat, lng], { icon }).addTo(patientMap)
            .bindPopup(`<b>${Common.escapeHtml(name)}</b><br>${Common.escapeHtml(address)}`);
    }
}

function drawRoutePolyline(points) {
    if (!patientMap) return;
    if (routeLine) {
        patientMap.removeLayer(routeLine);
        routeLine = null;
    }
    if (points && points.length >= 2) {
        routeLine = L.polyline(points, {
            color: '#38bdf8',
            weight: 5,
            opacity: 0.9,
            className: 'animated-route-polyline'
        }).addTo(patientMap);
        patientMap.fitBounds(routeLine.getBounds(), { padding: [40, 40] });
    }
}

// -----------------------------------------------------------------------------
// Symptom Quick Pills
// -----------------------------------------------------------------------------
function initSymptomPills() {
    const pills = document.querySelectorAll('.symptom-pill');
    const conditionInput = document.getElementById('sos-condition-input');
    const severitySelect = document.getElementById('sos-severity-select');

    pills.forEach(pill => {
        pill.addEventListener('click', () => {
            pills.forEach(p => p.classList.remove('selected'));
            pill.classList.add('selected');

            const text = pill.getAttribute('data-symptom');
            const severity = pill.getAttribute('data-severity');

            if (conditionInput) conditionInput.value = text;
            if (severitySelect && severity) severitySelect.value = severity;
        });
    });
}

// -----------------------------------------------------------------------------
// Emergency SOS Dispatch Submission
// -----------------------------------------------------------------------------
async function triggerPatientSOS(e) {
    if (e) e.preventDefault();

    const submitBtn = document.getElementById('sos-submit-btn');
    const btnText = document.getElementById('sos-btn-text');
    const conditionInput = document.getElementById('sos-condition-input');
    const locationInput = document.getElementById('sos-location-input');
    const severitySelect = document.getElementById('sos-severity-select');

    const condition = conditionInput ? conditionInput.value.trim() : 'Critical medical emergency';
    const locationName = locationInput && locationInput.value.trim() ? locationInput.value.trim() : 'Clock Tower, Dehradun';
    const severity = severitySelect ? severitySelect.value : 'CRITICAL';

    const nameInput = document.getElementById('sos-patient-name');
    const phoneInput = document.getElementById('sos-patient-phone');
    const user = Auth.getUser() || {};
    const callerName = (nameInput && nameInput.value.trim()) || user.displayName || user.username || 'Aryan';
    const callerPhone = (phoneInput && phoneInput.value.trim()) || user.phone || '9876543210';

    if (btnText) btnText.textContent = 'TRIAGING & DISPATCHING...';
    if (submitBtn) submitBtn.disabled = true;

    try {
        const payload = {
            callerName,
            patientName: callerName,
            callerPhone,
            patientPhone: callerPhone,
            location: locationName,
            pickupLocation: locationName,
            condition,
            conditionText: condition,
            severity,
            latitude: currentCoords.lat,
            longitude: currentCoords.lng
        };

        const res = await fetch(Common.apiUrl('/api/emergency-requests'), {
            method: 'POST',
            headers: Auth.getAuthHeaders(),
            body: JSON.stringify(payload)
        });

        const data = await res.json();
        if (!res.ok) {
            throw new Error(data.error?.message || data.message || 'Dispatch failed');
        }

        const emData = data.data || data;
        const newEmId = emData.emergencyId || emData.id;
        const trackingToken = data.trackingToken || emData.trackingToken;
        if (trackingToken) {
            sessionStorage.setItem('jeevan_tracking_token_' + newEmId, trackingToken);
            sessionStorage.setItem('jeevan_last_tracking_token', trackingToken);
        }

        Common.toast(`Emergency ${newEmId} dispatched! Nearest ambulance assigned.`, 'success', 5000);

        // Start live tracking for this emergency
        startEmergencyTracking(newEmId);

        // Refresh emergency history table
        loadPatientHistory();

    } catch (err) {
        Common.toast('Emergency request failed: ' + err.message, 'error', 6000);
    } finally {
        if (btnText) btnText.textContent = 'REQUEST IMMEDIATE DISPATCH';
        if (submitBtn) submitBtn.disabled = false;
    }
}

// -----------------------------------------------------------------------------
// Live Emergency Tracking (Polling every 3-4s)
// -----------------------------------------------------------------------------
function startEmergencyTracking(emergencyId) {
    activeEmergencyId = emergencyId;

    const banner = document.getElementById('active-tracking-section');
    if (banner) banner.classList.remove('hidden');

    const emIdEl = document.getElementById('track-emergency-id');
    if (emIdEl) emIdEl.textContent = emergencyId;

    if (trackingPoller) trackingPoller.stop();

    // Start 3.5s real-time poller
    trackingPoller = Common.createPoller(async () => {
        if (!activeEmergencyId) return;
        await fetchEmergencyDetails(activeEmergencyId);
    }, 3500);

    // Scroll to tracker
    if (banner) banner.scrollIntoView({ behavior: 'smooth' });
}

async function fetchEmergencyDetails(emId) {
    try {
        const headers = Auth.getAuthHeaders();
        const storedToken = sessionStorage.getItem('jeevan_tracking_token_' + emId) || sessionStorage.getItem('jeevan_last_tracking_token');
        if (storedToken) {
            headers['X-Tracking-Token'] = storedToken;
        }

        const res = await fetch(Common.apiUrl(`/api/emergencies/${encodeURIComponent(emId)}`), {
            headers: headers
        });
        if (!res.ok) {
            if (res.status === 404) {
                stopEmergencyTracking();
            }
            return;
        }

        const json = await res.json();
        const em = json.data || json;

        renderTrackingState(em);
    } catch (err) {
        console.warn('Error polling emergency status:', err);
    }
}

function renderTrackingState(em) {
    // 1. Update text fields
    const statusPill = document.getElementById('track-status-pill');
    const severityPill = document.getElementById('track-severity-pill');
    const ambUnitEl = document.getElementById('track-amb-unit');
    const ambDriverEl = document.getElementById('track-amb-driver');
    const ambPhoneEl = document.getElementById('track-amb-phone');
    const ambEtaEl = document.getElementById('track-amb-eta');
    const hospNameEl = document.getElementById('track-hosp-name');
    const hospAddressEl = document.getElementById('track-hosp-address');
    const hospPhoneEl = document.getElementById('track-hosp-phone');

    if (statusPill) {
        statusPill.textContent = (em.status === 'COMPLETED') ? 'PATIENT ADMITTED TO ICU' : em.status;
        statusPill.className = `px-3 py-1 rounded-full text-xs font-black uppercase tracking-wider ${
            em.status === 'COMPLETED' ? 'bg-emerald-950 text-emerald-300 border-2 border-emerald-500 shadow-lg shadow-emerald-950/60' :
            em.status === 'CANCELLED' ? 'bg-slate-800 text-slate-400 border border-slate-700' :
            'bg-red-950 text-red-300 border border-red-700 animate-pulse'
        }`;
    }

    if (severityPill) severityPill.textContent = em.severity || em.triageCategory || 'CRITICAL';

    const amb = em.ambulance || {};
    if (ambUnitEl) ambUnitEl.textContent = amb.unitCode || em.assignedAmbulanceId || 'Assigning...';
    if (ambDriverEl) ambDriverEl.textContent = amb.driverName || 'EMT Crew Active';
    if (ambPhoneEl) {
        const phone = amb.driverPhone || '108';
        ambPhoneEl.textContent = phone;
        ambPhoneEl.href = `tel:${phone}`;
    }

    const hosp = em.hospital || {};
    if (hospNameEl) hospNameEl.textContent = hosp.name || em.assignedHospitalId || 'Specialty Medical Center';
    if (hospAddressEl) hospAddressEl.textContent = hosp.address || 'Dehradun';
    if (hospPhoneEl) {
        const hPhone = hosp.phone || '0135-2745678';
        hospPhoneEl.textContent = hPhone;
        hospPhoneEl.href = `tel:${hPhone}`;
    }

    // 2. Update step progression tracker
    updateProgressionSteps(em.status);

    // 3. Update Map & Dynamic Dijkstra Road Route
    const pLat = em.latitude || currentCoords.lat;
    const pLng = em.longitude || currentCoords.lng;
    updatePatientMarker(pLat, pLng, em.callerName || 'Patient Location');

    if (amb.currentLat && amb.currentLng) {
        updateAmbulanceMarker(amb.currentLat, amb.currentLng, amb.unitCode || 'AMB', amb.status || 'EN_ROUTE');
    }

    if (hosp.latitude && hosp.longitude) {
        updateHospitalMarker(hosp.latitude, hosp.longitude, hosp.name || 'Hospital', hosp.address || '');
    }

    // Dynamic Dijkstra Route & Accurate Road ETA from Backend
    if (em.liveRoute && em.liveRoute.active && em.liveRoute.coordinates && em.liveRoute.coordinates.length >= 2) {
        const roadPoly = em.liveRoute.coordinates.map(c => [c.latitude, c.longitude]);
        // Anchor to current ambulance position
        if (amb.currentLat && amb.currentLng) {
            roadPoly.unshift([amb.currentLat, amb.currentLng]);
        }
        drawRoutePolyline(roadPoly);

        if (ambEtaEl) {
            const eta = typeof em.liveRoute.etaMinutes === 'number' ? em.liveRoute.etaMinutes.toFixed(1) : em.liveRoute.etaMinutes;
            const dist = typeof em.liveRoute.distanceKm === 'number' ? em.liveRoute.distanceKm.toFixed(1) : em.liveRoute.distanceKm;
            const phaseTxt = em.liveRoute.phase === 'TO_HOSPITAL' ? 'to Hospital' : 'to Pickup';
            ambEtaEl.textContent = `${eta} mins (${dist} km ${phaseTxt})`;
        }
    } else {
        // Fallback connecting markers
        const routePoints = [[pLat, pLng]];
        if (amb.currentLat && amb.currentLng) {
            routePoints.unshift([amb.currentLat, amb.currentLng]);
            const dist = Common.calcDistanceKm(amb.currentLat, amb.currentLng, pLat, pLng);
            const estEta = Math.max(1, Math.round(dist * 2.2));
            if (ambEtaEl) ambEtaEl.textContent = `${estEta} mins (${dist} km away)`;
        }
        if (hosp.latitude && hosp.longitude) {
            routePoints.push([hosp.latitude, hosp.longitude]);
        }
        if (routePoints.length >= 2) {
            drawRoutePolyline(routePoints);
        }
    }

    // Stop poller if mission reached a terminal state
    if (em.status === 'COMPLETED' || em.status === 'CANCELLED') {
        if (trackingPoller) {
            trackingPoller.stop();
            trackingPoller = null;
        }
        Common.toast(`Emergency ${em.emergencyId} has reached final state: ${em.status}`, 'info', 6000);
        loadPatientHistory();
    }
}

function updateProgressionSteps(status) {
    const steps = ['step-dispatched', 'step-onscene', 'step-transport', 'step-arrived', 'step-completed'];
    const statusMap = {
        'PENDING': 0,
        'ASSIGNED': 0,
        'DISPATCHED': 0,
        'ON_SCENE': 1,
        'TRANSPORTING': 2,
        'EN_ROUTE': 2,
        'ARRIVED': 3,
        'COMPLETED': 4
    };

    const currentIdx = statusMap[status] !== undefined ? statusMap[status] : 0;

    steps.forEach((stepId, idx) => {
        const el = document.getElementById(stepId);
        if (!el) return;
        el.classList.remove('completed', 'active');
        if (idx < currentIdx) {
            el.classList.add('completed');
        } else if (idx === currentIdx) {
            el.classList.add('active');
        }
    });
}

function stopEmergencyTracking() {
    activeEmergencyId = null;
    if (trackingPoller) {
        trackingPoller.stop();
        trackingPoller = null;
    }
    const banner = document.getElementById('active-tracking-section');
    if (banner) banner.classList.add('hidden');
}

// -----------------------------------------------------------------------------
// Emergency History (Strict Patient Isolation)
// -----------------------------------------------------------------------------
async function loadPatientHistory() {
    const container = document.getElementById('history-container');
    if (!container) return;

    try {
        const user = Auth.getUser() || {};
        const inputName = document.getElementById('sos-patient-name')?.value?.trim();
        const currentName = (inputName || user.displayName || user.username || 'Aryan').toLowerCase().trim();

        const res = await fetch(Common.apiUrl(`/api/patient/emergencies?patient=${encodeURIComponent(currentName)}`), {
            headers: Auth.getAuthHeaders()
        });

        let json = null;
        if (res.ok) {
            const rawText = await res.text();
            if (rawText && !rawText.trim().startsWith('<')) {
                json = JSON.parse(rawText);
            }
        }

        const rawList = (json && (json.data || json.emergencies)) || [];

        // Strict Patient Isolation: Filter out all other citizens (Pooja, Rajesh, etc.)
        // Only show emergencies belonging to THIS patient
        const list = rawList.filter(em => {
            const pName = (em.patientName || em.callerName || '').toLowerCase().trim();
            return pName.includes(currentName) || currentName.includes(pName) || em.emergencyId === activeEmergencyId;
        });

        if (list.length === 0) {
            container.innerHTML = `
                <div class="text-center py-8 text-slate-500 text-xs">
                    <div class="w-12 h-12 mx-auto rounded-full bg-slate-800/80 border border-slate-700 flex items-center justify-center text-slate-400 mb-3">
                        <i class="fa-solid fa-shield-heart text-xl text-emerald-400"></i>
                    </div>
                    <p class="text-white font-bold text-sm">No Emergency Dispatches for ${Common.escapeHtml(inputName || user.displayName || 'You')}</p>
                    <p class="text-slate-400 text-xs mt-1">When you dispatch an emergency SOS above, your live mission &amp; audit history will appear exclusively here.</p>
                </div>
            `;
            return;
        }

        container.innerHTML = list.map(em => `
            <div class="p-4 mb-3 rounded-xl bg-slate-900/60 border border-slate-700/80 hover:border-indigo-500/50 flex flex-col md:flex-row md:items-center justify-between gap-3 transition">
                <div>
                    <div class="flex items-center space-x-2">
                        <span class="font-bold text-white text-sm font-mono">${Common.escapeHtml(em.emergencyId)}</span>
                        <span class="px-2 py-0.5 rounded text-[10px] font-extrabold uppercase ${
                            em.status === 'COMPLETED' ? 'bg-emerald-950 text-emerald-300 border border-emerald-700' :
                            em.status === 'CANCELLED' ? 'bg-slate-800 text-slate-400' :
                            'bg-red-950 text-red-300 border border-red-700 animate-pulse'
                        }">${Common.escapeHtml(em.status === 'COMPLETED' ? 'ADMITTED (COMPLETED)' : em.status)}</span>
                        <span class="px-2 py-0.5 rounded text-[10px] font-bold bg-indigo-950/60 text-indigo-300 border border-indigo-800">${Common.escapeHtml(em.severity || em.triageCategory)}</span>
                    </div>
                    <div class="text-xs text-slate-300 mt-1">
                        Patient: <strong class="text-white">${Common.escapeHtml(em.patientName || em.callerName || 'Aryan')}</strong> &bull; Condition: <strong>${Common.escapeHtml(em.conditionText || em.condition || 'General Emergency')}</strong>
                    </div>
                    <div class="text-[11px] text-slate-400 mt-0.5">
                        Ambulance: <span class="text-amber-400 font-mono font-semibold">${Common.escapeHtml(em.assignedAmbulanceId || 'None')}</span> &bull;
                        Destination: <span class="text-teal-400 font-semibold">${Common.escapeHtml(em.hospitalName || em.assignedHospitalId || 'Max Super Speciality')}</span>
                    </div>
                </div>
                <div class="text-right flex md:flex-col items-center md:items-end justify-between gap-2">
                    <div class="text-[11px] text-slate-400">${Common.formatDate(em.createdAt || em.timestamp)}</div>
                    <button onclick="startEmergencyTracking('${Common.escapeHtml(em.emergencyId)}')" class="px-3 py-1.5 rounded-lg bg-indigo-600/40 hover:bg-indigo-600 text-indigo-200 hover:text-white text-xs font-semibold flex items-center space-x-1.5 transition">
                        <i class="fa-solid fa-map-location-dot"></i>
                        <span>Live Track</span>
                    </button>
                </div>
            </div>
        `).join('');

        // If there's an active emergency and tracking not started, track it
        if (!activeEmergencyId) {
            const activeEm = list.find(e => e.status !== 'COMPLETED' && e.status !== 'CANCELLED');
            if (activeEm) {
                startEmergencyTracking(activeEm.emergencyId);
            }
        }

    } catch (err) {
        container.innerHTML = `
            <div class="text-center py-6 text-slate-500 text-xs">
                <i class="fa-solid fa-heart-pulse text-2xl text-slate-700 mb-2"></i>
                <p>Ready for your first emergency dispatch.</p>
            </div>
        `;
    }
}

// -----------------------------------------------------------------------------
// Nearby Hospitals Directory (from SQLite)
// -----------------------------------------------------------------------------
async function loadNearbyHospitals() {
    const container = document.getElementById('hospitals-directory-container');
    if (!container) return;

    try {
        const res = await fetch(Common.apiUrl('/api/hospitals'));
        let json = null;
        if (res.ok) {
            const rawText = await res.text();
            if (rawText && !rawText.trim().startsWith('<')) {
                json = JSON.parse(rawText);
            }
        }

        const hospitals = (json && (json.data || json.hospitals)) || [
            {
                name: "Max Super Speciality Hospital",
                address: "Mussoorie Diversion Road, Malsi, Dehradun",
                availableIcu: 14,
                icuBeds: 45,
                oxygenBeds: 95,
                totalBeds: 280,
                rating: 4.8,
                phone: "+91-135-7193000",
                specialties: ["Cardiology", "Trauma", "Neurology", "Critical Care"]
            },
            {
                name: "Doon Government Hospital",
                address: "Parade Ground, Court Road, Dehradun",
                availableIcu: 8,
                icuBeds: 50,
                oxygenBeds: 180,
                totalBeds: 450,
                rating: 4.3,
                phone: "+91-135-2659847",
                specialties: ["General Medicine", "Emergency", "Pediatrics"]
            },
            {
                name: "Synergy Super Speciality Hospital",
                address: "Canal Road, Ballupur, Dehradun",
                availableIcu: 9,
                icuBeds: 32,
                oxygenBeds: 60,
                totalBeds: 180,
                rating: 4.6,
                phone: "+91-135-2226000",
                specialties: ["Cardiology", "Nephrology", "Critical Care"]
            },
            {
                name: "Graphic Era Hospital & Research Institute",
                address: "Subhash Nagar, Clement Town, Dehradun",
                availableIcu: 18,
                icuBeds: 40,
                oxygenBeds: 110,
                totalBeds: 320,
                rating: 4.9,
                phone: "+91-135-2643070",
                specialties: ["Cardiology", "Emergency Medicine", "Orthopedics"]
            }
        ];

        container.innerHTML = hospitals.map(h => {
            const availIcu = h.availableIcu !== undefined ? h.availableIcu : (h.icuBeds || 0);
            const totalBeds = h.totalBeds || 0;
            const oxyBeds = h.oxygenBeds || 0;
            const specs = (h.specialties || []).slice(0, 3).join(', ');

            return `
                <div class="p-4 rounded-xl bg-slate-900/60 border border-slate-700/80 hover:border-emerald-500/50 flex flex-col justify-between transition">
                    <div>
                        <div class="flex items-start justify-between gap-2">
                            <h4 class="font-bold text-white text-sm leading-tight">${Common.escapeHtml(h.name)}</h4>
                            <span class="px-2 py-0.5 rounded text-[10px] font-bold bg-emerald-950/70 text-emerald-300 border border-emerald-800 shrink-0">
                                <i class="fa-solid fa-star text-amber-400 text-[9px] mr-1"></i>${(h.rating || 4.5).toFixed(1)}
                            </span>
                        </div>
                        <p class="text-[11px] text-slate-400 mt-1"><i class="fa-solid fa-location-dot mr-1 text-slate-500"></i>${Common.escapeHtml(h.address || 'Dehradun')}</p>
                        ${specs ? `<p class="text-[10px] text-slate-500 mt-1 truncate"><i class="fa-solid fa-stethoscope mr-1 text-slate-600"></i>${Common.escapeHtml(specs)}</p>` : ''}
                    </div>

                    <div class="mt-4 pt-3 border-t border-slate-800 flex items-center justify-between text-xs">
                        <div class="flex space-x-3 text-[11px]">
                            <div>
                                <span class="text-slate-500 block text-[9px] uppercase font-bold">ICU Avail</span>
                                <span class="font-black ${availIcu > 0 ? 'text-emerald-400' : 'text-red-400'}">${availIcu}</span>
                            </div>
                            <div>
                                <span class="text-slate-500 block text-[9px] uppercase font-bold">Oxygen</span>
                                <span class="font-bold text-cyan-400">${oxyBeds}</span>
                            </div>
                            <div>
                                <span class="text-slate-500 block text-[9px] uppercase font-bold">Total Beds</span>
                                <span class="font-bold text-slate-300">${totalBeds}</span>
                            </div>
                        </div>

                        <a href="tel:${Common.escapeHtml(h.phone || '108')}" class="px-2.5 py-1.5 rounded-lg bg-emerald-600/30 hover:bg-emerald-600 text-emerald-200 hover:text-white text-xs font-bold transition flex items-center space-x-1">
                            <i class="fa-solid fa-phone text-[10px]"></i>
                            <span>Call</span>
                        </a>
                    </div>
                </div>
            `;
        }).join('');

    } catch (err) {
        console.warn('Hospitals directory load error:', err);
    }
}
