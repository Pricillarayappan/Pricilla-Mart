package com.pricillamart.controllers;

import com.pricillamart.services.AuthService;
import com.pricillamart.services.ChatbotService;
import com.pricillamart.utils.ApiResponse;
import com.pricillamart.utils.JwtUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.Map;

@RestController
@RequestMapping("/api")
@CrossOrigin(origins = "*")
public class ChatbotController {

    @Autowired
    private ChatbotService chatbotService;

    @Autowired
    private AuthService authService;

    @PostMapping("/chatbot")
    public ResponseEntity<ApiResponse<Map<String, Object>>> chat(
            @RequestHeader(value = "Authorization", required = false) String authHeader,
            @RequestBody Map<String, String> body) {

        String message = body.get("message");
        if (message == null || message.trim().isEmpty()) {
            return ResponseEntity.badRequest().body(ApiResponse.error("Message cannot be empty."));
        }

        JwtUtil.TokenPayload user = authService.authenticate(authHeader);
        Map<String, Object> replyData = chatbotService.processMessage(message.trim(), user);

        return ResponseEntity.ok(ApiResponse.success("Chatbot response generated.", replyData));
    }
}
