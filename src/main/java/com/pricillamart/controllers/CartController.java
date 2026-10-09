package com.pricillamart.controllers;

import com.pricillamart.models.CartItem;
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
public class CartController {

    @Autowired
    private DatabaseService databaseService;

    @Autowired
    private AuthService authService;

    @GetMapping("/cart")
    public ResponseEntity<ApiResponse<Object>> getCart(@RequestHeader(value = "Authorization", required = false) String authHeader) {
        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in to view cart."));
        }

        List<CartItem> items = databaseService.getCartItems(user.getUserId());
        double subtotal = items.stream().mapToDouble(CartItem::getSubtotal).sum();

        Map<String, Object> data = new HashMap<>();
        data.put("items", items);
        data.put("subtotal", subtotal);
        data.put("total", subtotal);
        data.put("itemCount", items.size());

        return ResponseEntity.ok(ApiResponse.success("Cart fetched successfully.", data));
    }

    @PostMapping("/cart")
    public ResponseEntity<ApiResponse<Object>> addToCart(
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, Object> body) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in to add items."));
        }

        try {
            int productId = Integer.parseInt(body.get("productId").toString());
            int quantity = body.containsKey("quantity") ? Integer.parseInt(body.get("quantity").toString()) : 1;

            if (quantity <= 0) {
                return ResponseEntity.badRequest().body(ApiResponse.error("Quantity must be at least 1."));
            }

            databaseService.addToCart(user.getUserId(), productId, quantity);
            return ResponseEntity.ok(ApiResponse.success("Item added to cart.", null));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error(e.getMessage()));
        }
    }

    @PutMapping("/cart/{id}")
    public ResponseEntity<ApiResponse<Object>> updateCartQuantity(
            @PathVariable("id") int cartItemId,
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, Object> body) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }

        try {
            int newQuantity = Integer.parseInt(body.get("quantity").toString());
            databaseService.updateCartQuantity(user.getUserId(), cartItemId, newQuantity);
            return ResponseEntity.ok(ApiResponse.success("Cart quantity updated.", null));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error(e.getMessage()));
        }
    }

    @DeleteMapping("/cart/{id}")
    public ResponseEntity<ApiResponse<String>> removeFromCart(
            @PathVariable("id") int cartItemId,
            @RequestHeader(value = "Authorization", required = false) String authHeader) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }

        try {
            databaseService.removeFromCart(user.getUserId(), cartItemId);
            return ResponseEntity.ok(ApiResponse.success("Item removed from cart.", null));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error(e.getMessage()));
        }
    }

    @DeleteMapping("/cart")
    public ResponseEntity<ApiResponse<String>> clearCart(@RequestHeader(value = "Authorization", required = false) String authHeader) {
        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }

        try {
            databaseService.clearCart(user.getUserId());
            return ResponseEntity.ok(ApiResponse.success("Cart cleared.", null));
        } catch (Exception e) {
            return ResponseEntity.internalServerError().body(ApiResponse.error(e.getMessage()));
        }
    }
}
