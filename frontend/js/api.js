/**
 * PRICILLA MART - REST API CLIENT WRAPPER
 * Centralized AJAX / Fetch utility with JWT handling and Toast alerts
 */

const API = {
    baseUrl: '/api',

    getToken() {
        return localStorage.getItem('pm_token');
    },

    setToken(token) {
        localStorage.setItem('pm_token', token);
    },

    getUser() {
        const u = localStorage.getItem('pm_user');
        return u ? JSON.parse(u) : null;
    },

    setUser(user) {
        localStorage.setItem('pm_user', JSON.stringify(user));
    },

    clearAuth() {
        localStorage.removeItem('pm_token');
        localStorage.removeItem('pm_user');
    },

    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const headers = {
            'Content-Type': 'application/json',
            ...(options.headers || {})
        };

        const token = this.getToken();
        if (token) {
            headers['Authorization'] = `Bearer ${token}`;
        }

        try {
            const resp = await fetch(url, {
                ...options,
                headers
            });

            const data = await resp.json().catch(() => ({}));

            if (!resp.ok) {
                const errorMsg = data.error || `HTTP error ${resp.status}`;
                throw new Error(errorMsg);
            }

            return data;
        } catch (err) {
            console.error(`API Error on [${options.method || 'GET'} ${endpoint}]:`, err.message);
            throw err;
        }
    },

    get(endpoint, params = {}) {
        const q = new URLSearchParams();
        Object.entries(params).forEach(([k, v]) => {
            if (v !== undefined && v !== null && v !== '') {
                q.append(k, v);
            }
        });
        const queryStr = q.toString() ? `?${q.toString()}` : '';
        return this.request(`${endpoint}${queryStr}`, { method: 'GET' });
    },

    post(endpoint, body = {}) {
        return this.request(endpoint, {
            method: 'POST',
            body: JSON.stringify(body)
        });
    },

    put(endpoint, body = {}) {
        return this.request(endpoint, {
            method: 'PUT',
            body: JSON.stringify(body)
        });
    },

    delete(endpoint) {
        return this.request(endpoint, { method: 'DELETE' });
    },

    showToast(message, type = 'info') {
        let container = document.querySelector('.toast-container');
        if (!container) {
            container = document.createElement('div');
            container.className = 'toast-container';
            document.body.appendChild(container);
        }

        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        const icon = type === 'success' ? '✓' : type === 'error' ? '⚠' : 'ℹ';
        toast.innerHTML = `<span>${icon}</span> <span>${message}</span>`;
        container.appendChild(toast);

        setTimeout(() => {
            toast.style.opacity = '0';
            toast.style.transform = 'translateX(100%)';
            toast.style.transition = 'all 0.3s ease';
            setTimeout(() => toast.remove(), 300);
        }, 4000);
    }
};
