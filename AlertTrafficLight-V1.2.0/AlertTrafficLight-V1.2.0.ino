/*******************************************************************************************
 ** Autor: Mario Ammerschuber                                                             **
 ** Datum: Oktober 2022                                                                      **
 ** Version: 1.20                                                                       ** 
 ** Lizenz:  Creative Commons Zero v1.0 Universal                                         **
 *******************************************************************************************
*/

/************************( Importieren der Bibliotheken )************************/

#include <ESP8266WiFi.h>                    // über Bibliothek installieren
#include <Timezone.h>                       
#include <TimeLib.h>                        
#include <WiFiUdp.h>
#include "LittleFS.h"                       // Little Filesystem - über Bibliothek installieren
#include <pitches.h>                        // for passiv Buzzer
#include <ESP8266WebServer.h>               

ESP8266WebServer  server(80); // Webserver 
/*#define ROT 2     // NodeMCU v3 D4 -> GPIO 2
  #define GELB 0    // NodeMCU v3 D3 -> GPIO 0
  #define GRUEN 4   // NodeMCU v3 D2 -> GPIO 4*/
#define GRUEN 2 // NodeMCU v3 D4 -> GPIO 2
#define GELB 0  // NodeMCU v3 D3 -> GPIO 0
#define ROT 4   // NodeMCU v3 D2 -> GPIO 4

// ####################################################################################################
String ssid = "";     // SSID of local network 
String TempSSID = ""; // SSID of local network 
String password = ""; // Password on network 
// ####################################################################################################

// NTP Serverpool für Deutschland:
static const char ntpServerName[] = "de.pool.ntp.org";    //Finde lokale Server unter http://www.pool.ntp.org/zone/de
const int timeZone = 0;                     // 0 wenn mit <Timezone.h> gearbeitet wird !
WiFiUDP Udp;
unsigned int localPort = 1701; // lokaler Port zum Abhören von UDP-Paketen
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

// - Timezone. - //
// Bearbeiten Sie diese Zeilen entsprechend Ihrer Zeitzone und Sommerzeit.
// TimeZone Einstellungen Details https://github.com/JChristensen/Timezone
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Time (Frankfurt, Paris)
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time (Frankfurt, Paris)
Timezone CE(CEST, CET);

// passiv Buzzer Init
// ****************************************************************************************************
const int buzzerPin = D1; //for ESP8266 Microcontroller (D4 dont work with ESP8266 Microcontroller)
// const int buzzerPin = -1; // no Buzzer
// ****************************************************************************************************

/********************( Definieren der Globale Variablen und Konstanten)********************/
int h, m, s, w, mo, ye, d; // Time
String fsh,fsm,fss;
int waitloop = 0;
int WiFiConnectLoop = 0;
int WiFiConnectLoop2 = 0;
int MaxWiFiConnectLoop = 30;  // Maximale Anzahl Loops bei Verbindung mit dem WLAN 
bool ClockWiFiConnectFail = false; // false standard
unsigned long NoWiFiTime = 0;
bool WeekEnd = false;
int StartStopServers = 0;
int toggle = 0;
int BuzzerTrigger = 0;

bool littleFSInit = true;

/*****************************************( Setup )****************************************/
void setup() {
 Serial.begin(115200); // Serielle Verbindung mit 115200 Baud
  MyWaitLoop(250); //waits for data
  clearscreen(); // Serial Monitor clear screen

  // Init passiv Buzzer
  if (buzzerPin != -1) {
  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);} // kein Ton

  LoadAdjustments(); // SSID und Pawwort für WiFi laden

// ***First Start***
     // Little-Filesystem formatieren
     // LittleFS.format(); // alle Dateien  danch wieder deaktivieren und nochmal hochladen  
  
// Pins als Output festlegen.
pinMode(ROT, OUTPUT);
pinMode(GELB, OUTPUT);
pinMode(GRUEN, OUTPUT);

Serial.println("*** Alert-Traffic-Light ***");
Serial.println("");

if ((String)ssid == "") {
    Serial.println("");
    Serial.println("***** First Start  *****"); // erster Start der Uhr
    Serial.println("");
    } 

  // Initial LED's
  digitalWrite(ROT, LOW); 
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW); 
  SwitchTrafficLight(true,false,false,true); // Rot
  SwitchTrafficLight(false,true,false,true); // gelb
  SwitchTrafficLight(false,false,true,true); // grün
  MyWaitLoop(1000); 

  // WiFi Passwort - only for DebugModus
  // password = "wrongpass1234567890";

if ((String)ssid != ""){
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WiFi SSID: ");
  Serial.println(ssid);
  wificonnect(); // WLAN Verbindung herstellen
  } else {
  ClockWiFiConnectFail = true;
  Serial.println("");
  Serial.println("");} 
  
  if (ClockWiFiConnectFail == false)
  {
  Serial.println("UDP-Server starten ...");
  WiFiUDPStart();  // UPD Server starten 
  MyWaitLoop(250); //waits for data
  
// Get all information of your LittleFS
    if (littleFSInit == true)
    {
    Serial.println(F("Little Filesystem Init - done."));
    FSInfo fs_info;
    LittleFS.info(fs_info);
    Serial.println("File sistem info.");
    Serial.print("Total space:      ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
    Serial.print("Total space used: ");
    Serial.print(fs_info.usedBytes);
    Serial.println("byte");
    Serial.print("Block size:       ");
    Serial.print(fs_info.blockSize);
    Serial.println("byte");
    Serial.print("Page size:        ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
    Serial.print("Max open files:   ");
    Serial.println(fs_info.maxOpenFiles);
    Serial.print("Max path length:  ");
    Serial.println(fs_info.maxPathLength);
    Serial.println();
    // Open dir folder
    Dir dir = LittleFS.openDir("/");
    // Cycle all the content
    while (dir.next()) {
        // get filename
        Serial.print(dir.fileName());
        Serial.print(" - ");
        // If element have a size display It else write 0
        if(dir.fileSize()) {
            File f = dir.openFile("r");
            Serial.println(f.size());
            f.close();
        }else{
            Serial.println("0");
        }   
    }
   Serial.println("_______________________________");
   Serial.println("");
   }
  Serial.println("");

   getTimeLocal(); // aktuelle Zeit holen 
  
  // LED's - Startstatus setzen
  BuzzerBeep(); // Start Beep
  TrafficLightControlStart(); // Startvalues Traficlight
  Serial.println("Alert-Traffic-Light gestartet ...");
  }
  else
  { // ************* kein Password für SSID ***************
  // Serial.println("[Loaded-WIFI] SSID: " + (String)ssid); // Only for Debuging 
  // Serial.println("[Loaded-WIFI] PASS: " + (String)password); // Only for Debuging 
     
  if (ssid != ""){ 
  Serial.println("WLAN-Verbindung fehlgeschlagen!");  
  MyWaitLoop(100);} // kurze Pause
  
  
  else {
  Serial.println("Alert-Traffic-Light  - Starte Access Point ... [Alert-Traffic-Light]"); 
  SwitchTrafficLight(false,true,false,true); // Gelb
  if (buzzerPin != -1) {StartBuzzerBeep();}
  for(int i=0; i<5; i++) {
  SwitchTrafficLight(false,true,false,true); // Gelb EIN 
  SwitchTrafficLight(false,false,false,true);} // Gelb AUS
  SwitchTrafficLight(false,true,false,true); // Gelb EIN
  Serial.println("");
  
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  CaptivePortal(); // Start Captive Portal (WiFi-Manager)
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     }
  }    
}
/*************************************( Setup - Ende )*************************************/
 
/*************************************(Hauptprogramm)**************************************/
void loop() {
if (ssid == "") {server.handleClient();} else {   
// Check ob Internetverbindung noch besteht
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) 
  {
  wificonnect(); // WLAN-Verbindung herstellen
  if (ClockWiFiConnectFail == true)
  { // Wenn WiFi Connection fail
  Serial.println("WLAN-Verbindung: getrennt - "+FormatedDateTime());
  WiFiConnectLoop2 += 1; 
  if (WiFiConnectLoop2 > 10) {
  WiFiUDPStop(); // UDP Server stoppen  
  // Nach 10 durchläufen Access Point starten
  WiFiConnectLoop2 = 0;
  
  // Change Status LED's
  SwitchTrafficLight(true,true,true,true); // Alle An
  MyWaitLoop(2000);
  SwitchTrafficLight(false,false,false,true); // Alle Aus
  // Little-Filesystem formatieren
  LittleFS.format(); // alle Dateien löschen 
  Serial.println("");
  Serial.println("Alert-Traffic-Light wird neu gestartet");
  ESP.restart();}}}

  if (ClockWiFiConnectFail == false)
  {  // wenn WiFi Connection OK
  getTimeLocal(); // aktuelle Uhrzeit holen
  TrafficLightControlLoop(); // Loop TrafficLight
  } else
    { // wenn kein WLAN
    BlinkLED(ROT);
    }  
} 
}
 /*************************************(Hauptprogramm - Ende)**************************************/
