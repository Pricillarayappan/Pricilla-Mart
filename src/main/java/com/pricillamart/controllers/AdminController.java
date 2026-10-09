package com.pricillamart.controllers;

import com.pricillamart.models.User;
import com.pricillamart.services.AuthService;
import com.pricillamart.services.DatabaseService;
import com.pricillamart.utils.ApiResponse;
import com.pricillamart.utils.JwtUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api")
@CrossOrigin(origins = "*")
public class AdminController {

    @Autowired
    private DatabaseService databaseService;

    @Autowired
    private AuthService authService;

    @GetMapping("/admin/stats")
    public ResponseEntity<ApiResponse<Map<String, Object>>> getAdminStats(
            @RequestHeader(value = "Authorization", required = false) String authHeader) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid() || !"admin".equalsIgnoreCase(user.getRole())) {
            return ResponseEntity.status(403).body(ApiResponse.error("Forbidden: Admin access required."));
        }

        Map<String, Object> stats = databaseService.getAdminStats();
        return ResponseEntity.ok(ApiResponse.success("Admin stats retrieved.", stats));
    }

    @GetMapping("/admin/users")
    public ResponseEntity<ApiResponse<List<User>>> getAdminUsers(
            @RequestParam(value = "role", required = false, defaultValue = "") String role,
            @RequestHeader(value = "Authorization", required = false) String authHeader) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid() || !"admin".equalsIgnoreCase(user.getRole())) {
            return ResponseEntity.status(403).body(ApiResponse.error("Forbidden: Admin access required."));
        }

        List<User> users = databaseService.getAllUsers(role);
        return ResponseEntity.ok(ApiResponse.success("Users retrieved.", users));
    }

    @GetMapping("/seller/stats")
    public ResponseEntity<ApiResponse<Map<String, Object>>> getSellerStats(
            @RequestHeader(value = "Authorization", required = false) String authHeader) {

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid() || (!"seller".equalsIgnoreCase(user.getRole()) && !"admin".equalsIgnoreCase(user.getRole()))) {
            return ResponseEntity.status(403).body(ApiResponse.error("Forbidden: Seller access required."));
        }

        Map<String, Object> stats = databaseService.getSellerStats(user.getUserId());
        return ResponseEntity.ok(ApiResponse.success("Seller stats retrieved.", stats));
    }
}
