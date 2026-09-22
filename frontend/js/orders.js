/**
 * PRICILLA MART - ORDERS & CHECKOUT CONTROLLER
 */

const Orders = {
    async initCheckout() {
        const user = API.getUser();
        if (!user) {
            window.location.href = '/login.html';
            return;
        }

        // Prefill form
        const nameField = document.getElementById('checkout-name');
        const emailField = document.getElementById('checkout-email');
        const phoneField = document.getElementById('checkout-phone');

        if (nameField) nameField.value = user.name || '';
        if (emailField) emailField.value = user.email || '';
        if (phoneField) phoneField.value = user.phone || '';

        // Load summary items
        const summaryItemsContainer = document.getElementById('checkout-items-summary');
        const totalAmountEl = document.getElementById('checkout-total-amount');

        try {
            const res = await API.get('/cart');
            if (res.success && res.data) {
                const { items, total, itemCount } = res.data;
                if (itemCount === 0) {
                    API.showToast('Your cart is empty. Please add items before checking out.', 'warning');
                    setTimeout(() => { window.location.href = '/products.html'; }, 1000);
                    return;
                }

                if (summaryItemsContainer) {
                    summaryItemsContainer.innerHTML = items.map(it => `
                        <div style="display: flex; justify-content: space-between; margin-bottom: 0.6rem; font-size: 0.9rem;">
                            <span>${it.productName} × <strong>${it.quantity}</strong></span>
                            <span>₹${Number(it.subtotal).toLocaleString('en-IN')}</span>
                        </div>
                    `).join('');
                }

                if (totalAmountEl) {
                    totalAmountEl.innerText = `₹${Number(total).toLocaleString('en-IN')}`;
                }
            }
        } catch (err) {
            API.showToast('Error loading cart summary.', 'error');
        }

        // Handle Confirm Order
        const checkoutForm = document.getElementById('checkout-form');
        if (checkoutForm) {
            checkoutForm.addEventListener('submit', async (e) => {
                e.preventDefault();
                const buyerName = document.getElementById('checkout-name').value.trim();
                const email = document.getElementById('checkout-email').value.trim();
                const phone = document.getElementById('checkout-phone').value.trim();
                const deliveryAddress = document.getElementById('checkout-address').value.trim();

                const submitBtn = checkoutForm.querySelector('button[type="submit"]');
                submitBtn.disabled = true;
                submitBtn.innerHTML = `<i class="fas fa-spinner fa-spin"></i> Processing Order...`;

                try {
                    const res = await API.post('/orders', {
                        buyerName, email, phone, deliveryAddress
                    });

                    if (res.success && res.data) {
                        const orderCode = res.data.orderCode || 'PRC10001';
                        this.showSuccessModal(orderCode, res.data.totalAmount);
                    }
                } catch (err) {
                    API.showToast(err.message || 'Checkout failed. Please try again.', 'error');
                    submitBtn.disabled = false;
                    submitBtn.innerHTML = `<i class="fas fa-check-circle"></i> Confirm Order`;
                }
            });
        }
    },

    showSuccessModal(orderCode, totalAmount) {
        const modal = document.getElementById('order-success-modal');
        const codeEl = document.getElementById('success-order-code');
        const amountEl = document.getElementById('success-order-amount');

        if (codeEl) codeEl.innerText = orderCode;
        if (amountEl && totalAmount) amountEl.innerText = `₹${Number(totalAmount).toLocaleString('en-IN')}`;

        if (modal) {
            modal.classList.add('active');
        } else {
            alert(`ORDER SUCCESSFUL!\nYour Order ID: ${orderCode}\nThank you for shopping with Pricilla Mart.`);
            window.location.href = '/orders.html';
        }
    },

    async loadOrderHistory(containerId = 'orders-list-container') {
        const container = document.getElementById(containerId);
        if (!container) return;

        const user = API.getUser();
        if (!user) {
            window.location.href = '/login.html';
            return;
        }

        container.innerHTML = `
            <div style="text-align: center; padding: 3rem; color: var(--secondary);">
                <i class="fas fa-spinner fa-spin fa-2x"></i>
                <p style="margin-top: 0.8rem; font-weight: 600;">Fetching your order history...</p>
            </div>
        `;

        try {
            const res = await API.get('/orders');
            if (res.success && res.data) {
                if (res.data.length === 0) {
                    container.innerHTML = `
                        <div style="text-align: center; padding: 4rem 1rem; background: var(--card-bg); border-radius: var(--radius-md);">
                            <i class="fas fa-clipboard-list fa-3x" style="color: var(--secondary); margin-bottom: 1rem;"></i>
                            <h3>No previous orders found</h3>
                            <p style="color: var(--text-muted); margin-bottom: 1.5rem;">You haven't placed any orders yet. Discover trending deals today!</p>
                            <a href="/products.html" class="btn btn-primary">Start Shopping</a>
                        </div>
                    `;
                    return;
                }

                container.innerHTML = res.data.map(order => this.renderOrderCard(order)).join('');
            }
        } catch (err) {
            container.innerHTML = `<p style="color: var(--error);">Failed to load orders: ${err.message}</p>`;
        }
    },

    renderOrderCard(order) {
        const statusClass = `status-${order.status.toLowerCase()}`;
        return `
            <div style="background: var(--card-bg); border-radius: var(--radius-md); padding: 1.5rem; margin-bottom: 1.5rem; border: 1px solid var(--border-color); box-shadow: var(--shadow-sm);">
                <div style="display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid var(--border-color); padding-bottom: 0.8rem; margin-bottom: 1rem; flex-wrap: wrap; gap: 0.5rem;">
                    <div>
                        <span style="font-size: 0.85rem; color: var(--text-muted);">ORDER PLACED:</span>
                        <div style="font-weight: 700; color: var(--text-dark);">${order.createdAt}</div>
                    </div>
                    <div>
                        <span style="font-size: 0.85rem; color: var(--text-muted);">ORDER ID:</span>
                        <div style="font-weight: 800; color: var(--primary);">${order.orderCode}</div>
                    </div>
                    <div>
                        <span style="font-size: 0.85rem; color: var(--text-muted);">TOTAL:</span>
                        <div style="font-weight: 800; color: var(--primary);">₹${Number(order.totalAmount).toLocaleString('en-IN')}</div>
                    </div>
                    <div>
                        <span class="status-badge ${statusClass}">● ${order.status}</span>
                    </div>
                </div>

                <div style="margin-bottom: 1rem;">
                    <div style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 0.3rem;">DELIVERY ADDRESS:</div>
                    <div style="font-size: 0.9rem; color: var(--text-dark);">${order.deliveryAddress} (${order.buyerName} | ${order.phone})</div>
                </div>

                <div style="background: var(--white); border-radius: var(--radius-sm); padding: 1rem; border: 1px solid rgba(136, 112, 142, 0.2);">
                    <div style="font-size: 0.85rem; font-weight: 700; color: var(--secondary); margin-bottom: 0.6rem;">ORDER ITEMS:</div>
                    ${order.items.map(it => `
                        <div style="display: flex; align-items: center; justify-content: space-between; padding: 0.5rem 0; border-bottom: 1px dashed #eee; font-size: 0.9rem;">
                            <div style="display: flex; align-items: center; gap: 0.8rem;">
                                <img src="${it.imageUrl}" alt="${it.productName}" style="width: 48px; height: 48px; border-radius: 4px; object-fit: cover;">
                                <div>
                                    <div style="font-weight: 700; color: var(--primary);">${it.productName}</div>
                                    <div style="font-size: 0.8rem; color: var(--text-muted);">Sold by: ${it.sellerName} | Qty: ${it.quantity}</div>
                                </div>
                            </div>
                            <div style="text-align: right;">
                                <div style="font-weight: 700; color: var(--primary);">₹${Number(it.subtotal).toLocaleString('en-IN')}</div>
                                <button class="btn btn-sm btn-outline" style="margin-top: 0.3rem; font-size: 0.75rem; padding: 0.2rem 0.6rem;" onclick="window.location.href='/product-details.html?id=${it.productId}#reviews-section'">
                                    Rate & Review
                                </button>
                            </div>
                        </div>
                    `).join('')}
                </div>
            </div>
        `;
    },

    async updateOrderStatus(orderId, newStatus, callback) {
        try {
            const res = await API.put(`/orders/${orderId}/status`, { status: newStatus });
            if (res.success) {
                API.showToast(`Order status updated to ${newStatus}`, 'success');
                if (callback) callback();
            }
        } catch (err) {
            API.showToast(err.message || 'Failed to update order status.', 'error');
        }
    }
};
