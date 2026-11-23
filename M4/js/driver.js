// ==========================================================================
// JEEVAN MARG -- DRIVER PORTAL JAVASCRIPT (driver.js)
// Real-time mission dispatch, step-by-step lifecycle actions, GPS broadcast & navigation
// ==========================================================================

let driverAmbulance = null;
let activeMission = null;
let missionPoller = null;
let gpsWatchId = null;
let isGpsBroadcasting = false;
let simulationInterval = null;
let simulationIndex = 0;
let lastKnownMissionId = null;
let driverMap = null;
let ambMarker = null;
let patientMarker = null;
let hospitalMarker = null;
let routeLine = null;

document.addEventListener('DOMContentLoaded', async () => {
    // 1. Verify Authentication & Role
    if (!Auth.requireRole(['DRIVER', 'ADMIN'])) return;

    const user = Auth.getUser();
    renderDriverHeader(user);

    // Inject portal quick switcher for evaluation convenience
    if (Common.injectPortalSwitcher) {
        Common.injectPortalSwitcher('driver');
    }

    // 2. Initialize Cockpit Navigation Map
    initDriverMap();

    // 3. Load Ambulance Information
    await loadAmbulanceInfo();

    // 4. Start Mission Poller (every 3s)
    missionPoller = Common.createPoller(async () => {
        await pollAssignedMission();
    }, 3000);
});

// -----------------------------------------------------------------------------
// Driver Header & Profile
// -----------------------------------------------------------------------------
function renderDriverHeader(user) {
    const nameEl = document.getElementById('driver-name-display');
    const badgeEl = document.getElementById('driver-vehicle-badge');
    if (nameEl) nameEl.textContent = user.displayName || user.username;
    if (badgeEl && user.linkedAmbulanceId) {
        badgeEl.textContent = user.linkedAmbulanceId;
    }
}

let driverRouteCasing = null;

// -----------------------------------------------------------------------------
// Leaflet Navigation Map
// -----------------------------------------------------------------------------
function initDriverMap() {
    const mapEl = document.getElementById('driver-map');
    if (!mapEl || typeof L === 'undefined') return;

    driverMap = L.map('driver-map', {
        zoomControl: true,
        attributionControl: false
    }).setView([30.3244, 78.0418], 13);

    const osmDetailed = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '&copy; OpenStreetMap'
    });
    const cartoVoyager = L.tileLayer('https://{s}.basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}{r}.png', {
        maxZoom: 19,
        subdomains: 'abcd',
        attribution: '&copy; CartoDB'
    });

    osmDetailed.addTo(driverMap);

    const baseMaps = {
        "High-Detail Roads (OSM)": osmDetailed,
        "Clean Street Map (Carto)": cartoVoyager
    };
    L.control.layers(baseMaps, null, { position: 'topright' }).addTo(driverMap);
}

function updateMapMarkers(ambLat, ambLng, pLat, pLng, hLat, hLng, hName) {
    if (!driverMap) return;

    const points = [];

    // 1. Ambulance Marker
    if (ambLat && ambLng) {
        if (ambMarker) {
            ambMarker.setLatLng([ambLat, ambLng]);
        } else {
            const icon = Common.createPinIcon('ambulance', 'fa-truck-medical');
            ambMarker = L.marker([ambLat, ambLng], { icon }).addTo(driverMap)
                .bindPopup('<b>My Ambulance</b><br>Current Location');
        }
        points.push([ambLat, ambLng]);
    }

    // 2. Patient Pickup Marker
    if (pLat && pLng) {
        if (patientMarker) {
            patientMarker.setLatLng([pLat, pLng]);
        } else {
            const icon = Common.createPinIcon('patient', 'fa-location-dot');
            patientMarker = L.marker([pLat, pLng], { icon }).addTo(driverMap)
                .bindPopup('<b>Patient Pickup Location</b>');
        }
        points.push([pLat, pLng]);
    } else if (patientMarker) {
        driverMap.removeLayer(patientMarker);
        patientMarker = null;
    }

    // 3. Hospital Marker
    if (hLat && hLng) {
        if (hospitalMarker) {
            hospitalMarker.setLatLng([hLat, hLng]);
        } else {
            const icon = Common.createPinIcon('hospital', 'fa-hospital');
            hospitalMarker = L.marker([hLat, hLng], { icon }).addTo(driverMap)
                .bindPopup(`<b>${Common.escapeHtml(hName || 'Hospital')}</b><br>Destination`);
        }
        points.push([hLat, hLng]);
    } else if (hospitalMarker) {
        driverMap.removeLayer(hospitalMarker);
        hospitalMarker = null;
    }

    // 4. Draw route: Use Dijkstra road polyline if provided, else fallback to straight lines
    if (routeLine) {
        driverMap.removeLayer(routeLine);
        routeLine = null;
    }
    if (driverRouteCasing) {
        driverMap.removeLayer(driverRouteCasing);
        driverRouteCasing = null;
    }

    if (activeMission && activeMission.liveRoute && activeMission.liveRoute.coordinates && activeMission.liveRoute.coordinates.length >= 2) {
        const roadPoly = activeMission.liveRoute.coordinates.map(c => [c.latitude, c.longitude]);
        if (ambLat && ambLng) {
            roadPoly.unshift([ambLat, ambLng]);
        }
        driverRouteCasing = L.polyline(roadPoly, {
            color: '#0f172a',
            weight: 8,
            opacity: 0.85
        }).addTo(driverMap);
        routeLine = L.polyline(roadPoly, {
            color: '#f59e0b',
            weight: 5,
            opacity: 0.95,
            className: 'animated-route-polyline'
        }).addTo(driverMap);
        driverMap.fitBounds(routeLine.getBounds(), { padding: [50, 50] });
    } else if (points.length >= 2) {
        driverRouteCasing = L.polyline(points, {
            color: '#0f172a',
            weight: 8,
            opacity: 0.85
        }).addTo(driverMap);
        routeLine = L.polyline(points, {
            color: '#f59e0b',
            weight: 5,
            opacity: 0.9,
            className: 'animated-route-polyline'
        }).addTo(driverMap);
        driverMap.fitBounds(routeLine.getBounds(), { padding: [50, 50] });
    } else if (points.length === 1) {
        driverMap.setView(points[0], 14);
    }
}

// -----------------------------------------------------------------------------
// Load Ambulance Telemetry & Details
// -----------------------------------------------------------------------------
async function loadAmbulanceInfo() {
    try {
        const res = await fetch(Common.apiUrl('/api/driver/ambulance'), {
            headers: Auth.getAuthHeaders()
        });
        if (!res.ok) throw new Error('Failed to load ambulance details');

        const json = await res.json();
        driverAmbulance = json.data || json;

        renderAmbulanceTelemetry(driverAmbulance);

        if (driverMap && driverAmbulance.currentLat && driverAmbulance.currentLng) {
            driverMap.setView([driverAmbulance.currentLat, driverAmbulance.currentLng], 14);
            updateMapMarkers(driverAmbulance.currentLat, driverAmbulance.currentLng);
        }

    } catch (err) {
        console.error('Ambulance info load error:', err);
        Common.toast('Failed to load assigned vehicle: ' + err.message, 'error');
    }
}

function renderAmbulanceTelemetry(amb) {
    const idEl = document.getElementById('amb-id-val');
    const codeEl = document.getElementById('amb-code-val');
    const typeEl = document.getElementById('amb-type-val');
    const stationEl = document.getElementById('amb-station-val');
    const statusBadge = document.getElementById('amb-status-badge');
    const availBtn = document.getElementById('toggle-avail-btn');

    if (idEl) idEl.textContent = amb.ambulanceId || amb.id || '-';
    if (codeEl) codeEl.textContent = amb.unitCode || '-';
    if (typeEl) typeEl.textContent = amb.type || 'BLS';
    if (stationEl) stationEl.textContent = amb.stationLocationId || 'Central Station';

    const status = amb.status || 'READY';
    if (statusBadge) {
        statusBadge.textContent = status;
        statusBadge.className = `px-3 py-1 rounded-full text-xs font-black uppercase tracking-wider ${
            status === 'READY' ? 'bg-emerald-950 text-emerald-300 border border-emerald-700' :
            status === 'MAINTENANCE' ? 'bg-amber-950 text-amber-300 border border-amber-700' :
            'bg-red-950 text-red-300 border border-red-700 animate-pulse'
        }`;
    }

    if (availBtn) {
        if (amb.assignedEmergencyId) {
            availBtn.disabled = true;
            availBtn.className = "px-3 py-1.5 rounded-lg bg-slate-800 text-slate-500 text-xs font-bold cursor-not-allowed";
            availBtn.title = "Cannot change availability while mission is active";
            availBtn.textContent = "MISSION IN PROGRESS";
        } else {
            availBtn.disabled = false;
            if (status === 'READY') {
                availBtn.className = "px-3 py-1.5 rounded-lg bg-amber-600/30 hover:bg-amber-600 text-amber-200 hover:text-white text-xs font-bold transition";
                availBtn.textContent = "SET TO MAINTENANCE";
            } else {
                availBtn.className = "px-3 py-1.5 rounded-lg bg-emerald-600/30 hover:bg-emerald-600 text-emerald-200 hover:text-white text-xs font-bold transition";
                availBtn.textContent = "SET TO READY";
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Toggle Availability (READY vs MAINTENANCE)
// -----------------------------------------------------------------------------
async function toggleAmbulanceAvailability() {
    if (!driverAmbulance) return;
    if (driverAmbulance.assignedEmergencyId) {
        Common.toast('Cannot alter availability while an active emergency is assigned.', 'warning');
        return;
    }

    const newStatus = (driverAmbulance.status === 'READY') ? 'MAINTENANCE' : 'READY';

    try {
        const res = await fetch(Common.apiUrl('/api/driver/ambulance/status'), {
            method: 'PATCH',
            headers: Auth.getAuthHeaders(),
            body: JSON.stringify({ status: newStatus })
        });

        const data = await res.json();
        if (!res.ok) throw new Error(data.error?.message || data.message || 'Update failed');

        Common.toast(`Vehicle availability set to ${newStatus}`, 'success');
        await loadAmbulanceInfo();

    } catch (err) {
        Common.toast('Status update failed: ' + err.message, 'error');
    }
}

// -----------------------------------------------------------------------------
// Poll Assigned Mission
// -----------------------------------------------------------------------------
async function pollAssignedMission() {
    try {
        const res = await fetch(Common.apiUrl('/api/driver/assigned-emergency'), {
            headers: Auth.getAuthHeaders()
        });
        if (!res.ok) return;

        const json = await res.json();
        const mission = json.data;

        if (!mission || !mission.emergencyId || mission.status === 'COMPLETED' || mission.status === 'CANCELLED') {
            activeMission = null;
            lastKnownMissionId = null;
            renderActiveMissionCard(null);
            return;
        }

        if (mission && mission.emergencyId && mission.emergencyId !== lastKnownMissionId) {
            lastKnownMissionId = mission.emergencyId;
            Common.playEmergencyAlertSound();
            Common.toast(`NEW EMERGENCY MISSION ASSIGNED: ${mission.emergencyId} (${mission.patientName || 'Patient'})`, 'error', 6000);
        }

        activeMission = mission;
        renderActiveMissionCard(mission);

    } catch (err) {
        console.warn('Mission poll error:', err);
    }
}

function renderActiveMissionCard(mission) {
    const standByContainer = document.getElementById('mission-standby-view');
    const activeContainer = document.getElementById('mission-active-view');

    if (!mission || !mission.emergencyId || mission.status === 'COMPLETED' || mission.status === 'CANCELLED') {
        if (standByContainer) standByContainer.classList.remove('hidden');
        if (activeContainer) activeContainer.classList.add('hidden');
        if (patientMarker && driverMap) { driverMap.removeLayer(patientMarker); patientMarker = null; }
        if (hospitalMarker && driverMap) { driverMap.removeLayer(hospitalMarker); hospitalMarker = null; }
        if (routeLine && driverMap) { driverMap.removeLayer(routeLine); routeLine = null; }
        if (driverRouteCasing && driverMap) { driverMap.removeLayer(driverRouteCasing); driverRouteCasing = null; }
        return;
    }

    if (standByContainer) standByContainer.classList.add('hidden');
    if (activeContainer) activeContainer.classList.remove('hidden');

    // Fill mission details
    const emIdEl = document.getElementById('mission-em-id');
    const sevBadge = document.getElementById('mission-sev-badge');
    const statusBadge = document.getElementById('mission-status-badge');
    const patientNameEl = document.getElementById('mission-patient-name');
    const patientPhoneEl = document.getElementById('mission-patient-phone');
    const locationEl = document.getElementById('mission-location');
    const conditionEl = document.getElementById('mission-condition');
    const hospNameEl = document.getElementById('mission-hosp-name');
    const hospAddressEl = document.getElementById('mission-hosp-address');
    const hospPhoneEl = document.getElementById('mission-hosp-phone');

    if (emIdEl) emIdEl.textContent = mission.emergencyId;
    if (sevBadge) sevBadge.textContent = mission.severity || mission.triageCategory || 'CRITICAL';
    if (statusBadge) statusBadge.textContent = mission.status;

    if (patientNameEl) patientNameEl.textContent = mission.patientName || mission.callerName || 'Patient';
    if (patientPhoneEl) {
        const phone = mission.patientPhone || mission.callerPhone || '108';
        patientPhoneEl.textContent = phone;
        patientPhoneEl.href = `tel:${phone}`;
    }
    if (locationEl) locationEl.textContent = mission.pickupLocation || mission.location || 'Dehradun';
    if (conditionEl) conditionEl.textContent = mission.condition || mission.conditionText || 'Emergency';

    // Populate Relative / Emergency Contact if provided
    const relCard = document.getElementById('mission-relative-card');
    const relNameEl = document.getElementById('mission-relative-name');
    const relTypeEl = document.getElementById('mission-relative-rel');
    const relPhoneEl = document.getElementById('mission-relative-phone');

    if (relCard) {
        if (mission.relativeName || mission.relativePhone) {
            relCard.classList.remove('hidden');
            if (relNameEl) relNameEl.textContent = mission.relativeName || 'Emergency Contact';
            if (relTypeEl) relTypeEl.textContent = mission.relativeRelation ? `(${mission.relativeRelation})` : '';
            if (relPhoneEl) {
                const rPhone = mission.relativePhone || '108';
                relPhoneEl.href = `tel:${rPhone}`;
                relPhoneEl.title = `Call ${mission.relativeName || 'Relative'} (${rPhone})`;
            }
        } else {
            relCard.classList.add('hidden');
        }
    }

    const hosp = mission.hospital || {};
    if (hospNameEl) hospNameEl.textContent = hosp.name || mission.assignedHospitalId || 'Hospital';
    if (hospAddressEl) hospAddressEl.textContent = hosp.address || 'Dehradun';
    if (hospPhoneEl) {
        const hPhone = hosp.phone || '0135-2745678';
        hospPhoneEl.textContent = hPhone;
        hospPhoneEl.href = `tel:${hPhone}`;
    }

    // Render Action Progression Buttons based on current emergency status
    renderLifecycleActions(mission.status);

    // Update map
    const ambLat = driverAmbulance ? driverAmbulance.currentLat : 30.3244;
    const ambLng = driverAmbulance ? driverAmbulance.currentLng : 78.0418;
    updateMapMarkers(ambLat, ambLng, mission.latitude, mission.longitude, hosp.latitude, hosp.longitude, hosp.name);
}

function renderLifecycleActions(status) {
    const actionsContainer = document.getElementById('mission-action-controls');
    if (!actionsContainer) return;

    let buttonsHtml = '';

    if (status === 'DISPATCHED' || status === 'ASSIGNED') {
        const patientName = activeMission ? (activeMission.patientName || activeMission.callerName || 'Patient') : 'Patient';
        const pickupLoc = activeMission ? (activeMission.pickupLocation || activeMission.location || 'Dehradun') : 'Dehradun';
        buttonsHtml = `
            <div class="p-3 bg-amber-950/70 border border-amber-600/70 rounded-xl mb-3">
                <div class="flex items-center justify-between text-xs font-bold text-amber-300 mb-1">
                    <span><i class="fa-solid fa-bell animate-bounce mr-1.5"></i> NEW DISPATCH ASSIGNMENT</span>
                    <span class="px-2 py-0.5 rounded bg-amber-900/80 text-amber-200 border border-amber-700 text-[10px]">Decision Pending</span>
                </div>
                <div class="text-[11px] text-slate-300">
                    Patient <strong class="text-white">${Common.escapeHtml(patientName)}</strong> requires immediate ALS response at <strong>${Common.escapeHtml(pickupLoc)}</strong>.
                </div>
            </div>
            <div class="grid grid-cols-2 gap-2">
                <button onclick="acceptEmergencyMission()" class="py-3 px-2 rounded-xl bg-emerald-600 hover:bg-emerald-500 text-white font-extrabold text-xs shadow-lg shadow-emerald-600/30 flex items-center justify-center space-x-1.5 transition">
                    <i class="fa-solid fa-circle-check text-sm"></i>
                    <span>Accept Mission</span>
                </button>
                <button onclick="declineEmergencyMission()" class="py-3 px-2 rounded-xl bg-slate-800 hover:bg-slate-700 text-rose-300 font-bold text-xs border border-rose-800/60 flex items-center justify-center space-x-1.5 transition">
                    <i class="fa-solid fa-circle-xmark text-sm"></i>
                    <span>Decline / Pass</span>
                </button>
            </div>
        `;
    } else if (status === 'EN_ROUTE') {
        buttonsHtml = `
            <div class="p-2.5 bg-indigo-950/60 border border-indigo-700/60 rounded-xl mb-3 text-center text-xs font-bold text-indigo-300">
                <i class="fa-solid fa-route mr-1.5"></i> Mission Accepted &bull; Driving to Patient Pickup
            </div>
            <button onclick="advanceMissionStatus('ON_SCENE')" class="action-btn-large w-full bg-amber-600 hover:bg-amber-500 text-white shadow-amber-600/40">
                <i class="fa-solid fa-location-dot"></i>
                <span>1. Confirm Arrival At Scene (ON_SCENE)</span>
            </button>
        `;
    } else if (status === 'ON_SCENE') {
        buttonsHtml = `
            <button onclick="advanceMissionStatus('TRANSPORTING')" class="action-btn-large w-full bg-blue-600 hover:bg-blue-500 text-white shadow-blue-600/40">
                <i class="fa-solid fa-truck-medical"></i>
                <span>2. Patient Boarded &bull; Begin Hospital Transport</span>
            </button>
        `;
    } else if (status === 'TRANSPORTING') {
        buttonsHtml = `
            <button onclick="advanceMissionStatus('ARRIVED')" class="action-btn-large w-full bg-indigo-600 hover:bg-indigo-500 text-white shadow-indigo-600/40">
                <i class="fa-solid fa-hospital"></i>
                <span>3. Arrived At Hospital Emergency Wing</span>
            </button>
        `;
    } else if (status === 'ARRIVED') {
        buttonsHtml = `
            <div class="p-3 bg-indigo-950/70 border border-indigo-600/70 rounded-xl mb-3 text-center text-xs font-bold text-indigo-300">
                <i class="fa-solid fa-hospital mr-1.5"></i> Arrived At Hospital Emergency Wing &bull; Patient Handover in Progress
            </div>
            <button onclick="advanceMissionStatus('COMPLETED')" class="action-btn-large w-full bg-emerald-600 hover:bg-emerald-500 text-white shadow-emerald-600/40">
                <i class="fa-solid fa-circle-check"></i>
                <span>4. Complete Mission &bull; Release Vehicle to Standby</span>
            </button>
        `;
    } else if (status === 'COMPLETED') {
        const patientName = activeMission ? (activeMission.patientName || activeMission.callerName || 'Patient') : 'Patient';
        buttonsHtml = `
            <div class="p-4 bg-emerald-950/80 border-2 border-emerald-500 rounded-2xl text-center text-xs font-bold text-emerald-200 shadow-xl shadow-emerald-950/50">
                <div class="w-12 h-12 mx-auto rounded-full bg-emerald-600/30 border border-emerald-500 flex items-center justify-center text-emerald-300 text-xl mb-2">
                    <i class="fa-solid fa-hospital-user"></i>
                </div>
                <div class="text-sm font-black text-white">PATIENT ADMITTED AT HOSPITAL</div>
                <div class="text-[11px] text-emerald-300 mt-1">${Common.escapeHtml(patientName)} successfully admitted to ICU &bull; Bed allocated &bull; Mission Completed</div>
                <div class="mt-3">
                    <button onclick="advanceMissionStatus('READY')" class="w-full py-2.5 rounded-xl bg-emerald-600 hover:bg-emerald-500 text-white font-extrabold text-xs shadow-md transition flex items-center justify-center space-x-1.5">
                        <i class="fa-solid fa-rotate-right"></i>
                        <span>Acknowledge &amp; Return Vehicle to Standby (READY)</span>
                    </button>
                </div>
            </div>
        `;
    } else {
        buttonsHtml = `
            <div class="p-3 bg-emerald-950/60 border border-emerald-700/60 rounded-xl text-center text-xs font-bold text-emerald-300">
                <i class="fa-solid fa-circle-check mr-1.5"></i> Emergency Mission Completed &bull; Vehicle Standby
            </div>
        `;
    }

    actionsContainer.innerHTML = buttonsHtml;
}

async function acceptEmergencyMission() {
    if (!activeMission) return;
    try {
        Common.playEmergencyAlertSound();
        await advanceMissionStatus('EN_ROUTE');
        Common.toast(`Emergency Mission Accepted! Routing to ${activeMission.patientName || 'patient'}...`, 'success', 5000);
    } catch (e) {
        Common.toast('Accept error: ' + e.message, 'error');
    }
}

async function declineEmergencyMission() {
    if (!activeMission) return;
    try {
        const emId = activeMission.emergencyId;
        await fetch(Common.apiUrl('/api/driver/ambulance/status'), {
            method: 'PATCH',
            headers: Auth.getAuthHeaders(),
            body: JSON.stringify({ status: 'READY', emergencyId: emId, action: 'DECLINE' })
        });
        Common.toast('Emergency mission declined. Ambulance returned to standby.', 'warning', 5000);
        activeMission = null;
        lastKnownMissionId = null;
        renderActiveMissionCard(null);
        await loadAmbulanceInfo();
    } catch (e) {
        Common.toast('Decline error: ' + e.message, 'error');
    }
}

// -----------------------------------------------------------------------------
// Advance Mission Status Lifecycle
// -----------------------------------------------------------------------------
async function advanceMissionStatus(targetStatus) {
    if (!activeMission) return;

    try {
        const emId = activeMission.emergencyId;
        const res = await fetch(Common.apiUrl('/api/driver/ambulance/status'), {
            method: 'PATCH',
            headers: Auth.getAuthHeaders(),
            body: JSON.stringify({ status: targetStatus, emergencyId: emId })
        });

        const data = await res.json();
        if (!res.ok) throw new Error(data.error?.message || data.message || 'Status transition failed');

        if (targetStatus === 'COMPLETED' || targetStatus === 'READY') {
            Common.toast('Emergency Mission Completed! Vehicle released to Standby.', 'success', 6000);
            activeMission = null;
            lastKnownMissionId = null;
            renderActiveMissionCard(null);
        } else {
            Common.toast(`Mission status updated: ${targetStatus}`, 'success');
            // Refresh mission and vehicle info
            await pollAssignedMission();
        }
        await loadAmbulanceInfo();

    } catch (err) {
        Common.toast('Transition error: ' + err.message, 'error');
    }
}

// -----------------------------------------------------------------------------
// Live GPS Location Broadcasting with Error Resilience & Retry Handling
// -----------------------------------------------------------------------------
let consecutiveGpsFailures = 0;

async function toggleGpsBroadcast() {
    const btn = document.getElementById('toggle-gps-btn');
    const beacon = document.getElementById('gps-beacon-indicator');
    const statusText = document.getElementById('gps-telemetry-text');

    if (!isGpsBroadcasting) {
        isGpsBroadcasting = true;
        if (btn) btn.innerHTML = `<i class="fa-solid fa-satellite-dish fa-spin mr-1.5 text-emerald-400"></i>Stop GPS Broadcasting`;
        if (beacon) beacon.className = "w-2.5 h-2.5 rounded-full bg-emerald-500 gps-beacon-active mr-2";
        if (statusText) statusText.textContent = "Acquiring GPS fix & broadcasting telemetry...";

        Common.toast('GPS location broadcasting started', 'info');

        // Watch device position
        if (navigator.geolocation) {
            gpsWatchId = navigator.geolocation.watchPosition(
                async (pos) => {
                    await sendGpsCoordinate(pos.coords.latitude, pos.coords.longitude, pos.coords.accuracy);
                },
                (err) => {
                    console.warn('[Driver GPS] Hardware watch notice:', err.message);
                    if (statusText) {
                        if (err.code === 1) {
                            statusText.innerHTML = `<span class="text-amber-400"><i class="fa-solid fa-triangle-exclamation mr-1"></i>Permission Denied. Station Coordinates Active.</span>`;
                        } else {
                            statusText.innerHTML = `<span class="text-amber-400"><i class="fa-solid fa-satellite-dish mr-1"></i>GPS Weak. Station Fallback Active.</span>`;
                        }
                    }
                    // Fallback to station coordinates broadcast
                    if (driverAmbulance && driverAmbulance.currentLat && driverAmbulance.currentLng) {
                        sendGpsCoordinate(driverAmbulance.currentLat, driverAmbulance.currentLng, 15);
                    }
                },
                { enableHighAccuracy: true, timeout: 6000, maximumAge: 3000 }
            );
        } else {
            // Periodic fallback broadcast of current ambulance position
            gpsWatchId = setInterval(async () => {
                if (driverAmbulance && driverAmbulance.currentLat && driverAmbulance.currentLng) {
                    await sendGpsCoordinate(driverAmbulance.currentLat, driverAmbulance.currentLng, 20);
                }
            }, 4000);
        }

    } else {
        isGpsBroadcasting = false;
        if (gpsWatchId) {
            if (navigator.geolocation && typeof gpsWatchId === 'number' && !String(gpsWatchId).includes('interval')) {
                navigator.geolocation.clearWatch(gpsWatchId);
            } else {
                clearInterval(gpsWatchId);
            }
            gpsWatchId = null;
        }

        if (btn) btn.innerHTML = `<i class="fa-solid fa-satellite-dish mr-1.5"></i>Start GPS Broadcasting`;
        if (beacon) beacon.className = "w-2.5 h-2.5 rounded-full bg-slate-500 mr-2";
        if (statusText) statusText.textContent = "GPS Broadcasting Standby";

        Common.toast('GPS location broadcasting stopped', 'info');
    }
}

async function sendGpsCoordinate(lat, lng, accuracy = 5) {
    if (!lat || !lng || isNaN(lat) || isNaN(lng)) return;

    try {
        const res = await fetch(Common.apiUrl('/api/driver/location'), {
            method: 'POST',
            headers: Auth.getAuthHeaders(),
            body: JSON.stringify({ latitude: lat, longitude: lng })
        });
        if (res.ok) {
            consecutiveGpsFailures = 0;
            const statusText = document.getElementById('gps-telemetry-text');
            if (statusText) {
                const accText = accuracy ? `(&plusmn;${Math.round(accuracy)}m)` : '';
                statusText.innerHTML = `Live GPS: <strong class="text-emerald-400 font-mono">${lat.toFixed(4)}, ${lng.toFixed(4)}</strong> ${accText} &bull; ${new Date().toLocaleTimeString()}`;
            }
            if (driverAmbulance) {
                driverAmbulance.currentLat = lat;
                driverAmbulance.currentLng = lng;
            }
            if (ambMarker) ambMarker.setLatLng([lat, lng]);
        } else {
            throw new Error(`HTTP ${res.status}`);
        }
    } catch (e) {
        consecutiveGpsFailures++;
        console.warn(`[GPS Broadcast] Transmission error (count ${consecutiveGpsFailures}):`, e.message);
        const statusText = document.getElementById('gps-telemetry-text');
        if (statusText) {
            statusText.innerHTML = `<span class="text-rose-400"><i class="fa-solid fa-triangle-exclamation mr-1"></i>Offline / Retrying (${consecutiveGpsFailures})...</span>`;
        }
    }
}

// -----------------------------------------------------------------------------
// Live Route Drive Simulation (Along Dijkstra Shortest Path)
// -----------------------------------------------------------------------------
function toggleRouteSimulation() {
    const btn = document.getElementById('simulate-route-btn');

    if (simulationInterval) {
        clearInterval(simulationInterval);
        simulationInterval = null;
        if (btn) btn.innerHTML = `<i class="fa-solid fa-play"></i><span>Simulate Route Drive</span>`;
        Common.toast('Route drive simulation paused', 'info');
        return;
    }

    if (!activeMission) {
        Common.toast('No active emergency mission assigned to simulate drive.', 'warning');
        return;
    }

    // Extract Dijkstra path nodes from backend liveRoute
    let waypoints = [];
    if (activeMission.liveRoute && activeMission.liveRoute.coordinates && activeMission.liveRoute.coordinates.length > 0) {
        waypoints = activeMission.liveRoute.coordinates.map(c => ({ lat: c.latitude, lng: c.longitude, name: c.name || '' }));
    } else {
        if (activeMission.latitude && activeMission.longitude) {
            waypoints.push({ lat: activeMission.latitude, lng: activeMission.longitude, name: 'Pickup Point' });
        }
        if (activeMission.hospital && activeMission.hospital.latitude) {
            waypoints.push({ lat: activeMission.hospital.latitude, lng: activeMission.hospital.longitude, name: 'Hospital Destination' });
        }
    }

    if (waypoints.length === 0) {
        Common.toast('No route waypoints found to simulate.', 'warning');
        return;
    }

    simulationIndex = 0;
    if (btn) btn.innerHTML = `<i class="fa-solid fa-pause text-amber-400"></i><span>Simulating Drive...</span>`;
    Common.toast('Simulating live ambulance drive along C++ Dijkstra route...', 'success');

    simulationInterval = setInterval(async () => {
        if (simulationIndex >= waypoints.length) {
            clearInterval(simulationInterval);
            simulationInterval = null;
            if (btn) btn.innerHTML = `<i class="fa-solid fa-play"></i><span>Simulate Route Drive</span>`;
            Common.toast('Ambulance reached destination node!', 'success');
            return;
        }

        const pt = waypoints[simulationIndex];
        simulationIndex++;
        await sendGpsCoordinate(pt.lat, pt.lng, 8);
        if (pt.name) {
            Common.toast(`Ambulance passing landmark: ${pt.name}`, 'info', 2000);
        }
    }, 2000);
}

window.toggleRouteSimulation = toggleRouteSimulation;
window.acceptEmergencyMission = acceptEmergencyMission;
window.declineEmergencyMission = declineEmergencyMission;
