#include <Stepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>

#define pinSentuh 7
#define pinBuzzer 4

#define batasHujan 700
#define batasSiang 200

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int stepsPerRevolution = 7199;

bool masuk = false;
bool stepperOn = false;

bool skrgHujan = false;
bool lastHujan = false;

bool skrgSiang = true;
bool lastSiang = true;

String cuaca = "";
String waktu = "";

char ssid[] = "fh_1eaaf0";
char pass[] = "Wlan1500";
char auth[] = "E0kUtQXmLUDAT_7w9VkAbU0mv8nG5Pp8";

Stepper myStepper = Stepper(stepsPerRevolution, 8, 10, 9, 11);

const int rxPin = 0;
const int txPin = 1;
SoftwareSerial serial(rxPin, txPin);

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  pinMode(pinSentuh, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  digitalWrite(pinBuzzer, LOW);

  lcd.begin();
}

void loop() {
  Blynk.run();
  cekSentuh();

  // Mengirim data melalui komunikasi serial SoftwareSerial
  String dataToSend = "Data yang ingin dikirim";
  serial.println(dataToSend);

  // Menerima data dari komunikasi serial SoftwareSerial
  if (serial.available()) {
    String receivedData = serial.readStringUntil('\n');
    // Lakukan sesuatu dengan data yang diterima

    Serial.print("Data diterima dari komunikasi serial: ");
    Serial.println(receivedData);
  }   

  if (masuk) {
    Serial.println("JEMURAN DI DALAM");
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    Serial.println("JEMURAN DI LUAR");
    digitalWrite(LED_BUILTIN, HIGH);
  }

  skrgHujan = statHujan();
  if (skrgHujan) {
    Serial.println("Sekarang Hujan");
    cuaca = "Hujan";
  } else {
    Serial.println("Sekarang Kering");
    cuaca = "Kering";
  }
  skrgSiang = statSiang();

  if (skrgSiang) {
    Serial.println("Sekarang Siang");
    waktu = "Siang";
  } else {
    Serial.println("Sekarang Malam");
    waktu = "Malam";
  }

  printLCD("  Waktu: " + waktu, "  Cuaca: " + cuaca);

  if (skrgHujan != lastHujan) {
    if (skrgHujan && !masuk) {
      stepperOn = true;
    }
    if (skrgSiang && masuk && !skrgHujan) {
      stepperOn = true;
    }
  }

  if (skrgSiang != lastSiang) {
    if (!skrgSiang && !masuk) {
      stepperOn = true;
    }
    if (skrgSiang && masuk && !skrgHujan) {
      stepperOn = true;
    }
  }

  if (stepperOn) {
    if (masuk) {
      ulur();
    } else {
      tarik();
    }
  }

  lastHujan = skrgHujan;
  lastSiang = skrgSiang;
  delay(500);

  BLYNK_WRITE(value);
}

void printLCD(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void cekSentuh() {
  if (digitalRead(pinSentuh) == HIGH) {
    Serial.println("SENSOR DISENTUH");
    bunyi(1, 0.2);

    if (stepperOn == false) {
      stepperOn = true;
    }
  }
  delay(500);
}

void tarik() {
  printLCD("Masukkan", "Jemuran!");
  bunyi(2, 0.5);
  Serial.println("TARIK!");
  myStepper.setSpeed(5);
  myStepper.step(stepsPerRevolution);
  stepperOn = false;
  Serial.println("BERHENTI");
  bunyi(1, 1);
  masuk = !masuk;
}

bool statHujan() {
  int nilaiHujan = digitalRead(A0);
  if (nilaiHujan < batasHujan) {
    return true;
  } else {
    return false;
  }
}

bool statSiang() {
  int nilaiCahaya = analogRead(A1);
  if (nilaiCahaya < batasSiang) {
    return true;
  } else {
    return false;
  }
}

void ulur() {
  Serial.println("ULUR!");
  printLCD("Keluarkan", "Jemuran!");
  bunyi(1, 0.5);
  myStepper.setSpeed(5);
  myStepper.step(-stepsPerRevolution);
  stepperOn = false;
  Serial.println("BERHENTI");
  bunyi(1, 1);
  masuk = !masuk;
}

void bunyi(int berapaKali, float detik) {
  for (int i = 0; i < berapaKali; i++) {
    digitalWrite(pinBuzzer, HIGH);
    delay(detik * 1000);
    digitalWrite(pinBuzzer, LOW);
    delay(detik * 1000);
  }
}

BLYNK_WRITE(V0) {
  int value = param.asInt(); // Mendapatkan nilai dari widget Blynk
  // Lakukan sesuatu berdasarkan nilai widget
  if (value == 1) {
    
    tarik();
  } else {
    ulur();
  }
}
