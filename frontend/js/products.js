/**
 * PRICILLA MART - PRODUCTS & CATALOG CONTROLLER
 */

const Products = {
    async loadCategories(containerId = 'category-chips-container') {
        const container = document.getElementById(containerId);
        if (!container) return;

        try {
            const res = await API.get('/categories');
            if (res.success && res.data) {
                let html = `<button class="category-chip active" data-id="0"><i class="fas fa-th-large"></i> All</button>`;
                res.data.forEach(cat => {
                    html += `
                        <button class="category-chip" data-id="${cat.id}">
                            <i class="${cat.icon}"></i> ${cat.name} (${cat.productCount})
                        </button>
                    `;
                });
                container.innerHTML = html;

                // Click event on category chip
                container.querySelectorAll('.category-chip').forEach(btn => {
                    btn.addEventListener('click', () => {
                        container.querySelectorAll('.category-chip').forEach(b => b.classList.remove('active'));
                        btn.classList.add('active');
                        const catId = parseInt(btn.getAttribute('data-id')) || 0;
                        Products.loadProducts({ category_id: catId === 0 ? '' : catId });
                    });
                });
            }
        } catch (err) {
            console.error('Failed to load categories:', err);
        }
    },

    async loadProducts(params = {}, containerId = 'product-grid-container') {
        const container = document.getElementById(containerId);
        if (!container) return;

        container.innerHTML = `
            <div style="grid-column: 1/-1; text-align: center; padding: 3rem; color: var(--secondary);">
                <i class="fas fa-spinner fa-spin fa-2x"></i>
                <p style="margin-top: 0.8rem; font-weight: 600;">Loading Pricilla Mart products...</p>
            </div>
        `;

        try {
            const res = await API.get('/products', params);
            if (res.success && res.data) {
                if (res.data.length === 0) {
                    container.innerHTML = `
                        <div style="grid-column: 1/-1; text-align: center; padding: 4rem 1rem; background: var(--card-bg); border-radius: var(--radius-md);">
                            <i class="fas fa-box-open fa-3x" style="color: var(--secondary); margin-bottom: 1rem;"></i>
                            <h3>No products found</h3>
                            <p style="color: var(--text-muted);">Try adjusting your search keywords or category filters.</p>
                        </div>
                    `;
                    return;
                }

                container.innerHTML = res.data.map(p => this.renderProductCard(p)).join('');

                // Bind Add to Cart buttons
                container.querySelectorAll('.add-to-cart-btn').forEach(btn => {
                    btn.addEventListener('click', (e) => {
                        e.stopPropagation();
                        const pId = parseInt(btn.getAttribute('data-id'));
                        Cart.addItem(pId, 1);
                    });
                });
            }
        } catch (err) {
            container.innerHTML = `<p style="grid-column: 1/-1; color: var(--error); text-align: center;">Error loading products: ${err.message}</p>`;
        }
    },

    renderProductCard(p) {
        const isOutOfStock = p.stockQuantity <= 0;
        const stars = '★'.repeat(Math.round(p.averageRating || 5)) + '☆'.repeat(5 - Math.round(p.averageRating || 5));
        const targetUrl = `${typeof Auth !== 'undefined' ? Auth.getPath('product-details.html') : 'product-details.html'}?id=${p.id}`;

        return `
            <div class="product-card" onclick="window.location.href='${targetUrl}'" style="cursor: pointer;">
                <div class="product-image-wrap">
                    <img src="${p.imageUrl}" alt="${p.name}" class="product-image" onerror="this.src='https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600&auto=format&fit=crop&q=80'">
                    <span class="category-tag">${p.categoryName}</span>
                </div>
                <div class="product-info">
                    <h3 class="product-title" title="${p.name}">${p.name}</h3>
                    <div class="product-seller"><i class="fas fa-store"></i> Sold by: ${p.sellerName}</div>
                    <div class="product-rating">
                        <span>${stars}</span>
                        <span class="rating-count">(${p.reviewCount || 0})</span>
                    </div>
                    <div class="product-bottom">
                        <div>
                            <div class="product-price">₹${Number(p.price).toLocaleString('en-IN')}</div>
                            <div class="stock-status ${isOutOfStock ? 'stock-out' : 'stock-in'}">
                                ${isOutOfStock ? 'Out of Stock' : `${p.stockQuantity} in stock`}
                            </div>
                        </div>
                        <button class="btn btn-primary btn-sm add-to-cart-btn" data-id="${p.id}" ${isOutOfStock ? 'disabled' : ''}>
                            <i class="fas fa-cart-plus"></i> Add
                        </button>
                    </div>
                </div>
            </div>
        `;
    },

    async loadProductDetails() {
        const urlParams = new URLSearchParams(window.location.search);
        const productId = urlParams.get('id');
        if (!productId) {
            window.location.href = (typeof Auth !== 'undefined' ? Auth.getPath('products.html') : 'products.html');
            return;
        }

        const container = document.getElementById('product-detail-container');
        if (!container) return;

        try {
            const res = await API.get(`/products/${productId}`);
            if (res.success && res.data) {
                const p = res.data;
                const isOutOfStock = p.stockQuantity <= 0;
                const stars = '★'.repeat(Math.round(p.averageRating || 5)) + '☆'.repeat(5 - Math.round(p.averageRating || 5));

                container.innerHTML = `
                    <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap: 2.5rem; background: var(--card-bg); padding: 2.5rem; border-radius: var(--radius-lg); box-shadow: var(--shadow-sm); border: 1px solid var(--border-color);">
                        <div style="text-align: center;">
                            <img src="${p.imageUrl}" alt="${p.name}" style="width: 100%; max-height: 420px; object-fit: cover; border-radius: var(--radius-md); box-shadow: var(--shadow-sm);">
                        </div>
                        <div style="display: flex; flex-direction: column;">
                            <span class="category-tag" style="position: static; display: inline-block; width: fit-content; margin-bottom: 0.8rem;">${p.categoryName}</span>
                            <h1 style="font-size: 2rem; margin-bottom: 0.5rem; color: var(--primary);">${p.name}</h1>
                            <div style="color: var(--text-muted); margin-bottom: 1rem;">Sold by: <strong>${p.sellerName}</strong></div>
                            
                            <div class="product-rating" style="font-size: 1.1rem; margin-bottom: 1.5rem;">
                                <span>${stars}</span>
                                <span class="rating-count" style="font-size: 0.95rem;">${p.averageRating.toFixed(1)} / 5.0 (${p.reviewCount} verified reviews)</span>
                            </div>

                            <div style="font-size: 2.2rem; font-weight: 800; color: var(--primary); margin-bottom: 1rem;">
                                ₹${Number(p.price).toLocaleString('en-IN')}
                            </div>

                            <div style="margin-bottom: 1.5rem;">
                                <span class="stock-status ${isOutOfStock ? 'stock-out' : 'stock-in'}" style="font-size: 1rem; font-weight: 700;">
                                    ${isOutOfStock ? '✖ Currently Out of Stock' : `✔ In Stock (${p.stockQuantity} available)`}
                                </span>
                            </div>

                            <div style="margin-bottom: 2rem;">
                                <h4 style="margin-bottom: 0.5rem;">Product Description:</h4>
                                <p style="color: var(--text-dark); line-height: 1.7;">${p.description}</p>
                            </div>

                            <div style="display: flex; align-items: center; gap: 1rem; margin-top: auto;">
                                <div class="qty-control" style="height: 44px;">
                                    <button class="qty-btn" id="detail-qty-minus" style="width: 40px; height: 100%; font-size: 1.2rem;">-</button>
                                    <span class="qty-display" id="detail-qty-val" style="width: 45px; font-size: 1.1rem;">1</span>
                                    <button class="qty-btn" id="detail-qty-plus" style="width: 40px; height: 100%; font-size: 1.2rem;">+</button>
                                </div>
                                <button class="btn btn-primary" id="detail-add-cart-btn" style="padding: 0.75rem 2rem; font-size: 1.05rem;" ${isOutOfStock ? 'disabled' : ''}>
                                    <i class="fas fa-shopping-cart"></i> Add to Cart
                                </button>
                            </div>
                        </div>
                    </div>
                `;

                // Quantity selector
                let currentQty = 1;
                const qtyVal = document.getElementById('detail-qty-val');
                const minusBtn = document.getElementById('detail-qty-minus');
                const plusBtn = document.getElementById('detail-qty-plus');
                const addBtn = document.getElementById('detail-add-cart-btn');

                minusBtn.addEventListener('click', () => {
                    if (currentQty > 1) {
                        currentQty--;
                        qtyVal.innerText = currentQty;
                    }
                });

                plusBtn.addEventListener('click', () => {
                    if (currentQty < p.stockQuantity) {
                        currentQty++;
                        qtyVal.innerText = currentQty;
                    } else {
                        API.showToast('Maximum available stock reached.', 'warning');
                    }
                });

                addBtn.addEventListener('click', () => {
                    Cart.addItem(p.id, currentQty);
                });

                // Load reviews
                if (typeof Reviews !== 'undefined') {
                    Reviews.loadProductReviews(p.id);
                }
            }
        } catch (err) {
            container.innerHTML = `<p style="color: var(--error);">Error loading product details: ${err.message}</p>`;
        }
    },

    bindSearch() {
        const searchInput = document.getElementById('nav-search-input');
        if (searchInput) {
            searchInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    const q = searchInput.value.trim();
                    if (window.location.pathname.includes('products.html')) {
                        Products.loadProducts({ search: q });
                    } else {
                        const target = `${typeof Auth !== 'undefined' ? Auth.getPath('products.html') : 'products.html'}?q=${encodeURIComponent(q)}`;
                        window.location.href = target;
                    }
                }
            });
        }
    }
};

document.addEventListener('DOMContentLoaded', () => {
    Products.bindSearch();
});
