/**
 * PRICILLA MART - REVIEWS & RATINGS CONTROLLER
 */

const Reviews = {
    selectedRating: 5,

    initRatingPicker() {
        const stars = document.querySelectorAll('.star-rating-picker i');
        stars.forEach(star => {
            star.addEventListener('click', () => {
                const val = parseInt(star.getAttribute('data-value'));
                Reviews.selectedRating = val;
                Reviews.renderStars(val);
            });
            star.addEventListener('mouseenter', () => {
                const val = parseInt(star.getAttribute('data-value'));
                Reviews.renderStars(val);
            });
        });

        const picker = document.querySelector('.star-rating-picker');
        if (picker) {
            picker.addEventListener('mouseleave', () => {
                Reviews.renderStars(Reviews.selectedRating);
            });
        }
    },

    renderStars(count) {
        const stars = document.querySelectorAll('.star-rating-picker i');
        stars.forEach((s, idx) => {
            if (idx < count) {
                s.className = 'fas fa-star';
                s.style.color = '#d48b11';
            } else {
                s.className = 'far fa-star';
                s.style.color = 'var(--secondary)';
            }
        });
    },

    async loadProductReviews(productId) {
        const container = document.getElementById('reviews-list-container');
        if (!container) return;

        try {
            const res = await API.get(`/products/${productId}/reviews`);
            if (res.success && res.data) {
                const reviews = Array.isArray(res.data) ? res.data : (res.data.reviews || []);
                const reviewCount = res.data.reviewCount !== undefined ? res.data.reviewCount : reviews.length;

                const countEl = document.getElementById('reviews-count-badge');
                if (countEl) countEl.innerText = `${reviewCount} Reviews`;

                if (reviews.length === 0) {
                    container.innerHTML = `
                        <div style="padding: 2rem; text-align: center; color: var(--text-muted); background: var(--white); border-radius: var(--radius-md);">
                            <p>No customer reviews yet. Be the first to review this product!</p>
                        </div>
                    `;
                    return;
                }

                container.innerHTML = reviews.map(r => {
                    const stars = '★'.repeat(r.rating) + '☆'.repeat(5 - r.rating);
                    return `
                        <div style="background: var(--white); padding: 1.2rem; border-radius: var(--radius-md); margin-bottom: 1rem; border: 1px solid rgba(136, 112, 142, 0.2); box-shadow: var(--shadow-sm);">
                            <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.5rem;">
                                <div>
                                    <strong style="color: var(--primary);"><i class="fas fa-user-circle"></i> ${r.buyerName}</strong>
                                    <span style="font-size: 0.75rem; background: #e0f2fe; color: #0369a1; padding: 0.15rem 0.5rem; border-radius: 999px; margin-left: 0.4rem; font-weight: 600;">
                                        ✔ Verified Purchase
                                    </span>
                                </div>
                                <span style="font-size: 0.8rem; color: var(--text-muted);">${r.createdAt}</span>
                            </div>
                            <div style="color: #d48b11; margin-bottom: 0.5rem; font-size: 0.95rem;">${stars}</div>
                            <p style="color: var(--text-dark); font-size: 0.92rem; line-height: 1.5;">${r.comment}</p>
                        </div>
                    `;
                }).join('');
            }
        } catch (err) {
            container.innerHTML = `<p style="color: var(--error);">Error loading reviews: ${err.message}</p>`;
        }
    },

    async submitReview(productId) {
        const user = API.getUser();
        if (!user) {
            API.showToast('Please log in to submit a review.', 'info');
            setTimeout(() => {
                window.location.href = (typeof Auth !== 'undefined' ? Auth.getPath('login.html') : 'login.html');
            }, 800);
            return;
        }

        const commentField = document.getElementById('review-comment-input');
        if (!commentField) return;

        const comment = commentField.value.trim();
        if (!comment) {
            API.showToast('Please write a brief comment sharing your experience.', 'warning');
            return;
        }

        try {
            const res = await API.post('/reviews', {
                productId,
                rating: Reviews.selectedRating,
                comment
            });

            if (res.success) {
                API.showToast(res.message || 'Review submitted successfully!', 'success');
                commentField.value = '';
                Reviews.loadProductReviews(productId);
            }
        } catch (err) {
            API.showToast(err.message || 'Could not submit review.', 'error');
        }
    }
};
