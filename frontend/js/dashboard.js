/**
 * PRICILLA MART - SELLER & ADMIN DASHBOARD CONTROLLER
 */

const Dashboard = {
    // ---------------- SELLER HUB ----------------
    async initSellerDashboard() {
        if (!Auth.protectRoute('seller')) return;

        try {
            const res = await API.get('/seller/stats');
            if (res.success && res.data) {
                const s = res.data;
                const pEl = document.getElementById('seller-stat-products');
                const oEl = document.getElementById('seller-stat-orders');
                const rEl = document.getElementById('seller-stat-revenue');

                if (pEl) pEl.innerText = s.totalProducts;
                if (oEl) oEl.innerText = s.totalOrders;
                if (rEl) rEl.innerText = `₹${Number(s.totalSales).toLocaleString('en-IN')}`;
            }
        } catch (err) {
            console.error('Failed to load seller stats:', err);
        }

        this.loadSellerProducts();
    },

    async loadSellerProducts() {
        const user = API.getUser();
        if (!user) return;

        const tableBody = document.getElementById('seller-products-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="6" style="text-align:center; padding: 2rem;"><i class="fas fa-spinner fa-spin"></i> Loading inventory...</td></tr>`;

        try {
            const res = await API.get('/products', { seller_id: user.id });
            if (res.success && res.data) {
                if (res.data.length === 0) {
                    tableBody.innerHTML = `<tr><td colspan="6" style="text-align:center; padding: 2rem;">No products listed yet. Click "+ Add New Product" to start selling!</td></tr>`;
                    return;
                }

                tableBody.innerHTML = res.data.map(p => `
                    <tr>
                        <td>
                            <div style="display: flex; align-items: center; gap: 0.8rem;">
                                <img src="${p.imageUrl}" alt="${p.name}" style="width: 44px; height: 44px; border-radius: 6px; object-fit: cover;">
                                <div>
                                    <div style="font-weight: 700; color: var(--primary);">${p.name}</div>
                                    <div style="font-size: 0.75rem; color: var(--text-muted);">${p.categoryName}</div>
                                </div>
                            </div>
                        </td>
                        <td><strong>₹${Number(p.price).toLocaleString('en-IN')}</strong></td>
                        <td>
                            <span class="${p.stockQuantity > 0 ? 'stock-in' : 'stock-out'}" style="font-weight: 700;">
                                ${p.stockQuantity} units
                            </span>
                        </td>
                        <td>★ ${p.averageRating.toFixed(1)} (${p.reviewCount})</td>
                        <td>${p.createdAt.split(' ')[0]}</td>
                        <td>
                            <div style="display: flex; gap: 0.4rem;">
                                <button class="btn btn-sm btn-outline" onclick="Dashboard.openEditProductModal(${JSON.stringify(p).replace(/"/g, '&quot;')})">
                                    <i class="fas fa-edit"></i> Edit
                                </button>
                                <button class="btn btn-sm btn-danger" onclick="Dashboard.deleteProduct(${p.id})">
                                    <i class="fas fa-trash"></i>
                                </button>
                            </div>
                        </td>
                    </tr>
                `).join('');
            }
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="6" style="color:var(--error); text-align:center;">Failed to load products: ${err.message}</td></tr>`;
        }
    },

    openAddProductModal() {
        const modal = document.getElementById('product-modal');
        const form = document.getElementById('product-modal-form');
        const title = document.getElementById('product-modal-title');
        if (!modal || !form) return;

        form.reset();
        document.getElementById('modal-prod-id').value = '';
        title.innerText = 'Add New Product';
        modal.classList.add('active');
    },

    openEditProductModal(prod) {
        const modal = document.getElementById('product-modal');
        const title = document.getElementById('product-modal-title');
        if (!modal) return;

        title.innerText = 'Edit Product Details';
        document.getElementById('modal-prod-id').value = prod.id;
        document.getElementById('modal-prod-name').value = prod.name;
        document.getElementById('modal-prod-desc').value = prod.description;
        document.getElementById('modal-prod-category').value = prod.categoryId;
        document.getElementById('modal-prod-price').value = prod.price;
        document.getElementById('modal-prod-stock').value = prod.stockQuantity;
        document.getElementById('modal-prod-image').value = prod.imageUrl;

        modal.classList.add('active');
    },

    closeProductModal() {
        const modal = document.getElementById('product-modal');
        if (modal) modal.classList.remove('active');
    },

    async saveProductForm(e) {
        e.preventDefault();
        const id = document.getElementById('modal-prod-id').value;
        const name = document.getElementById('modal-prod-name').value.trim();
        const description = document.getElementById('modal-prod-desc').value.trim();
        const categoryId = document.getElementById('modal-prod-category').value;
        const price = document.getElementById('modal-prod-price').value;
        const stockQuantity = document.getElementById('modal-prod-stock').value;
        const imageUrl = document.getElementById('modal-prod-image').value.trim();

        const payload = { name, description, categoryId, price, stockQuantity, imageUrl };

        try {
            if (id) {
                // Update
                const res = await API.put(`/products/${id}`, payload);
                if (res.success) {
                    API.showToast('Product updated successfully!', 'success');
                    this.closeProductModal();
                    this.loadSellerProducts();
                }
            } else {
                // Create
                const res = await API.post('/products', payload);
                if (res.success) {
                    API.showToast('Product added to your store!', 'success');
                    this.closeProductModal();
                    this.loadSellerProducts();
                }
            }
        } catch (err) {
            API.showToast(err.message || 'Error saving product.', 'error');
        }
    },

    async deleteProduct(productId) {
        if (!confirm('Are you sure you want to remove this product? This action cannot be undone.')) {
            return;
        }

        try {
            const res = await API.delete(`/products/${productId}`);
            if (res.success) {
                API.showToast('Product removed successfully.', 'info');
                const user = API.getUser();
                if (user && user.role === 'admin') {
                    this.loadAdminProducts();
                } else {
                    this.loadSellerProducts();
                }
            }
        } catch (err) {
            API.showToast(err.message || 'Could not delete product.', 'error');
        }
    },

    async loadSellerOrders() {
        if (!Auth.protectRoute('seller')) return;

        const tableBody = document.getElementById('seller-orders-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 2rem;"><i class="fas fa-spinner fa-spin"></i> Loading received orders...</td></tr>`;

        try {
            const res = await API.get('/orders');
            if (res.success && res.data) {
                if (res.data.length === 0) {
                    tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 2rem;">No orders received yet.</td></tr>`;
                    return;
                }

                tableBody.innerHTML = res.data.map(o => `
                    <tr>
                        <td><strong>${o.orderCode}</strong></td>
                        <td>${o.buyerName}<br><span style="font-size:0.8rem; color:var(--text-muted);">${o.phone}</span></td>
                        <td>
                            ${o.items.map(it => `<div>${it.productName} × ${it.quantity}</div>`).join('')}
                        </td>
                        <td>
                            <strong>₹${Number(o.totalAmount).toLocaleString('en-IN')}</strong>
                            <div style="font-size:0.75rem; color:var(--secondary); margin-top:2px;">
                                <i class="${o.paymentMethod === 'UPI' ? 'fas fa-qrcode' : o.paymentMethod === 'Credit Card' ? 'fas fa-credit-card' : 'fas fa-money-bill-wave'}"></i> ${o.paymentMethod || 'Cash on Delivery'}
                            </div>
                        </td>
                        <td>${o.createdAt.split(' ')[0]}</td>
                        <td>
                            <span class="status-badge status-${o.status.toLowerCase()}">${o.status}</span>
                        </td>
                        <td>
                            <select class="form-select" style="padding: 0.3rem 0.5rem; font-size: 0.85rem;" onchange="Dashboard.changeOrderStatus(${o.id}, this.value)">
                                <option value="Pending" ${o.status === 'Pending' ? 'selected' : ''}>Pending</option>
                                <option value="Confirmed" ${o.status === 'Confirmed' ? 'selected' : ''}>Confirmed</option>
                                <option value="Shipped" ${o.status === 'Shipped' ? 'selected' : ''}>Shipped</option>
                                <option value="Delivered" ${o.status === 'Delivered' ? 'selected' : ''}>Delivered</option>
                                <option value="Cancelled" ${o.status === 'Cancelled' ? 'selected' : ''}>Cancelled</option>
                            </select>
                        </td>
                    </tr>
                `).join('');
            }
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="7" style="color:var(--error); text-align:center;">Failed to load orders: ${err.message}</td></tr>`;
        }
    },

    async changeOrderStatus(orderId, newStatus) {
        Orders.updateOrderStatus(orderId, newStatus, () => {
            const user = API.getUser();
            if (user && user.role === 'admin') {
                this.loadAdminOrders();
            } else {
                this.loadSellerOrders();
            }
        });
    },

    // ---------------- ADMIN PANEL ----------------
    async initAdminDashboard() {
        if (!Auth.protectRoute('admin')) return;

        try {
            const res = await API.get('/admin/stats');
            if (res.success && res.data) {
                const s = res.data;
                document.getElementById('admin-stat-users').innerText = s.totalUsers;
                document.getElementById('admin-stat-buyers').innerText = s.totalBuyers;
                document.getElementById('admin-stat-sellers').innerText = s.totalSellers;
                document.getElementById('admin-stat-products').innerText = s.totalProducts;
                document.getElementById('admin-stat-orders').innerText = s.totalOrders;
                document.getElementById('admin-stat-sales').innerText = `₹${Number(s.totalSales).toLocaleString('en-IN')}`;
            }
        } catch (err) {
            console.error('Failed to load admin stats:', err);
        }
    },

    async loadAdminUsers(roleFilter = '') {
        if (!Auth.protectRoute('admin')) return;

        const tableBody = document.getElementById('admin-users-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="6" style="text-align:center; padding: 2rem;"><i class="fas fa-spinner fa-spin"></i> Fetching users...</td></tr>`;

        try {
            const res = await API.get('/admin/users', { role: roleFilter });
            if (res.success && res.data) {
                tableBody.innerHTML = res.data.map(u => `
                    <tr>
                        <td>#${u.id}</td>
                        <td><strong>${u.name}</strong></td>
                        <td>${u.email}</td>
                        <td>${u.phone}</td>
                        <td>
                            <span class="user-badge" style="background: ${u.role === 'admin' ? 'var(--primary)' : u.role === 'seller' ? 'var(--secondary)' : 'var(--card-bg)'}; color: #fff;">
                                ${u.role}
                            </span>
                        </td>
                        <td>${u.createdAt}</td>
                    </tr>
                `).join('');
            }
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="6" style="color:var(--error); text-align:center;">Failed to load users: ${err.message}</td></tr>`;
        }
    },

    async loadAdminProducts() {
        if (!Auth.protectRoute('admin')) return;

        const tableBody = document.getElementById('admin-products-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 2rem;"><i class="fas fa-spinner fa-spin"></i> Fetching products...</td></tr>`;

        try {
            const res = await API.get('/products');
            if (res.success && res.data) {
                tableBody.innerHTML = res.data.map(p => `
                    <tr>
                        <td>#${p.id}</td>
                        <td>
                            <div style="display: flex; align-items: center; gap: 0.6rem;">
                                <img src="${p.imageUrl}" alt="${p.name}" style="width: 40px; height: 40px; border-radius: 4px; object-fit: cover;">
                                <strong>${p.name}</strong>
                            </div>
                        </td>
                        <td>${p.categoryName}</td>
                        <td>${p.sellerName}</td>
                        <td>₹${Number(p.price).toLocaleString('en-IN')}</td>
                        <td>${p.stockQuantity}</td>
                        <td>
                            <button class="btn btn-sm btn-danger" onclick="Dashboard.deleteProduct(${p.id})">
                                <i class="fas fa-trash"></i> Remove
                            </button>
                        </td>
                    </tr>
                `).join('');
            }
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="7" style="color:var(--error); text-align:center;">Failed to load products: ${err.message}</td></tr>`;
        }
    },

    async loadAdminOrders() {
        if (!Auth.protectRoute('admin')) return;

        const tableBody = document.getElementById('admin-orders-table-body');
        if (!tableBody) return;

        tableBody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 2rem;"><i class="fas fa-spinner fa-spin"></i> Fetching orders...</td></tr>`;

        try {
            const res = await API.get('/orders');
            if (res.success && res.data) {
                tableBody.innerHTML = res.data.map(o => `
                    <tr>
                        <td><strong>${o.orderCode}</strong></td>
                        <td>${o.buyerName}</td>
                        <td>${o.items.length} items</td>
                        <td>
                            <strong>₹${Number(o.totalAmount).toLocaleString('en-IN')}</strong>
                            <div style="font-size:0.75rem; color:var(--secondary); margin-top:2px;">
                                <i class="${o.paymentMethod === 'UPI' ? 'fas fa-qrcode' : o.paymentMethod === 'Credit Card' ? 'fas fa-credit-card' : 'fas fa-money-bill-wave'}"></i> ${o.paymentMethod || 'Cash on Delivery'}
                            </div>
                        </td>
                        <td>${o.createdAt.split(' ')[0]}</td>
                        <td>
                            <span class="status-badge status-${o.status.toLowerCase()}">${o.status}</span>
                        </td>
                        <td>
                            <select class="form-select" style="padding: 0.3rem 0.5rem; font-size: 0.85rem;" onchange="Dashboard.changeOrderStatus(${o.id}, this.value)">
                                <option value="Pending" ${o.status === 'Pending' ? 'selected' : ''}>Pending</option>
                                <option value="Confirmed" ${o.status === 'Confirmed' ? 'selected' : ''}>Confirmed</option>
                                <option value="Shipped" ${o.status === 'Shipped' ? 'selected' : ''}>Shipped</option>
                                <option value="Delivered" ${o.status === 'Delivered' ? 'selected' : ''}>Delivered</option>
                                <option value="Cancelled" ${o.status === 'Cancelled' ? 'selected' : ''}>Cancelled</option>
                            </select>
                        </td>
                    </tr>
                `).join('');
            }
        } catch (err) {
            tableBody.innerHTML = `<tr><td colspan="7" style="color:var(--error); text-align:center;">Failed to load orders: ${err.message}</td></tr>`;
        }
    }
};
