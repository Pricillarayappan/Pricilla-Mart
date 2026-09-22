/**
 * PRICILLA MART - AUTHENTICATION & SESSION CONTROLLER
 */

const Auth = {

    // ============================================
    // GET CORRECT FRONTEND PATH
    // ============================================
    getPath(path) {

        const currentPath = window.location.pathname;

        // If current page is inside seller folder
        if (currentPath.includes('/seller/')) {
            return '../' + path;
        }

        // If current page is inside admin folder
        if (currentPath.includes('/admin/')) {
            return '../' + path;
        }

        // Root frontend pages
        return path;
    },


    // ============================================
    // INITIALIZE AUTH
    // ============================================
    init() {
        this.updateNavbar();
        this.bindEvents();
    },


    // ============================================
    // UPDATE NAVBAR
    // ============================================
    updateNavbar() {

        const user = API.getUser();

        const authContainer =
            document.getElementById('nav-auth-container');

        const roleDashboardLink =
            document.getElementById('nav-role-dashboard');

        if (!authContainer) return;


        // ========================================
        // USER LOGGED IN
        // ========================================
        if (user) {

            let dashboardHtml = '';


            // ADMIN
            if (user.role === 'admin') {

                dashboardHtml = `
                    <a href="${this.getPath('admin/dashboard.html')}"
                       class="user-badge">

                        <i class="fas fa-shield-alt"></i>
                        Admin Panel

                    </a>
                `;
            }


            // SELLER
            else if (user.role === 'seller') {

                dashboardHtml = `
                    <a href="${this.getPath('seller/dashboard.html')}"
                       class="user-badge">

                        <i class="fas fa-store"></i>
                        Seller Hub

                    </a>
                `;
            }


            authContainer.innerHTML = `

                ${dashboardHtml}

                <a href="${this.getPath('profile.html')}"
                   title="My Profile">

                    <i class="fas fa-user-circle"></i>
                    ${user.name.split(' ')[0]}

                </a>


                <a href="#"
                   id="nav-logout-btn"
                   title="Logout"
                   style="color: var(--light);">

                    <i class="fas fa-sign-out-alt"></i>
                    Logout

                </a>

            `;


            // LOGOUT BUTTON
            const logoutBtn =
                document.getElementById('nav-logout-btn');

            if (logoutBtn) {

                logoutBtn.addEventListener('click', (e) => {

                    e.preventDefault();

                    this.logout();

                });
            }

        }


        // ========================================
        // USER NOT LOGGED IN
        // ========================================
        else {

            authContainer.innerHTML = `

                <a href="${this.getPath('login.html')}">

                    <i class="fas fa-sign-in-alt"></i>
                    Login

                </a>


                <a href="${this.getPath('register.html')}"
                   class="btn btn-sm btn-secondary"
                   style="color: #fff;">

                    Register

                </a>

            `;
        }
    },


    // ============================================
    // BIND EVENTS
    // ============================================
    bindEvents() {


        // ========================================
        // LOGIN FORM
        // ========================================
        const loginForm =
            document.getElementById('login-form');


        if (loginForm) {

            loginForm.addEventListener(
                'submit',
                async (e) => {

                    e.preventDefault();


                    const email =
                        document
                            .getElementById('login-email')
                            .value
                            .trim();


                    const password =
                        document
                            .getElementById('login-password')
                            .value;


                    try {

                        const res =
                            await API.post(
                                '/login',
                                {
                                    email,
                                    password
                                }
                            );


                        if (res.success && res.data) {

                            API.setToken(
                                res.data.token
                            );

                            API.setUser(
                                res.data.user
                            );


                            API.showToast(
                                `Welcome back, ${res.data.user.name}!`,
                                'success'
                            );


                            // ROLE BASED REDIRECTION
                            setTimeout(() => {

                                if (
                                    res.data.user.role ===
                                    'admin'
                                ) {

                                    window.location.href =
                                        this.getPath(
                                            'admin/dashboard.html'
                                        );

                                }

                                else if (
                                    res.data.user.role ===
                                    'seller'
                                ) {

                                    window.location.href =
                                        this.getPath(
                                            'seller/dashboard.html'
                                        );

                                }

                                else {

                                    window.location.href =
                                        this.getPath(
                                            'index.html'
                                        );
                                }

                            }, 800);
                        }

                    }

                    catch (err) {

                        API.showToast(
                            err.message ||
                            'Login failed. Please check credentials.',
                            'error'
                        );

                    }

                }
            );
        }


        // ========================================
        // REGISTER FORM
        // ========================================
        const regForm =
            document.getElementById('register-form');


        if (regForm) {

            regForm.addEventListener(
                'submit',
                async (e) => {

                    e.preventDefault();


                    const name =
                        document
                            .getElementById('reg-name')
                            .value
                            .trim();


                    const email =
                        document
                            .getElementById('reg-email')
                            .value
                            .trim();


                    const phone =
                        document
                            .getElementById('reg-phone')
                            .value
                            .trim();


                    const password =
                        document
                            .getElementById('reg-password')
                            .value;


                    const confirmPassword =
                        document
                            .getElementById(
                                'reg-confirm-password'
                            )
                            .value;


                    const role =
                        document
                            .getElementById('reg-role')
                            .value;


                    // PASSWORD CHECK
                    if (password !== confirmPassword) {

                        API.showToast(
                            'Passwords do not match.',
                            'error'
                        );

                        return;
                    }


                    try {

                        const res =
                            await API.post(
                                '/register',
                                {
                                    name,
                                    email,
                                    phone,
                                    password,
                                    confirmPassword,
                                    role
                                }
                            );


                        if (res.success && res.data) {

                            API.showToast(
                                'Registration successful! Redirecting to login...',
                                'success'
                            );


                            setTimeout(() => {

                                window.location.href =
                                    this.getPath(
                                        'login.html'
                                    );

                            }, 1200);
                        }

                    }

                    catch (err) {

                        API.showToast(
                            err.message ||
                            'Registration failed.',
                            'error'
                        );

                    }

                }
            );
        }


        // ========================================
        // MOBILE NAVIGATION
        // ========================================
        const menuBtn =
            document.querySelector('.mobile-menu-btn');

        const navLinks =
            document.querySelector('.nav-links');


        if (menuBtn && navLinks) {

            menuBtn.addEventListener('click', () => {

                navLinks.classList.toggle('show');

            });
        }
    },


    // ============================================
    // LOGOUT
    // ============================================
    logout() {

        API.clearAuth();


        API.showToast(
            'You have been logged out.',
            'info'
        );


        setTimeout(() => {

            window.location.href =
                this.getPath('login.html');

        }, 600);
    },


    // ============================================
    // PROTECT ROUTE
    // ============================================
    protectRoute(requiredRole = null) {

        const user = API.getUser();


        // NOT LOGGED IN
        if (!user) {

            API.showToast(
                'Please log in to view this page.',
                'error'
            );


            setTimeout(() => {

                window.location.href =
                    this.getPath('login.html');

            }, 500);


            return false;
        }


        // WRONG ROLE
        if (
            requiredRole &&
            user.role !== requiredRole &&
            user.role !== 'admin'
        ) {

            API.showToast(
                'Access denied: Unauthorized role.',
                'error'
            );


            setTimeout(() => {

                window.location.href =
                    this.getPath('index.html');

            }, 500);


            return false;
        }


        return true;
    },


    // ============================================
    // DEMO CREDENTIALS
    // ============================================
    fillDemoCredentials(role) {

        const emailInput =
            document.getElementById('login-email');

        const passInput =
            document.getElementById('login-password');


        if (!emailInput || !passInput) return;


        if (role === 'admin') {

            emailInput.value =
                'admin@pricillamart.com';

            passInput.value =
                'Admin@123';
        }


        else if (role === 'seller') {

            emailInput.value =
                'techworld@pricillamart.com';

            passInput.value =
                'Seller@123';
        }


        else if (role === 'buyer') {

            emailInput.value =
                'rahul@gmail.com';

            passInput.value =
                'Buyer@123';
        }
    }
};


// ============================================
// START AUTH
// ============================================
document.addEventListener(
    'DOMContentLoaded',
    () => {

        Auth.init();

    }
);