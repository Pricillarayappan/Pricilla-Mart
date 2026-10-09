package com.pricillamart.utils;

import java.net.URLDecoder;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class JwtUtil {

    public static class TokenPayload {
        private int userId;
        private String email;
        private String role;
        private long expiresAt;
        private boolean valid;

        public TokenPayload() {}

        public TokenPayload(int userId, String email, String role, long expiresAt, boolean valid) {
            this.userId = userId;
            this.email = email;
            this.role = role;
            this.expiresAt = expiresAt;
            this.valid = valid;
        }

        public int getUserId() { return userId; }
        public String getEmail() { return email; }
        public String getRole() { return role; }
        public long getExpiresAt() { return expiresAt; }
        public boolean isValid() { return valid; }
    }

    public static String createToken(int userId, String email, String role, String secret) {
        long exp = (System.currentTimeMillis() / 1000L) + (86400L * 7L);
        String payload = userId + ":" + email + ":" + role + ":" + exp;
        String encodedPayload = Base64.getUrlEncoder().withoutPadding().encodeToString(payload.getBytes(StandardCharsets.UTF_8));
        String signature = PasswordUtil.hashPassword(encodedPayload + "." + secret);
        return encodedPayload + "." + signature;
    }

    public static TokenPayload verifyToken(String token, String secret) {
        if (token == null || !token.contains(".")) {
            return new TokenPayload(0, "", "", 0, false);
        }

        int dot = token.lastIndexOf('.');
        String encodedPayload = token.substring(0, dot);
        String receivedSig = token.substring(dot + 1);

        String expectedSig = PasswordUtil.hashPassword(encodedPayload + "." + secret);
        if (!receivedSig.equalsIgnoreCase(expectedSig)) {
            return new TokenPayload(0, "", "", 0, false);
        }

        try {
            String decoded;
            try {
                decoded = new String(Base64.getUrlDecoder().decode(encodedPayload), StandardCharsets.UTF_8);
            } catch (Exception ex) {
                decoded = URLDecoder.decode(encodedPayload, StandardCharsets.UTF_8);
            }
            String[] parts = decoded.split(":");
            if (parts.length != 4) {
                return new TokenPayload(0, "", "", 0, false);
            }

            int userId = Integer.parseInt(parts[0]);
            String email = parts[1];
            String role = parts[2];
            long expiresAt = Long.parseLong(parts[3]);

            if (expiresAt < (System.currentTimeMillis() / 1000L)) {
                return new TokenPayload(0, "", "", 0, false); // Expired
            }

            return new TokenPayload(userId, email, role, expiresAt, true);
        } catch (Exception e) {
            return new TokenPayload(0, "", "", 0, false);
        }
    }
}
