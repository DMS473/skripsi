#include <ESP32Servo.h> // Mengimpor library ESP32Servo untuk mengontrol servo

// Deklarasi objek Servo untuk setiap servo yang digunakan
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

// Definisi pin GPIO untuk setiap servo
const int SERVO1_PIN = 18;
const int SERVO2_PIN = 15;
const int SERVO3_PIN = 23;
const int SERVO4_PIN = 19;
const int SERVO5_PIN = 21;

// Variabel untuk menyimpan waktu terakhir kali perintah diterima
unsigned long lastCommandTime = 0;
// Konstanta untuk durasi timeout inaktivitas (5000 ms = 5 detik)
const long INACTIVITY_TIMEOUT_MS = 5000;

// Flag untuk melacak apakah servo saat ini aktif (terhubung dan menerima sinyal PWM)
// Ubah ini menjadi array atau individual flag jika setiap servo punya status aktif/nonaktif sendiri.
// Untuk tujuan ini, kita akan mempertahankan 'servosAreActive' untuk sebagian besar servo,
// dan melacak servo3 secara terpisah jika diperlukan logika detach/attach individu.

// Kita bisa pakai array boolean untuk status attach setiap servo secara individual
bool servoAttached[6] = {false, false, false, false, false, false}; // Index 0 tidak digunakan

void setup() {
  // Memulai komunikasi serial pada baud rate 115200 untuk debugging
  Serial.begin(115200);
  // Memulai komunikasi serial kedua (Serial2) pada baud rate 115200
  // Digunakan untuk menerima perintah dari Raspberry Pi (RX=16, TX=17)
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  // Menghubungkan setiap objek servo ke pin GPIO yang ditentukan saat startup
  // Ini memulai pengiriman sinyal PWM ke servo
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);
  servo4.attach(SERVO4_PIN);
  servo5.attach(SERVO5_PIN);

  // Set semua flag servoAttached menjadi true setelah attach awal
  servoAttached[1] = true;
  servoAttached[2] = true;
  servoAttached[3] = true;
  servoAttached[4] = true;
  servoAttached[5] = true;

  // Menginisialisasi waktu perintah terakhir dengan waktu saat ini
  lastCommandTime = millis();

  // Pesan ke Serial Monitor bahwa ESP32 siap
  Serial.println("ESP32 siap menerima perintah dari RPi...");
}

void loop() {
  // Memeriksa apakah ada data yang tersedia di Serial2 (dari Raspberry Pi)
  if (Serial2.available() > 0) {
    // Membaca string yang diterima hingga karakter newline ('\n')
    String data = Serial2.readStringUntil('\n');
    // Mencari indeks pemisah ':' dalam string data
    int separatorIndex = data.indexOf(':');

    // Jika pemisah ditemukan (indeks > 0)
    if (separatorIndex > 0) {
      // Mengambil ID servo dari bagian string sebelum pemisah dan mengubahnya menjadi integer
      int servo_id = data.substring(0, separatorIndex).toInt();
      // Mengambil nilai sudut dari bagian string setelah pemisah dan mengubahnya menjadi integer
      int angle = data.substring(separatorIndex + 1).toInt();
      // Membatasi nilai sudut antara 0 dan 180 derajat
      angle = constrain(angle, 0, 180);

      // Logika re-attach servo
      if (servo_id == 3) {
        // Jika perintah untuk servo 3 dan servo 3 belum attach, attach servo 3
        if (!servoAttached[3]) {
          servo3.attach(SERVO3_PIN);
          servoAttached[3] = true;
          Serial.println("Servo 3 diaktifkan kembali (perintah spesifik).");
        }
      } else {
        // Jika perintah untuk servo lain (bukan servo 3)
        // dan servo tersebut belum attach, attach servo yang bersangkutan.
        // Juga, jika ada servo lain yang detach (2,4,5), attach kembali mereka.
        // Ini memastikan servo lain aktif kembali saat ada perintah untuk mereka.
        if (!servoAttached[servo_id]) {
           switch (servo_id) {
             case 1: servo1.attach(SERVO1_PIN); break;
             case 2: servo2.attach(SERVO2_PIN); break;
             // case 3: sudah ditangani di atas
             case 4: servo4.attach(SERVO4_PIN); break;
             case 5: servo5.attach(SERVO5_PIN); break;
           }
           servoAttached[servo_id] = true;
           Serial.printf("Servo %d diaktifkan kembali (perintah umum).\n", servo_id);
        }
        // Pastikan servo 1, 2, 4, 5 yang tadinya mungkin detach (karena INACTIVITY_TIMEOUT_MS) di-attach jika ada perintah masuk untuk mereka.
        // Ini penting jika hanya servo 3 yang detach secara independen.
        // Logika ini akan lebih baik jika setiap servo punya flag detach/attachnya sendiri.
        // Mari kita ubah pendekatan agar lebih modular.
      }
      // Re-attach semua servo kecuali servo 3 jika ada perintah untuk servo lain
      // Jika ada perintah untuk servo non-3 dan servo tersebut detached, attach kembali.
      // Ini bagian yang agak tricky karena Anda ingin servo 3 detached independen.

      // **Versi Sederhana:** Jika ada perintah masuk untuk servo manapun, pastikan servo yang dituju attach.
      // Servo 1 tidak pernah detach di kode Anda sebelumnya, jadi kita anggap selalu aktif.
      if (!servoAttached[servo_id]) {
          switch (servo_id) {
              case 1: // servo1 tidak pernah di-detach
              case 2: servo2.attach(SERVO2_PIN); break;
              case 3: servo3.attach(SERVO3_PIN); break;
              case 4: servo4.attach(SERVO4_PIN); break;
              case 5: servo5.attach(SERVO5_PIN); break;
          }
          servoAttached[servo_id] = true;
          Serial.printf("Servo %d diaktifkan kembali.\n", servo_id);
      }


      // Menggerakkan servo yang sesuai berdasarkan ID
      switch (servo_id) {
        case 1: servo1.write(angle); break;
        case 2: servo2.write(angle); break;
        case 3: servo3.write(angle); break;
        case 4: servo4.write(angle); break;
        case 5: servo5.write(angle); break;
      }
      // Mencetak informasi gerakan servo ke Serial Monitor
      Serial.printf("Servo %d: %d\n", servo_id, angle);

      // Memperbarui waktu perintah terakhir ke waktu saat ini
      lastCommandTime = millis();
    }
  }

  // Memeriksa inaktivitas dan detach servo
  // Servo 1 tidak pernah di-detach di kode Anda, kita pertahankan itu.
  if (millis() - lastCommandTime > INACTIVITY_TIMEOUT_MS) {
    if (servoAttached[2]) {
      servo2.detach();
      servoAttached[2] = false;
      Serial.println("Servo 2 dinonaktifkan.");
    }
    if (servoAttached[3]) { // Hanya detach servo 3 jika belum detach
      servo3.detach();
      servoAttached[3] = false;
      Serial.println("Servo 3 dinonaktifkan.");
    }
    if (servoAttached[4]) {
      servo4.detach();
      servoAttached[4] = false;
      Serial.println("Servo 4 dinonaktifkan.");
    }
    if (servoAttached[5]) {
      servo5.detach();
      servoAttached[5] = false;
      Serial.println("Servo 5 dinonaktifkan.");
    }
    // Note: Servo 1 tetap aktif sesuai kode awal Anda (tidak ada detach).
    // Jika Anda ingin pesan ini hanya muncul sekali:
    // static bool timeoutMessageShown = false;
    // if (!timeoutMessageShown) {
    //   Serial.println("Tidak ada perintah selama 5 detik. Beberapa servo dinonaktifkan.");
    //   timeoutMessageShown = true;
    // }
  } else {
    // timeoutMessageShown = false; // Reset flag jika ada perintah baru
  }
}