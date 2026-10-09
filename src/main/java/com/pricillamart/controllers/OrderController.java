package com.pricillamart.controllers;

import com.pricillamart.models.Order;
import com.pricillamart.services.AuthService;
import com.pricillamart.services.DatabaseService;
import com.pricillamart.utils.ApiResponse;
import com.pricillamart.utils.JwtUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api")
@CrossOrigin(origins = "*")
public class OrderController {

    @Autowired
    private DatabaseService databaseService;

    @Autowired
    private AuthService authService;

    @PostMapping("/orders")
    public ResponseEntity<ApiResponse<Object>> createOrder(
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, String> body) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in to complete checkout."));
        }

        try {
            String buyerName = body.get("buyerName");
            String email = body.get("email");
            String phone = body.get("phone");
            String address = body.get("deliveryAddress");

            if (buyerName == null || email == null || phone == null || address == null) {
                return ResponseEntity.badRequest().body(ApiResponse.error("All checkout fields are required."));
            }

            Order order = databaseService.createOrder(user.getUserId(), buyerName, email, phone, address);

            Map<String, Object> data = new HashMap<>();
            data.put("orderCode", order.getOrderCode());
            data.put("totalAmount", order.getTotalAmount());

            return ResponseEntity.status(201).body(ApiResponse.success("Order placed successfully. Thank you for shopping with Pricilla Mart!", data));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error(e.getMessage()));
        }
    }

    @GetMapping("/orders")
    public ResponseEntity<ApiResponse<List<Order>>> getOrders(
            @RequestHeader(value = "Authorization", required = false) String authHeader) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }

        List<Order> orders;
        if ("admin".equalsIgnoreCase(user.getRole())) {
            orders = databaseService.getAllOrders();
        } else if ("seller".equalsIgnoreCase(user.getRole())) {
            orders = databaseService.getOrdersBySeller(user.getUserId());
        } else {
            orders = databaseService.getOrdersByBuyer(user.getUserId());
        }

        return ResponseEntity.ok(ApiResponse.success("Orders fetched successfully.", orders));
    }

    @PutMapping("/orders/{id}/status")
    public ResponseEntity<ApiResponse<String>> updateOrderStatus(
            @PathVariable("id") int orderId,
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, String> body) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid() || (!"seller".equalsIgnoreCase(user.getRole()) && !"admin".equalsIgnoreCase(user.getRole()))) {
            return ResponseEntity.status(403).body(ApiResponse.error("Forbidden: Only sellers and admins can update order status."));
        }

        String status = body.get("status");
        if (status == null || status.trim().isEmpty()) {
            return ResponseEntity.badRequest().body(ApiResponse.error("New order status is required."));
        }

        try {
            databaseService.updateOrderStatus(orderId, status);
            return ResponseEntity.ok(ApiResponse.success("Order status updated successfully to " + status + ".", null));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error("Failed to update status: " + e.getMessage()));
        }
    }
}
