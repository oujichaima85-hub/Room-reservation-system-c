# 🏢 Room Reservation Management System

A robust console-based application developed in C for managing coworking spaces and meeting rooms.  
The system handles reservations, billing, availability verification and performance statistics.

---

## ✨ Features

### 📋 Reservation Management
- Availability verification with anti-overlap algorithm
- Room capacity validation
- Reservation lifecycle management:
  - CONFIRMED
  - CANCELLED
  - INVOICED
- Automatic data persistence using text files

### 💰 Billing & Pricing
- Dynamic pricing based on room hourly rates
- Automatic invoice generation:
  - `FACTURE_<ID>.txt`

### 📊 Statistics & Analytics
- Revenue calculation per room
- Top 3 most reserved rooms
- Monthly reservation statistics

---

## 📂 Data Files

### `config_salles.txt`
Defines room configuration.

Format:
```txt
name;capacity;hourly_rate;equipment
```

### `reservations.txt`
Stores reservation history.

Format:
```txt
id;client_name;room_name;date;start_time;end_time;persons;rate;total;status
```

---

## 🛠 Technologies
- C
- File Handling
- Algorithms
- Data Structures

---

## 🚀 Compilation & Execution

```bash
gcc main.c -o SalleManager
./SalleManager
```

---

## 🎯 Project Objectives
- Practice structured programming in C
- Implement file persistence
- Develop reservation management algorithms
- Improve problem-solving and system design skills
- 
