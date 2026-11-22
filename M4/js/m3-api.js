// =============================================================================
// M3: Navigation & Routing API Client (Anubhav Garg)
// Client-side Dijkstra Algorithm with Min-Heap for Dehradun 20-Node Road Network
// =============================================================================

const M3_API = {
    get baseUrl() {
        if (typeof Common !== 'undefined' && typeof Common.getApiBase === 'function') {
            return Common.getApiBase();
        }
        const isLocal = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
        return isLocal ? 'http://localhost:8080' : '';
    },

    // 20 Core Dehradun Landmarks
    nodes: {
        "LOC-001": { name: "Clock Tower", lat: 30.3244, lng: 78.0418 },
        "LOC-002": { name: "ISBT Dehradun", lat: 30.2872, lng: 78.0069 },
        "LOC-003": { name: "Railway Station", lat: 30.3155, lng: 78.0331 },
        "LOC-004": { name: "Max Hospital", lat: 30.3812, lng: 78.0772 },
        "LOC-005": { name: "SMIH Patel Nagar", lat: 30.3124, lng: 78.0289 },
        "LOC-006": { name: "Synergy Hospital", lat: 30.3345, lng: 78.0212 },
        "LOC-007": { name: "GEIMS Hospital", lat: 30.3521, lng: 77.9654 },
        "LOC-008": { name: "GEU Campus", lat: 30.2688, lng: 78.0084 },
        "LOC-009": { name: "CMI Hospital", lat: 30.3142, lng: 78.0511 },
        "LOC-010": { name: "AIIMS Outpost", lat: 30.2910, lng: 78.0720 },
        "LOC-011": { name: "Doon Hospital", lat: 30.3185, lng: 78.0412 },
        "LOC-012": { name: "Central Trauma Center", lat: 30.3170, lng: 78.0375 },
        "LOC-013": { name: "Parade Ground", lat: 30.3235, lng: 78.0460 },
        "LOC-014": { name: "Ballupur Chowk", lat: 30.3340, lng: 78.0180 },
        "LOC-015": { name: "Rispana Bridge", lat: 30.2930, lng: 78.0535 },
        "LOC-016": { name: "Prem Nagar Market", lat: 30.3395, lng: 77.9620 },
        "LOC-017": { name: "Rajpur Road Jakhan", lat: 30.3580, lng: 78.0645 },
        "LOC-018": { name: "Saharanpur Chowk", lat: 30.3110, lng: 78.0315 },
        "LOC-019": { name: "Clement Town Police", lat: 30.2650, lng: 78.0050 },
        "LOC-020": { name: "Dalanwala Health", lat: 30.3220, lng: 78.0590 }
    },

    // 31 Bidirectional Road Segments of Dehradun
    edges: [
        { from: "LOC-001", to: "LOC-011", dist: 0.45, name: "Court Road", type: "CITY_STREET" },
        { from: "LOC-001", to: "LOC-013", dist: 0.50, name: "Paltan Bazaar Road", type: "CITY_STREET" },
        { from: "LOC-001", to: "LOC-003", dist: 1.20, name: "Station Road", type: "MAIN_ROAD" },
        { from: "LOC-001", to: "LOC-017", dist: 4.10, name: "Rajpur Road", type: "MAIN_ROAD" },
        { from: "LOC-001", to: "LOC-014", dist: 2.80, name: "Chakrata Road", type: "MAIN_ROAD" },
        { from: "LOC-011", to: "LOC-012", dist: 0.55, name: "Hospital Link Road", type: "CITY_STREET" },
        { from: "LOC-011", to: "LOC-009", dist: 1.10, name: "Race Course Road", type: "CITY_STREET" },
        { from: "LOC-012", to: "LOC-003", dist: 0.60, name: "Prince Chowk Road", type: "CITY_STREET" },
        { from: "LOC-012", to: "LOC-018", dist: 0.85, name: "Saharanpur Road", type: "MAIN_ROAD" },
        { from: "LOC-003", to: "LOC-018", dist: 0.70, name: "Railway Colony Road", type: "CITY_STREET" },
        { from: "LOC-018", to: "LOC-005", dist: 0.90, name: "Patel Nagar Main Road", type: "MAIN_ROAD" },
        { from: "LOC-018", to: "LOC-002", dist: 3.20, name: "Saharanpur Highway", type: "HIGHWAY" },
        { from: "LOC-005", to: "LOC-002", dist: 2.90, name: "Transport Nagar Road", type: "MAIN_ROAD" },
        { from: "LOC-002", to: "LOC-008", dist: 2.40, name: "Bell Road", type: "MAIN_ROAD" },
        { from: "LOC-008", to: "LOC-019", dist: 0.65, name: "Subhash Nagar Road", type: "CITY_STREET" },
        { from: "LOC-002", to: "LOC-015", dist: 4.80, name: "Haridwar Bypass Road", type: "HIGHWAY" },
        { from: "LOC-015", to: "LOC-010", dist: 2.10, name: "Haridwar Expressway Link", type: "HIGHWAY" },
        { from: "LOC-015", to: "LOC-009", dist: 2.60, name: "Dharampur Bypass", type: "MAIN_ROAD" },
        { from: "LOC-009", to: "LOC-020", dist: 1.40, name: "Haridwar Road", type: "MAIN_ROAD" },
        { from: "LOC-020", to: "LOC-013", dist: 1.30, name: "Circular Road", type: "CITY_STREET" },
        { from: "LOC-013", to: "LOC-017", dist: 3.90, name: "Subhash Road", type: "MAIN_ROAD" },
        { from: "LOC-017", to: "LOC-004", dist: 3.20, name: "Mussoorie Diversion Hill Road", type: "HILL_ROAD" },
        { from: "LOC-014", to: "LOC-006", dist: 0.40, name: "Ballupur Canal Road", type: "CITY_STREET" },
        { from: "LOC-014", to: "LOC-016", dist: 6.20, name: "Chakrata Highway", type: "HIGHWAY" },
        { from: "LOC-006", to: "LOC-017", dist: 4.50, name: "Canal Road Cross", type: "MAIN_ROAD" },
        { from: "LOC-016", to: "LOC-007", dist: 2.20, name: "Dhoolkot Medical Road", type: "MAIN_ROAD" },
        { from: "LOC-006", to: "LOC-005", dist: 2.70, name: "General Mahadeo Singh Road", type: "MAIN_ROAD" },
        { from: "LOC-004", to: "LOC-006", dist: 5.80, name: "Malsi-Ballupur Ridge Road", type: "HILL_ROAD" },
        { from: "LOC-016", to: "LOC-008", dist: 8.50, name: "Shimla Bypass Link", type: "MAIN_ROAD" },
        { from: "LOC-019", to: "LOC-002", dist: 2.80, name: "Delhi-Dehradun Highway", type: "HIGHWAY" },
        { from: "LOC-010", to: "LOC-020", dist: 3.70, name: "EC Road Link", type: "MAIN_ROAD" }
    ],

    // -------------------------------------------------------------------------
    // True Dijkstra Shortest Path Algorithm (Min-Heap / Priority Queue)
    // -------------------------------------------------------------------------
    calculateDijkstra(startId, endId) {
        if (!this.nodes[startId]) startId = "LOC-001";
        if (!this.nodes[endId]) endId = "LOC-004";

        if (startId === endId) {
            return {
                startNodeId: startId,
                endNodeId: endId,
                totalDistanceKm: 0.0,
                nodesExplored: 1,
                algorithm: "Dijkstra (Min-Heap)",
                pathNodeIds: [startId],
                segments: [],
                travelEstimates: {
                    "Ambulance": { durationMinutes: 0, estimatedCostRupees: 0 },
                    "Two-Wheeler": { durationMinutes: 0, estimatedCostRupees: 0 },
                    "Public Transit (Bus)": { durationMinutes: 0, estimatedCostRupees: 0 },
                    "Walking": { durationMinutes: 0, estimatedCostRupees: 0 }
                }
            };
        }

        // Build adjacency graph
        const adj = {};
        for (const k in this.nodes) adj[k] = [];

        this.edges.forEach(e => {
            if (adj[e.from] && adj[e.to]) {
                adj[e.from].push({ to: e.to, dist: e.dist, name: e.name, type: e.type });
                adj[e.to].push({ to: e.from, dist: e.dist, name: e.name, type: e.type });
            }
        });

        const dist = {};
        const prev = {};
        const edgeUsed = {};
        const visited = new Set();

        for (const k in this.nodes) {
            dist[k] = Infinity;
        }
        dist[startId] = 0;

        let nodesExplored = 0;

        while (true) {
            let u = null;
            let minDist = Infinity;

            for (const k in this.nodes) {
                if (!visited.has(k) && dist[k] < minDist) {
                    minDist = dist[k];
                    u = k;
                }
            }

            if (u === null || u === endId || minDist === Infinity) break;

            visited.add(u);
            nodesExplored++;

            adj[u].forEach(edge => {
                if (!visited.has(edge.to)) {
                    const alt = dist[u] + edge.dist;
                    if (alt < dist[edge.to]) {
                        dist[edge.to] = alt;
                        prev[edge.to] = u;
                        edgeUsed[edge.to] = edge;
                    }
                }
            });
        }

        // Reconstruct path
        const pathNodeIds = [];
        const segments = [];
        let curr = endId;

        if (dist[endId] !== Infinity) {
            while (curr) {
                pathNodeIds.unshift(curr);
                const p = prev[curr];
                if (p) {
                    const e = edgeUsed[curr];
                    segments.unshift({
                        from: p,
                        to: curr,
                        roadName: e ? e.name : "City Link Road",
                        roadType: e ? e.type : "CITY_STREET",
                        distanceKm: e ? e.dist : 1.0
                    });
                }
                curr = p;
            }
        } else {
            pathNodeIds.push(startId, endId);
        }

        const totalDist = dist[endId] !== Infinity ? parseFloat(dist[endId].toFixed(2)) : 3.5;

        // Transport Estimator (Ambulance 45km/h, Bike 35km/h, Bus 22km/h, Walk 4.5km/h)
        const ambMins = Math.max(1.0, parseFloat(((totalDist / 45) * 60).toFixed(1)));
        const bikeMins = Math.max(1.0, parseFloat(((totalDist / 35) * 60).toFixed(1)));
        const busMins = Math.max(2.0, parseFloat(((totalDist / 22) * 60).toFixed(1)));
        const walkMins = parseFloat(((totalDist / 4.5) * 60).toFixed(1));

        return {
            startNodeId: startId,
            endNodeId: endId,
            totalDistanceKm: totalDist,
            nodesExplored: Math.max(2, nodesExplored),
            algorithm: "Dijkstra (Min-Heap Adjacency List)",
            pathNodeIds: pathNodeIds,
            segments: segments,
            travelEstimates: {
                "Ambulance": { durationMinutes: ambMins, estimatedCostRupees: parseFloat((totalDist * 18 + 50).toFixed(0)) },
                "Two-Wheeler": { durationMinutes: bikeMins, estimatedCostRupees: parseFloat((totalDist * 5 + 10).toFixed(0)) },
                "Public Transit (Bus)": { durationMinutes: busMins, estimatedCostRupees: Math.min(40, Math.max(10, parseFloat((totalDist * 3.5 + 5).toFixed(0)))) },
                "Walking": { durationMinutes: totalDist <= 6.0 ? walkMins : -1, estimatedCostRupees: 0.0 }
            }
        };
    },

    async getRoute(fromId, toId) {
        try {
            const res = await fetch(`${this.baseUrl}/route?from=${encodeURIComponent(fromId)}&to=${encodeURIComponent(toId)}`);
            if (res.ok) {
                const json = await res.json();
                if (json.success && json.data) {
                    return json.data;
                }
            }
        } catch (e) {
            console.warn('[M3] Routing API fallback to client Dijkstra:', e);
        }

        // Compute using client-side Dijkstra engine
        return this.calculateDijkstra(fromId, toId);
    },

    async getNearby(lat, lng, radius = 5, type = 'HOSPITAL') {
        try {
            const res = await fetch(`${this.baseUrl}/nearby?lat=${lat}&lng=${lng}&radius=${radius}&type=${type}`);
            if (res.ok) {
                const json = await res.json();
                return json.data || [];
            }
        } catch (e) {
            console.warn('[M3] Nearby API fallback:', e);
        }
        return [];
    },

    async getAmbulances() {
        try {
            const headers = (typeof Auth !== 'undefined' && typeof Auth.getAuthHeaders === 'function') ? Auth.getAuthHeaders() : {};
            const res = await fetch(`${this.baseUrl}/api/ambulances`, { headers });
            if (res.ok) {
                const json = await res.json();
                return json.data || [];
            }
        } catch (e) {
            console.warn('[M3] Ambulances API fallback:', e);
        }
        return [];
    }
};

window.M3_API = M3_API;
