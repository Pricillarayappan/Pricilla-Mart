package com.pricillamart.services;

import com.pricillamart.models.Order;
import com.pricillamart.models.Product;
import com.pricillamart.utils.JwtUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.*;

@Service
public class ChatbotService {

    @Autowired
    private DatabaseService databaseService;

    public Map<String, Object> processMessage(String message, JwtUtil.TokenPayload user) {
        String msgLower = (message != null) ? message.toLowerCase() : "";
        String reply;
        List<Product> recommendations = new ArrayList<>();
        List<String> quickReplies = new ArrayList<>();

        if (containsAny(msgLower, "hi", "hello", "hey", "who are you")) {
            reply = "Hello! I am your Pricilla Mart AI Assistant. How can I help you today?";
            quickReplies = Arrays.asList("Find Laptops", "Show Electronics", "Track Orders", "How to Buy");
        } else if (containsAny(msgLower, "laptop", "computer", "pc", "macbook")) {
            reply = "Sure! You can explore laptops in our Electronics category. Here are our top picks:";
            recommendations = databaseService.getProducts("Laptop", 1, 0, "", 0, 0);
            quickReplies = Arrays.asList("View Specs", "All Electronics", "Add to Cart");
        } else if (containsAny(msgLower, "mobile", "phone", "smartphone", "5g")) {
            reply = "We have high-end 5G smartphones with superb cameras in our Electronics department:";
            recommendations = databaseService.getProducts("Mobile", 1, 0, "", 0, 0);
            quickReplies = Arrays.asList("View Mobile", "Compare Price", "Check Headphones");
        } else if (containsAny(msgLower, "headphone", "audio", "earphone", "sound")) {
            reply = "Looking for great sound? Check out our ANC wireless audio gear:";
            recommendations = databaseService.getProducts("Headphones", 1, 0, "", 0, 0);
            quickReplies = Arrays.asList("View Headphones", "Electronics Category", "Smart Watch");
        } else if (containsAny(msgLower, "watch", "smartwatch", "handbag", "accessories")) {
            reply = "Here are popular picks in our Accessories collection:";
            recommendations = databaseService.getProducts("", 3, 0, "", 0, 0);
            quickReplies = Arrays.asList("Smart Watch", "Vintage Handbag", "All Accessories");
        } else if (containsAny(msgLower, "fashion", "jacket", "cloth", "dress")) {
            reply = "Explore stylish apparel in our Fashion department:";
            recommendations = databaseService.getProducts("", 2, 0, "", 0, 0);
            quickReplies = Arrays.asList("Denim Jacket", "Explore Fashion", "Accessories");
        } else if (containsAny(msgLower, "order", "track", "delivery", "status")) {
            if (user != null && user.isValid()) {
                List<Order> orders = databaseService.getOrdersByBuyer(user.getUserId());
                if (!orders.isEmpty()) {
                    Order latest = orders.get(0);
                    reply = "Your latest order is " + latest.getOrderCode() + " (Status: " + latest.getStatus() + ") placed on " + latest.getCreatedAt() + " for ₹" + String.format("%,d", (int) latest.getTotalAmount()) + ". You can track it in your Orders tab!";
                    quickReplies = Arrays.asList("Go to Orders", "Buy more items", "Write a review");
                } else {
                    reply = "You don't have any active orders yet. Browse our catalog and place your first order today!";
                    quickReplies = Arrays.asList("Explore Products", "Top Categories");
                }
            } else {
                reply = "To view your personalized order history and live shipment tracking, please log in with your Buyer account.";
                quickReplies = Arrays.asList("Login now", "Register Account");
            }
        } else if (containsAny(msgLower, "cart", "how to buy", "how to order", "checkout")) {
            reply = "Ordering is easy on Pricilla Mart! 1) Browse products, 2) Click 'Add to Cart', 3) Open your Cart in the navbar, and 4) Proceed to Checkout with your delivery address. No real payment required for this capstone demo!";
            quickReplies = Arrays.asList("View Cart", "Browse Products", "Check My Orders");
        } else {
            recommendations = databaseService.searchProductsForChatbot(message, 3);
            if (!recommendations.isEmpty()) {
                reply = "Here are matching products from our catalog:";
                quickReplies = Arrays.asList("View Details", "Search more", "All Categories");
            } else {
                reply = "I'm here to help! You can ask me about laptops, smartphones, fashion, smartwatches, tracking orders, or how to checkout.";
                quickReplies = Arrays.asList("Show Electronics", "Best Sellers", "Track Orders", "Categories");
            }
        }

        Map<String, Object> resp = new HashMap<>();
        resp.put("reply", reply);
        resp.put("recommendations", recommendations);
        resp.put("quickReplies", quickReplies);
        return resp;
    }

    private boolean containsAny(String text, String... keywords) {
        for (String kw : keywords) {
            if (text.contains(kw)) return true;
        }
        return false;
    }
}
