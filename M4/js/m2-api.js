// M2: Healthcare Discovery API Client (Rishi Jain)

const M2_API = {
    get baseUrl() {
        if (typeof Common !== 'undefined' && typeof Common.getApiBase === 'function') {
            const b = Common.getApiBase();
            return b ? `${b}/api` : '/api';
        }
        const isLocal = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
        return isLocal ? 'http://localhost:8080/api' : '/api';
    },

    // Fallback static hospital registry if C++ server is offline
    defaultHospitals: [
        {
            id: "LOC-004",
            name: "Max Super Speciality Hospital",
            address: "Mussoorie Diversion Road, Johri, Malsi, Dehradun - 248001",
            phone: "+91 135 716 2222",
            emergency: "24/7 Available",
            totalBeds: 220,
            icuBeds: 28,
            availableIcu: 6,
            oxygenBeds: 45,
            specialties: ["Trauma & Emergency", "Cardiology", "Neurology", "Oncology", "Orthopedics"],
            latitude: 30.3812,
            longitude: 78.0772,
            rating: 4.8
        },
        {
            id: "LOC-005",
            name: "Shri Mahant Indiresh Hospital (SMIH)",
            address: "Patel Nagar, Dehradun, Uttarakhand - 248001",
            phone: "+91 135 667 2600",
            emergency: "24/7 Trauma Center",
            totalBeds: 1500,
            icuBeds: 120,
            availableIcu: 18,
            oxygenBeds: 250,
            specialties: ["Trauma & Emergency", "General Care", "Nephrology", "Cardiology"],
            latitude: 30.3124,
            longitude: 78.0289,
            rating: 4.6
        },
        {
            id: "LOC-006",
            name: "Synergy Institute of Medical Sciences",
            address: "Ballupur Chowk, Dehradun, Uttarakhand - 248001",
            phone: "+91 135 253 1111",
            emergency: "24/7 Critical Care",
            totalBeds: 180,
            icuBeds: 45,
            availableIcu: 9,
            oxygenBeds: 60,
            specialties: ["Neurology", "Cardiology", "Trauma & Emergency", "Critical Care"],
            latitude: 30.3345,
            longitude: 78.0212,
            rating: 4.7
        },
        {
            id: "LOC-007",
            name: "Graphic Era Institute of Medical Sciences (GEIMS)",
            address: "16th Milestone, Chakrata Road, Dhoolkot, Dehradun - 248008",
            phone: "+91 180 088 97351",
            emergency: "24/7 Emergency",
            totalBeds: 500,
            icuBeds: 80,
            availableIcu: 14,
            oxygenBeds: 120,
            specialties: ["Trauma & Emergency", "General Care", "Orthopedics", "Paediatrics"],
            latitude: 30.3521,
            longitude: 77.9654,
            rating: 4.9
        },
        {
            id: "LOC-011",
            name: "Doon Government Hospital",
            address: "New Road, Race Course, Dehradun, Uttarakhand - 248001",
            phone: "+91 135 265 9227",
            emergency: "24/7 Govt Emergency",
            totalBeds: 350,
            icuBeds: 30,
            availableIcu: 4,
            oxygenBeds: 90,
            specialties: ["Trauma & Emergency", "General Care"],
            latitude: 30.3185,
            longitude: 78.0412,
            rating: 4.3
        },
        {
            id: "LOC-009",
            name: "CMI Hospital",
            address: "54 Haridwar Road, Dharampur, Dehradun, Uttarakhand - 248001",
            phone: "+91 135 262 2333",
            emergency: "24/7 Emergency Wing",
            totalBeds: 100,
            icuBeds: 20,
            availableIcu: 3,
            oxygenBeds: 40,
            specialties: ["General Care", "Gynaecology", "Paediatrics"],
            latitude: 30.3142,
            longitude: 78.0511,
            rating: 4.4
        }
    ],

    async fetchHospitals(query = '', specialty = 'all') {
        try {
            const baseStr = this.baseUrl.startsWith('http') ? this.baseUrl : `${window.location.origin}${this.baseUrl}`;
            const url = new URL(`${baseStr}/hospitals/search`);
            if (query) url.searchParams.set('query', query);
            if (specialty && specialty !== 'all') url.searchParams.set('specialty', specialty);

            const res = await fetch(url.toString());
            if (res.ok) {
                const json = await res.json();
                if (json.data && json.data.length > 0) {
                    return json.data;
                }
            }
        } catch (e) {
            console.warn('[M2] Server search fallback:', e);
        }

        // Local fallback filtering
        return this.defaultHospitals.filter(h => {
            const matchesQuery = !query ||
                h.name.toLowerCase().includes(query.toLowerCase()) ||
                h.address.toLowerCase().includes(query.toLowerCase());
            const matchesSpec = specialty === 'all' || h.specialties.includes(specialty);
            return matchesQuery && matchesSpec;
        });
    }
};

window.M2_API = M2_API;
