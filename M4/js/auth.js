// JEEVAN MARG - Client-side Authentication & Session Management
// Provides token management, API helpers, and RBAC utilities

const Auth = {
    TOKEN_KEY: 'jeevan_marg_auth_token',
    USER_KEY: 'jeevan_marg_auth_user',

    resolveUrl(path) {
        if (typeof Common !== 'undefined' && typeof Common.apiUrl === 'function') {
            return Common.apiUrl(path);
        }
        return path;
    },

    getToken() {
        return localStorage.getItem(this.TOKEN_KEY);
    },

    getUser() {
        try {
            const raw = localStorage.getItem(this.USER_KEY);
            return raw ? JSON.parse(raw) : null;
        } catch (e) {
            return null;
        }
    },

    setSession(token, user) {
        localStorage.setItem(this.TOKEN_KEY, token);
        localStorage.setItem(this.USER_KEY, JSON.stringify(user));
    },

    clearSession() {
        localStorage.removeItem(this.TOKEN_KEY);
        localStorage.removeItem(this.USER_KEY);
    },

    isLoggedIn() {
        return !!this.getToken();
    },

    getAuthHeaders() {
        const headers = { 'Content-Type': 'application/json' };
        const token = this.getToken();
        if (token) {
            headers['Authorization'] = 'Bearer ' + token;
        }
        return headers;
    },

    async login(username, password) {
        try {
            const res = await fetch(this.resolveUrl('/api/auth/login'), {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            });
            const data = await res.json();
            if (!res.ok) {
                throw new Error(data.error?.message || data.message || 'Login failed');
            }
            this.setSession(data.token, data.user);
            return data;
        } catch (netErr) {
            // Showcase Cloud Mode: When deployed on Netlify without local C++ daemon, authenticate pre-seeded demo accounts
            const uLower = (username || '').toLowerCase().trim();
            const demoAccounts = {
                'admin': { role: 'ADMIN', username: 'admin', displayName: 'System Administrator' },
                'driver_amb01': { role: 'DRIVER', username: 'driver_amb01', displayName: 'Ramesh Singh (AMB-01)', linkedAmbulanceId: 'AMB-01' },
                'driver_amb02': { role: 'DRIVER', username: 'driver_amb02', displayName: 'Suresh Rawat (AMB-02)', linkedAmbulanceId: 'AMB-02' },
                'hosp_max': { role: 'HOSPITAL', username: 'hosp_max', displayName: 'Max Super Speciality Desk', linkedHospitalId: 'LOC-004' },
                'hosp_doon': { role: 'HOSPITAL', username: 'hosp_doon', displayName: 'Doon Govt Hospital Desk', linkedHospitalId: 'LOC-011' },
                'hosp_synergy': { role: 'HOSPITAL', username: 'hosp_synergy', displayName: 'Synergy Hospital Desk', linkedHospitalId: 'LOC-006' },
                'patient_demo': { role: 'PATIENT', username: 'patient_demo', displayName: 'Demo Citizen Patient' }
            };

            if (demoAccounts[uLower]) {
                const u = demoAccounts[uLower];
                const mockToken = 'showcase-session-' + uLower;
                this.setSession(mockToken, u);
                if (typeof Common !== 'undefined' && Common.toast) {
                    Common.toast('Signed in via Showcase Cloud Mode (Connected to Dehradun demo data)', 'info', 4000);
                }
                return { success: true, token: mockToken, user: u, isDemoMode: true };
            }

            throw new Error(netErr.message || 'Could not connect to authentication server.');
        }
    },

    async register(username, password, displayName, phone) {
        const res = await fetch(this.resolveUrl('/api/auth/register'), {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                username,
                password,
                displayName: displayName || username,
                phone: phone || ''
            })
        });
        const data = await res.json();
        if (!res.ok) {
            throw new Error(data.error?.message || data.message || 'Registration failed');
        }
        return data;
    },

    async logout() {
        const token = this.getToken();
        if (token) {
            try {
                await fetch(this.resolveUrl('/api/auth/logout'), {
                    method: 'POST',
                    headers: this.getAuthHeaders()
                });
            } catch (e) {
                console.warn('Logout network error:', e);
            }
        }
        this.clearSession();
        window.location.href = '/login.html';
    },

    async fetchMe() {
        const token = this.getToken();
        if (!token) return null;
        try {
            const res = await fetch(this.resolveUrl('/api/auth/me'), {
                headers: this.getAuthHeaders()
            });
            if (!res.ok) {
                this.clearSession();
                return null;
            }
            const data = await res.json();
            if (data.user) {
                localStorage.setItem(this.USER_KEY, JSON.stringify(data.user));
                return data.user;
            }
        } catch (e) {
            console.warn('fetchMe error:', e);
        }
        return null;
    },

    redirectForRole(role) {
        if (role === 'PATIENT') {
            window.location.href = '/patient.html';
        } else if (role === 'DRIVER') {
            window.location.href = '/driver.html';
        } else if (role === 'HOSPITAL') {
            window.location.href = '/hospital.html';
        } else if (role === 'ADMIN') {
            window.location.href = '/index.html';
        } else {
            window.location.href = '/index.html';
        }
    },

    requireRole(allowedRoles, redirectUrl = '/login.html') {
        const user = this.getUser();
        if (!this.isLoggedIn() || !user) {
            window.location.href = redirectUrl;
            return false;
        }
        if (!allowedRoles.includes(user.role)) {
            alert('Access Denied: You do not have permission for this portal (' + user.role + ')');
            this.redirectForRole(user.role);
            return false;
        }
        return true;
    }
};

window.Auth = Auth;
