// M4: Live Real-Time Ambulance Tracker & Path Simulator

let trackingInterval = null;
let currentStepIndex = 0;
let routeCoordinates = [];

function startLiveAmbulanceTracking(pathNodeIds) {
    if (!mapInstance) return;

    // Clear previous tracking if running
    if (trackingInterval) {
        clearInterval(trackingInterval);
        trackingInterval = null;
    }
    if (simulatedAmbulanceMarker) {
        mapInstance.removeLayer(simulatedAmbulanceMarker);
        simulatedAmbulanceMarker = null;
    }

    routeCoordinates = [];
    pathNodeIds.forEach(id => {
        const node = M3_API.nodes[id];
        if (node) {
            routeCoordinates.push([node.lat, node.lng]);
        }
    });

    if (routeCoordinates.length < 2) return;

    currentStepIndex = 0;
    const startPos = routeCoordinates[0];

    // Create custom siren ambulance icon
    const ambIcon = L.divIcon({
        className: 'ambulance-pulse-icon',
        html: `<div style="background-color: #ef4444; width: 42px; height: 42px; border-radius: 50%; display: flex; align-items: center; justify-content: center; color: white; box-shadow: 0 0 20px #ef4444; border: 3px solid white; animation: pulse-ring 1s infinite;">
                 <i class="fa-solid fa-truck-medical fa-beat" style="font-size: 18px;"></i>
               </div>`,
        iconSize: [42, 42],
        iconAnchor: [21, 21]
    });

    simulatedAmbulanceMarker = L.marker(startPos, { icon: ambIcon }).addTo(mapInstance);
    simulatedAmbulanceMarker.bindPopup("<b>Unit #DHN-108-01 (ALS)</b><br>En route with siren active!").openPopup();

    const etaDisplay = document.getElementById('tracking-eta-live');
    const statusPill = document.getElementById('tracking-status-pill');
    let remainingMins = (routeCoordinates.length * 1.8);

    if (statusPill) {
        statusPill.innerText = "EN ROUTE (ALS)";
        statusPill.className = "px-3 py-1 bg-red-600 text-white rounded-full text-xs font-bold uppercase animate-pulse";
    }

    trackingInterval = setInterval(() => {
        if (currentStepIndex < routeCoordinates.length - 1) {
            currentStepIndex++;
            const nextCoord = routeCoordinates[currentStepIndex];

            // Animate marker transition
            simulatedAmbulanceMarker.setLatLng(nextCoord);
            mapInstance.panTo(nextCoord);

            remainingMins = Math.max(0, remainingMins - 1.8);
            if (etaDisplay) {
                etaDisplay.innerText = remainingMins <= 0.1 ? "ARRIVING NOW" : `~${remainingMins.toFixed(1)} mins remaining`;
            }
        } else {
            // Reached destination!
            clearInterval(trackingInterval);
            trackingInterval = null;
            simulatedAmbulanceMarker.bindPopup("<b>ARRIVED!</b><br>Ambulance has reached patient location.").openPopup();

            if (statusPill) {
                statusPill.innerText = "ARRIVED ON SCENE";
                statusPill.className = "px-3 py-1 bg-emerald-600 text-white rounded-full text-xs font-bold uppercase";
            }
            if (etaDisplay) etaDisplay.innerText = "Arrived (0 mins)";

            showToast("🚨 Ambulance has arrived at the emergency location!", "success");
        }
    }, 2500); // Progresses along path nodes every 2.5s
}

window.startLiveAmbulanceTracking = startLiveAmbulanceTracking;
