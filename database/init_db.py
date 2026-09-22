"""
Pricilla Mart - Database Initialization Utility Script
Executes schema.sql and seed.sql to create database/pricillamart.db
"""
import os
import sqlite3

def init_database():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    db_path = os.path.join(base_dir, "pricillamart.db")
    schema_path = os.path.join(base_dir, "schema.sql")
    seed_path = os.path.join(base_dir, "seed.sql")

    print(f"[*] Initializing database at: {db_path}")

    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Enable foreign keys
    cursor.execute("PRAGMA foreign_keys = ON;")

    # Apply Schema
    if os.path.exists(schema_path):
        print(f"[*] Applying schema from: {schema_path}")
        with open(schema_path, "r", encoding="utf-8") as f:
            cursor.executescript(f.read())
    else:
        print(f"[!] Warning: Schema file not found at {schema_path}")

    # Apply Seed Data
    if os.path.exists(seed_path):
        print(f"[*] Seeding data from: {seed_path}")
        with open(seed_path, "r", encoding="utf-8") as f:
            cursor.executescript(f.read())
    else:
        print(f"[!] Warning: Seed file not found at {seed_path}")

    conn.commit()
    conn.close()
    print("[+] Database successfully initialized with sample data!")

if __name__ == "__main__":
    init_database()
