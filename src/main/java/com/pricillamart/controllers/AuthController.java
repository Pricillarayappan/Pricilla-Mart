package com.pricillamart.controllers;

import com.pricillamart.models.User;
import com.pricillamart.services.AuthService;
import com.pricillamart.services.DatabaseService;
import com.pricillamart.utils.ApiResponse;
import com.pricillamart.utils.JwtUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.Map;
import java.util.Optional;

@RestController
@RequestMapping("/api")
@CrossOrigin(origins = "*")
public class AuthController {

    @Autowired
    private AuthService authService;

    @Autowired
    private DatabaseService databaseService;

    @PostMapping("/register")
    public ResponseEntity<ApiResponse<Object>> register(@RequestBody Map<String, String> body) {
        try {
            String name = body.get("name");
            String email = body.get("email");
            String phone = body.get("phone");
            String password = body.get("password");
            String confirmPassword = body.get("confirmPassword");
            String role = body.get("role");

            Map<String, Object> data = authService.register(name, email, phone, password, confirmPassword, role);
            return ResponseEntity.status(201).body(ApiResponse.success("Account registered successfully.", data));
        } catch (IllegalArgumentException e) {
            return ResponseEntity.badRequest().body(ApiResponse.error(e.getMessage()));
        } catch (Exception e) {
            return ResponseEntity.internalServerError().body(ApiResponse.error(e.getMessage()));
        }
    }

    @PostMapping("/login")
    public ResponseEntity<ApiResponse<Object>> login(@RequestBody Map<String, String> body) {
        try {
            String email = body.get("email");
            String password = body.get("password");

            Map<String, Object> data = authService.login(email, password);
            return ResponseEntity.ok(ApiResponse.success("Login successful.", data));
        } catch (IllegalArgumentException e) {
            return ResponseEntity.status(401).body(ApiResponse.error(e.getMessage()));
        } catch (Exception e) {
            return ResponseEntity.internalServerError().body(ApiResponse.error(e.getMessage()));
        }
    }

    @PostMapping("/logout")
    public ResponseEntity<ApiResponse<String>> logout() {
        return ResponseEntity.ok(ApiResponse.success("Logged out successfully.", null));
    }

    @GetMapping("/profile")
    public ResponseEntity<ApiResponse<Object>> getProfile(@RequestHeader(value = "Authorization", required = false) String authHeader) {
        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        if (!user.isValid()) {
            return ResponseEntity.status(401).body(ApiResponse.error("Unauthorized. Please log in."));
        }

        Optional<User> opt = databaseService.findUserById(user.getUserId());
        if (opt.isEmpty()) {
            return ResponseEntity.status(404).body(ApiResponse.error("User not found."));
        }
        return ResponseEntity.ok(ApiResponse.success("User profile fetched.", opt.get()));
    }
}
