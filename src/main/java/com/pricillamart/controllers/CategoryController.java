package com.pricillamart.controllers;

import com.pricillamart.models.Category;
import com.pricillamart.services.DatabaseService;
import com.pricillamart.utils.ApiResponse;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api")
@CrossOrigin(origins = "*")
public class CategoryController {

    @Autowired
    private DatabaseService databaseService;

    @GetMapping("/categories")
    public ResponseEntity<ApiResponse<List<Category>>> getCategories() {
        List<Category> categories = databaseService.getAllCategories();
        return ResponseEntity.ok(ApiResponse.success("Categories fetched successfully.", categories));
    }
}
