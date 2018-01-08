#include <Ultrasonic.h>
#include <DHT.h>
#include <Wire.h>
#include <Servo.h>
#include <BH1750.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Fingerprint.h>
#include <WiFiEsp.h>


#define SONAR_MEASUREMENT   250
#define SENSORS_MEASUREMENT 1000
#define SERENA_TIME      500
#define DAY              60000 //30000 //
#define MAX_DISTANCE     60
#define POSITION_CLOSED  170
#define POSITION_OPENED  110
#define MINLUX           0
#define MAXLUX           15000
#define NUM_LEDS1        8
#define NUM_LEDS2        8
#define BRIGHTNESS       50
#define MICRO_TTHRESHOLD 300
#define TIME_EIHGT_AM    480 //10000//
#define TIME_EIHGT_PM    1200//20000//
#define TIME_MORG        100
#define WAIT_TIME        300
#define BUZZ_TIME        500

#define LED1_PIN        4
#define LED2_PIN        11
#define TRIGGER_PIN     12
#define ECHO_PIN        13
#define REED_SWITCH_PIN 5
#define LASER_PIN       7
#define DHTPIN          3
#define RGB_PIN         4
#define SERVO_PIN       10
#define BUZZER_PIN      6
#define MICRO_PIN       1
#define RED_LED         8
#define GREEN_LED       9




char ssid[] = "NSK";     // your network SSID (name)
char pass[] = "qwerty123";  // your network password

bool debug = true;
bool mode = false;

int status = WL_IDLE_STATUS;     // the Wifi radio's status
bool k1 = 0, k2 = 0, k3 = 0, k4 = 0, k5 = 0;
char server[] = "xn--54-6kcaak0bhc9av1cd3dyh.xn--p1ai";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 300L;
// Initialize the Wifi client library
WiFiEspClient client;

bool alarmState = false;
bool serenaState = false;
bool redLightState = false;
bool justAlarmChanged = false;
bool light2Change = false;
bool light1Change = false;
bool jaluziChange = false;
bool isBlue = true;
bool rSwitch;
bool laser;
bool us;
bool isTimer = false;
bool jaluziState = false;
bool light1State = false;
bool light2State = false;
bool voiceState = false;
bool buzz_once = false;
bool jalJust = false;
bool justChanged = false;

float t = 0;
float h = 0;
uint32_t lightness = 0;
uint8_t rangeDistance = 0;
uint8_t SonarAlarmDistance = 0;
uint16_t micro = 0;

unsigned long wait_time = 0;
unsigned long lastSonarRead = millis();
unsigned long lastMeasurment = millis();
unsigned long lastTime = millis();
unsigned long lastSerena = millis();
unsigned long Time = 0;
unsigned long lastMorg1 = millis();
unsigned long lastForTimer = millis();
unsigned long lastBuzz = millis();
unsigned long time0 = 0;
float timeNow;
bool time_last = false;
DHT dht(DHTPIN, DHT22);
Servo myservo;
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_LEDS1, LED1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_LEDS2, LED2_PIN, NEO_GRB + NEO_KHZ800);
//NewPing sonar(TRIGGER_PIN, ECHO_PIN, 100);
Ultrasonic sonar(TRIGGER_PIN, ECHO_PIN);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);
BH1750 lightMeter;


void write();
int getFingerprintIDez();
void readSensors();
void lifeSycleNight();
void lifeSycleNight();
void lifeSyclesAlarm(); 
void lifeSycleDay();
void lightClear(int n);
void SetFlashOn1Light();
void setJaluzi(bool state);
void transferWithServer();
void setLEDColor1(int r, int g, int b);
void setLEDColor2(int r, int g, int b);
void send_sensors();
void send_buttons();
void read_button();
void printWifiStatus();
void wifi_init();
void read_serial();

void setup() {
  time0 = millis();
  pinMode(RED_LED, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  Serial.begin(115200);
  if(mode)wifi_init();
  dht.begin();
  finger.begin(57600);
  lightMeter.begin();
  myservo.attach(SERVO_PIN);

  lastMeasurment = millis();
  lastSonarRead = millis();
  lastMeasurment = millis();
  lastTime = millis();
  lastSerena = millis();
  lastMorg1 = millis();
  lastForTimer = millis();
  lastBuzz = millis();
  
  /*
  SonarAlarmDistance = sonar.ping_cm()-2;

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
  }

  strip1.begin();
  strip2.begin();

  strip1.setBrightness(BRIGHTNESS);
  strip2.setBrightness(BRIGHTNESS);  
  */

  if(mode) send_buttons();
}
bool f_s = true;

void loop() {

  Time = (millis() - time0) % DAY;
  timeNow = map(Time, 0, DAY, 0, 1440);
  Serial.print("\nВремя:   ");
  int hour = timeNow / 60;
  int minutes = ((int)timeNow) % 60;

  Serial.print(hour);
  Serial.print(":");
  Serial.println(minutes);
  write();
  
  if(!mode) read_serial();
  /* 
  if ((millis() - lastMeasurment) > SENSORS_MEASUREMENT) {
    readSensors();    
    lastMeasurment = millis();
  }
  */
  /*
  read_button();

  if (millis() - lastConnectionTime > postingInterval) send_sensors(); 
  
  */

  if(serenaState)
  {
    //вкл
    if (millis() - lastBuzz < BUZZ_TIME) 
    {
      tone(BUZZER_PIN, 30);
      digitalWrite(13, HIGH);
    } 
    else 
    {
      noTone(BUZZER_PIN);
      digitalWrite(13, LOW);
      if (millis() - lastBuzz > 2 * BUZZ_TIME) 
      {
        lastBuzz = millis();
      }

    }
  }
  else
  {
    noTone(BUZZER_PIN);
      digitalWrite(13, LOW);
  }

  if(jaluziChange)
  {
    jaluziChange = 0;
    if(jaluziState) myservo.write(POSITION_CLOSED);
    else myservo.write(POSITION_OPENED);
  }
  if (alarmState) 
  {
    // сигнализация вкл

    if(justAlarmChanged)
    {
      justAlarmChanged = 0;

      if(light1State)
      {
        setLEDColor1(0,0,0);
        light1State = 0;
      }

      if(light2State)
      {
        setLEDColor2(0,0,0);
        light2State = 0;        
      }
      
      if(!jaluziState)
      {
        jaluziState = 1;
        jaluziChange = 1;    
      }      
    }

    if(rSwitch || us) 
    {
      redLightState = true;
      isBlue = true;
      serenaState = true;
    }
    else
    {
      redLightState = false;
      isBlue = false;
      serenaState = false;

    }

    if(laser)
    {
      redLightState = true;
    }
    else
    {
      redLightState = false;
    }

  }
  else
  {
    // сигнализация выкл
    if(justAlarmChanged)
    {
      justAlarmChanged = 0;
    }
    if(timeNow < TIME_EIHGT_AM || timeNow > TIME_EIHGT_PM)
    {
      if(!time_last)
      {
        time_last = 1; 

        jaluziState = 1;
        jaluziChange = 1;

        light1State = 1;
        light1Change = 1; 

        light2State = 0;
        light2Change = 1;
      }
      // проверять свет

    }
    else
    {
      //день
      if(time_last)
      {
       time_last = 0;

       jaluziState = 0;
       jaluziChange = 1;

       light1State = 1;
       light1Change = 1; 

       light2State = 1;
       light2Change = 1;
      }    

      if(light1Change)
      {
        light1Change = 0;
        if(light1State)
        {
          int br = map(lightness, MINLUX, MAXLUX,70, 255);
          setLEDColor1(br,br,br);
          if(debug) Serial.println("light1 on auto");
        }
        else
        {
          setLEDColor1(0,0,0);
          if(debug) Serial.println("light1 off");
        }  
      }
      
      if(light2Change)
      {
        light2Change = 0;
        if(light2State)
        {
          int br = map(lightness, MINLUX, MAXLUX,0, 255);
          setLEDColor2(br,br,br);
          if(debug) Serial.println("light2 on auto");
        }
        else
        {
          setLEDColor2(0,0,0);
          if(debug) Serial.println("light2 off");
        }
      }  

      
        
    }

  }


  

  if (redLightState) {
    if (millis() - lastMorg1 < TIME_MORG) 
    {
      setLEDColor1(200, 0, 0);
    } 
    else 
    {
      if (isBlue) {
        setLEDColor1(0, 0, 200);
      } else {
        setLEDColor1(0, 0, 0);
      }
      if (millis() - lastMorg1 > 2 * TIME_MORG) {
        lastMorg1 = millis();
        setLEDColor1(0, 0, 0);
      }

    }
  }  

}
/*  end loop */



void write() {
  Serial.println("\n=+=Принятые данные=+=");
  Serial.print("Этаж 1:       ");
  Serial.println(light1State);
  Serial.print("Этаж 2:       ");
  Serial.println(light2State);
  Serial.print("Шторы:        ");
  Serial.println(jaluziState);
  Serial.print("Cигнализация: ");
  Serial.println(alarmState);
  Serial.print("Голос:        ");
  Serial.println(voiceState);
  Serial.print("Температура:    ");
  Serial.println(t);
  Serial.print("Влажность:      ");
  Serial.println(h);
  Serial.print("УльтраСоник:    ");
  Serial.println(rangeDistance);
  Serial.print("Геркон:         ");
  Serial.println(rSwitch);
  Serial.print("Лазер:          ");
  Serial.println(laser);
  Serial.print("Освещенность:   ");
  Serial.println(lightness);
  Serial.print("Шум:            ");
  Serial.println(micro);
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;

  // found a match!
  if(debug)
  {
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);  
  }

  if(serenaState)
  {
    serenaState = false;
  }
  else
  {
    alarmState = !alarmState;
    justAlarmChanged = true;    
  }

  if(alarmState || serenaState) {
    serenaState = false;
    redLightState = false;
  }


  return finger.fingerID;
}

void readSensors() {//Данные с датчиков
  getFingerprintIDez();
  h = dht.readHumidity();
  t = dht.readTemperature();
  rangeDistance = sonar.Ranging(CM);
  lightness = map(lightMeter.readLightLevel(), MINLUX, MAXLUX, 0, 100);
  //lightness = lightMeter.readLightLevel();
  rSwitch = digitalRead(REED_SWITCH_PIN);
  laser = !digitalRead(LASER_PIN);
  micro = analogRead(MICRO_PIN);
  us = (rangeDistance < SonarAlarmDistance);
  //отправляем на сервер

  if(debug) write();
}

void setLEDColor1(int r, int g, int b)
{  
  for (int i = 0; i < NUM_LEDS1; i++) 
  {
    strip1.setPixelColor(i, strip1.Color(r, g, b));
    strip1.show();
  }
}

void setLEDColor2(int r, int g, int b)
{  
  for (int i = 0; i < NUM_LEDS2; i++) 
  {
    strip2.setPixelColor(i, strip2.Color(r, g, b));
    strip2.show();
  }
}

void send_sensors()
{
  client.stop();
  if (client.connect(server, 80)) {

    client.print(F("GET /transfer/get.php?temp="));
    client.print(t);
    client.print("&&hum=");
    client.print(h);
    client.print("&&light=");
    client.print(lightness);
    client.print("&&door=");
    client.print(rSwitch);
    client.print("&&alarm=");
    client.print(alarmState);
    client.println(" HTTP/1.1");
    client.print(F("Host: "));
    client.println(server);
    client.println("Connection: close");
    client.println();
    lastConnectionTime = millis();
  }
  else  if(debug)   Serial.println("Connection failed");

}

void send_buttons()
{ 
  client.stop();
  if (client.connect(server, 80)) {

    client.print(F("GET /transfer/t.php?k1="));
    client.print(alarmState);
    client.print("&&k2=");
    client.print(light1State);
    client.print("&&k3=");
    client.print(light2State);
    client.print("&&k4=");
    client.print(jaluziState);
    client.print("&&k5=");
    client.print(voiceState);
    client.println(" HTTP/1.1");
    client.print(F("Host: "));
    client.println(server);
    client.println("Connection: close");
    client.println();
    lastConnectionTime = millis();
  }
  else   if(debug) Serial.println("Connection failed");

}

void read_button()
{
  while (client.available()) {
    char c = client.read();
    if (c == '+')
    {
      delay(1);
      k1 = int(client.read() - '0');
      if(k1 != alarmState) 
      {
        justAlarmChanged = 1;
        alarmState = k1;  
      }
      delay(1);
      k2 = int(client.read() - '0');
      if(k2 != light1State) 
      {
        light1Change = 1;
        light1State = k2;  
      }
      delay(1);
      k3 = int(client.read() - '0');
      if(k3 != light2State) 
      {
        light2Change = 1;
        light2State = k3;  
      }
      delay(1);
      k4 = int(client.read() - '0');
      if(k4 != jaluziState) 
      {
        jaluziChange = 1;
        jaluziState = k4;  
      }
      delay(1);
      voiceState = int(client.read() - '0');
    }
  }
}
void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void wifi_init()
{
  Serial2.begin(115200);
  WiFi.init(&Serial2);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");

  if(debug) printWifiStatus();
  send_buttons();
}

void read_serial()
{
  if(Serial.available())
  {
    switch(Serial.read())
    {
      case 'f':
      {
        justAlarmChanged = 1;
        if(Serial.read() == '0') alarmState = 0;
        else alarmState = 1;   
        break;
      }
      case 'j':
      {
        jaluziChange = 1;
        if(Serial.read() == '0') jaluziState = 0;
        else jaluziState = 1;   
        break;
      }
      case 'd':
      {
        if(Serial.read() == '0') rSwitch = 0;
        else rSwitch = 1;
        break;
      }
      case 'u':
      {
        if(Serial.read() == '0') laser = 0;
        else laser = 1;
        break;
      }
      case 'v':
      {
        if(Serial.read() == '0') us = 0;
        else us = 1;
        break;
      }
      case '1':
      {
        light1Change = 1;
        if(Serial.read() == '0') light1State = 0;
        else light1State = 1;
        break;
      }
      case '2':
      {
        light2Change = 1;
        if(Serial.read() == '0') light2State = 0;
        else light2State = 1;
        break;
      }  
    }
  }
}