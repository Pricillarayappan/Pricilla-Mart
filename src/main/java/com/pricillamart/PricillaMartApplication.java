package com.pricillamart;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class PricillaMartApplication {

    public static void main(String[] args) {
        System.out.println("============================================================");
        System.out.println("                     PRICILLA MART                          ");
        System.out.println("          FULL-STACK JAVA SPRING BOOT APPLICATION           ");
        System.out.println("============================================================");
        System.out.println("[System] Starting Spring Boot embedded server on port 8080...");

        SpringApplication.run(PricillaMartApplication.class, args);

        System.out.println("------------------------------------------------------------");
        System.out.println("[Server] Pricilla Mart is LIVE at: http://localhost:8080");
        System.out.println("DEFAULT DEMO ACCOUNTS:");
        System.out.println("  Admin:  admin@pricillamart.com     | Password: Admin@123");
        System.out.println("  Seller: techworld@pricillamart.com | Password: Seller@123");
        System.out.println("  Buyer:  rahul@gmail.com            | Password: Buyer@123");
        System.out.println("============================================================");
    }
}
