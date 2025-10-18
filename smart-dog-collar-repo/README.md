# Smart Dog Collar

Real time health and location monitoring for dogs using an ESP32 based collar, cellular or WiFi connectivity, Firebase Realtime Database, and a live web dashboard.

## Highlights
Continuous heart rate, temperature, and activity monitoring
Live GPS location on map
SOS button and instant email alerts
Remote buzzer control from the dashboard
Simple web dashboard with Firebase sync

## Repository structure
firmware
Source code for the ESP32 collar. Upload to your board with Arduino IDE

web
Dashboard files for live monitoring. Open in a browser or host on GitHub Pages

backend
A small Python service that sends email alerts when SOS is active

cloud
Example database structure and permissive rules for quick prototyping

hardware
Bill of materials and notes

docs
Project report, slides, functional requirements, and research paper draft

## Hardware checklist
ESP32 development board
DS18B20 temperature sensor
Pulse sensor or MAX30102 PPG module
ADXL345 or MPU6050 accelerometer
SIMCom A7672S or SIM800L GSM or GNSS module
SOS push button
Passive buzzer
3 x 18650 Li ion cells with 3S BMS and charger
MP1584EN buck regulator

## Firmware Setup
Install Arduino IDE
Install ESP32 board support
Open firmware/SmartDogCollar.ino
Adjust pin definitions to your wiring
Upload to ESP32

Sensor libraries
OneWire for DS18B20
Wire for I2C sensors
Any vendor libraries you prefer for MAX30102 or MPU6050

## Cloud Setup
Create a Firebase Realtime Database
Copy your database URL
For quick testing keep rules open as in cloud/firebase.rules.json
Use cloud/database_structure.sample.json to seed initial nodes

Realtime paths
collar
Temp, BPM, Status, sos, latitude, longitude

Buzzer
on

## Web Dashboard Setup
The dashboard reads and writes from Firebase directly
Edit web/dashboard/index.html and set your Firebase config if needed
You can host with GitHub Pages
Create a new repo and push
Enable Pages in settings and choose the main branch root

## Email Alert Service
The backend monitors the collar node
When sos is 1 it sends an email with a Google Maps link
Steps
Copy .env.example to .env and fill values
Place your Firebase service account JSON at cloud/pvtkey.json
Install dependencies
pip install flask firebase_admin python-dotenv
Start the service
python backend/email_service.py

## Run the whole system
Power the collar and wait for cellular or WiFi connection
The ESP32 posts vitals and location to Firebase
Open the dashboard to see live data and control the buzzer
Press SOS on the collar to test alerts

## Results
You can include screenshots of the dashboard and hardware photos in images
Add field measurements and logs if required by your evaluation

## Notes on credentials
The example dashboard includes a public Firebase config suitable for prototyping
Use restrictive Firebase rules for any real deployment
Never commit private keys or service account JSON

## License
MIT License
