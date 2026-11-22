// ==========================================================================
// JEEVAN MARG -- COMMON UTILITY & CLIENT HELPERS
// Shared across Patient, Driver, Hospital, and Admin Portals
// ==========================================================================

const Common = {
    // -------------------------------------------------------------------------
    // API Base URL Resolution (Supports Localhost, Custom Ports & Netlify)
    // -------------------------------------------------------------------------
    getApiBase() {
        if (window.JEEVAN_MARG_API_URL) return window.JEEVAN_MARG_API_URL;
        try {
            const stored = localStorage.getItem('jeevan_marg_api_url');
            if (stored) return stored;
        } catch (e) {}

        if (window.location && window.location.protocol && window.location.protocol.startsWith('http')) {
            const isLocal = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
            if (isLocal) {
                return window.location.port === '8080' ? '' : 'http://localhost:8080';
            }
            return '';
        }
        return 'http://localhost:8080';
    },

    apiUrl(endpoint) {
        const base = this.getApiBase();
        const ep = endpoint.startsWith('/') ? endpoint : '/' + endpoint;
        return base ? `${base}${ep}` : ep;
    },

    // -------------------------------------------------------------------------
    // Toast Notification System
    // -------------------------------------------------------------------------
    toast(message, type = 'info', duration = 4000) {
        let container = document.getElementById('toast-container');
        if (!container) {
            container = document.createElement('div');
            container.id = 'toast-container';
            document.body.appendChild(container);
        }

        const icons = {
            success: 'fa-circle-check',
            error: 'fa-circle-exclamation',
            warning: 'fa-triangle-exclamation',
            info: 'fa-circle-info'
        };

        const iconClass = icons[type] || icons.info;
        const toastEl = document.createElement('div');
        toastEl.className = `toast toast-${type}`;
        toastEl.innerHTML = `
            <i class="fa-solid ${iconClass} text-base flex-shrink-0"></i>
            <span class="flex-1">${message}</span>
            <button onclick="this.parentElement.remove()" class="opacity-60 hover:opacity-100 ml-2">&times;</button>
        `;

        container.appendChild(toastEl);

        setTimeout(() => {
            toastEl.classList.add('toast-out');
            setTimeout(() => toastEl.remove(), 250);
        }, duration);
    },

    // -------------------------------------------------------------------------
    // Date & Time Formatter
    // -------------------------------------------------------------------------
    formatDate(dateStr) {
        if (!dateStr || dateStr === 'Just Now') return 'Just Now';
        try {
            const d = new Date(dateStr.replace(' ', 'T'));
            if (isNaN(d.getTime())) return dateStr;
            return d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }) + ' &bull; ' +
                   d.toLocaleDateString([], { month: 'short', day: 'numeric' });
        } catch (e) {
            return dateStr;
        }
    },

    // -------------------------------------------------------------------------
    // HTML Sanitization
    // -------------------------------------------------------------------------
    escapeHtml(str) {
        if (!str) return '';
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    },

    // -------------------------------------------------------------------------
    // Known Dehradun Landmarks (for Manual Geocoding & GPS Fallback)
    // -------------------------------------------------------------------------
    LANDMARKS: [
        { name: "Clock Tower", lat: 30.3244, lng: 78.0418, address: "Rajpur Road, Clock Tower Chowk, Dehradun" },
        { name: "ISBT Dehradun", lat: 30.2872, lng: 78.0069, address: "Haridwar Bypass Road, Majra, Dehradun" },
        { name: "Railway Station", lat: 30.3155, lng: 78.0331, address: "Station Road, Lakkhi Bagh, Dehradun" },
        { name: "Max Hospital", lat: 30.3812, lng: 78.0772, address: "Mussoorie Diversion Road, Johri, Malsi, Dehradun" },
        { name: "Mahant Indiresh Hospital", lat: 30.3124, lng: 78.0289, address: "Patel Nagar, Dehradun" },
        { name: "Synergy Hospital", lat: 30.3345, lng: 78.0212, address: "Ballupur Chowk, Canal Road, Dehradun" },
        { name: "Graphic Era Hospital (GEIMS)", lat: 30.3521, lng: 77.9654, address: "16th Milestone, Chakrata Road, Dhoolkot" },
        { name: "Graphic Era University (GEU)", lat: 30.2688, lng: 78.0084, address: "566/6 Bell Road, Clement Town, Dehradun" },
        { name: "CMI Hospital", lat: 30.3142, lng: 78.0511, address: "54 Haridwar Road, Dharampur, Dehradun" },
        { name: "Doon Hospital", lat: 30.3185, lng: 78.0412, address: "New Road, Race Course, Dehradun" },
        { name: "Ballupur Chowk", lat: 30.3340, lng: 78.0180, address: "Chakrata Road Junction, Ballupur, Dehradun" },
        { name: "Rispana Bridge", lat: 30.2930, lng: 78.0535, address: "Haridwar Bypass - Rispana River, Dehradun" },
        { name: "Prem Nagar", lat: 30.3395, lng: 77.9620, address: "Chakrata Road, Prem Nagar, Dehradun" },
        { name: "Jakhan", lat: 30.3580, lng: 78.0645, address: "Rajpur Road, Jakhan, Dehradun" },
        { name: "Saharanpur Chowk", lat: 30.3110, lng: 78.0315, address: "Saharanpur Road, Patel Nagar North, Dehradun" },
        { name: "Clement Town", lat: 30.2650, lng: 78.0050, address: "Post Office Road, Clement Town, Dehradun" },
        { name: "Dalanwala", lat: 30.3220, lng: 78.0590, address: "Circular Road, Dalanwala, Dehradun" }
    ],

    // Resolve text to nearest known landmark
    resolveLandmark(query) {
        if (!query) return null;
        const q = query.toLowerCase().trim();
        for (const lm of Common.LANDMARKS) {
            if (q.includes(lm.name.toLowerCase()) || lm.name.toLowerCase().includes(q)) {
                return lm;
            }
        }
        return null;
    },

    // -------------------------------------------------------------------------
    // Browser Geolocation with Structured Permissions & Error Handling
    // -------------------------------------------------------------------------
    getCurrentPosition(defaultCoords = { lat: 30.3244, lng: 78.0418 }) {
        return new Promise((resolve) => {
            if (!navigator.geolocation) {
                resolve({
                    ...defaultCoords,
                    isFallback: true,
                    status: 'UNSUPPORTED',
                    error: 'Browser does not support HTML5 Geolocation API.'
                });
                return;
            }

            navigator.geolocation.getCurrentPosition(
                (pos) => {
                    const lat = pos.coords.latitude;
                    const lng = pos.coords.longitude;
                    const accuracy = pos.coords.accuracy || 10;

                    // Sanity check coordinates
                    if (isNaN(lat) || isNaN(lng) || lat < -90 || lat > 90 || lng < -180 || lng > 180) {
                        resolve({
                            ...defaultCoords,
                            isFallback: true,
                            status: 'INVALID_COORDS',
                            error: 'Received invalid GPS coordinate data.'
                        });
                        return;
                    }

                    resolve({
                        lat,
                        lng,
                        accuracy,
                        isFallback: false,
                        status: 'OK',
                        timestamp: pos.timestamp || Date.now()
                    });
                },
                (err) => {
                    let status = 'ERROR';
                    let errorMsg = 'GPS location lookup failed.';

                    switch (err.code) {
                        case 1: // PERMISSION_DENIED
                            status = 'PERMISSION_DENIED';
                            errorMsg = 'GPS permission denied. Enter location manually or select on map.';
                            break;
                        case 2: // POSITION_UNAVAILABLE
                            status = 'POSITION_UNAVAILABLE';
                            errorMsg = 'GPS signal unavailable. Using landmark default.';
                            break;
                        case 3: // TIMEOUT
                            status = 'TIMEOUT';
                            errorMsg = 'GPS request timed out. Using default Dehradun location.';
                            break;
                        default:
                            errorMsg = err.message || errorMsg;
                    }

                    console.warn(`[Geolocation] ${status}: ${errorMsg}`);
                    resolve({
                        ...defaultCoords,
                        isFallback: true,
                        status,
                        error: errorMsg
                    });
                },
                { enableHighAccuracy: true, timeout: 7000, maximumAge: 5000 }
            );
        });
    },

    // -------------------------------------------------------------------------
    // Haversine Distance in Kilometers
    // -------------------------------------------------------------------------
    calcDistanceKm(lat1, lon1, lat2, lon2) {
        const R = 6371; // Earth's radius km
        const dLat = (lat2 - lat1) * Math.PI / 180;
        const dLon = (lon2 - lon1) * Math.PI / 180;
        const a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
                  Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) *
                  Math.sin(dLon / 2) * Math.sin(dLon / 2);
        const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
        return parseFloat((R * c).toFixed(2));
    },

    // -------------------------------------------------------------------------
    // Poller Controller
    // Automatically stops on page unload, tab hide, or explicit cancel
    // -------------------------------------------------------------------------
    createPoller(fn, intervalMs = 4000) {
        let timer = null;
        let isRunning = true;
        let isExecuting = false;

        const loop = async () => {
            if (!isRunning) return;
            if (!isExecuting) {
                isExecuting = true;
                try {
                    await fn();
                } catch (e) {
                    console.error('Poller error:', e);
                } finally {
                    isExecuting = false;
                }
            }
            if (isRunning) {
                timer = setTimeout(loop, intervalMs);
            }
        };

        // Start immediately
        loop();

        const stop = () => {
            isRunning = false;
            if (timer) clearTimeout(timer);
        };

        window.addEventListener('beforeunload', stop);
        return { stop };
    },

    // -------------------------------------------------------------------------
    // Leaflet Custom Icon Factory with Siren & Pulse Animations
    // -------------------------------------------------------------------------
    createPinIcon(type, iconFaClass) {
        if (typeof L === 'undefined') return null;

        if (type === 'ambulance') {
            return L.divIcon({
                className: 'ambulance-icon-container',
                html: `
                    <div class="ambulance-siren-marker">
                        <div class="ambulance-siren-beacon"></div>
                        <div class="ambulance-siren-body">
                            <i class="fa-solid ${iconFaClass || 'fa-truck-medical'}"></i>
                        </div>
                    </div>
                `,
                iconSize: [38, 38],
                iconAnchor: [19, 19],
                popupAnchor: [0, -22]
            });
        }

        if (type === 'hospital') {
            return L.divIcon({
                className: 'dest-icon-container',
                html: `
                    <div class="dest-pin-marker">
                        <div class="dest-pin-beacon"></div>
                        <div class="dest-pin-body">
                            <i class="fa-solid ${iconFaClass || 'fa-hospital'}"></i>
                        </div>
                    </div>
                `,
                iconSize: [36, 36],
                iconAnchor: [18, 18],
                popupAnchor: [0, -20]
            });
        }

        if (type === 'patient') {
            return L.divIcon({
                className: 'patient-icon-container',
                html: `
                    <div class="dest-pin-marker">
                        <div class="dest-pin-beacon" style="background: rgba(59, 130, 246, 0.4);"></div>
                        <div class="dest-pin-body" style="background: #1e3a8a; border-color: #60a5fa; color: #bfdbfe;">
                            <i class="fa-solid ${iconFaClass || 'fa-user'}"></i>
                        </div>
                    </div>
                `,
                iconSize: [36, 36],
                iconAnchor: [18, 18],
                popupAnchor: [0, -20]
            });
        }

        return L.divIcon({
            className: 'custom-pin-container',
            html: `<div class="custom-pin custom-pin-${type} w-8 h-8"><i class="fa-solid ${iconFaClass}"></i></div>`,
            iconSize: [32, 32],
            iconAnchor: [16, 16],
            popupAnchor: [0, -18]
        });
    },

    // -------------------------------------------------------------------------
    // Animated Route Polyline Helper
    // -------------------------------------------------------------------------
    drawAnimatedRoute(map, latLngPoints, color = '#38bdf8', weight = 5) {
        if (!map || !latLngPoints || latLngPoints.length < 2) return null;

        const line = L.polyline(latLngPoints, {
            color: color,
            weight: weight,
            opacity: 0.9,
            lineCap: 'round',
            lineJoin: 'round',
            className: 'animated-route-polyline'
        }).addTo(map);

        return line;
    },

    // -------------------------------------------------------------------------
    // Audio-Visual Emergency Dispatch Chime (Web Audio API Synthesizer)
    // -------------------------------------------------------------------------
    playEmergencyAlertSound() {
        try {
            const AudioCtx = window.AudioContext || window.webkitAudioContext;
            if (!AudioCtx) return;
            const ctx = new AudioCtx();
            const now = ctx.currentTime;

            // Tone 1: 880 Hz (A5)
            const osc1 = ctx.createOscillator();
            const gain1 = ctx.createGain();
            osc1.type = 'sine';
            osc1.frequency.setValueAtTime(880, now);
            gain1.gain.setValueAtTime(0.12, now);
            gain1.gain.exponentialRampToValueAtTime(0.001, now + 0.35);
            osc1.connect(gain1);
            gain1.connect(ctx.destination);
            osc1.start(now);
            osc1.stop(now + 0.35);

            // Tone 2: 1174 Hz (D6)
            const osc2 = ctx.createOscillator();
            const gain2 = ctx.createGain();
            osc2.type = 'sine';
            osc2.frequency.setValueAtTime(1174, now + 0.2);
            gain2.gain.setValueAtTime(0.12, now + 0.2);
            gain2.gain.exponentialRampToValueAtTime(0.001, now + 0.65);
            osc2.connect(gain2);
            gain2.connect(ctx.destination);
            osc2.start(now + 0.2);
            osc2.stop(now + 0.65);
        } catch (e) {
            console.debug('WebAudio play omitted:', e);
        }
    },

    // -------------------------------------------------------------------------
    // Portal Quick Switcher (Injected in Page Header)
    // -------------------------------------------------------------------------
    injectPortalSwitcher(activeModule = '') {
        const navContainer = document.querySelector('header .max-w-7xl');
        if (!navContainer || document.getElementById('portal-quick-switcher')) return;

        const switcher = document.createElement('div');
        switcher.id = 'portal-quick-switcher';
        switcher.className = 'portal-quick-switcher hidden lg:inline-flex';
        switcher.innerHTML = `
            <a href="/patient.html" class="portal-switch-pill ${activeModule === 'patient' ? 'active' : ''}" title="Citizen Emergency SOS">
                <i class="fa-solid fa-person-falling-burst"></i>
                <span>Citizen SOS</span>
            </a>
            <a href="/driver.html" class="portal-switch-pill ${activeModule === 'driver' ? 'active' : ''}" title="Ambulance Cockpit">
                <i class="fa-solid fa-truck-medical"></i>
                <span>Driver</span>
            </a>
            <a href="/hospital.html" class="portal-switch-pill ${activeModule === 'hospital' ? 'active' : ''}" title="Hospital ER & Beds">
                <i class="fa-solid fa-hospital"></i>
                <span>Hospital</span>
            </a>
            <a href="/index.html" class="portal-switch-pill ${activeModule === 'admin' ? 'active' : ''}" title="System Admin Console">
                <i class="fa-solid fa-shield-halved"></i>
                <span>Admin</span>
            </a>
        `;

        // Insert after brand logo
        const brandEl = navContainer.querySelector('.flex.items-center.space-x-3');
        if (brandEl && brandEl.nextSibling) {
            navContainer.insertBefore(switcher, brandEl.nextSibling);
        } else {
            navContainer.appendChild(switcher);
        }
    }
};

window.Common = Common;
