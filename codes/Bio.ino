#include <Servo.h> #Server
#include <LiquidCrystal.h> #LCD
#include <Adafruit_Fingerprint.h> #Fingerprint
#include <SoftwareSerial.h> #Serial

#define CLASS_OVER_MS  30000 // 30 sec
#define EMERGENCY_MS   10000 // 10 sec

#define butt1 3
#define butt2 5
#define butt3 7
#define butt4 9
#define led1 4
#define led2 6
#define led3 8
#define buzz 2

SoftwareSerial mySerial(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);

Servo servo;
bool classStartFlag, emergencyFlag;
int id;
long ct, ct2;

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  lcd.begin(16, 2);
  finger.verifyPassword();

  pinMode(butt1, INPUT_PULLUP);
  pinMode(butt2, INPUT_PULLUP);
  pinMode(butt3, INPUT_PULLUP);
  pinMode(butt4, INPUT_PULLUP);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(buzz, OUTPUT);

  servo.attach(12);
  servo.write(0);
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("-FP SECURITY-- ");

  if (classStartFlag == 1) {
    lcd.setCursor(0, 1);
    lcd.print("PLS PUT FINGER..");

    id = matchFinger();
    if (id >= 0) {
      lcd.clear();
      lcd.print("FINGER MATCHED");
      lcd.setCursor(0, 1);
      lcd.print((String)"ID: " + id);
      Serial.println((String)"ID=" + id);
      delay(1500);
    }
    else if (id == -2) {
      lcd.clear();
      lcd.print(" ACCESS DENIED ");
      delay(1500);
    }
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print(" SECURITY DOOR ");
  }

  if (classStartFlag && (millis() - ct > CLASS_OVER_MS)) {
    classStartFlag = 0;
    Serial.println("OVER");
    digitalWrite(led1, 0);
    servo.write(0);
    ct = millis();
  }

  if (emergencyFlag && (millis() - ct2 > EMERGENCY_MS)) {
    emergencyFlag = 0;
    digitalWrite(led2, 0);
    servo.write(0);
    ct2 = millis();
  }

  if (Serial.available()) {
    String rx = Serial.readString();

    if (rx.startsWith("START")) {
      classStartFlag = 1;
      digitalWrite(led1, 1);
      servo.write(90);
      ct = millis();
    }

    else if (rx.startsWith("ENROLL=")) {
      rx.remove(0, 7);
      id = rx.toInt();
      lcd.clear();
      lcd.print((String)"ENROLL ID: " + id);
      lcd.setCursor(0, 1);
      lcd.print("PLS PUT FINGER..");
      if (fingerEnroll(id) == 1) {
        lcd.clear();
        lcd.print("ADDED.");
        Serial.println("EOK");
        delay(1500);
      }
      else {
        lcd.clear();
        lcd.print("ERROR!");
        Serial.println("ERR");
        delay(1500);
      }
    }
  }

  if (!digitalRead(butt1))addTeacherFinger(1);
  if (!digitalRead(butt2))addTeacherFinger(2);
  if (!digitalRead(butt3))addTeacherFinger(3);
  if (!digitalRead(butt4)) {
    lcd.clear();
    lcd.print(" AUTHO. PERSON ");
    lcd.setCursor(0, 1);
    lcd.print(" ONLY ACCESS ");
    while (!(matchFinger() >= 1 && matchFinger() <= 3));
    //    lcd.setCursor(0, 1);
    //    lcd.print("WAITING FOR F2..");
    //    while (matchFinger() != 2);
    //    lcd.setCursor(0, 1);
    //    lcd.print("WAITING FOR F3..");
    //    while (matchFinger() != 3);
    lcd.clear();
    ct2 = millis();
    emergencyFlag = 1;
    digitalWrite(led2, 1);
    servo.write(90);
  }
}

void addTeacherFinger(int id) {
  lcd.clear();
  lcd.print((String)"ADDING FINGER " + id);
  lcd.setCursor(0, 1);
  lcd.print("PLS PUT FINGER..");
  digitalWrite(led3, 1);

  if (fingerEnroll(id) == 1) {
    lcd.clear();
    lcd.print("ADDED.");
    delay(1500);
  }
  else {
    lcd.clear();
    lcd.print("ERROR!");
    delay(1500);
  }
  digitalWrite(led3, 0);
}

bool fingerEnroll(int id) {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return 0;

  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return 0;

  p = finger.createModel();
  if (p != FINGERPRINT_OK) return 0;

  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) return 0;
  return 1;
}

int matchFinger() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;

  return finger.fingerID;
}