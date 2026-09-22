/**
 * PRICILLA MART - AI CHATBOT CONTROLLER
 * Contextual Assistant for product discovery, checkout advice, and viva demo
 */

const Chatbot = {
    isOpen: false,

    init() {
        this.injectChatbotUI();
        this.bindEvents();
    },

    injectChatbotUI() {
        if (document.getElementById('chatbot-root')) return;

        const wrapper = document.createElement('div');
        wrapper.id = 'chatbot-root';
        wrapper.innerHTML = `
            <button class="chatbot-trigger" id="chatbot-toggle-btn" title="Chat with Pricilla Mart AI">
                <i class="fas fa-robot"></i>
            </button>

            <div class="chatbot-window" id="chatbot-window">
                <div class="chat-header">
                    <div class="chat-title-group">
                        <div class="chat-status-dot"></div>
                        <div>
                            <h4>Pricilla AI Assistant</h4>
                            <div style="font-size: 0.72rem; color: var(--light);">Online | Capstone Assistant</div>
                        </div>
                    </div>
                    <button class="modal-close" id="chatbot-close-btn" style="color: #fff; font-size: 1.2rem;">&times;</button>
                </div>

                <div class="chat-messages" id="chatbot-messages">
                    <div class="chat-msg chat-msg-bot">
                        Hi! I'm <strong>Pricilla Mart AI Assistant</strong>. How can I help you today?
                        <div class="chat-quick-replies">
                            <button class="quick-reply-btn" onclick="Chatbot.sendPredefined('I need a laptop')">Find Laptops</button>
                            <button class="quick-reply-btn" onclick="Chatbot.sendPredefined('Show me fashion products')">Fashion</button>
                            <button class="quick-reply-btn" onclick="Chatbot.sendPredefined('How do I track my order?')">Track Orders</button>
                            <button class="quick-reply-btn" onclick="Chatbot.sendPredefined('How does checkout work?')">How to Buy</button>
                        </div>
                    </div>
                </div>

                <form class="chat-input-area" id="chatbot-form">
                    <input type="text" id="chatbot-input" placeholder="Ask anything (e.g. laptop, watch, order)..." autocomplete="off">
                    <button type="submit" class="btn btn-primary btn-sm" style="padding: 0 1rem;">
                        <i class="fas fa-paper-plane"></i>
                    </button>
                </form>
            </div>
        `;
        document.body.appendChild(wrapper);
    },

    bindEvents() {
        const toggleBtn = document.getElementById('chatbot-toggle-btn');
        const closeBtn = document.getElementById('chatbot-close-btn');
        const chatWindow = document.getElementById('chatbot-window');
        const form = document.getElementById('chatbot-form');
        const input = document.getElementById('chatbot-input');

        if (toggleBtn && chatWindow) {
            toggleBtn.addEventListener('click', () => {
                this.isOpen = !this.isOpen;
                chatWindow.classList.toggle('active', this.isOpen);
                if (this.isOpen && input) input.focus();
            });
        }

        if (closeBtn && chatWindow) {
            closeBtn.addEventListener('click', () => {
                this.isOpen = false;
                chatWindow.classList.remove('active');
            });
        }

        if (form && input) {
            form.addEventListener('submit', (e) => {
                e.preventDefault();
                const msg = input.value.trim();
                if (!msg) return;
                input.value = '';
                this.sendMessage(msg);
            });
        }
    },

    sendPredefined(text) {
        this.sendMessage(text);
    },

    async sendMessage(text) {
        const msgContainer = document.getElementById('chatbot-messages');
        if (!msgContainer) return;

        // Append user bubble
        const userBubble = document.createElement('div');
        userBubble.className = 'chat-msg chat-msg-user';
        userBubble.innerText = text;
        msgContainer.appendChild(userBubble);
        msgContainer.scrollTop = msgContainer.scrollHeight;

        // Append typing indicator
        const typingEl = document.createElement('div');
        typingEl.className = 'chat-msg chat-msg-bot';
        typingEl.innerHTML = `<i class="fas fa-circle-notch fa-spin"></i> Thinking...`;
        msgContainer.appendChild(typingEl);
        msgContainer.scrollTop = msgContainer.scrollHeight;

        try {
            const res = await API.post('/chatbot', { message: text });
            typingEl.remove();

            if (res.success && res.data) {
                const botBubble = document.createElement('div');
                botBubble.className = 'chat-msg chat-msg-bot';

                let replyHtml = `<div>${res.data.reply}</div>`;

                // Recommendations
                if (res.data.recommendations && res.data.recommendations.length > 0) {
                    replyHtml += `<div style="margin-top: 0.8rem; display: flex; flex-direction: column; gap: 0.5rem;">`;
                    res.data.recommendations.forEach(p => {
                        replyHtml += `
                            <div style="display: flex; align-items: center; gap: 0.6rem; background: var(--white); padding: 0.5rem; border-radius: 6px; border: 1px solid var(--border-color); cursor: pointer;" onclick="window.location.href='/product-details.html?id=${p.id}'">
                                <img src="${p.imageUrl}" alt="${p.name}" style="width: 36px; height: 36px; border-radius: 4px; object-fit: cover;">
                                <div style="flex: 1; min-width: 0;">
                                    <div style="font-size: 0.8rem; font-weight: 700; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; color: var(--primary);">${p.name}</div>
                                    <div style="font-size: 0.75rem; color: var(--primary); font-weight: 800;">₹${Number(p.price).toLocaleString('en-IN')}</div>
                                </div>
                                <button class="btn btn-primary btn-sm" style="font-size: 0.7rem; padding: 0.2rem 0.5rem;" onclick="event.stopPropagation(); Cart.addItem(${p.id}, 1)">+ Cart</button>
                            </div>
                        `;
                    });
                    replyHtml += `</div>`;
                }

                // Quick replies
                if (res.data.quickReplies && res.data.quickReplies.length > 0) {
                    replyHtml += `<div class="chat-quick-replies">`;
                    res.data.quickReplies.forEach(qr => {
                        replyHtml += `<button class="quick-reply-btn" onclick="Chatbot.sendPredefined('${qr}')">${qr}</button>`;
                    });
                    replyHtml += `</div>`;
                }

                botBubble.innerHTML = replyHtml;
                msgContainer.appendChild(botBubble);
            }
        } catch (err) {
            typingEl.innerHTML = `Sorry, I encountered an issue: ${err.message}`;
        }

        msgContainer.scrollTop = msgContainer.scrollHeight;
    }
};

document.addEventListener('DOMContentLoaded', () => {
    Chatbot.init();
});
