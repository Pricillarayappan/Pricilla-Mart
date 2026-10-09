package com.pricillamart.services;

import com.pricillamart.models.User;
import com.pricillamart.utils.JwtUtil;
import com.pricillamart.utils.PasswordUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

@Service
public class AuthService {

    @Autowired
    private DatabaseService databaseService;

    @Value("${app.jwt.secret:pricillamart_secret_key_college_capstone_2026}")
    private String jwtSecret;

    public Map<String, Object> register(String name, String email, String phone, String password, String confirmPassword, String role) throws Exception {
        if (name == null || email == null || phone == null || password == null || confirmPassword == null || role == null) {
            throw new IllegalArgumentException("All registration fields are required.");
        }
        if (!password.equals(confirmPassword)) {
            throw new IllegalArgumentException("Passwords do not match.");
        }
        if (password.length() < 6) {
            throw new IllegalArgumentException("Password must be at least 6 characters.");
        }
        if (!"buyer".equalsIgnoreCase(role) && !"seller".equalsIgnoreCase(role)) {
            throw new IllegalArgumentException("Role must be 'buyer' or 'seller'.");
        }

        if (databaseService.findUserByEmail(email).isPresent()) {
            throw new IllegalArgumentException("An account with this email already exists.");
        }

        User u = new User();
        u.setName(name);
        u.setEmail(email);
        u.setPhone(phone);
        u.setPasswordHash(PasswordUtil.hashPassword(password));
        u.setRole(role.toLowerCase());

        int newId = databaseService.createUser(u);
        String token = JwtUtil.createToken(newId, email, role.toLowerCase(), jwtSecret);

        Map<String, Object> userData = new HashMap<>();
        userData.put("id", newId);
        userData.put("name", name);
        userData.put("email", email);
        userData.put("phone", phone);
        userData.put("role", role.toLowerCase());

        Map<String, Object> resp = new HashMap<>();
        resp.put("token", token);
        resp.put("user", userData);
        return resp;
    }

    public Map<String, Object> login(String email, String password) throws Exception {
        if (email == null || password == null || email.trim().isEmpty() || password.isEmpty()) {
            throw new IllegalArgumentException("Email and password are required.");
        }

        Optional<User> opt = databaseService.findUserByEmail(email.trim());
        if (opt.isEmpty()) {
            throw new IllegalArgumentException("Invalid email or password.");
        }

        User u = opt.get();
        if (!PasswordUtil.verify(password, u.getPasswordHash())) {
            throw new IllegalArgumentException("Invalid email or password.");
        }

        String token = JwtUtil.createToken(u.getId(), u.getEmail(), u.getRole(), jwtSecret);

        Map<String, Object> userData = new HashMap<>();
        userData.put("id", u.getId());
        userData.put("name", u.getName());
        userData.put("email", u.getEmail());
        userData.put("phone", u.getPhone());
        userData.put("role", u.getRole());

        Map<String, Object> resp = new HashMap<>();
        resp.put("token", token);
        resp.put("user", userData);
        return resp;
    }

    public JwtUtil.TokenPayload authenticate(String authHeader) {
        if (authHeader == null || !authHeader.startsWith("Bearer ")) {
            return new JwtUtil.TokenPayload(0, "", "", 0, false);
        }
        return JwtUtil.verifyToken(authHeader.substring(7).trim(), jwtSecret);
    }
}
