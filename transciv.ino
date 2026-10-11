#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define CE_PIN   9
#define CSN_PIN 10

// Konfigurasi Jalur Pin Output Sesuai Request
#define PIN_ESC    2   // ESC dikunci di pin D2
#define PIN_SERVO1 3   // Servo 1 di pin D3
#define PIN_SERVO2 4   // Servo 2 di pin D4
#define PIN_SERVO3 5   // Servo 3 di pin D5

RF24 radio(CE_PIN, CSN_PIN);
const byte alamat[6] = "06207";

struct PaketData {
  int stikKiriH;
  int stikKiriV;
  int stikKananX;
  int stikKananY;
  int aux1;
  int aux2;
  int aux3;
};

PaketData dataDiterima;
Servo objekESC;
Servo objekServo1;
Servo objekServo2;
Servo objekServo3;

unsigned long waktuTerakhirData = 0;

void setup() {
  objekESC.attach(PIN_ESC, 1000, 2000); 
  objekServo1.attach(PIN_SERVO1);
  objekServo2.attach(PIN_SERVO2);
  objekServo3.attach(PIN_SERVO3);

  // Inisialisasi awal posisi aman
  objekESC.writeMicroseconds(1000); 
  objekServo1.write(90);
  objekServo2.write(90);
  objekServo3.write(90);

  if (!radio.begin()) {
    while (1); 
  }
  
  radio.openReadingPipe(1, alamat);
  radio.setDataRate(RF24_250KBPS); 
  radio.setPALevel(RF24_PA_LOW);   
  radio.startListening();          
}

void loop() {
  if (radio.available()) {
    radio.read(&dataDiterima, sizeof(PaketData));
    waktuTerakhirData = millis(); 
    
    int nilaiAnalogThrottle = dataDiterima.stikKiriV;
    int nilaiESC = 1000; // Default awal adalah mati (1000 us)

    // LOGIKA STIK TENGAH = NOL THROTTLE (Toleransi deadband 530)
    if (nilaiAnalogThrottle <= 530) {
      nilaiESC = 1000;
    } 
    else {
      nilaiESC = map(nilaiAnalogThrottle, 530, 1023, 1000, 2000);
    }

    // Eksekusi ke ESC di D2
    objekESC.writeMicroseconds(nilaiESC);

    // Kontrol Servo (Mengikuti stik penuh 0-180 derajat)
    int sudutServo1 = map(dataDiterima.stikKiriH, 0, 1023, 0, 180);
    int sudutServo2 = map(dataDiterima.stikKananX, 0, 1023, 0, 180);
    int sudutServo3 = map(dataDiterima.stikKananY, 0, 1023, 0, 180);

    objekServo1.write(sudutServo1);
    objekServo2.write(sudutServo2);
    objekServo3.write(sudutServo3);
  }

  // Failsafe 1 detik
  if (millis() - waktuTerakhirData > 1000) {
    objekESC.writeMicroseconds(1000); 
  }
}