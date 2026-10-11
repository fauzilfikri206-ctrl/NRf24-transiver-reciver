#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

// Sesuaikan pin analog ini dengan kabel joystick di remotemu
#define STIK_KIRI_H  A2  
#define STIK_KIRI_V  A3  
#define STIK_KANAN_X A1  
#define STIK_KANAN_Y A0  

RF24 radio(CE_PIN, CSN_PIN);
const byte alamat[6] = "06207"; // Harus sama persis dengan receiver

struct PaketData {
  int stikKiriH;
  int stikKiriV;
  int stikKananX;
  int stikKananY;
  int aux1;
  int aux2;
  int aux3;
};

PaketData dataRemote;
unsigned long waktuTerakhirKirim = 0;
const unsigned long intervalKirim = 20; // Dipercepat ke 20ms agar respons kontrol lebih real-time

void setup() {
  if (!radio.begin()) {
    while (1); // Proteksi jika NRF tidak terdeteksi
  }
  
  radio.openWritingPipe(alamat);
  radio.setDataRate(RF24_250KBPS); // Kecepatan rendah = jangkauan lebih jauh & stabil
  radio.setPALevel(RF24_PA_LOW);   // Naikkan ke RF24_PA_MAX kalau sudah siap diterbangkan
  radio.stopListening();
}

void loop() {
  unsigned long waktuSekarang = millis();

  if (waktuSekarang - waktuTerakhirKirim >= intervalKirim) {
    waktuTerakhirKirim = waktuSekarang;

    // Membaca nilai analog (0 - 1023)
    dataRemote.stikKiriH  = analogRead(STIK_KIRI_H);
    dataRemote.stikKiriV  = analogRead(STIK_KIRI_V);
    dataRemote.stikKananX = analogRead(STIK_KANAN_X);
    dataRemote.stikKananY = analogRead(STIK_KANAN_Y);
    
    // Nilai default untuk tombol/potensio tambahan jika belum dipasang
    dataRemote.aux1 = 512;
    dataRemote.aux2 = 512;
    dataRemote.aux3 = 512;

    // Mengirim data ke receiver
    radio.write(&dataRemote, sizeof(PaketData));
  }
}