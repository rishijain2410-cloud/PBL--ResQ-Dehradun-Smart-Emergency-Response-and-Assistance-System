/**
 * JEEVANROUTE — Smart Emergency Response & Assistance System
 * Vanilla JavaScript Frontend Architecture (M1, M2, M3 Modules)
 */

// 1. CONFIGURATION & FUTURE API ADAPTER
const APP_CONFIG = {
  mode: "demo",
  apiBaseUrl: "http://127.0.0.1:8080",
  storageKey: "JEEVANROUTE_DATA_V2.4"
};

// 2. SEED DEMONSTRATION DATA (Dehradun Graph, Locations, Hospitals, Services, Requests, Contacts)
const SEED_DATA = {
  locations: [
    { id: "LOC-001", name: "Clock Tower", category: "Landmark", lat: 30.3165, lng: 78.0322 },
    { id: "LOC-002", name: "ISBT Dehradun", category: "Transit", lat: 30.2797, lng: 78.0038 },
    { id: "LOC-003", name: "Dehradun Railway Station", category: "Transit", lat: 30.3113, lng: 78.0381 },
    { id: "LOC-004", name: "Rajpur Road", category: "Commercial", lat: 30.3344, lng: 78.0432 },
    { id: "LOC-005", name: "Clement Town", category: "Residential", lat: 30.2522, lng: 77.9945 },
    { id: "LOC-006", name: "Prem Nagar", category: "Residential", lat: 30.3392, lng: 77.9534 },
    { id: "LOC-007", name: "Ballupur Chowk", category: "Junction", lat: 30.3245, lng: 78.0185 },
    { id: "LOC-008", name: "Gandhi Park", category: "Landmark", lat: 30.3201, lng: 78.0411 },
    { id: "LOC-009", name: " Raipur", category: "Residential", lat: 30.3312, lng: 78.1023 },
    { id: "LOC-010", name: "Sahastradhara Crossing", category: "Junction", lat: 30.3521, lng: 78.0754 },
    { id: "LOC-011", name: "Doon Government Hospital", category: "Hospital", lat: 30.3195, lng: 78.0350 },
    { id: "LOC-012", name: "Apollo Clinic and Hospital", category: "Hospital", lat: 30.3385, lng: 78.0480 },
    { id: "LOC-013", name: "Parade Ground Emergency Station", category: "Emergency", lat: 30.3220, lng: 78.0435 },
    { id: "LOC-014", name: "Dehradun Police Headquarters", category: "Police", lat: 30.3100, lng: 78.0450 },
    { id: "LOC-015", name: "Fire Station Rajpur Road", category: "Fire", lat: 30.3400, lng: 78.0500 },
    { id: "LOC-016", name: "Central Trauma Center", category: "Hospital", lat: 30.3140, lng: 78.0390 },
    { id: "LOC-017", name: "City Hospital Rajpur Road", category: "Hospital", lat: 30.3420, lng: 78.0520 },
    { id: "LOC-018", name: "Synergy Community Hospital", category: "Hospital", lat: 30.2980, lng: 78.0120 },
    { id: "LOC-019", name: "Paltan Bazaar Emergency Post", category: "Emergency", lat: 30.3130, lng: 78.0370 },
    { id: "LOC-020", name: "Clock Tower Emergency Dispatch Post", category: "Emergency", lat: 30.3170, lng: 78.0325 }
  ],

  // 30 Road Edges for C++ M3 Graph Simulation
  edges: [
    { from: "LOC-001", to: "LOC-011", road: "Court Road", distance: 0.45 },
    { from: "LOC-001", to: "LOC-003", road: "Station Road", distance: 1.20 },
    { from: "LOC-001", to: "LOC-008", road: "Gandhi Marg", distance: 0.60 },
    { from: "LOC-001", to: "LOC-013", road: "Parade Avenue", distance: 0.80 },
    { from: "LOC-003", to: "LOC-019", road: "Paltan Street", distance: 0.35 },
    { from: "LOC-003", to: "LOC-014", road: "Police Line Rd", distance: 0.90 },
    { from: "LOC-003", to: "LOC-002", road: "ISBT Expressway", distance: 4.50 },
    { from: "LOC-002", to: "LOC-005", road: "Clement Highway", distance: 3.20 },
    { from: "LOC-002", to: "LOC-018", road: "Synergy Link", distance: 2.80 },
    { from: "LOC-007", to: "LOC-001", road: "Chakrata Road", distance: 2.10 },
    { from: "LOC-007", to: "LOC-003", road: "Station Link", distance: 1.80 },
    { from: "LOC-007", to: "LOC-016", road: "Trauma Bypass", distance: 1.10 },
    { from: "LOC-004", to: "LOC-012", road: "Apollo Way", distance: 0.50 },
    { from: "LOC-004", to: "LOC-017", road: "City Hospital Rd", distance: 0.85 },
    { from: "LOC-004", to: "LOC-015", road: "Fire Station Rd", distance: 1.40 },
    { from: "LOC-004", to: "LOC-010", road: "Sahastradhara Rd", distance: 3.10 },
    { from: "LOC-010", to: "LOC-009", road: "Raipur Connector", distance: 2.50 },
    { from: "LOC-008", to: "LOC-004", road: "Rajpur Main", distance: 1.50 },
    { from: "LOC-011", to: "LOC-016", road: "Hospital Lane", distance: 0.30 },
    { from: "LOC-013", to: "LOC-004", road: "Parade North", distance: 1.20 },
    { from: "LOC-005", to: "LOC-007", road: "Clement-Ballupur Rd", distance: 4.10 },
    { from: "LOC-006", to: "LOC-007", road: "Prem Nagar Main", distance: 5.20 },
    { from: "LOC-006", to: "LOC-002", road: "Prem-ISBT Link", distance: 4.80 },
    { from: "LOC-012", to: "LOC-017", road: "Rajpur Medical Corridor", distance: 0.70 },
    { from: "LOC-015", to: "LOC-010", road: "Fire-Sahastradhara Rd", distance: 2.20 },
    { from: "LOC-018", to: "LOC-007", road: "Synergy-Ballupur Link", distance: 3.50 },
    { from: "LOC-019", to: "LOC-008", road: "Bazaar-Gandhi Link", distance: 0.75 },
    { from: "LOC-014", to: "LOC-013", road: "Police-Parade Way", distance: 0.55 },
    { from: "LOC-020", to: "LOC-001", road: "Clock Tower Post Rd", distance: 0.10 },
    { from: "LOC-020", to: "LOC-011", distance: 0.32, road: "Dispatch Direct Rd" }
  ],

  hospitals: [
    {
      id: "HSP-001",
      name: "Doon Government Hospital",
      location: "LOC-011 (Court Road)",
      departments: ["Emergency & Trauma", "Cardiology", "Orthopedics", "Pediatrics"],
      doctors: ["Dr. R. K. Verma", "Dr. Sunita Rawat", "Dr. A. K. Gupta"],
      services: ["Trauma Center", "ICU", "Oxygen Support", "Blood Bank", "Ambulance Dispatch"],
      bedsAvailable: 42,
      totalBeds: 150,
      rating: 4.4,
      phone: "+91 135 2659220",
      notes: "Primary government referral hospital in central Dehradun."
    },
    {
      id: "HSP-002",
      name: "Apollo Clinic and Hospital",
      location: "LOC-012 (Rajpur Road)",
      departments: ["Emergency & Trauma", "Cardiology", "Neurology", "General Medicine"],
      doctors: ["Dr. Manoj Thapa", "Dr. Priya Sharma"],
      services: ["Trauma Center", "ICU", "Oxygen Support", "Pharmacy 24x7"],
      bedsAvailable: 18,
      totalBeds: 75,
      rating: 4.6,
      phone: "+91 135 2741122",
      notes: "Advanced multispecialty care facility on Rajpur Road."
    },
    {
      id: "HSP-003",
      name: "City Hospital Rajpur Road",
      location: "LOC-017 (Rajpur Road)",
      departments: ["Orthopedics", "Pediatrics", "Emergency & Trauma"],
      doctors: ["Dr. Suresh Nautiyal", "Dr. Neha Negi"],
      services: ["ICU", "Oxygen Support", "Ambulance Dispatch"],
      bedsAvailable: 12,
      totalBeds: 50,
      rating: 4.2,
      phone: "+91 135 2743300",
      notes: "Reliable multispecialty hospital with dedicated emergency ward."
    },
    {
      id: "HSP-004",
      name: "Synergy Community Hospital",
      location: "LOC-018 (Clement Town)",
      departments: ["Cardiology", "Neurology", "Emergency & Trauma", "Orthopedics"],
      doctors: ["Dr. V. K. Pant", "Dr. Anjali Joshi"],
      services: ["Trauma Center", "ICU", "Oxygen Support", "Blood Bank", "Helipad"],
      bedsAvailable: 25,
      totalBeds: 100,
      rating: 4.7,
      phone: "+91 135 2642200",
      notes: "State-of-the-art hospital facility with advanced trauma care."
    },
    {
      id: "HSP-005",
      name: "Central Trauma Center",
      location: "LOC-016 (Near Railway Station)",
      departments: ["Emergency & Trauma", "Orthopedics"],
      doctors: ["Dr. Naveen Chauhan"],
      services: ["Trauma Center", "ICU", "Oxygen Support"],
      bedsAvailable: 8,
      totalBeds: 30,
      rating: 4.1,
      phone: "+91 135 2721100",
      notes: "Specialized emergency trauma stabilization center."
    }
  ],

  services: [
    { id: "SRV-001", name: "Doon Emergency Ambulance 1", category: "Ambulance", status: "Available", contact: "+91 135 2650001", location: "LOC-001 (Clock Tower)" },
    { id: "SRV-002", name: "Railway Station Rapid Ambulance", category: "Ambulance", status: "Available", contact: "+91 135 2650002", location: "LOC-003 (Railway Station)" },
    { id: "SRV-003", name: "ISBT Transit Ambulance", category: "Ambulance", status: "Assigned", contact: "+91 135 2650003", location: "LOC-002 (ISBT)" },
    { id: "SRV-004", name: "Air Ambulance Unit Alpha", category: "Air Ambulance", status: "Available", contact: "+91 135 2650004", location: "LOC-018 (Synergy Helipad)" },
    { id: "SRV-005", name: "Dehradun Police Headquarters Unit", category: "Police", status: "Available", contact: "+91 135 2710100", location: "LOC-014 (Police HQ)" },
    { id: "SRV-006", name: "Paltan Bazaar Police Post", category: "Police", status: "Available", contact: "+91 135 2710101", location: "LOC-019 (Paltan Bazaar)" },
    { id: "SRV-007", name: "Fire Station Rajpur Road", category: "Fire Service", status: "Available", contact: "+91 135 2710200", location: "LOC-015 (Fire Station)" },
    { id: "SRV-008", name: "Parade Ground Emergency Post", category: "Ambulance", status: "Available", contact: "+91 135 2650005", location: "LOC-013 (Parade Ground)" }
  ],

  requests: [
    {
      id: "REQ-1001",
      incidentType: "Medical Emergency",
      description: "Severe chest pain reported near Clock Tower.",
      locationId: "LOC-001",
      locationName: "Clock Tower",
      lat: 30.3165,
      lng: 78.0322,
      priority: "Critical",
      requiredService: "Ambulance",
      assignedService: "SRV-001 (Doon Emergency Ambulance 1)",
      status: "En Route",
      contact: "+91 98765 43210",
      timestamp: new Date(Date.now() - 15 * 60000).toISOString()
    },
    {
      id: "REQ-1002",
      incidentType: "Road Accident",
      description: "Two-wheeler collision near ISBT Dehradun expressway.",
      locationId: "LOC-002",
      locationName: "ISBT Dehradun",
      lat: 30.2797,
      lng: 78.0038,
      priority: "High",
      requiredService: "Ambulance",
      assignedService: "SRV-003 (ISBT Transit Ambulance)",
      status: "Accepted",
      contact: "+91 98111 22334",
      timestamp: new Date(Date.now() - 45 * 60000).toISOString()
    },
    {
      id: "REQ-1003",
      incidentType: "Fire",
      description: "Minor electrical short circuit in commercial shop.",
      locationId: "LOC-004",
      locationName: "Rajpur Road",
      lat: 30.3344,
      lng: 78.0432,
      priority: "Medium",
      requiredService: "Fire Service",
      assignedService: "SRV-007 (Fire Station Rajpur Road)",
      status: "Completed",
      contact: "+91 97123 45678",
      timestamp: new Date(Date.now() - 120 * 60000).toISOString()
    }
  ],

  contacts: [
    { id: "CON-001", name: "Doon Government Hospital Dispatch", category: "Hospital Dispatch", phone: "+91 135 2659220", notes: "24x7 Emergency helpline & trauma desk" },
    { id: "CON-002", name: "Dehradun Police Control Room", category: "Police Control", phone: "+91 135 2710100", notes: "Main police emergency dispatch HQ" },
    { id: "CON-003", name: "Fire Station Rajpur Road", category: "Fire Station", phone: "+91 135 2710200", notes: "Central fire emergency response unit" },
    { id: "CON-004", name: "Apollo Clinic Emergency Desk", category: "Trauma Specialist", phone: "+91 135 2741122", notes: "Ambulance & trauma coordination desk" },
    { id: "CON-005", name: "Municipal Disaster Management Cell", category: "Municipal Authority", phone: "+91 135 2710500", notes: "District emergency operations center" }
  ]
};

// 3. APPLICATION STATE
let appState = {
  currentView: "dashboard",
  requests: [],
  services: [],
  hospitals: [],
  locations: [],
  edges: [],
  contacts: [],
  activeRoute: null,
  hospitalServiceFilter: "",
  notifications: [
    { id: 1, text: "Emergency REQ-1001 status updated to En Route.", time: "10 mins ago" },
    { id: 2, text: "Ambulance SRV-003 dispatched to ISBT Dehradun.", time: "30 mins ago" },
    { id: 3, text: "System M3 Dijkstra Graph verified (20 nodes, 30 edges).", time: "1 hour ago" }
  ]
};

// 4. LOCAL STORAGE PERSISTENCE & INITIALIZATION
function initApp() {
  loadDataFromStorage();
  setupEventListeners();
  updateDateTime();
  setInterval(updateDateTime, 1000);
  renderCurrentView();
  updateKPIs();
  renderNotificationsBadge();
}

function loadDataFromStorage() {
  try {
    const stored = localStorage.getItem(APP_CONFIG.storageKey);
    if (stored) {
      const parsed = JSON.parse(stored);
      appState.requests = parsed.requests || SEED_DATA.requests;
      appState.services = parsed.services || SEED_DATA.services;
      appState.contacts = parsed.contacts || SEED_DATA.contacts;
      appState.hospitals = SEED_DATA.hospitals || SEED_DATA.hospitals;
      appState.locations = SEED_DATA.locations || SEED_DATA.locations;
      appState.edges = SEED_DATA.edges || SEED_DATA.edges;
    } else {
      resetToSeedData();
    }
  } catch (e) {
    console.warn("Storage load error, falling back to seed data:", e);
    resetToSeedData();
  }
}

function saveDataToStorage() {
  try {
    const dataToSave = {
      requests: appState.requests,
      services: appState.services,
      contacts: appState.contacts,
      hospitals: appState.hospitals,
      locations: appState.locations,
      edges: appState.edges
    };
    localStorage.setItem(APP_CONFIG.storageKey, JSON.stringify(dataToSave));
  } catch (e) {
    console.error("Failed to save to localStorage:", e);
  }
}

function resetToSeedData() {
  appState.requests = JSON.parse(JSON.stringify(SEED_DATA.requests));
  appState.services = JSON.parse(JSON.stringify(SEED_DATA.services));
  appState.contacts = JSON.parse(JSON.stringify(SEED_DATA.contacts));
  appState.hospitals = JSON.parse(JSON.stringify(SEED_DATA.hospitals));
  appState.locations = JSON.parse(JSON.stringify(SEED_DATA.locations));
  appState.edges = JSON.parse(JSON.stringify(SEED_DATA.edges));
  saveDataToStorage();
  showToast("Demo data successfully reset to initial seed.", "success");
  renderCurrentView();
  updateKPIs();
}

// 5. SIDEBAR NAVIGATION
function setupEventListeners() {
  const navButtons = document.querySelectorAll('.sidebar-nav .nav-item');
  navButtons.forEach(btn => {
    btn.addEventListener('click', () => {
      const viewId = btn.getAttribute('data-view');
      switchView(viewId);
    });
  });

  const mobileMenuBtn = document.getElementById('mobileMenuBtn');
  if (mobileMenuBtn) {
    mobileMenuBtn.addEventListener('click', () => {
      document.getElementById('sidebar').classList.toggle('mobile-open');
    });
  }

  const resetBtn = document.getElementById('resetDemoBtn');
  if (resetBtn) {
    resetBtn.addEventListener('click', () => {
      if (confirm("Reset all demo requests, services, and contacts to initial demo state?")) {
        resetToSeedData();
      }
    });
  }
}

function switchView(viewId) {
  appState.currentView = viewId;

  // Update sidebar active class
  document.querySelectorAll('.sidebar-nav .nav-item').forEach(item => {
    if (item.getAttribute('data-view') === viewId) {
      item.classList.add('active');
    } else {
      item.classList.remove('active');
    }
  });

  // Update section visibility
  document.querySelectorAll('.view-section').forEach(sec => {
    sec.classList.remove('active');
  });
  const targetSec = document.getElementById(`view-${viewId}`);
  if (targetSec) {
    targetSec.classList.add('active');
  }

  // Update top nav title
  const titles = {
    dashboard: "Emergency Response Command Center",
    emergency: "Emergency & SOS Module",
    ambulances: "Ambulances & Emergency Services",
    hospitals: "Hospital Finder — M2 Module",
    navigation: "Navigation & Routes — M3 Module",
    contacts: "Emergency Contacts Directory",
    history: "Request History & Timeline"
  };
  document.getElementById('currentPageTitle').textContent = titles[viewId] || "Dashboard";

  // Close mobile sidebar if open
  document.getElementById('sidebar').classList.remove('mobile-open');

  // Render specific view data
  renderCurrentView();
}

function renderCurrentView() {
  updateKPIs();
  switch(appState.currentView) {
    case 'dashboard':
      renderDashboardTable();
      break;
    case 'emergency':
      renderEmergencyList();
      break;
    case 'ambulances':
      renderServicesList();
      break;
    case 'hospitals':
      renderHospitalCatalog();
      break;
    case 'navigation':
      initNavigationModule();
      break;
    case 'contacts':
      renderContactsList();
      break;
    case 'history':
      renderHistoryList();
      break;
  }
}


// Initialize state
document.addEventListener('DOMContentLoaded', () => {
  initStorage();
  initNavigation();
});
