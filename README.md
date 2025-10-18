# 🐾 Smart Dog Collar

An IoT-based smart collar designed for real-time monitoring of a dog’s **health, activity, and location** using ESP32, GSM/GPS, and Firebase.  
It enables **24/7 tracking**, **SOS alerts**, and **cloud-based dashboards** for complete pet safety and wellness.

---

## 💡 Features
- **Health Monitoring:** Tracks temperature, heart rate, and activity levels (resting, walking, running)  
- **GPS Tracking:** Provides real-time location using GSM/GNSS  
- **Cloud Connectivity:** Syncs data to Firebase in real time  
- **Emergency Alerts:** Sends email notifications via Flask + SMTP when abnormal readings or SOS events occur  
- **Web Dashboard:** Interactive interface with maps and live vitals  
- **Remote Buzzer:** Enables owners to locate pets through a controlled buzzer  

---

## 🧠 System Architecture
1. Sensors collect body temperature, heart rate, and motion data  
2. ESP32 processes and transmits data to Firebase using Wi-Fi or GSM  
3. Cloud backend triggers alerts through Flask (Python) and SMTP  
4. Owner accesses live data through a responsive web dashboard  

---

## 🧰 Hardware Components
| Component | Purpose |
|------------|----------|
| ESP32 | Main controller |
| DS18B20 | Temperature sensor |
| MAX30102 / Pulse Sensor | Heart rate monitoring |
| ADXL345 / MPU6050 | Activity recognition |
| SIM800L / SIMCom A7672S | GSM + GPS communication |
| SOS Button | Emergency alerts |
| Buzzer | Remote recall alert |
| 3S Li-ion Battery Pack | Power supply |
| MP1584EN Regulator | Voltage regulation |

---

## ⚙️ Software Stack
- **Microcontroller:** ESP32 (Arduino IDE, C++)  
- **Backend:** Firebase Realtime Database + Flask (Python)  
- **Frontend:** HTML, CSS, JavaScript, Leaflet.js (map)  
- **Cloud Services:** Firebase + PythonAnywhere  
- **Email Notifications:** SMTP (Gmail App Password)  

---

## 🧩 Folder Structure

# 🐾 Smart Dog Collar

An IoT-based smart collar designed for real-time monitoring of a dog’s **health, activity, and location** using ESP32, GSM/GPS, and Firebase.  
It enables **24/7 tracking**, **SOS alerts**, and **cloud-based dashboards** for complete pet safety and wellness.

---

## 💡 Features
- **Health Monitoring:** Tracks temperature, heart rate, and activity levels (resting, walking, running)  
- **GPS Tracking:** Provides real-time location using GSM/GNSS  
- **Cloud Connectivity:** Syncs data to Firebase in real time  
- **Emergency Alerts:** Sends email notifications via Flask + SMTP when abnormal readings or SOS events occur  
- **Web Dashboard:** Interactive interface with maps and live vitals  
- **Remote Buzzer:** Enables owners to locate pets through a controlled buzzer  

---

## 🧠 System Architecture
![System Architecture](images/architecture.jpg)
*Data flow from sensors → ESP32 → Firebase → Dashboard and email alert.*

---

## ⚙️ Circuit Connection Diagram
![Circuit Diagram](images/circuit.jpg)
*Complete wiring showing ESP32, GSM/GPS module, sensors, and power setup.*

---

## 🔧 Hardware Prototype
### Version 1
![Hardware Prototype 1](images/hardware1.jpg)
### Version 2
![Hardware Prototype 2](images/hardware2.jpg)
*Working prototype of Smart Dog Collar assembled on perfboard.*

---

## 💻 Dashboard Interface
![Dashboard Screenshot](images/dashboard.jpg)
*Live vitals and real-time tracking interface built using HTML, JS, and Firebase.*

---

## 🧩 Implementation Screens
![Implementation Overview](images/implementation.jpg)
*Live vitals + remote control and real-time map preview.*

---

## 🚨 Alert Demo
![Alert Email Demo](images/alert-demo.jpg)
*Automatic SOS email sent to the owner with GPS coordinates.*

---

## 🗺️ Navigation Demo
![Navigation Demo](images/navigation-demo.jpg)
*One-click navigation link opening Google Maps for live pet location.*

---

## 🧰 Hardware Components
| Component | Purpose |
|------------|----------|
| ESP32 | Main controller |
| DS18B20 | Temperature sensor |
| MAX30102 / Pulse Sensor | Heart rate monitoring |
| ADXL345 / MPU6050 | Activity recognition |
| SIM800L / SIMCom A7672S | GSM + GPS communication |
| SOS Button | Emergency alerts |
| Buzzer | Remote recall alert |
| 3S Li-ion Battery Pack | Power supply |
| MP1584EN Regulator | Voltage regulation |

---

## ⚙️ Software Stack
- **Microcontroller:** ESP32 (Arduino IDE, C++)  
- **Backend:** Firebase Realtime Database + Flask (Python)  
- **Frontend:** HTML, CSS, JavaScript, Leaflet.js (map)  
- **Cloud Services:** Firebase + PythonAnywhere  
- **Email Notifications:** SMTP (Gmail App Password)  

---

## 🚀 Getting Started

### 1. Firmware Setup
- Install **Arduino IDE**  
- Add ESP32 board package  
- Open `firmware/SmartDogCollar.ino`  
- Connect your ESP32 and upload the sketch  

### 2. Firebase Setup
- Create a Firebase Realtime Database  
- Update database URL in your firmware and backend  
- Import `cloud/database_structure.sample.json` as initial data  

### 3. Backend (Email Alert Service)
```bash
pip install flask firebase_admin python-dotenv
python backend/email_service.py

