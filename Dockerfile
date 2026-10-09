FROM python:3.11-slim

WORKDIR /app

# Copy application files
COPY . /app

# Default environment configuration
ENV PORT=8080
EXPOSE 8080

CMD ["python", "dev_server.py"]
