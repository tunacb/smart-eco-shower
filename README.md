# 🚿 Akıllı Su Geri Kazanım ve IoT Tabanlı Duş Otomasyon Sistemi

Bu proje, banyolarda su istenilen sıcaklığa gelene kadar boşa akan temiz soğuk suyu kurtarmak, depolamak ve gerektiğinde geri dönüştürmek amacıyla tasarlanmış **ESP32 tabanlı bir akıllı ev (IoT) otomasyon sistemidir.** 

Sistem; sıcaklık, akış ve sıvı seviye sensörlerinden aldığı verileri işleyerek valfleri ve su pompasını otonom olarak yönetir. Ayrıca **Blynk IoT** platformu üzerinden dünyanın her yerinden kontrol edilebilir.

## 🚀 Öne Çıkan Özellikler (Mühendislik Çözümleri)

* **Edge Computing (Çevrimdışı Çalışma Modu):** Sistem, Blynk platformuna bağlı çalışmasına rağmen internet koptuğunda çökmez. ESP32, asenkron (non-blocking) yapısı sayesinde yerel sensör verilerini okumaya ve valf/motor güvenliğini sağlamaya devam eder.
* **Kuru Çalışma Koruması (Dry-Run Protection):** Depodaki su tamamen bittiğinde alt şamandıra bunu algılar ve motorun yanmasını önlemek için acil durum kesmesi (override) uygular.
* **Taşma Koruması:** Depo tam kapasiteye ulaştığında üst şamandıra devreye girerek valfleri otomatik olarak güvenli konuma alır.
* **Donanımsal Kesmeler (Interrupts):** Su akış sensörünün hızlı sinyalleri, veri kaybı yaşanmaması için `IRAM_ATTR` donanımsal kesme fonksiyonu ile okunur.
* **Kullanıcı UX ve Esneklik:** Depolanan su sadece duş sırasında değil, Blynk üzerinden "Manuel Boşaltım" modu (V6) aktif edilerek farklı amaçlar için de (çiçek sulama, kova doldurma vb.) tahliye edilebilir.

## 🛠️ Kullanılan Donanımlar ve Teknolojiler

* **Mikrodenetleyici:** ESP32 Development Board
* **Sensörler:** 
  * DS18B20 Su Geçirmez Sıcaklık Sensörü
  * YF-S201 Su Akış Sensörü (Hall Effect)
  * 2x Sıvı Seviye Sensörü (Şamandıra)
* **Aktüatörler ve Sürücüler:**
  * L298N Motor Sürücü (Pompa kontrolü için)
  * 12V DC Su Pompası (Santrifüj)
  * 2x 12V Selenoid Valf
  * Çoklu Röle Kartı (Active-Low)
* **Yazılım:** C++ (Arduino IDE), Blynk IoT Platformu

## 🔌 Pin Bağlantı Şeması (Pinout)

| Bileşen | ESP32 Pini | Not |
| :--- | :--- | :--- |
| **DS18B20 Sıcaklık Sensörü** | `GPIO 4` | 4.7kΩ Pull-Up direnci ile |
| **YF-S201 Akış Sensörü** | `GPIO 14` | Interrupt (Düşen Kenar) |
| **Alt Şamandıra (Su Bitti)** | `GPIO 13` | INPUT_PULLUP |
| **Üst Şamandıra (Taşma)** | `GPIO 25` | INPUT_PULLUP |
| **Valf 1 (Depo Yönü)** | `GPIO 26` | Röle üzerinden |
| **Valf 2 (Duş Yönü)** | `GPIO 27` | Röle üzerinden |
| **Pompa Hız (L298N ENA)** | `GPIO 18` | PWM Kontrolü |
| **Pompa Yön 1 (L298N IN1)** | `GPIO 19` | |
| **Pompa Yön 2 (L298N IN2)** | `GPIO 21` | |

> ⚠️ **Güvenlik Uyarısı:** L298N motor sürücüsünü ve selenoid valfleri besleyen 12V güç kaynağının (+) ucu KESİNLİKLE ESP32'ye bağlanmamalıdır. Sistemdeki tüm modüllerin GND (Toprak) hatları ortaklanmıştır (Common Ground).

## 📱 Blynk IoT Kurulumu (Sanal Pinler)

Sistemi Blynk uygulaması üzerinden kontrol etmek için aşağıdaki "Datastream" (Veri Akışı) yapılandırmasını kullanabilirsiniz:

* **`V0` (Switch):** Sistemi Başlat / Beklemeye Al
* **`V1` (Value Display):** Anlık Su Sıcaklığı (°C)
* **`V2` (String Display):** Sistem Durumu (Isıtılıyor, Duş Hazır, Beklemede vb.)
* **`V3` (String Display):** Depo Doluluk Durumu (Tam Dolu, Boş, Su Var)
* **`V4` (Gauge):** Pompa Çalışma Durumu / Hızı
* **`V5` (Switch):** Duş Sırasında Depo Suyunu Kullan (Aç/Kapat)
* **`V6` (Button):** Manuel Depoyu Boşalt (Pompayı Zorla Çalıştır)

## ⚙️ Kurulum ve Çalıştırma

1. Depoyu klonlayın: `git clone https://github.com/KULLANICI_ADIN/akilli-dus-sistemi.git`
2. Arduino IDE'yi açın ve `OneWire`, `DallasTemperature` ve `BlynkSimpleEsp32` kütüphanelerinin kurulu olduğundan emin olun.
3. Kodu açarak ağ ayarlarınızı ve Blynk bilgilerinizi güncelleyin:
   ```cpp
   #define BLYNK_TEMPLATE_ID "TEMPLATE_ID_BURAYA"
   #define BLYNK_DEVICE_NAME "Akilli Dus"
   #define BLYNK_AUTH_TOKEN "AUTH_TOKEN_BURAYA"
   char ssid[] = "WIFI_ADINIZ";
   char pass[] = "WIFI_SIFRENIZ";
