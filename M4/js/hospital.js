// ==========================================================================
// JEEVAN MARG -- HOSPITAL PORTAL JAVASCRIPT (hospital.js)
// Real-time capacity & bed management, incoming dispatch admissions & history
// ==========================================================================

let hospitalProfile = null;
let emergenciesPoller = null;
let hospitalMap = null;
let hospitalMarker = null;
let queueAmbulanceMarkers = {};
let queueRouteLines = {};
let lastKnownQueueCount = 0;

document.addEventListener('DOMContentLoaded', async () => {
    // 1. Enforce Role-Based Access
    if (!Auth.requireRole(['HOSPITAL', 'ADMIN'])) return;

    const user = Auth.getUser();
    renderHospitalUserHeader(user);

    // Inject portal quick switcher for demonstration ease
    if (Common.injectPortalSwitcher) {
        Common.injectPortalSwitcher('hospital');
    }

    // 2. Load Hospital Details & Capacity
    await loadHospitalData();

    // 3. Initialize Hospital Radar Map
    initHospitalMap();

    // 4. Initialize Bed Management Form
    initBedManagementForm();

    // 5. Start Real-time Queue Poller (every 3s)
    emergenciesPoller = Common.createPoller(async () => {
        await loadHospitalEmergencies();
    }, 3000);
});

// -----------------------------------------------------------------------------
// Radar Map Initialization
// -----------------------------------------------------------------------------
function initHospitalMap() {
    const mapEl = document.getElementById('hospital-map');
    if (!mapEl || typeof L === 'undefined') return;

    const defaultLat = (hospitalProfile && hospitalProfile.latitude) ? hospitalProfile.latitude : 30.3185;
    const defaultLng = (hospitalProfile && hospitalProfile.longitude) ? hospitalProfile.longitude : 78.0412;

    hospitalMap = L.map('hospital-map', {
        zoomControl: true,
        attributionControl: false
    }).setView([defaultLat, defaultLng], 13);

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 18,
        opacity: 0.85
    }).addTo(hospitalMap);

    // Place Hospital Facility Pin
    const hospIcon = Common.createPinIcon('hospital', 'fa-hospital');
    hospitalMarker = L.marker([defaultLat, defaultLng], { icon: hospIcon }).addTo(hospitalMap)
        .bindPopup(`<b>${Common.escapeHtml(hospitalProfile?.name || 'Hospital Facility')}</b><br>Emergency Care & Trauma Center`);
}

// -----------------------------------------------------------------------------
// Header & User Display
// -----------------------------------------------------------------------------
function renderHospitalUserHeader(user) {
    const userEl = document.getElementById('hosp-admin-name');
    const roleBadge = document.getElementById('hosp-user-badge');
    if (userEl) userEl.textContent = user.displayName || user.username;
    if (roleBadge && user.linkedHospitalId) {
        roleBadge.textContent = user.linkedHospitalId;
    }
}

// -----------------------------------------------------------------------------
// Load Hospital Details & Bed Capacity
// -----------------------------------------------------------------------------
async function loadHospitalData() {
    try {
        const res = await fetch(Common.apiUrl('/api/hospital/profile'), {
            headers: Auth.getAuthHeaders()
        });
        if (!res.ok) throw new Error('Failed to load hospital profile');

        const json = await res.json();
        hospitalProfile = json.data || json;

        renderHospitalProfile(hospitalProfile);
        renderCapacityMetrics(hospitalProfile);

    } catch (err) {
        console.error('Hospital profile load error:', err);
        Common.toast('Failed to load facility data: ' + err.message, 'error');
    }
}

function renderHospitalProfile(hosp) {
    const nameEl = document.getElementById('hosp-name-display');
    const addressEl = document.getElementById('hosp-address-display');
    const phoneEl = document.getElementById('hosp-phone-display');
    const specsEl = document.getElementById('hosp-specs-display');

    if (nameEl) nameEl.textContent = hosp.name || hosp.hospitalId;
    if (addressEl) addressEl.textContent = hosp.address || 'Dehradun, Uttarakhand';
    if (phoneEl) {
        const phone = hosp.phone || '0135-2745678';
        phoneEl.textContent = phone;
        phoneEl.href = `tel:${phone}`;
    }

    if (specsEl && hosp.specialties && hosp.specialties.length > 0) {
        specsEl.innerHTML = hosp.specialties.map(s => `
            <span class="px-2 py-0.5 rounded text-[10px] font-semibold bg-teal-950/70 text-teal-300 border border-teal-800">
                ${Common.escapeHtml(s)}
            </span>
        `).join('');
    }
}

function renderCapacityMetrics(hosp) {
    const totalBedsEl = document.getElementById('metric-total-beds');
    const icuTotalEl = document.getElementById('metric-icu-total');
    const icuAvailEl = document.getElementById('metric-icu-avail');
    const oxygenBedsEl = document.getElementById('metric-oxygen-beds');
    const admittedEl = document.getElementById('metric-admitted-patients');
    const admittedSubtext = document.getElementById('metric-admitted-subtext');
    const icuChangeEl = document.getElementById('metric-icu-change');
    const icuGaugeCard = document.getElementById('icu-gauge-card');

    const total = hosp.totalBeds || 0;
    const icuTotal = hosp.icuBeds !== undefined ? hosp.icuBeds : (hosp.icuTotal || 0);
    const icuAvail = hosp.availableIcu !== undefined ? hosp.availableIcu : (hosp.icuAvailable || 0);
    const oxygen = hosp.oxygenBeds || 0;
    const admitted = hosp.admittedCount !== undefined ? hosp.admittedCount : 0;

    if (totalBedsEl) totalBedsEl.textContent = total;
    if (icuTotalEl) icuTotalEl.textContent = icuTotal;
    if (icuAvailEl) icuAvailEl.textContent = icuAvail;
    if (oxygenBedsEl) oxygenBedsEl.textContent = oxygen;
    if (admittedEl) admittedEl.textContent = admitted;
    if (admittedSubtext) admittedSubtext.textContent = `+${admitted} admitted this session`;
    if (icuChangeEl) icuChangeEl.textContent = `${icuAvail} of ${icuTotal} available`;

    // Form inputs pre-fill
    const inpTotal = document.getElementById('inp-total-beds');
    const inpIcuTotal = document.getElementById('inp-icu-total');
    const inpIcuAvail = document.getElementById('inp-icu-avail');
    const inpOxy = document.getElementById('inp-oxygen-beds');

    if (inpTotal) inpTotal.value = total;
    if (inpIcuTotal) inpIcuTotal.value = icuTotal;
    if (inpIcuAvail) inpIcuAvail.value = icuAvail;
    if (inpOxy) inpOxy.value = oxygen;

    // ICU gauge styling
    if (icuGaugeCard) {
        icuGaugeCard.classList.remove('icu-gauge-high', 'icu-gauge-medium', 'icu-gauge-critical');
        if (icuAvail >= 6) {
            icuGaugeCard.classList.add('icu-gauge-high');
        } else if (icuAvail >= 1) {
            icuGaugeCard.classList.add('icu-gauge-medium');
        } else {
            icuGaugeCard.classList.add('icu-gauge-critical');
        }
    }
}

// -----------------------------------------------------------------------------
// Bed Management Form & Persistence
// -----------------------------------------------------------------------------
function initBedManagementForm() {
    const form = document.getElementById('bed-management-form');
    if (!form) return;

    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        const totalBeds = parseInt(document.getElementById('inp-total-beds').value, 10);
        const icuTotal = parseInt(document.getElementById('inp-icu-total').value, 10);
        const icuAvailable = parseInt(document.getElementById('inp-icu-avail').value, 10);
        const oxygenBeds = parseInt(document.getElementById('inp-oxygen-beds').value, 10);

        // Validation
        if (isNaN(totalBeds) || totalBeds < 0 ||
            isNaN(icuTotal) || icuTotal < 0 ||
            isNaN(icuAvailable) || icuAvailable < 0 ||
            isNaN(oxygenBeds) || oxygenBeds < 0) {
            Common.toast('Bed numbers must be valid non-negative integers.', 'error');
            return;
        }

        if (icuAvailable > icuTotal) {
            Common.toast('Available ICU beds cannot exceed Total ICU beds.', 'error');
            return;
        }

        const btn = document.getElementById('save-beds-btn');
        if (btn) btn.disabled = true;

        try {
            const res = await fetch(Common.apiUrl('/api/hospital/beds'), {
                method: 'PATCH',
                headers: Auth.getAuthHeaders(),
                body: JSON.stringify({ totalBeds, icuTotal, icuAvailable, oxygenBeds })
            });

            const data = await res.json();
            if (!res.ok) throw new Error(data.error?.message || data.message || 'Update failed');

            Common.toast('Bed capacity saved to SQLite database successfully!', 'success');
            await loadHospitalData();

        } catch (err) {
            Common.toast('Update error: ' + err.message, 'error');
        } finally {
            if (btn) btn.disabled = false;
        }
    });
}

// -----------------------------------------------------------------------------
// Incoming Emergencies Queue & History
// -----------------------------------------------------------------------------
async function loadHospitalEmergencies() {
    try {
        const res = await fetch(Common.apiUrl('/api/hospital/emergencies'), {
            headers: Auth.getAuthHeaders()
        });
        if (!res.ok) return;

        const json = await res.json();
        const list = json.data || json.emergencies || [];

        const activeQueue = list.filter(e => e.status !== 'COMPLETED' && e.status !== 'CANCELLED');
        const pastAdmissions = list.filter(e => e.status === 'COMPLETED' || e.status === 'CANCELLED');

        // Play audio alert if new emergency transport just routed to this hospital
        if (activeQueue.length > lastKnownQueueCount) {
            Common.playEmergencyAlertSound();
            Common.toast(`New incoming emergency dispatch (${activeQueue[0].emergencyId}) routed to facility!`, 'warning', 5000);
        }
        lastKnownQueueCount = activeQueue.length;

        renderIncomingQueue(activeQueue);
        renderAdmissionHistory(pastAdmissions);
        updateHospitalRadarMap(activeQueue);

        const countBadge = document.getElementById('incoming-queue-count');
        if (countBadge) countBadge.textContent = activeQueue.length;

    } catch (err) {
        console.warn('Emergencies poll error:', err);
    }
}

function updateHospitalRadarMap(activeList) {
    if (!hospitalMap || typeof L === 'undefined') return;

    const hudHospName = document.getElementById('hud-hosp-name');
    const hudEtaText = document.getElementById('hud-eta-text');

    if (hudHospName && hospitalProfile) {
        hudHospName.textContent = hospitalProfile.name || 'Hospital ER';
    }

    if (activeList.length === 0) {
        if (hudEtaText) hudEtaText.textContent = 'No incoming emergency transports in queue.';
        // Clear old ambulance markers
        Object.keys(queueAmbulanceMarkers).forEach(k => {
            hospitalMap.removeLayer(queueAmbulanceMarkers[k]);
            delete queueAmbulanceMarkers[k];
        });
        Object.keys(queueRouteLines).forEach(k => {
            hospitalMap.removeLayer(queueRouteLines[k]);
            delete queueRouteLines[k];
        });
        return;
    }

    const first = activeList[0];
    const ambCode = first.assignedAmbulanceId || 'Ambulance';
    const firstSev = first.severity || 'CRITICAL';
    if (hudEtaText) {
        hudEtaText.innerHTML = `<strong>${Common.escapeHtml(ambCode)}</strong> &bull; <span class="text-rose-400 font-bold">${firstSev}</span> &bull; En Route (${Common.escapeHtml(first.status)})`;
    }

    const boundsPoints = [];
    if (hospitalProfile && hospitalProfile.latitude && hospitalProfile.longitude) {
        boundsPoints.push([hospitalProfile.latitude, hospitalProfile.longitude]);
    }

    activeList.forEach(em => {
        const ambId = em.assignedAmbulanceId || em.emergencyId;
        const lat = em.latitude;
        const lng = em.longitude;

        if (lat && lng) {
            boundsPoints.push([lat, lng]);

            if (queueAmbulanceMarkers[ambId]) {
                queueAmbulanceMarkers[ambId].setLatLng([lat, lng]);
            } else {
                const ambIcon = Common.createPinIcon('ambulance', 'fa-truck-medical');
                const marker = L.marker([lat, lng], { icon: ambIcon }).addTo(hospitalMap)
                    .bindPopup(`<b>${Common.escapeHtml(ambId)}</b><br>Case: <b>${Common.escapeHtml(em.emergencyId)}</b><br>Patient: ${Common.escapeHtml(em.patientName || 'Emergency')}<br>Status: <span class="text-amber-400 font-bold">${Common.escapeHtml(em.status)}</span>`);
                queueAmbulanceMarkers[ambId] = marker;
            }

            // Draw animated connecting route polyline to hospital
            if (hospitalProfile && hospitalProfile.latitude && hospitalProfile.longitude) {
                const routePts = [[lat, lng], [hospitalProfile.latitude, hospitalProfile.longitude]];
                if (queueRouteLines[ambId]) {
                    queueRouteLines[ambId].setLatLngs(routePts);
                } else if (Common.drawAnimatedRoute) {
                    queueRouteLines[ambId] = Common.drawAnimatedRoute(hospitalMap, routePts, '#14b8a6', 4);
                }
            }
        }
    });

    if (boundsPoints.length >= 2) {
        hospitalMap.fitBounds(boundsPoints, { padding: [50, 50], maxZoom: 14 });
    }
}

function renderIncomingQueue(activeList) {
    const container = document.getElementById('incoming-queue-container');
    if (!container) return;

    if (activeList.length === 0) {
        container.innerHTML = `
            <div class="text-center py-8 text-slate-500 text-xs">
                <i class="fa-solid fa-bed-pulse text-3xl text-slate-700 mb-2"></i>
                <p>No active incoming emergency transports routed to this hospital.</p>
                <p class="text-[10px] text-slate-600 mt-1">Waiting for citizen SOS dispatch from Central Engine...</p>
            </div>
        `;
        return;
    }

    container.innerHTML = activeList.map(em => {
        const isCritical = (em.severity === 'CRITICAL');
        const isArrived = (em.status === 'ARRIVED');

        return `
            <div class="incoming-patient-card ${isCritical ? 'severity-critical' : 'severity-high'} mb-3">
                <div class="flex flex-col sm:flex-row sm:items-center justify-between gap-2">
                    <div>
                        <div class="flex items-center space-x-2">
                            <span class="font-bold text-white text-sm font-mono">${Common.escapeHtml(em.emergencyId)}</span>
                            <span class="px-2 py-0.5 rounded text-[10px] font-black uppercase ${
                                isCritical ? 'bg-red-950 text-red-300 border border-red-700 animate-pulse' :
                                'bg-amber-950 text-amber-300 border border-amber-700'
                            }">${Common.escapeHtml(em.severity || 'CRITICAL')}</span>
                            <span class="px-2 py-0.5 rounded text-[10px] font-bold bg-slate-800 text-slate-300 uppercase">${Common.escapeHtml(em.status)}</span>
                        </div>
                        <div class="text-xs text-slate-200 mt-1">
                            Patient: <strong>${Common.escapeHtml(em.patientName || em.callerName || 'Unknown')}</strong> &bull;
                            Contact: <strong>${Common.escapeHtml(em.patientPhone || em.callerPhone || '108')}</strong>
                        </div>
                        <div class="text-[11px] text-slate-400 mt-0.5">
                            Reported Condition: <span class="text-slate-200 font-semibold">${Common.escapeHtml(em.condition || em.conditionText || 'Emergency')}</span>
                        </div>
                        <div class="text-[11px] text-teal-400 mt-0.5 flex items-center space-x-1.5">
                            <i class="fa-solid fa-truck-medical text-xs"></i>
                            <span>Assigned Ambulance: <strong class="font-mono text-white">${Common.escapeHtml(em.assignedAmbulanceId || 'None')}</strong></span>
                        </div>
                    </div>

                    <div class="flex flex-col sm:items-end justify-between gap-2 mt-2 sm:mt-0">
                        <div class="text-[11px] text-slate-400">${Common.formatDate(em.createdAt || em.timestamp)}</div>
                        <div class="flex items-center space-x-2">
                            <button onclick="acceptEmergency('${Common.escapeHtml(em.emergencyId)}')" class="px-2.5 py-1.5 rounded-xl bg-emerald-700 hover:bg-emerald-600 text-white font-bold text-xs shadow-md shadow-emerald-700/30 transition flex items-center space-x-1.5 whitespace-nowrap" title="Acknowledge incoming dispatch & prep ICU bed">
                                <i class="fa-solid fa-check-double text-[10px]"></i>
                                <span>Accept &amp; Prep</span>
                            </button>
                            <button onclick="confirmAdmission('${Common.escapeHtml(em.emergencyId)}')" class="px-3 py-1.5 rounded-xl bg-teal-600 hover:bg-teal-500 text-white font-extrabold text-xs shadow-md shadow-teal-600/30 transition flex items-center space-x-1.5 whitespace-nowrap" title="Admit patient to ward and release ambulance">
                                <i class="fa-solid fa-hospital-user"></i>
                                <span>Admit Patient</span>
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

function renderAdmissionHistory(pastList) {
    const container = document.getElementById('admissions-history-container');
    if (!container) return;

    if (pastList.length === 0) {
        container.innerHTML = `
            <div class="text-center py-6 text-slate-500 text-xs">
                <p>No historical admissions recorded yet.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = pastList.map(em => `
        <div class="p-3 mb-2 rounded-xl bg-slate-900/50 border border-slate-800 flex items-center justify-between text-xs">
            <div>
                <span class="font-bold text-white font-mono">${Common.escapeHtml(em.emergencyId)}</span>
                <span class="text-slate-400 ml-2">${Common.escapeHtml(em.patientName || em.callerName || 'Patient')}</span>
                <span class="text-[10px] text-slate-500 ml-2">(${Common.escapeHtml(em.condition || 'Emergency')})</span>
            </div>
            <div class="flex items-center space-x-3">
                <span class="px-2 py-0.5 rounded text-[10px] font-bold ${em.status === 'COMPLETED' ? 'bg-emerald-950 text-emerald-300 border border-emerald-800' : 'bg-slate-800 text-slate-400'}">
                    ${Common.escapeHtml(em.status)}
                </span>
                <span class="text-[11px] text-slate-500">${Common.formatDate(em.createdAt || em.timestamp)}</span>
            </div>
        </div>
    `).join('');
}

// -----------------------------------------------------------------------------
// Accept & Prepare Trauma/ICU Ward
// -----------------------------------------------------------------------------
async function acceptEmergency(emergencyId) {
    try {
        const res = await fetch(Common.apiUrl(`/api/hospital/emergencies/${encodeURIComponent(emergencyId)}/accept`), {
            method: 'POST',
            headers: Auth.getAuthHeaders(),
            body: JSON.stringify({ emergencyId })
        });

        const data = await res.json();
        if (!res.ok) throw new Error(data.error?.message || data.message || 'Acceptance failed');

        Common.toast(`Hospital accepted dispatch ${emergencyId}. Emergency trauma team prepped!`, 'success');
        await loadHospitalEmergencies();

    } catch (err) {
        Common.toast('Accept error: ' + err.message, 'error');
    }
}

// -----------------------------------------------------------------------------
// Confirm Patient Admission
// -----------------------------------------------------------------------------
async function confirmAdmission(emergencyId) {
    if (!confirm(`Confirm patient admission for emergency case ${emergencyId}? This will allocate 1 ICU bed and release the ambulance to READY status.`)) return;

    try {
        const res = await fetch(Common.apiUrl(`/api/hospital/emergencies/${encodeURIComponent(emergencyId)}/admit`), {
            method: 'POST',
            headers: Auth.getAuthHeaders(),
            body: JSON.stringify({ emergencyId })
        });

        const data = await res.json();
        if (!res.ok) throw new Error(data.error?.message || data.message || 'Admission failed');

        const avail = data.availableIcu !== undefined ? data.availableIcu : (hospitalProfile?.availableIcu ? Math.max(0, hospitalProfile.availableIcu - 1) : 13);
        const admitted = data.admittedCount !== undefined ? data.admittedCount : ((hospitalProfile?.admittedCount || 0) + 1);

        Common.toast(`Emergency ${emergencyId} admitted! Available ICU Beds decremented to ${avail} (-1). Admitted Patients: ${admitted} (+1). Ambulance released to READY.`, 'success', 6000);

        // Flash visual notification on capacity cards
        const icuCard = document.getElementById('icu-gauge-card');
        const admittedCard = document.getElementById('metric-admitted-patients')?.parentElement;
        if (icuCard) {
            icuCard.classList.add('ring-4', 'ring-amber-400', 'animate-pulse');
            setTimeout(() => icuCard.classList.remove('ring-4', 'ring-amber-400', 'animate-pulse'), 3500);
        }
        if (admittedCard) {
            admittedCard.classList.add('ring-4', 'ring-emerald-400', 'animate-pulse');
            setTimeout(() => admittedCard.classList.remove('ring-4', 'ring-emerald-400', 'animate-pulse'), 3500);
        }

        // Refresh queue and metrics immediately
        await loadHospitalData();
        await loadHospitalEmergencies();

    } catch (err) {
        Common.toast('Admission error: ' + err.message, 'error');
    }
}

window.acceptEmergency = acceptEmergency;
window.confirmAdmission = confirmAdmission;
