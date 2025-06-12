#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define fire A3
#define buzz 13
#define light 10
#define fan 11
#define pump 12


#define wifi Serial

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


String wifiSSID = "iotmonitoring";
String wifiPASS = "12345678";

String HOST = "esinebd.com";  // 11
int gas, temp;
bool serverMode = 0;
bool smsFlag = false;

void setup() {
  wifi.begin(115200);

  lcd.begin(16, 2);

  pinMode(fire, INPUT);
  pinMode(buzz, OUTPUT);
  pinMode(light, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(pump, OUTPUT);
  
  wifi.println("AT+CWMODE=1"); 
  checkResponse();
  wifi.println((String)"AT+CWJAP=\"" + wifiSSID + "\",\"" + wifiPASS + "\""); 
  checkResponse();
 
}

void loop() {
  gas = analogRead(A5);
  temp = analogRead(A7);
  temp = temp * 0.4887;
  
  checkServer();
  delay(500);
  if(serverMode == 0){
    if(temp > 33) digitalWrite(light, 1);
    else digitalWrite(light, 0);

    if(gas > 350) digitalWrite(fan, 1);
    else if (gas < 100) digitalWrite(fan, 0);

    if(!digitalRead(fire)) digitalWrite(pump, 1);
    else digitalWrite(pump, 0); 
  }
  else {  
  }

  lcdprint(0, 0, (String)temp + (char)223 + "C  G:" + gas + "  F:" + !digitalRead(fire) + "  ");
  lcdprint(0, 1, (String)"L=" + digitalRead(light) + "  F=" + digitalRead(fan) + "  P=" + digitalRead(pump) + "  " + serverMode);
  
  send2Server();
  delay(500);
}
void lcdprint(byte x, byte y, String txt){
  lcd.setCursor(x, y);
  lcd.print(txt);
}

void send2Server(){
  String post = (String)"GET /projects/IoTWeather/update_machine.php?&temp=" + temp + "&gas=" + gas + "&fire=" + !digitalRead(fire) + " HTTP/1.1";
  int index = post.length() + 42;

  wifi.println("AT+CIPMUX=0");
  checkResponse(); 
  wifi.println("AT+CIPSTART=\"TCP\",\"" + HOST + "\",80");
  checkResponse();
  wifi.println((String)"AT+CIPSEND=" + index);
  checkResponse();
  wifi.println(post);  // 2
  wifi.println("HOST: " + HOST); // 8 + 11
  wifi.println("Connection: Close\r\n"); // 21
  checkResponse();
}

void checkServer(){
  String post = (String)"GET /projects/IoTWeather/stat_machine.php HTTP/1.1";
  int index = post.length() + 42;

  wifi.println("AT+CIPMUX=0");
  checkResponse(); 
  wifi.println("AT+CIPSTART=\"TCP\",\"" + HOST + "\",80");
  checkResponse();
  wifi.println((String)"AT+CIPSEND=" + index);
  checkResponse();
  wifi.println(post);  // 2
  wifi.println("HOST: " + HOST); // 8 + 11
  wifi.println("Connection: Close\r\n"); // 21
  String check = checkResponse();

  if(check.indexOf("server=0") != -1) serverMode = 0;
  else if(check.indexOf("server=1") != -1) serverMode = 1;

  if(serverMode == 1){
    if(check.indexOf("light=0") != -1) digitalWrite(light, 0);
    else if(check.indexOf("light=1") != -1) digitalWrite(light, 1);
    if(check.indexOf("fan=0") != -1) digitalWrite(fan, 0);
    else if(check.indexOf("fan=1") != -1) digitalWrite(fan, 1);
    if(check.indexOf("pump=0") != -1) digitalWrite(pump, 0);
    else if(check.indexOf("pump=1") != -1) digitalWrite(pump, 1);
  }
}

String checkResponse(){
  String r;
  while(!wifi.available());
  while(wifi.available()){
    r = wifi.readString();
  }
  delay(50);
  return r;
}