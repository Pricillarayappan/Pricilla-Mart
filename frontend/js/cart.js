/**
 * PRICILLA MART - SHOPPING CART CONTROLLER
 */

const Cart = {
    async addItem(productId, quantity = 1) {
        const user = API.getUser();
        if (!user) {
            API.showToast('Please log in to add items to your cart.', 'info');
            setTimeout(() => {
                window.location.href = (typeof Auth !== 'undefined' ? Auth.getPath('login.html') : 'login.html');
            }, 600);
            return;
        }

        try {
            const res = await API.post('/cart', { productId, quantity });
            if (res.success) {
                API.showToast('Product added to your cart!', 'success');
                this.updateBadge();
                if (window.location.pathname.includes('cart.html')) {
                    this.loadCart();
                }
            }
        } catch (err) {
            API.showToast(err.message || 'Failed to add item to cart.', 'error');
        }
    },

    async updateBadge() {
        const badge = document.querySelector('.cart-count-badge');
        if (!badge) return;

        const user = API.getUser();
        if (!user) {
            badge.innerText = '0';
            return;
        }

        try {
            const res = await API.get('/cart');
            if (res.success && res.data) {
                badge.innerText = res.data.itemCount || '0';
            }
        } catch (err) {
            badge.innerText = '0';
        }
    },

    async loadCart() {
        const container = document.getElementById('cart-items-container');
        const summaryContainer = document.getElementById('cart-summary-container');
        if (!container) return;

        const user = API.getUser();
        if (!user) {
            container.innerHTML = `
                <div style="text-align: center; padding: 4rem 1rem; background: var(--card-bg); border-radius: var(--radius-md);">
                    <i class="fas fa-shopping-cart fa-3x" style="color: var(--secondary); margin-bottom: 1rem;"></i>
                    <h3>Your cart is empty</h3>
                    <p style="color: var(--text-muted); margin-bottom: 1.5rem;">Please log in to view items saved in your cart.</p>
                    <a href="${typeof Auth !== 'undefined' ? Auth.getPath('login.html') : 'login.html'}" class="btn btn-primary">Login Now</a>
                </div>
            `;
            if (summaryContainer) summaryContainer.style.display = 'none';
            return;
        }

        try {
            const res = await API.get('/cart');
            if (res.success && res.data) {
                const { items, subtotal, total, itemCount } = res.data;

                if (itemCount === 0) {
                    container.innerHTML = `
                        <div style="text-align: center; padding: 4rem 1rem; background: var(--card-bg); border-radius: var(--radius-md);">
                            <i class="fas fa-shopping-bag fa-3x" style="color: var(--secondary); margin-bottom: 1rem;"></i>
                            <h3>Your shopping cart is empty</h3>
                            <p style="color: var(--text-muted); margin-bottom: 1.5rem;">Explore our vast collection of products and start shopping today.</p>
                            <a href="${typeof Auth !== 'undefined' ? Auth.getPath('products.html') : 'products.html'}" class="btn btn-primary">Browse Products</a>
                        </div>
                    `;
                    if (summaryContainer) summaryContainer.style.display = 'none';
                    return;
                }

                if (summaryContainer) summaryContainer.style.display = 'block';

                container.innerHTML = items.map(item => `
                    <div class="cart-item-card">
                        <img src="${item.imageUrl}" alt="${item.productName}" class="cart-thumb" onerror="this.src='https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600&auto=format&fit=crop&q=80'">
                        <div class="cart-details">
                            <h4 class="cart-product-title">${item.productName}</h4>
                            <div class="cart-product-price">₹${Number(item.price).toLocaleString('en-IN')}</div>
                            <div style="font-size: 0.8rem; color: var(--text-muted); margin-bottom: 0.8rem;">
                                Available stock: ${item.availableStock}
                            </div>
                            <div style="display: flex; align-items: center; justify-content: space-between;">
                                <div class="qty-control">
                                    <button class="qty-btn" onclick="Cart.changeQuantity(${item.id}, ${item.quantity - 1})">-</button>
                                    <span class="qty-display">${item.quantity}</span>
                                    <button class="qty-btn" onclick="Cart.changeQuantity(${item.id}, ${item.quantity + 1})" ${item.quantity >= item.availableStock ? 'disabled' : ''}>+</button>
                                </div>
                                <button class="btn btn-sm" style="color: var(--error); background: none;" onclick="Cart.removeItem(${item.id})">
                                    <i class="fas fa-trash-alt"></i> Remove
                                </button>
                            </div>
                        </div>
                    </div>
                `).join('');

                if (summaryContainer) {
                    summaryContainer.innerHTML = `
                        <div class="order-summary-card">
                            <h3 style="margin-bottom: 1.2rem; color: var(--primary); border-bottom: 1px solid var(--border-color); padding-bottom: 0.8rem;">
                                Order Summary
                            </h3>
                            <div class="summary-row">
                                <span>Total Items:</span>
                                <strong>${itemCount}</strong>
                            </div>
                            <div class="summary-row">
                                <span>Subtotal:</span>
                                <strong>₹${Number(subtotal).toLocaleString('en-IN')}</strong>
                            </div>
                            <div class="summary-row">
                                <span>Delivery Fee:</span>
                                <strong style="color: var(--success);">FREE</strong>
                            </div>
                            <div class="summary-row summary-total">
                                <span>Total Amount:</span>
                                <span>₹${Number(total).toLocaleString('en-IN')}</span>
                            </div>
                            <button class="btn btn-primary btn-block" style="margin-top: 1.5rem;" onclick="window.location.href='${typeof Auth !== 'undefined' ? Auth.getPath('checkout.html') : 'checkout.html'}'">
                                Proceed to Checkout <i class="fas fa-arrow-right"></i>
                            </button>
                        </div>
                    `;
                }
            }
        } catch (err) {
            container.innerHTML = `<p style="color: var(--error);">Error loading cart: ${err.message}</p>`;
        }
    },

    async changeQuantity(cartItemId, newQty) {
        try {
            const res = await API.put(`/cart/${cartItemId}`, { quantity: newQty });
            if (res.success) {
                this.loadCart();
                this.updateBadge();
            }
        } catch (err) {
            API.showToast(err.message || 'Could not update quantity.', 'error');
        }
    },

    async removeItem(cartItemId) {
        if (!confirm('Are you sure you want to remove this item from your cart?')) return;
        try {
            const res = await API.delete(`/cart/${cartItemId}`);
            if (res.success) {
                API.showToast('Item removed from cart.', 'info');
                this.loadCart();
                this.updateBadge();
            }
        } catch (err) {
            API.showToast(err.message || 'Could not remove item.', 'error');
        }
    }
};

document.addEventListener('DOMContentLoaded', () => {
    Cart.updateBadge();
});
