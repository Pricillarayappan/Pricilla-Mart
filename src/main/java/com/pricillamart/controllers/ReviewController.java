package com.pricillamart.controllers;

import com.pricillamart.models.Review;
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
public class ReviewController {

    @Autowired
    private DatabaseService databaseService;

    @Autowired
    private AuthService authService;

    @PostMapping("/reviews")
    public ResponseEntity<ApiResponse<String>> addReview(
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, Object> body) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in to write a review."));
        }

        try {
            int productId = Integer.parseInt(body.get("productId").toString());
            int rating = Integer.parseInt(body.get("rating").toString());
            String comment = (String) body.get("comment");

            if (rating < 1 || rating > 5 || comment == null || comment.trim().isEmpty()) {
                return ResponseEntity.badRequest().body(ApiResponse.error("Valid rating (1-5) and written comment are required."));
            }

            Review r = new Review();
            r.setProductId(productId);
            r.setBuyerId(user.getUserId());
            r.setRating(rating);
            r.setComment(comment.trim());

            databaseService.addReview(r);
            return ResponseEntity.status(201).body(ApiResponse.success("Review submitted successfully.", null));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(ApiResponse.error(e.getMessage()));
        }
    }

    @GetMapping("/products/{id}/reviews")
    public ResponseEntity<ApiResponse<Object>> getProductReviews(@PathVariable("id") int productId) {
        List<Review> reviews = databaseService.getReviewsByProduct(productId);
        double avg = reviews.isEmpty() ? 0.0 : reviews.stream().mapToInt(Review::getRating).average().orElse(0.0);

        Map<String, Object> data = new HashMap<>();
        data.put("reviews", reviews);
        data.put("averageRating", avg);
        data.put("reviewCount", reviews.size());

        return ResponseEntity.ok(ApiResponse.success("Reviews fetched successfully.", data));
    }
}
