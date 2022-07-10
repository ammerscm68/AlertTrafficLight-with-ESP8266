// ****************************Hilfsfunktionen******************************************************
void clearscreen() { 
for(int i=0; i<10; i++) {
Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}
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
BlinkLED(GELB);
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
if (millis() - NoWiFiTime > 500) 
 { 
 NoWiFiTime = millis();
 dots = !dots;
 if (dots) {
 if (farbe == ROT) {
  digitalWrite(ROT, HIGH);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW);  
 }
 if (farbe == GELB) {
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, HIGH);
  digitalWrite(GRUEN, LOW);  
 }
 if (farbe == GRUEN) {
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);  
 }
  } else 
     {
 digitalWrite(ROT, LOW); 
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW); 
      } 
 delay(1); // kurze Pause
 }  
}
// *************************************************************************************************
// *************************************************************************************************
