# Embedded Software & Hardware Requirements Specification (SRS)  
## Climate Surveillance System Prototype  

---

## 1. Introduction  

### 1.1 Purpose  
This document specifies the requirements for the embedded part of a prototype **climate surveillance system** designed to monitor temperature and humidity during climate-controlled transport. The system consists of **sensor packages** deployed with goods, **a logging device** and a **communication backend**, **user interfaces** for **senders, transporters, receivers, and logistics administrators**.  

### 1.2 Scope  
The prototype aims to demonstrate the feasibility of monitoring and transmitting environmental data from transported goods. It will:  
- Collect climate data (temperature, humidity) via distributed sensor packages.  
- Transmit measurements via **Arduino R4 WiFi** modules to a central **ESP32-S3 Zero server**.  
- Log sensor data locally on the ESP32-S3’s flash memory.  
- Forward logged data to a remote database endpoint in **YAML** format.  

Out of scope:  
- Provide basic views for different user roles, including data visualization and alerts.  
- Full sender registration, commercial transport ordering, and production-grade logistics integration.  
- Complete security hardening and large-scale database optimization.  

### 1.3 Definitions, Acronyms, and Abbreviations  
- **Sensor Package**: A unit containing sensors and an Arduino R4 WiFi.  
- **Server Node**: ESP32-S3 Zero device acting as local collector and forwarder.  
- **YAML**: Human-readable data serialization format (“YAML Ain’t Markup Language”).  

### 1.4 References  
-   Arduino Docs: Arduino Language Reference
- Arduino UNO R4 WiFi datasheet.  
- Espressif ESP32-S3 technical reference manual.  

---

## 2. Overall Description  

### 2.1 Product Perspective  
The system is a distributed IoT solution. Each **sensor package** collects local climate data and transmits it wirelessly to a **server node**, which aggregates and forwards the data to a remote database for monitoring and visualization.  

### 2.2 Product Functions  
- Measure temperature and humidity.  
- Periodically transmit data via WiFi.  
- Log data locally on ESP32-S3 flash memory.  
- Forward data to remote database endpoint in YAML format.  ruck, customer, or time period.  

### 2.3 Operating Environment  
- Hardware: Arduino R4 WiFi, ESP32-S3 Zero, DS18B20, DHT11
- Connectivity: WiFi 2.4 GHz.  
- Data format: YAML.  
- Remote endpoint: REST API endpoint (database-backed). 


``` yaml
192.168.1.100:12345
Sensor_data:
  temp: 25
  hum: 80
```

``` http
GET /packages/{id}
GET /packages/{id}/history
POST /packages/{id}/scan
```

### 2.4 Design and Implementation Constraints  
- Limited flash and RAM on microcontrollers.  
- Prototype only, not optimized for mass deployment.  
- Minimal energy optimization for demonstration purposes.  
- Sensor accuracy is not very precise

### 2.5 Assumptions and Dependencies  
- WiFi connectivity available during transport.  
- Remote database endpoint reachable.  
- Sensor accuracy within ±0.5 °C (temperature), ±3% RH (humidity).  

---

## 3. System Features  

### 3.1 Sensor Package  
**Description**: Collects and transmits temperature and humidity data.  

**Inputs**: Temperature and humidity sensors.  
**Outputs**: WiFi-transmitted YAML packets.  

**Functional Requirements**:  
- **(discuss)FR-1: Sensor package shall measure temperature and humidity every 60 seconds**  
- FR-2: Sensor package shall transmit data in YAML format to the server.  
- FR-3: Sensor package shall retry transmission up to 3 times in case of failure.  

### 3.2 Server (ESP32-S3)  
**Description**: Collects, logs, and forwards sensor data.  

**Functional Requirements**:  
- FR-4: Server shall receive data from multiple sensor packages.  
- FR-5: Server shall store all sensor data, on onboard flash, in YAML format, upon failure to connect to the database.  
- **(discuss)FR-6: Server shall forward stored data to a remote database endpoint every 60 seconds.**  
- FR-7: Server shall acknowledge successful receipt to sensor packages.  

---

## 4. External Interface Requirements  

### 4.1 Hardware Interfaces  
- Arduino R4 WiFi GPIO for temperature/humidity sensors.  

### 4.2 Software Interfaces  
- **(discuss)REST API endpoint for database communication.**  

### 4.3 Communication Interfaces  
- WiFi IEEE 802.11 b/g/n.  
- **(discuss)HTTP(S) transport for database communication.**  

---

## 5. Data Requirements  

### 5.1 Data Format (YAML)  
Server data packet:  

```yaml
sensor_packages:
  - package_id: 100
    sensors:
      temperature_c: 4.5
      humidity_percent: 72.1
    battery: 95
  - package_id: 99
    sensors:
      temperature_c: 4.6
      humidity_percent: 79.1
    battery: 82
```

 
Sensor package data packet:  

```yaml
package_id: 100
sensors:
  temperature_c: 4.5
  humidity_percent: 72.1
battery: 95
```

### 5.2 Data retention
- Data shall be retained on ESP32-S3 flash until successfully transmitted.

---

## 6. Non-Functional Requirements
- Reliability: Data is never lost, even if transmission temporarily fails.
- **(discuss)Performance: Sensor package measurement and transmission-to-database cycle ≤ 60 seconds.**
- Security: Basic transport encryption (HTTPS).
- Usability: Normal function without setup after loss of power
- Scalability: Prototype supports up to 100 sensor packages per server.

---

## 7. Prototype Limitations
- No redundancy mechanisms.