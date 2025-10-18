# Email alert microservice
# Monitors Firebase Realtime Database and sends SOS email notifications
# Environment: Python 3.x
# Hosting suggestion: PythonAnywhere or any VPS
#
# Fill in credentials in .env and place Firebase service account JSON in cloud/pvtkey.json

import os
import time
import smtplib
from email.mime.text import MIMEText
from dotenv import load_dotenv
from flask import Flask
import threading

import firebase_admin
from firebase_admin import credentials, db

load_dotenv()

DB_URL = os.getenv("FIREBASE_DB_URL", "https://dog-collar-c02a2-default-rtdb.firebaseio.com")
SERVICE_ACCOUNT = os.getenv("FIREBASE_SA_JSON", "cloud/pvtkey.json")

SMTP_SERVER = os.getenv("SMTP_SERVER", "smtp.gmail.com")
SMTP_PORT = int(os.getenv("SMTP_PORT", "587"))
SENDER_EMAIL = os.getenv("SENDER_EMAIL", "youremail@example.com")
APP_PASSWORD = os.getenv("APP_PASSWORD", "your_app_password")
RECIPIENTS = [e.strip() for e in os.getenv("RECIPIENTS", "owner@example.com").split(",")]

ALERT_COOLDOWN_SECS = int(os.getenv("ALERT_COOLDOWN_SECS", "10"))

cred = credentials.Certificate(SERVICE_ACCOUNT)
firebase_admin.initialize_app(cred, {"databaseURL": DB_URL})

app = Flask(__name__)
last_alert_epoch = 0

def send_email(lat, lon):
    subject = "Emergency Alert - Pet Collar"
    body = f"Emergency button pressed.\nLocation: https://www.google.com/maps?q={lat},{lon}"
    msg = MIMEText(body)
    msg["Subject"] = subject
    msg["From"] = SENDER_EMAIL
    msg["To"] = ", ".join(RECIPIENTS)

    with smtplib.SMTP(SMTP_SERVER, SMTP_PORT) as server:
        server.starttls()
        server.login(SENDER_EMAIL, APP_PASSWORD)
        server.sendmail(SENDER_EMAIL, RECIPIENTS, msg.as_string())

def monitor_firebase():
    global last_alert_epoch
    ref = db.reference("/collar")
    while True:
        try:
            data = ref.get()
            if data and data.get("sos", 0) == 1:
                now = time.time()
                if now - last_alert_epoch > ALERT_COOLDOWN_SECS:
                    lat = data.get("latitude", 0)
                    lon = data.get("longitude", 0)
                    send_email(lat, lon)
                    last_alert_epoch = now
            time.sleep(2)
        except Exception as e:
            print("Firebase read error:", e)
            time.sleep(5)

@app.route("/")
def home():
    return "Pet Collar Alert Service"

if __name__ == "__main__":
    threading.Thread(target=monitor_firebase, daemon=True).start()
    app.run(host="0.0.0.0", port=int(os.getenv("PORT", "5000")))
