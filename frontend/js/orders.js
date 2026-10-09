/**
 * PRICILLA MART - ORDERS & CHECKOUT CONTROLLER
 */

const Orders = {
    currentPaymentMethod: 'Cash on Delivery',

    selectPaymentMethod(method) {
        this.currentPaymentMethod = method;

        // Update cards visual state
        ['cod', 'upi', 'card'].forEach(k => {
            const card = document.getElementById(`pay-card-${k}`);
            const panel = document.getElementById(`panel-pay-${k}`);
            if (card) {
                card.style.borderColor = 'var(--border-color)';
                card.style.background = '#fff';
            }
            if (panel) panel.style.display = 'none';
        });

        if (method === 'Cash on Delivery') {
            const c = document.getElementById('pay-card-cod');
            const p = document.getElementById('panel-pay-cod');
            if (c) { c.style.borderColor = 'var(--primary)'; c.style.background = 'rgba(49, 42, 68, 0.05)'; }
            if (p) p.style.display = 'block';
        } else if (method === 'UPI') {
            const c = document.getElementById('pay-card-upi');
            const p = document.getElementById('panel-pay-upi');
            if (c) { c.style.borderColor = 'var(--primary)'; c.style.background = 'rgba(49, 42, 68, 0.05)'; }
            if (p) p.style.display = 'block';
        } else if (method === 'Credit Card') {
            const c = document.getElementById('pay-card-card');
            const p = document.getElementById('panel-pay-card');
            if (c) { c.style.borderColor = 'var(--primary)'; c.style.background = 'rgba(49, 42, 68, 0.05)'; }
            if (p) p.style.display = 'block';
        }

        const badge = document.getElementById('summary-payment-method-badge');
        if (badge) badge.innerText = method;
    },

    fillDemoUPI() {
        const upiInput = document.getElementById('checkout-upi-id');
        if (upiInput) {
            upiInput.value = 'rahul@okhdfcbank';
            API.showToast('Demo UPI ID filled!', 'info');
        }
    },

    fillDemoCard() {
        const nameInput = document.getElementById('checkout-card-name');
        const numInput = document.getElementById('checkout-card-number');
        const expInput = document.getElementById('checkout-card-expiry');
        const cvvInput = document.getElementById('checkout-card-cvv');

        if (nameInput) nameInput.value = 'Rahul Sharma';
        if (numInput) numInput.value = '4532 8920 1145 8899';
        if (expInput) expInput.value = '12/28';
        if (cvvInput) cvvInput.value = '889';

        API.showToast('Demo Credit Card details filled!', 'info');
    },

    async initCheckout() {
        const user = API.getUser();
        if (!user) {
            window.location.href = (typeof Auth !== 'undefined' ? Auth.getPath('login.html') : 'login.html');
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
                    setTimeout(() => {
                        window.location.href = (typeof Auth !== 'undefined' ? Auth.getPath('products.html') : 'products.html');
                    }, 1000);
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
                const paymentMethod = Orders.currentPaymentMethod || 'Cash on Delivery';

                // Optional validation for demo inputs
                if (paymentMethod === 'UPI') {
                    const upiId = document.getElementById('checkout-upi-id');
                    if (upiId && !upiId.value.trim()) {
                        Orders.fillDemoUPI();
                    }
                } else if (paymentMethod === 'Credit Card') {
                    const cNum = document.getElementById('checkout-card-number');
                    if (cNum && !cNum.value.trim()) {
                        Orders.fillDemoCard();
                    }
                }

                const submitBtn = checkoutForm.querySelector('button[type="submit"]');
                if (submitBtn) {
                    submitBtn.disabled = true;
                    submitBtn.innerHTML = `<i class="fas fa-spinner fa-spin"></i> Processing Order...`;
                }

                try {
                    const res = await API.post('/orders', {
                        buyerName, email, phone, deliveryAddress, paymentMethod
                    });

                    if (res.success && res.data) {
                        const orderCode = res.data.orderCode || 'PRC10001';
                        this.showSuccessModal(orderCode, res.data.totalAmount, res.data.paymentMethod || paymentMethod);
                    }
                } catch (err) {
                    API.showToast(err.message || 'Checkout failed. Please try again.', 'error');
                    if (submitBtn) {
                        submitBtn.disabled = false;
                        submitBtn.innerHTML = `<i class="fas fa-check-circle"></i> Confirm & Place Order`;
                    }
                }
            });
        }
    },

    showSuccessModal(orderCode, totalAmount, paymentMethod = 'Cash on Delivery') {
        const modal = document.getElementById('order-success-modal');
        const codeEl = document.getElementById('success-order-code');
        const amountEl = document.getElementById('success-order-amount');
        const payEl = document.getElementById('success-order-payment');

        if (codeEl) codeEl.innerText = orderCode;
        if (amountEl && totalAmount) amountEl.innerText = `₹${Number(totalAmount).toLocaleString('en-IN')}`;
        if (payEl) payEl.innerText = paymentMethod;

        if (modal) {
            modal.classList.add('active');
        } else {
            alert(`ORDER SUCCESSFUL!\nYour Order ID: ${orderCode}\nPayment: ${paymentMethod}\nThank you for shopping with Pricilla Mart.`);
            window.location.href = (typeof Auth !== 'undefined' ? Auth.getPath('orders.html') : 'orders.html');
        }
    },

    async loadOrderHistory(containerId = 'orders-list-container') {
        const container = document.getElementById(containerId);
        if (!container) return;

        const user = API.getUser();
        if (!user) {
            window.location.href = (typeof Auth !== 'undefined' ? Auth.getPath('login.html') : 'login.html');
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
                            <a href="${typeof Auth !== 'undefined' ? Auth.getPath('products.html') : 'products.html'}" class="btn btn-primary">Start Shopping</a>
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
        const statusClass = `status-${(order.status || 'processing').toLowerCase()}`;
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
                        <span style="font-size: 0.85rem; color: var(--text-muted);">PAYMENT METHOD:</span>
                        <div style="font-weight: 700; color: var(--text-dark); display: flex; align-items: center; gap: 0.3rem;">
                            <i class="${order.paymentMethod === 'UPI' ? 'fas fa-qrcode' : order.paymentMethod === 'Credit Card' ? 'fas fa-credit-card' : 'fas fa-money-bill-wave'}" style="color: var(--secondary);"></i>
                            ${order.paymentMethod || 'Cash on Delivery'}
                        </div>
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
                    ${(order.items || []).map(it => `
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
                                <button class="btn btn-sm btn-outline" style="margin-top: 0.3rem; font-size: 0.75rem; padding: 0.2rem 0.6rem;" onclick="window.location.href='${typeof Auth !== 'undefined' ? Auth.getPath('product-details.html') : 'product-details.html'}?id=${it.productId}#reviews-section'">
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
