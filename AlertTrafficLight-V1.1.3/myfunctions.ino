// ****************************Hilfsfunktionen******************************************************
void clearscreen() { 
for(int i=0; i<10; i++) {
Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}
}

// *************************************************************************************************

// passiv buzzer melody: (Big Ben)
int melody[] = {
NOTE_E3, NOTE_GS3, NOTE_FS3, NOTE_B2, 0,
NOTE_E3, NOTE_FS3, NOTE_GS3, NOTE_E3, 0,
NOTE_GS3, NOTE_E3, NOTE_FS3, NOTE_B2, 0,
NOTE_B2, NOTE_FS3, NOTE_GS3, NOTE_E3
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
4,4,4,2.5,4,
4,4,4,2.5,4,
4,4,4,2.5,4,
4,4,4,2.5};

// *************************************************************************************************

void LoadAdjustments(){
// *** Einstellungen laden .... ***

// WEMOS D1 Little-Filesystem aktivieren
    if (LittleFS.begin()){littleFSInit = true;}
    else{littleFSInit = false;}
    MyWaitLoop(250); //waits for data

if (littleFSInit == true)
{
  Serial.println("Load Adjustments ...");
  // Zugangsdaten WiFi laden
  if (LittleFS.exists("/ssid.txt")){
  lssid = SetupLoad("ssid.txt");} else {lssid = "";}
  lssid.trim(); // Leerzeichen vorne und Hinten entfernen
  if (LittleFS.exists("/ssid-passwd.txt")){
  lpassword = SetupLoad("ssid-passwd.txt");} else {lpassword = "";}
  lpassword.trim(); // Leerzeichen vorne und Hinten entfernen
  ssid = lssid.c_str();
  password = lpassword.c_str();
  // Serial.println("[Loaded-WIFI] SSID: " + (String)ssid); // Only for Debuging 
  // Serial.println("[Loaded-WIFI] PASS: " + (String)password); // Only for Debuging 
} else
    {
     // Standardwerte setzen bei "Little FS" Fehler
    ssid = "";
    password = "";
    }  
}

// *************************************************************************************************

void SaveAdjustments(){
Serial.println("Save Clock Adjustments ...");  
// Serial.println("[Save-WIFI] SSID: " + (String)wifiManager.getWiFiSSID());  // Only for Debuging 
  // Serial.println("[Save-WIFI] PASS: " + (String)wifiManager.getWiFiPass());   // Only for Debuging 
   
  // SSID speichern
  SetupSave("ssid.txt", (String)wifiManager.getWiFiSSID()); // SSID WLAN
  SetupSave("ssid-passwd.txt", (String)wifiManager.getWiFiPass()); // Password WLAN

  MyWaitLoop(500); // 0,5 sek. warten 
  Serial.println("WiFI - Zugangsdaten gespeichert... "); 
}

// *************************************************************************************************

// Setup Einstellungen laden und speichern
String SetupLoad(String file_name) {
String result = ""; // init
  File this_file = LittleFS.open(file_name, "r");
  if (!this_file) { // failed to open the file, retrn empty result
    return result;
  }
  while (this_file.available()) {
      result += (char)this_file.read();
  }
  this_file.close();
  MyWaitLoop(10); //wait
  return result;
}

// *************************************************************************************************

bool SetupSave(String file_name, String contents) {  
  File this_file = LittleFS.open(file_name, "w");
  if (!this_file) { // failed to open the file, return false
    return false;
  }
  int bytesWritten = this_file.print(contents);
 
  if (bytesWritten == 0) { // write failed
      return false;
  }
  this_file.close();
  MyWaitLoop(10); //wait
  return true;
}

// *************************************************************************************************

String FormatedDateTime()
{
String DateTimeString;
// Uhrzeit
if (h < 10){DateTimeString = "0"+String(h); } else {DateTimeString = String(h);}
if (m < 10){DateTimeString += ":0"+String(m);} else {DateTimeString += ":"+String(m);}
if (s < 10){DateTimeString += ":0"+String(s)+" Uhr / ";} else{DateTimeString += ":"+String(s)+" Uhr / ";}
// Datum
if (d < 10){DateTimeString += "0"+String(d);} else {DateTimeString += String(d);}
if (mo < 10){DateTimeString += ".0"+String(mo)+"."+String(ye);} else {DateTimeString += "."+String(mo)+"."+String(ye);}
return DateTimeString;    
}

// *************************************************************************************************

int FormatedTime()
{
String fTime;  
getTimeLocal(); // aktuelle Zeit holen 
if (h < 10) {fsh = "0"+String(h);} else {fsh = String(h);}  
if (m < 10) {fsm = "0"+String(m);} else {fsm = String(m);}  
// if (s < 10) {fss = "0"+String(s);} else {fss = String(s);}  
fTime =fsh+fsm;
// Serial.println("Formated Time: "+fTime); // Only for Debuging 
return fTime.toInt();
}

// *************************************************************************************************

void wificonnect() 
{
WiFiConnectLoop = 0; // Variable zurücksetzen  
WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) 
{ 
MyWaitLoop(500);
Serial.print("."); 
BlinkLEDWifiConnect();
WiFiConnectLoop += 1;
if (WiFiConnectLoop >= MaxWiFiConnectLoop) {break;}
}
if (WiFi.status() != WL_CONNECTED)
{ClockWiFiConnectFail = true;
} else
{
ClockWiFiConnectFail = false;
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Serial.print("WiFi Connect-Loop: "+String(WiFiConnectLoop)+"/"+String(MaxWiFiConnectLoop)); // Zeit bis Connected
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Serial.println("");
Serial.print("Verbunden mit IP-Adresse: "); Serial.println(WiFi.localIP());
}
}

// *************************************************************************************************

void WiFiUDPStart()
{ 
Udp.begin(localPort);
Serial.print("lokaler Port: ");
Serial.println(Udp.localPort());
Serial.println("NTP-Time - Warten auf die Synchronisation");
setSyncProvider(getNtpTime);
setSyncInterval(86400); // Anzahl der Sekunden zwischen dem erneuten Synchronisieren ein. 86400 = 1 Tag
// setSyncInterval(60); // Test
Serial.println("UDP Server gestartet - "+FormatedDateTime());  
}

// *************************************************************************************************

void WiFiUDPStop()
{
Udp.stop();
Serial.println("UDP Server gestoppt - "+FormatedDateTime());   
}

// *************************************************************************************************

void MyWaitLoop(int wlt)
{// Pause Loop
waitloop = 0; // Variable zurücksetzen
while (waitloop < wlt) // Pause in Millisekunden
{ 
waitloop += 1; // inc waitloop 1
delay(1);
yield(); // Equivalent zu  Application.ProcessMessages; 
// Serial.println(String(waitloop));
}
}

// *************************************************************************************************

void BlinkLED(int farbe) {
 toggle = !toggle;
 if (BuzzerTrigger == 0){  
 BuzzerTrigger += 1;
 if (buzzerPin != -1) {beep(NOTE_E3, 350);} // BuzzerBeep
 }
 if (toggle) {
 if (farbe == ROT) {
 BlinkTrafficLight(true,false,false); // Rot
 }
 if (farbe == GELB) {
 BlinkTrafficLight(false,true,false); // Gelb
 }
 if (farbe == GRUEN) {
 BlinkTrafficLight(false,false,true); // Grün
 }
  } else 
     {
 digitalWrite(ROT, LOW); 
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW); 
      } 
MyWaitLoop(500); // kurze Pause 
}

// *************************************************************************************************

void BlinkLEDWifiConnect() {
toggle = !toggle;
 if (toggle) {
 BlinkTrafficLight(false,true,false); // Gelb
 }
  else 
     {
  BlinkTrafficLight(false,false,true); // Grün
      } 
 delay(1); // kurze Pause
}


// *************************************************************************************************

void SwitchTrafficLight(bool TLRed, bool TLYellow, bool TLGreen, bool Start){
if (!Start && buzzerPin != -1) {beep(NOTE_E3, 350);}  
if (TLRed){digitalWrite(ROT, HIGH);} else {digitalWrite(ROT, LOW);}
if (TLYellow){digitalWrite(GELB, HIGH);} else {digitalWrite(GELB, LOW);}
if (TLGreen){digitalWrite(GRUEN, HIGH);} else {digitalWrite(GRUEN, LOW);}
BuzzerTrigger = 0; // Zurücksetzen
MyWaitLoop(1000);
}

// *************************************************************************************************

void BlinkTrafficLight(bool TLRed, bool TLYellow, bool TLGreen){ 
if (TLRed){digitalWrite(ROT, HIGH);} else {digitalWrite(ROT, LOW);}
if (TLYellow){digitalWrite(GELB, HIGH);} else {digitalWrite(GELB, LOW);}
if (TLGreen){digitalWrite(GRUEN, HIGH);} else {digitalWrite(GRUEN, LOW);}
}

// *************************************************************************************************

void beep(int note, int duration){
tone(buzzerPin, note, duration);
MyWaitLoop(duration);}


// *************************************************************************************************

/*void BuzzerBeep()
{
beep(NOTE_A4, 500);
beep(NOTE_A4, 500);    
beep(NOTE_A4, 500);
beep(NOTE_F4, 350);
beep(NOTE_C5, 150);  
beep(NOTE_A4, 500);
beep(NOTE_F4, 350);
beep(NOTE_C5, 150);
beep(NOTE_A4, 650);
} */

// *************************************************************************************************

void BuzzerBeep()
{   
beep(NOTE_A4, 250);
beep(NOTE_F4, 250);
beep(NOTE_A4, 250);
}

// *************************************************************************************************

void StartBuzzerBeep()
{
// iterate over the notes of the melody:
for (int thisNote = 0; thisNote < 19; thisNote++) {
// to calculate the note duration, take one second divided by the note type.
//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
int noteDuration = 1000 / noteDurations[thisNote];
tone(buzzerPin, melody[thisNote], noteDuration);
// to distinguish the notes, set a minimum time between them.
// the note's duration + 30% seems to work well:
int pauseBetweenNotes = noteDuration * 1.70; // Default = 1.30
MyWaitLoop(pauseBetweenNotes);
// stop the tone playing:
noTone(buzzerPin);
}  
}

// *************************************************************************************************

void ShortBuzzerBeep()
{
// iterate over the notes of the melody:
for (int thisNote = 0; thisNote < 4; thisNote++) {
// to calculate the note duration, take one second divided by the note type.
//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
int noteDuration = 1000 / noteDurations[thisNote];
tone(buzzerPin, melody[thisNote], noteDuration);
// to distinguish the notes, set a minimum time between them.
// the note's duration + 30% seems to work well:
int pauseBetweenNotes = noteDuration * 1.30;
MyWaitLoop(pauseBetweenNotes);
// stop the tone playing:
noTone(buzzerPin);
}  
}

// *************************************************************************************************

void BigBenBuzzerBeep()
{
// iterate over the notes of the melody:
for (int thisNote = 0; thisNote < 19; thisNote++) {
// to calculate the note duration, take one second divided by the note type.
//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
int noteDuration = 1000 / noteDurations[thisNote];
tone(buzzerPin, melody[thisNote], noteDuration);
// to distinguish the notes, set a minimum time between them.
// the note's duration + 30% seems to work well:
int pauseBetweenNotes = noteDuration * 1.30;
MyWaitLoop(pauseBetweenNotes);
// stop the tone playing:
noTone(buzzerPin);
}  
}

// *************************************************************************************************

void StarWarsBuzzerBeep()
{
beep(NOTE_A4, 500);
 beep(NOTE_A4, 500);    
  beep(NOTE_A4, 500);
   beep(NOTE_F4, 350);
    beep(NOTE_C5, 150);  
     beep(NOTE_A4, 500);
      beep(NOTE_F4, 350);
       beep(NOTE_C5, 150);
        beep(NOTE_A4, 650);
 
MyWaitLoop(500);
 
beep(NOTE_E5, 500);
 beep(NOTE_E5, 500);
  beep(NOTE_E5, 500);  
   beep(NOTE_F5, 350);
    beep(NOTE_C5, 150);
     beep(NOTE_GS4, 500);
      beep(NOTE_F4, 350);
       beep(NOTE_C5, 150);
        beep(NOTE_A4, 650);
        
MyWaitLoop(500);

beep(NOTE_A5, 500);
 beep(NOTE_A4, 300);
  beep(NOTE_A4, 150);
   beep(NOTE_A5, 500);
    beep(NOTE_GS5, 325);
     beep(NOTE_G5, 175);
      beep(NOTE_FS5, 125);
       beep(NOTE_F5, 125);    
        beep(NOTE_FS5, 250);
        
MyWaitLoop(325);

beep(NOTE_AS4, 250);
 beep(NOTE_DS5, 500);
  beep(NOTE_D5, 325);  
   beep(NOTE_CS5, 175);  
    beep(NOTE_C5, 125);  
     beep(NOTE_AS4, 125);  
      beep(NOTE_C5, 250);  
 
MyWaitLoop(350);
}
// *************************************************************************************************
// *************************************************************************************************
