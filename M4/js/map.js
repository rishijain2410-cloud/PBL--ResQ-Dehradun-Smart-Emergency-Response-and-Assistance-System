// M4: Leaflet Map Controller & Visualizer (Aryan Pundir)

let mapInstance = null;
let currentRoutePolyline = null;
let originMarker = null;
let hospitalMarkers = [];
let ambulanceMarkers = [];
let simulatedAmbulanceMarker = null;

function initLeafletMap() {
    if (mapInstance) return;

    // Centered on Dehradun, Uttarakhand
    mapInstance = L.map('leaflet-map', {
        zoomControl: true,
        attributionControl: false
    }).setView([30.3244, 78.0350], 13);

    // High-Detail OpenStreetMap Roads Layer
    const osmDetailed = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; OpenStreetMap contributors | ResQ Dehradun',
        maxZoom: 19
    });

    // Clean Street Map (CartoDB Voyager) for enhanced road readability
    const cartoVoyager = L.tileLayer('https://{s}.basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}{r}.png', {
        attribution: '&copy; CartoDB &copy; OpenStreetMap contributors',
        maxZoom: 19,
        subdomains: 'abcd'
    });

    osmDetailed.addTo(mapInstance);

    // Add Layer Switcher Control
    const baseMaps = {
        "High-Detail Roads (OSM)": osmDetailed,
        "Clean Street Map (Carto)": cartoVoyager
    };
    L.control.layers(baseMaps, null, { position: 'topright' }).addTo(mapInstance);

    // Place Hospital Markers
    renderHospitalMarkers();

    // Place Ambulance Fleet Markers
    renderAmbulanceMarkers();
}

function getMarkerIcon(type) {
    let color = '#22c55e'; // Green for hospital
    let iconClass = 'fa-hospital';

    if (type === 'emergency') {
        color = '#ef4444'; // Red
        iconClass = 'fa-triangle-exclamation';
    } else if (type === 'ambulance') {
        color = '#3b82f6'; // Blue
        iconClass = 'fa-truck-medical';
    } else if (type === 'user') {
        color = '#8b5cf6'; // Purple
        iconClass = 'fa-location-dot';
    }

    return L.divIcon({
        className: 'custom-div-icon',
        html: `<div style="background-color: ${color}; width: 34px; height: 34px; border-radius: 50%; display: flex; align-items: center; justify-content: center; color: white; box-shadow: 0 4px 10px rgba(0,0,0,0.3); border: 2px solid white;">
                 <i class="fa-solid ${iconClass}" style="font-size: 15px;"></i>
               </div>`,
        iconSize: [34, 34],
        iconAnchor: [17, 17],
        popupAnchor: [0, -18]
    });
}

async function renderHospitalMarkers() {
    hospitalMarkers.forEach(m => mapInstance.removeLayer(m));
    hospitalMarkers = [];

    let hospitals = M2_API.defaultHospitals;
    try {
        const apiUrl = (typeof Common !== 'undefined' && Common.apiUrl) ? Common.apiUrl('/api/hospitals') : '/api/hospitals';
        const res = await fetch(apiUrl);
        if (res.ok) {
            const json = await res.json();
            if (json.data && json.data.length > 0) {
                hospitals = json.data;
            }
        }
    } catch (e) {
        // Fallback to static registry
    }

    hospitals.forEach(h => {
        const marker = L.marker([h.latitude, h.longitude], { icon: getMarkerIcon('hospital') })
            .addTo(mapInstance)
            .bindPopup(`
                <div class="p-1">
                    <h4 class="font-bold text-sm text-slate-900">${h.name}</h4>
                    <p class="text-xs text-slate-500 mb-2">${h.address}</p>
                    <div class="flex items-center justify-between text-xs mb-3 bg-slate-50 p-2 rounded">
                        <span>Available ICU: <b class="text-emerald-600">${h.availableIcu !== undefined ? h.availableIcu : h.icuBeds}</b> / ${h.icuBeds || h.totalBeds}</span>
                        <span>Rating: <b class="text-amber-500">${h.rating || 4.5} ★</b></span>
                    </div>
                    <button onclick="selectHospitalForRouting('${h.id}')" class="w-full py-1.5 bg-emerald-600 hover:bg-emerald-700 text-white rounded text-xs font-bold transition">
                        Route to this Hospital
                    </button>
                </div>
            `);
        hospitalMarkers.push(marker);
    });
}

async function renderAmbulanceMarkers() {
    ambulanceMarkers.forEach(m => mapInstance.removeLayer(m));
    ambulanceMarkers = [];

    const ambulances = await M3_API.getAmbulances();
    ambulances.forEach(a => {
        const marker = L.marker([a.latitude, a.longitude], { icon: getMarkerIcon('ambulance') })
            .addTo(mapInstance)
            .bindPopup(`
                <div class="p-1">
                    <h4 class="font-bold text-sm text-blue-700">${a.unitCode}</h4>
                    <p class="text-xs text-slate-600">Type: <b>${a.type}</b> | Driver: ${a.driver}</p>
                    <span class="inline-block mt-1 px-2 py-0.5 bg-blue-100 text-blue-800 rounded text-[11px] font-bold">Status: ${a.status}</span>
                </div>
            `);
        ambulanceMarkers.push(marker);
    });
}

let routeCasingLine = null;

function drawRouteOnMap(pathNodeIds) {
    if (!mapInstance) return;

    if (currentRoutePolyline) {
        mapInstance.removeLayer(currentRoutePolyline);
        currentRoutePolyline = null;
    }
    if (routeCasingLine) {
        mapInstance.removeLayer(routeCasingLine);
        routeCasingLine = null;
    }

    const latLngs = [];
    pathNodeIds.forEach(id => {
        const node = M3_API.nodes[id];
        if (node) {
            latLngs.push([node.lat, node.lng]);
        }
    });

    if (latLngs.length < 2) return;

    // Dark casing line underneath for maximum visibility on complex road textures
    routeCasingLine = L.polyline(latLngs, {
        color: '#0f172a',
        weight: 9,
        opacity: 0.85,
        lineCap: 'round',
        lineJoin: 'round'
    }).addTo(mapInstance);

    // Draw main glowing animated route polyline
    currentRoutePolyline = L.polyline(latLngs, {
        color: '#10b981',
        weight: 5,
        opacity: 0.95,
        lineCap: 'round',
        lineJoin: 'round',
        className: 'animated-route-polyline'
    }).addTo(mapInstance);

    // Fit map view to path
    mapInstance.fitBounds(currentRoutePolyline.getBounds(), { padding: [40, 40] });

    // Mark origin
    if (originMarker) mapInstance.removeLayer(originMarker);
    originMarker = L.marker(latLngs[0], { icon: getMarkerIcon('user') })
        .addTo(mapInstance)
        .bindPopup("<b>Emergency Location (Origin)</b>");
}

let patientLiveMarker = null;
let patientAccuracyCircle = null;

function setPatientLiveMarker(lat, lng, accuracyMeters = 10, title = "Patient Live GPS Location") {
    if (!mapInstance) return;

    if (patientLiveMarker) {
        mapInstance.removeLayer(patientLiveMarker);
        patientLiveMarker = null;
    }
    if (patientAccuracyCircle) {
        mapInstance.removeLayer(patientAccuracyCircle);
        patientAccuracyCircle = null;
    }

    const patientIcon = L.divIcon({
        className: 'patient-live-radar-icon',
        html: `<div style="background-color: #9333ea; width: 38px; height: 38px; border-radius: 50%; display: flex; align-items: center; justify-content: center; color: white; box-shadow: 0 0 20px #9333ea; border: 3px solid white; animation: pulse-ring 1.5s infinite;">
                 <i class="fa-solid fa-person-circle-exclamation text-lg"></i>
               </div>`,
        iconSize: [38, 38],
        iconAnchor: [19, 19],
        popupAnchor: [0, -20]
    });

    patientAccuracyCircle = L.circle([lat, lng], {
        radius: Math.max(accuracyMeters, 25),
        color: '#9333ea',
        fillColor: '#c084fc',
        fillOpacity: 0.15,
        weight: 1.5,
        dashArray: '4, 4'
    }).addTo(mapInstance);

    patientLiveMarker = L.marker([lat, lng], { icon: patientIcon })
        .addTo(mapInstance)
        .bindPopup(`
            <div class="p-1">
                <span class="inline-block px-2 py-0.5 rounded text-[10px] font-bold bg-purple-100 text-purple-800 uppercase mb-1">Live Patient GPS</span>
                <h4 class="font-bold text-xs text-slate-900">${title}</h4>
                <p class="text-[11px] text-slate-500">Lat: ${lat.toFixed(5)}, Lng: ${lng.toFixed(5)}</p>
                <p class="text-[11px] text-slate-500">Accuracy: &plusmn;${accuracyMeters.toFixed(1)} meters</p>
                <button onclick="routeFromPatientGps()" class="mt-2 w-full py-1 bg-purple-600 hover:bg-purple-700 text-white rounded text-xs font-bold transition">
                    Route Ambulance from Here
                </button>
            </div>
        `)
        .openPopup();

    mapInstance.panTo([lat, lng]);
}

window.initLeafletMap = initLeafletMap;
window.drawRouteOnMap = drawRouteOnMap;
window.renderHospitalMarkers = renderHospitalMarkers;
window.setPatientLiveMarker = setPatientLiveMarker;
