# ============================================================
# PRICILLA MART - JAVA FULL-STACK PRODUCTION DOCKERFILE
# Multi-stage build using Eclipse Temurin OpenJDK 17 & Maven
# ============================================================

# Stage 1: Build Java Spring Boot Application with Maven
FROM maven:3.9.6-eclipse-temurin-17 AS build
WORKDIR /app

# Copy Maven Project Definition & Source Code
COPY pom.xml .
COPY src ./src

# Build production Spring Boot executable JAR (skip tests for quick build)
RUN mvn clean package -DskipTests

# Stage 2: Production JRE 17 Runtime
FROM eclipse-temurin:17-jre-jammy
WORKDIR /app

# Copy compiled JAR from build stage
COPY --from=build /app/target/*.jar app.jar

# Copy Frontend static assets and SQLite database
COPY frontend ./frontend
COPY database ./database

# Expose cloud dynamic port
ENV PORT=8080
EXPOSE 8080

# Launch Java Spring Boot Server
CMD ["sh", "-c", "java -Dserver.port=${PORT} -jar app.jar"]
