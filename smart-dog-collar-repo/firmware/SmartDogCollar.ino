#include <HardwareSerial.h>
#include <ArduinoJson.h>

static TaskHandle_t xTask1 = NULL, xTask2 = NULL, xTask3 = NULL;

volatile int gsmErrorFlag = 0;

unsigned long lastStatusUpdateMillis = 0;
bool gsmBusy = false;

HardwareSerial GSMSerial(2);

//Global variables
volatile float g_temp = 0.0;
volatile int g_bpm, g_sos = 0 , g_buzzer = 0;
String g_activity = "NULL";

//BPM
int PulseSensorPurplePin = 15;     // Pulse Sensor connected to analog pin
int LED = LED_BUILTIN;             // On-board LED will blink with heartbeat

int Signal;                        // Raw signal value
int Threshold = 2000;               // Threshold to detect beats (adjust as needed)

unsigned long lastBeatTime = 0;    // Time when last beat was detected
int BPM = 0;                       // Calculated Beats Per Minute
int IBI = 0;                       // Time between beats (Inter-Beat Interval)

// For smoothing BPM
#define MAX_HISTORY 10
int beatHistory[MAX_HISTORY];
int beatIndex = 0;
//

//Button
const int interruptPin = 26;

unsigned long sos_start_time = 0;  // store start time
const unsigned long sos_duration = 5000; // 5 seconds in ms

void IRAM_ATTR handleInterrupt() {

  if (digitalRead(interruptPin) == HIGH) {
    Serial.println("Button Pressed");
  } else {
    Serial.println("Button Released");
  }
  g_sos = 1;


}
//

//Buzzer
#define BUZZER_PIN 32

unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
//


//ADXL
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>


Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

//

//Temp

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 13  // Use GPIO number, not Dx

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//


int sendATCommandAndWait(const char* cmd, const char* expectedResponse, unsigned long timeout = 10000) {
  GSMSerial.println(cmd);

  unsigned long startMillis = millis();
  char line[128] = {0};
  int idx = 0;

  while (millis() - startMillis < timeout) {
    while (GSMSerial.available()) {
      char c = GSMSerial.read();
      if (c == '\r' || c == '\n') {
        if (idx > 0) {
          line[idx] = '\0';  // null-terminate
          Serial.println(line);
          if (strstr(line, expectedResponse)) {
            vTaskDelay(250 / portTICK_PERIOD_MS);
            return 1;
          }
          if (strstr(line, "ERROR")) {
            Serial.printf("GSM ERROR detected for command: %s\n", cmd);


            vTaskDelay(250 / portTICK_PERIOD_MS);
            gsmErrorFlag = 1;
            return 0;
          }
          idx = 0;
          memset(line, 0, sizeof(line));
        }
      } else if (idx < sizeof(line) - 1) {
        line[idx++] = c;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  Serial.printf("GSM response timeout for command: %s\n", cmd);
  if (strstr(cmd, "AT+HTTPACTION=4"))
  {
    gsmErrorFlag = 1;
  }


  return -1;
}

int initializeGSM() {

  if (sendATCommandAndWait("AT", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CPIN?", "+CPIN: READY") != 1) return 0;
  if (sendATCommandAndWait("AT+CSQ", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CREG=1", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CREG?", "+CREG: 1") != 1) return 0;
  if (sendATCommandAndWait("AT+CGDCONT=1,\"IP\",\"airtelgprs.com\"", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CNMP=2", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CGREG=1", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CGACT=1,1", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CGPADDR=1", "+CGPADDR:") != 1) return 0;
  if (sendATCommandAndWait("AT+CMEE=2", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CSSLCFG=\"sslversion\",0,4", "OK") != 1) return 0;

  return 1;


}

int initializeGNSS() {
  if (sendATCommandAndWait("AT+CGNSSPWR=0", "OK") != 1)return 0;
  if (sendATCommandAndWait("AT+CGNSSPWR=1", "READY", 20000) != 1) return 0;
  if (sendATCommandAndWait("AT+CGNSSPORTSWITCH=1,1", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CAGPS", "OK") != 1) return 0;
  if (sendATCommandAndWait("AT+CGNSSINFO", "+CGNSSINFO:") != 1)return 0;

  return 1;
}

void prvTask1(void *pvParameters)
{
  for (;;)
  {
  Signal = analogRead(PulseSensorPurplePin);   // Read sensor value
//  Serial.print("Signal: ");
//  Serial.print(Signal);

  unsigned long currentTime = millis();

  // Beat detection
  if (Signal > Threshold && (currentTime - lastBeatTime) > 500) { 
    // 300ms debounce (max 200 BPM)
    IBI = currentTime - lastBeatTime;
    lastBeatTime = currentTime;

    if (IBI > 0) {
      int currentBPM = 60000 / IBI; // BPM = 60,000ms / IBI
      beatHistory[beatIndex] = currentBPM;
      beatIndex = (beatIndex + 1) % MAX_HISTORY;

      // Average BPM smoothing
      long sum = 0;
      for (int i = 0; i < MAX_HISTORY; i++) {
        sum += beatHistory[i];
      }
      BPM = sum / MAX_HISTORY;

      digitalWrite(LED, HIGH);
    }
  } else {
    digitalWrite(LED, LOW);
  }

//  Serial.print("   BPM: ");
//  Serial.println(BPM);
  g_bpm=BPM;

  vTaskDelay(20 / portTICK_PERIOD_MS);

  }
}

void prvTask2(void *pvParameters)
{
  for (;;)
  {
     sensors_event_t event;
  accel.getEvent(&event);

  // Calculate acceleration magnitude (ignores direction)
  float magnitude = sqrt(
    event.acceleration.x * event.acceleration.x +
    event.acceleration.y * event.acceleration.y +
    event.acceleration.z * event.acceleration.z
  );

  // Classify activity (very simple thresholds)
  String activity;
  if (magnitude < 13.5) {
    activity = "Resting";
  } else if (magnitude < 18.5) {
    activity = "Walking";
  } else {
    activity = "Running";
  }

//  Serial.print("Activity: ");
//  Serial.print(activity);
//  Serial.print(" | Magnitude: ");
//  Serial.println(magnitude);
g_activity=activity;

  vTaskDelay(500 / portTICK_PERIOD_MS);

  }
}

void prvTask3(void *pvParameters)
{
  for (;;)
  {

//      Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
//  Serial.println("DONE");
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  float tempC = sensors.getTempCByIndex(0);
  if (tempC != DEVICE_DISCONNECTED_C)
  {
//    Serial.print("Temperature for the device 1 (index 0) is: ");
//    Serial.println(tempC);
    g_temp=tempC;
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }

  }
}


void setup()
{
  Serial.begin(115200);

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);

  pinMode(LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize ADXL
  if (!accel.begin()) {
    Serial.println("No ADXL345 detected. Check connections!");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_2_G); // ±2g for high resolution


  sensors.begin();


  

  xTaskCreate(prvTask1, "Task1", 4096, NULL, 2, &xTask1);
  xTaskCreate(prvTask2, "Task2", 4096, NULL, 1, &xTask2);
  xTaskCreate(prvTask3, "Task3", 4096, NULL, 1, &xTask3);


  GSMSerial.begin(115200, SERIAL_8N1, 16, 17);


  delay(10000);

  int result, result2;
  while (1) {
    result = initializeGSM();
    result2 = initializeGNSS();
    if (result && result2 ) {
      Serial.println("GSM initialized successfully");
      break;
    } else {
      Serial.println("GSM init failed. Retrying...");
      delay(5000);  // Wait before retrying
    }
  }





  Serial.println("Setup complete");







}

void loop()
{


  if (gsmErrorFlag) {
    gsmErrorFlag = 0;  // Clear it before retrying

    int retryCount = 0;
    const int maxRetries = 5;

    while (retryCount < maxRetries) {
      if (initializeGSM() && initializeGNSS()) {
        Serial.println("GSM & GNSS reinitialized after error");
        break;
      }
      retryCount++;
      Serial.println("Retrying GSM/GNSS init... (" + String(retryCount) + "/" + String(maxRetries) + ")");
      delay(3000);
    }

    if (retryCount == maxRetries) {
      Serial.println("GSM recovery failed. Restarting device...");
      sendATCommandAndWait("AT+CRESET", "OK");
      ESP.restart();
    }
  }




  // 1. Request GPS Info
  sendATCommandAndWait("AT+CGNSSPWR=1", "OK", 1000);
  GSMSerial.println("AT+CGNSSINFO");
  Serial.println("Requesting GNSS info...");

  char gnssData[128] = {0};
  int idx = 0;
  unsigned long start = millis();

  while (millis() - start < 2000 && idx < sizeof(gnssData) - 1) {
    if (GSMSerial.available()) {
      char c = GSMSerial.read();
      if (c == '\n') {
        gnssData[idx] = '\0';
        if (strstr(gnssData, "+CGNSSINFO:")) {
          break;
        }
        idx = 0;
      } else {
        gnssData[idx++] = c;
      }
    }
  }


  Serial.print("GNSS Raw: ");
  Serial.println(gnssData);

  if (strstr(gnssData, "ERROR") || strlen(gnssData) == 0) {
    initializeGNSS();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  // Default coordinates
  float latitude = 0.0;
  float longitude = 0.0;
  bool validGNSS = false;

  String gnssStr(gnssData);
  if (gnssStr.length() > 0) {
    int nIndex = gnssStr.indexOf(",N,");
    int eIndex = gnssStr.indexOf(",E,");

    if (nIndex > 10 && eIndex > 10) {
      String latStr = gnssStr.substring(nIndex - 10, nIndex);
      String lonStr = gnssStr.substring(eIndex - 10, eIndex);

      Serial.println("Parsed Latitude: " + latStr);
      Serial.println("Parsed Longitude: " + lonStr);

      latitude = latStr.toFloat();
      longitude = lonStr.toFloat();

      if (latitude != 0.0 && longitude != 0.0) {
        validGNSS = true;
        Serial.println("Parsed Latitude: " + latStr);
        Serial.println("Parsed Longitude: " + lonStr);
      }
    }
  }
  // Fallback if GNSS parsing failed
  if (!validGNSS) {
    Serial.println("GNSS invalid, falling back to CLBS...");
    GSMSerial.println("AT+CLBS=1,1");

    unsigned long clbsStart = millis();
    while (millis() - clbsStart < 20000) {
      if (GSMSerial.available()) {
        String line = GSMSerial.readStringUntil('\n');
        line.trim();
        Serial.println("CLBS Response: " + line);
        if (line.startsWith("+CLBS:")) {
          int firstComma = line.indexOf(',');
          int secondComma = line.indexOf(',', firstComma + 1);
          int thirdComma = line.indexOf(',', secondComma + 1);

          if (firstComma > 0 && secondComma > firstComma && thirdComma > secondComma) {
            String latStr = line.substring(firstComma + 1, secondComma);
            String lonStr = line.substring(secondComma + 1, thirdComma);
            latitude = latStr.toFloat();
            longitude = lonStr.toFloat();

            Serial.println("Fallback Latitude: " + latStr);
            Serial.println("Fallback Longitude: " + lonStr);
          }
          break;
        }
      }
    }
  }


  // 3. Build JSON payload
  char payload[256];
  snprintf(payload, sizeof(payload),
           "{\"Temp\":%.2f,\"BPM\":%2d,\"Status\":\"%s\",\"sos\":%d,\"latitude\": %.6f,\"longitude\": %.6f}",

           g_temp, g_bpm, g_activity, g_sos, latitude, longitude);

  Serial.println("Payload to POST:");
  Serial.println(payload);

  // 4. Send HTTP POST
  sendATCommandAndWait("AT+HTTPINIT", "OK");
  sendATCommandAndWait("AT+HTTPPARA=\"URL\",\"https://dog-collar-c02a2-default-rtdb.firebaseio.com/collar.json\"", "OK");
  sendATCommandAndWait("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK");
  String httpDataCmd = "AT+HTTPDATA=" + String(strlen(payload)) + ",10000";
  sendATCommandAndWait(httpDataCmd.c_str(), "DOWNLOAD");
  vTaskDelay(100 / portTICK_PERIOD_MS);
  sendATCommandAndWait(payload, "OK");

  sendATCommandAndWait("AT+HTTPACTION=4", "+HTTPACTION: 4,200", 20000);

  if (g_sos == 1 && sos_start_time == 0) {
    sos_start_time = millis();  // record the start time
  }

  // Check if 5 seconds have passed
  if (g_sos == 1 && (millis() - sos_start_time >= sos_duration)) {
    g_sos = 0;
    sos_start_time = 0; // reset for next trigger
  }



  sendATCommandAndWait("AT+HTTPTERM", "OK");


  vTaskDelay(100 / portTICK_PERIOD_MS);; // Small delay to reduce CPU usage


  sendATCommandAndWait("AT+HTTPINIT", "OK");
  sendATCommandAndWait("AT+HTTPPARA=\"URL\",\"https://dog-collar-c02a2-default-rtdb.firebaseio.com/Buzzer.json\"", "OK");
  sendATCommandAndWait("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK");
  sendATCommandAndWait("AT+HTTPACTION=0", "+HTTPACTION: 0,200", 20000);
  GSMSerial.println("AT+HTTPREAD=0,50"); // Adjust size as needed

  unsigned long httpStart = millis();
  while (millis() - httpStart < 5000) {
    if (GSMSerial.available()) {
      String line = GSMSerial.readStringUntil('\n');
      line.trim();
      Serial.println("HTTPREAD Line: " + line);

      if (line.startsWith("{")) { // JSON payload line
        int index = line.indexOf("\"on\":");
        if (index >= 0) {
          int on_value = line.substring(index + 5).toInt();
          g_buzzer = on_value;  // Store value
          Serial.println("Parsed value: " + String(g_buzzer));
        }

      }
    }
  }

  if (g_buzzer == 1 && !buzzerActive) {
    digitalWrite(BUZZER_PIN, HIGH);       // Turn ON buzzer
    buzzerStartTime = millis();           // Start timer
    buzzerActive = true;
  }

  // Check if 5 seconds passed
  if (buzzerActive && millis() - buzzerStartTime >= 5000) {
    digitalWrite(BUZZER_PIN, LOW);        // Turn OFF buzzer
    buzzerActive = false;
    g_buzzer = 0;                         // Reset variable
  }

  sendATCommandAndWait("AT+HTTPTERM", "OK");

}
