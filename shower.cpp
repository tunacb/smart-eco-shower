// BLYNK KİMLİK BİLGİLERİ 
#define BLYNK_TEMPLATE_ID "TMPLxxxxxx"
#define BLYNK_DEVICE_NAME "Akilli Dus"
#define BLYNK_AUTH_TOKEN "Sizin_Blynk_Tokeniniz"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- WİFİ BİLGİLERİ ---
char ssid[] = "WiFi_Adiniz";
char pass[] = "WiFi_Sifreniz";

// --- PİN TANIMLAMALARI ---
const int SICAKLIK_PIN = 4;     
const int AKIS_SENSORU = 14;    
const int ALT_SAMANDIRA = 13;   
const int UST_SAMANDIRA = 25;   
const int VALF_1_DEPO = 26;     
const int VALF_2_DUS = 27;      
const int POMPA_HIZ = 18;       
const int POMPA_YON1 = 19;      
const int POMPA_YON2 = 21;      

// --- SENSÖR KURULUMLARI ---
OneWire oneWire(SICAKLIK_PIN);
DallasTemperature sensors(&oneWire);
BlynkTimer timer;

// --- DEĞİŞKENLER VE DURUM KİLİTLERİ ---
volatile int pulseSayaci = 0;
unsigned long sonAkisZamani = 0;
bool muslukAcikMi = false;
float sicaklik = 0.0;

bool sistemAktif = false;       // V0: Duş Hazırlama Sistemi Başlat/Durdur
bool dusOturumuBasladi = false; 
bool dusHazirBildirimi = false; 

// YENİ EKLENEN KULLANICI SEÇENEKLERİ
bool depoSuyuKullan = true;     // V5: Duş sırasında pompayı kullan (Varsayılan: Açık)
bool manuelPompa = false;       // V6: Depoyu boşaltmak için manuel pompa butonu

// --- KESME FONKSİYONU (Akış Sensörü) ---
void IRAM_ATTR akisKesmesi() {
  pulseSayaci++;
}

// --- BLYNK KONTROLLERİ ---

// V0: Ana Sistemi Aç/Kapat
BLYNK_WRITE(V0) {
  sistemAktif = param.asInt(); 
  if (!sistemAktif) {
    digitalWrite(VALF_1_DEPO, LOW); 
    digitalWrite(VALF_2_DUS, LOW);  
    dusOturumuBasladi = false; 
    dusHazirBildirimi = false; 
    if(Blynk.connected()) Blynk.virtualWrite(V2, "Sistem Beklemede");
  } else {
    if(Blynk.connected()) Blynk.virtualWrite(V2, "Sistem Başladı, Suyu Isıtıyor...");
  }
}

// V5: Duşta Depo Suyunu Kullan (Switch - 1 veya 0)
BLYNK_WRITE(V5) {
  depoSuyuKullan = param.asInt();
}

// V6: Depoyu Boşalt / Manuel Pompa (Buton - 1 veya 0)
BLYNK_WRITE(V6) {
  manuelPompa = param.asInt();
}

// --- ANA KONTROL DÖNGÜSÜ (İnternet olsa da olmasa da çalışır) ---
void sistemKontrol() {
  // 1. Sensörleri Oku (İnternetten Bağımsız İşlem)
  sensors.requestTemperatures();
  sicaklik = sensors.getTempCByIndex(0);
  bool depoTasiyor = (digitalRead(UST_SAMANDIRA) == LOW); 
  bool depoBos = (digitalRead(ALT_SAMANDIRA) == LOW); 

  // 2. Akış Sensörü Kontrolü
  if (millis() - sonAkisZamani > 1000) { 
    muslukAcikMi = (pulseSayaci > 2);
    pulseSayaci = 0; 
    sonAkisZamani = millis();
  }

  // Blynk'e Veri Gönderimi (Sadece bağlıysa)
  if (Blynk.connected()) {
    Blynk.virtualWrite(V1, sicaklik); 
    if (depoTasiyor) Blynk.virtualWrite(V3, "TAM DOLU (Taşma Koruması)");
    else if (depoBos) Blynk.virtualWrite(V3, "BOŞ (Pompa Durduruldu)");
    else Blynk.virtualWrite(V3, "Su Var");
  }

  // --- SICAKLIK VE VALF KONTROLÜ (Sadece Sistem Aktifse Çalışır) ---
  if (sistemAktif) {
    if (sicaklik >= 40.0) {
      dusOturumuBasladi = true; 
    }

    if (depoTasiyor) {
      digitalWrite(VALF_1_DEPO, LOW);
      digitalWrite(VALF_2_DUS, HIGH);
      if(Blynk.connected()) Blynk.virtualWrite(V2, "DİKKAT: Depo Doldu!");
    } 
    else {
      if (dusOturumuBasladi) {
        digitalWrite(VALF_1_DEPO, LOW);  
        digitalWrite(VALF_2_DUS, HIGH);  
        if (Blynk.connected()) {
          Blynk.virtualWrite(V2, "Duş Kullanımda");
          if (!dusHazirBildirimi) {
            Blynk.logEvent("dus_hazir", "Su 40 dereceye ulaştı, duşunuz hazır!");
            dusHazirBildirimi = true;
          }
        }
      } else {
        digitalWrite(VALF_1_DEPO, HIGH); 
        digitalWrite(VALF_2_DUS, LOW);   
        if(Blynk.connected()) Blynk.virtualWrite(V2, "Soğuk Su Depolanıyor");
      }
    }
  }

  // --- POMPA KONTROLÜ (Geri Dönüşüm, Manuel Boşaltma & Kuru Çalışma Koruması) ---
  // Pompa kontrolü sistemAktif döngüsünün dışındadır! Çünkü adam sistemi başlatmadan da depoyu boşaltmak isteyebilir.
  
  if (depoBos) {
    // SU BİTTİ - KESİN DURUŞ (Her iki modda da motoru korur)
    analogWrite(POMPA_HIZ, 0);
    if(Blynk.connected()) {
      Blynk.virtualWrite(V4, 0); 
      // Eğer kullanıcı depoyu boşalt butonunu açık unuttuysa, su bitince uygulamadan butonu da kapat
      if (manuelPompa) {
        manuelPompa = false; 
        Blynk.virtualWrite(V6, 0); 
      }
    }
  } 
  else {
    // Depoda su var. Pompa ne zaman çalışmalı?
    // 1. Şart: Manuel Boşaltma (V6) açıksa.
    // 2. Şart: Akış var VE Sistem Aktif VE Kullanıcı duşta depo suyunu kullanmak (V5) istiyorsa.
    if (manuelPompa || (muslukAcikMi && sistemAktif && depoSuyuKullan)) {
      analogWrite(POMPA_HIZ, 255); // Pompayı tam güç çalıştır
      if(Blynk.connected()) Blynk.virtualWrite(V4, 100); 
    } else {
      // Hiçbir şart sağlanmıyorsa pompayı kapat
      analogWrite(POMPA_HIZ, 0);
      if(Blynk.connected()) Blynk.virtualWrite(V4, 0);
    }
  }
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  pinMode(ALT_SAMANDIRA, INPUT_PULLUP);
  pinMode(UST_SAMANDIRA, INPUT_PULLUP);
  pinMode(AKIS_SENSORU, INPUT_PULLUP);
  pinMode(VALF_1_DEPO, OUTPUT);
  pinMode(VALF_2_DUS, OUTPUT);
  pinMode(POMPA_HIZ, OUTPUT);
  pinMode(POMPA_YON1, OUTPUT);
  pinMode(POMPA_YON2, OUTPUT);

  digitalWrite(VALF_1_DEPO, LOW);
  digitalWrite(VALF_2_DUS, LOW);
  digitalWrite(POMPA_YON1, HIGH);
  digitalWrite(POMPA_YON2, LOW);
  analogWrite(POMPA_HIZ, 0); 

  attachInterrupt(digitalPinToInterrupt(AKIS_SENSORU), akisKesmesi, FALLING);

  // WiFi & Blynk Başlatma
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);

  timer.setInterval(1000L, sistemKontrol);
}

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  } else if (WiFi.status() == WL_CONNECTED) {
    Blynk.connect(); 
  }
  timer.run(); 
}