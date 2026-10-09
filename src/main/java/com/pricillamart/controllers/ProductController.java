package com.pricillamart.controllers;

import com.pricillamart.models.Product;
import com.pricillamart.services.AuthService;
import com.pricillamart.services.DatabaseService;
import com.pricillamart.utils.ApiResponse;
import com.pricillamart.utils.JwtUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;
import java.util.Optional;

@RestController
@RequestMapping("/api")
@CrossOrigin(origins = "*")
public class ProductController {

    @Autowired
    private DatabaseService databaseService;

    @Autowired
    private AuthService authService;

    @GetMapping("/products")
    public ResponseEntity<ApiResponse<List<Product>>> getProducts(
            @RequestParam(value = "q", required = false) String query,
            @RequestParam(value = "category_id", required = false, defaultValue = "0") int categoryId,
            @RequestParam(value = "seller_id", required = false, defaultValue = "0") int sellerId,
            @RequestParam(value = "sort", required = false, defaultValue = "") String sort,
            @RequestParam(value = "min_price", required = false, defaultValue = "0") double minPrice,
            @RequestParam(value = "max_price", required = false, defaultValue = "0") double maxPrice) {

        List<Product> products = databaseService.getProducts(query, categoryId, sellerId, sort, minPrice, maxPrice);
        return ResponseEntity.ok(ApiResponse.success("Products fetched successfully.", products));
    }

    @GetMapping("/products/{id}")
    public ResponseEntity<ApiResponse<Product>> getProductById(@PathVariable("id") int id) {
        Optional<Product> opt = databaseService.getProductById(id);
        if (opt.isEmpty()) {
            return ResponseEntity.status(404).body(ApiResponse.error("Product not found."));
        }
        return ResponseEntity.ok(ApiResponse.success("Product details fetched.", opt.get()));
    }

    @PostMapping("/products")
    public ResponseEntity<ApiResponse<Object>> createProduct(
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, Object> body) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }
        if (!"seller".equalsIgnoreCase(user.getRole()) && !"admin".equalsIgnoreCase(user.getRole())) {
            return ResponseEntity.status(403).body(ApiResponse.error("Forbidden: Only sellers and admins can add products."));
        }

        try {
            String name = (String) body.get("name");
            String description = (String) body.get("description");
            int categoryId = Integer.parseInt(body.get("categoryId").toString());
            double price = Double.parseDouble(body.get("price").toString());
            int stockQuantity = Integer.parseInt(body.get("stockQuantity").toString());
            String imageUrl = (String) body.get("imageUrl");

            if (name == null || name.trim().isEmpty() || price <= 0 || stockQuantity < 0) {
                return ResponseEntity.badRequest().body(ApiResponse.error("Invalid product details provided."));
            }

            if (imageUrl == null || imageUrl.trim().isEmpty()) {
                imageUrl = "https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600";
            }

            Product p = new Product();
            p.setName(name);
            p.setDescription(description);
            p.setCategoryId(categoryId);
            p.setPrice(price);
            p.setStockQuantity(stockQuantity);
            p.setImageUrl(imageUrl);
            p.setSellerId(user.getUserId());

            int newId = databaseService.createProduct(p);
            p.setId(newId);
            return ResponseEntity.status(201).body(ApiResponse.success("Product created successfully.", p));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error("Failed to create product: " + e.getMessage()));
        }
    }

    @PutMapping("/products/{id}")
    public ResponseEntity<ApiResponse<Object>> updateProduct(
            @PathVariable("id") int id,
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, Object> body) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }

        Optional<Product> existingOpt = databaseService.getProductById(id);
        if (existingOpt.isEmpty()) {
            return ResponseEntity.status(404).body(ApiResponse.error("Product not found."));
        }

        Product existing = existingOpt.get();
        boolean isAdmin = "admin".equalsIgnoreCase(user.getRole());
        if (!isAdmin && existing.getSellerId() != user.getUserId()) {
            return ResponseEntity.status(403).body(ApiResponse.error("Forbidden: You are only allowed to modify your own products."));
        }

        try {
            if (body.containsKey("name")) existing.setName((String) body.get("name"));
            if (body.containsKey("description")) existing.setDescription((String) body.get("description"));
            if (body.containsKey("categoryId")) existing.setCategoryId(Integer.parseInt(body.get("categoryId").toString()));
            if (body.containsKey("price")) existing.setPrice(Double.parseDouble(body.get("price").toString()));
            if (body.containsKey("stockQuantity")) existing.setStockQuantity(Integer.parseInt(body.get("stockQuantity").toString()));
            if (body.containsKey("imageUrl")) existing.setImageUrl((String) body.get("imageUrl"));

            if (isAdmin) {
                existing.setSellerId(0); // ignore seller id check in updateProduct
            }

            databaseService.updateProduct(existing);
            return ResponseEntity.ok(ApiResponse.success("Product updated successfully.", existing));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error("Failed to update product: " + e.getMessage()));
        }
    }

    @DeleteMapping("/products/{id}")
    public ResponseEntity<ApiResponse<String>> deleteProduct(
            @PathVariable("id") int id,
            @RequestHeader(value = "Authorization", required = false) String authHeader) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }

        Optional<Product> existingOpt = databaseService.getProductById(id);
        if (existingOpt.isEmpty()) {
            return ResponseEntity.status(404).body(ApiResponse.error("Product not found."));
        }

        boolean isAdmin = "admin".equalsIgnoreCase(user.getRole());
        if (!isAdmin && existingOpt.get().getSellerId() != user.getUserId()) {
            return ResponseEntity.status(403).body(ApiResponse.error("Forbidden: You are only allowed to delete your own products."));
        }

        try {
            databaseService.deleteProduct(id, user.getUserId(), isAdmin);
            return ResponseEntity.ok(ApiResponse.success("Product deleted successfully.", null));
        } catch (Exception e) {
            return ResponseEntity.internalServerError().body(ApiResponse.error("Failed to delete product: " + e.getMessage()));
        }
    }
}
