# ESP32-C5 IoT Security Testing Platform — „SPECTRA"
## Pełny Plan Architektury i Implementacji

---

## 1. PRZEGLĄD PROJEKTU

### 1.1 Cel
Stworzenie kompletnej platformy do testów bezpieczeństwa urządzeń IoT, działającej na ESP32-C5, wykorzystującej wszystkie interfejsy radiowe chipu: Wi-Fi 2.4 GHz, Wi-Fi 5 GHz, Bluetooth 5 LE, IEEE 802.15.4 (Zigbee 3.0 / Thread 1.3). Platforma przeznaczona wyłącznie do testowania własnych urządzeń IoT w kontrolowanym środowisku laboratoryjnym.

### 1.2 Platforma sprzętowa — ESP32-C5
- **CPU:** 32-bit RISC-V single-core, do 240 MHz (HP), LP core do 48 MHz
- **RAM:** 384 KB SRAM on-chip + zewnętrzny PSRAM (4–8 MB)
- **Flash:** 16 MB SPI Flash (typowo)
- **Wi-Fi:** 802.11ax (Wi-Fi 6), dual-band 2.4 GHz + 5 GHz, MU-MIMO, OFDMA, TWT
- **Bluetooth:** Bluetooth 5 LE, Coded PHY, 2M PHY, Advertising Extensions
- **802.15.4:** Zigbee 3.0, Thread 1.3, Matter
- **Krypto:** AES, SHA, RSA, ECC, HMAC — sprzętowe akceleratory
- **Inne:** Secure Boot, Flash Encryption, Key Management Controller, TEE

### 1.2.1 Wyświetlacz i dotyk — TFT LCD 2.8" ILI9341 + XPT2046
- **Wyświetlacz:** 2.8" TFT LCD, 320×240 px, 65K kolorów (RGB565), sterownik ILI9341
- **Dotyk:** Rezystancyjny panel dotykowy, kontroler XPT2046 (12-bit ADC)
- **Interfejs:** SPI (wspólna magistrala lub osobne linie CS), max 80 MHz (ILI9341), ~2 MHz (XPT2046)
- **Podświetlenie:** Sterowane przez GPIO (PWM — regulacja jasności)
- **Rozdzielczość dotyku:** 4096×4096 (mapowane na 320×240)
- **Pinout (przykładowy):**

| Sygnał | Pin ESP32-C5 | Opis |
|--------|-------------|------|
| TFT_MOSI | GPIO_X | SPI MOSI (wspólny z touch) |
| TFT_MISO | GPIO_X | SPI MISO (wspólny z touch) |
| TFT_SCLK | GPIO_X | SPI CLK (wspólny z touch) |
| TFT_CS | GPIO_X | Chip Select wyświetlacza |
| TFT_DC | GPIO_X | Data/Command |
| TFT_RST | GPIO_X | Reset wyświetlacza |
| TFT_BL | GPIO_X | Podświetlenie (PWM) |
| TOUCH_CS | GPIO_X | Chip Select dotyku (XPT2046) |
| TOUCH_IRQ | GPIO_X | Przerwanie dotyku (opcjonalnie) |

*Piny GPIO_X do określenia na podstawie dostępnych pinów ESP32-C5 i kolizji z SD/GPS/LED.*

### 1.3 Środowisko programistyczne
- **Framework:** ESP-IDF v5.5+ (najnowszy stabilny z obsługą ESP32-C5)
- **Język:** C (100%), bez Arduino
- **System:** FreeRTOS (wbudowany w ESP-IDF)
- **Budowanie:** CMake + idf.py
- **Interfejs użytkownika:** CLI przez UART/USB-CDC + TFT LCD 2.8" Touch (główny interfejs) + opcjonalnie web UI przez Wi-Fi AP
- **GUI Framework:** LVGL v9.x (Light and Versatile Graphics Library) — renderowanie UI na ILI9341

### 1.4 Architektura wysokopoziomowa

```
┌─────────────────────────────────────────────────────────┐
│                    SPECTRA — Main App                    │
├──────────┬──────────┬──────────┬──────────┬─────────────┤
│  WiFi    │  WiFi    │   BLE    │ 802.15.4 │  System     │
│  2.4 GHz │  5 GHz   │  Engine  │  Engine  │  Core       │
│  Engine  │  Engine  │          │ Zigbee/  │             │
│          │          │          │ Thread   │             │
├──────────┴──────────┴──────────┴──────────┴─────────────┤
│                   Packet Engine (PCAP/Analysis)          │
├─────────────────────────────────────────────────────────┤
│   CLI Parser + Web UI Server + TFT Touch GUI (LVGL)     │
├─────────────────────────────────────────────────────────┤
│        Storage Manager (SD Card / SPIFFS / NVS)         │
├─────────────────────────────────────────────────────────┤
│   ESP-IDF (FreeRTOS, LWIP, NimBLE, 802.15.4, SPI/DMA)  │
├─────────────────────────────────────────────────────────┤
│   ESP32-C5 Hardware + ILI9341 TFT + XPT2046 Touch (SPI) │
└─────────────────────────────────────────────────────────┘
```

---

## 2. PEŁNA LISTA FUNKCJONALNOŚCI

### 2.1 Moduł Wi-Fi (2.4 GHz + 5 GHz)

#### 2.1.1 Skanowanie i rekonesans
- **Skan AP (Access Points)** — aktywny i pasywny skan na obu pasmach z listą: SSID, BSSID, kanał, RSSI, typ szyfrowania (Open/WEP/WPA/WPA2/WPA3/WPA2-Enterprise), typ 802.11 (b/g/n/a/ac/ax)
- **Skan stacji (STA)** — wykrywanie klientów podłączonych do AP poprzez analizę probe request/response w trybie promiscuous
- **Skan kanałów** — automatyczne skanowanie wszystkich kanałów 2.4 GHz (1–14) i 5 GHz (36–165) z pomiarem zajętości spektrum
- **Fingerprinting AP** — identyfikacja producenta przez OUI, analiza Vendor-Specific IE, wykrywanie ukrytych SSID
- **Wykrywanie urządzeń rogue** — detekcja WiFi Pineapple (PineScan), Pwnagotchi, multi-SSID z jednego MAC
- **Wardriving** — ciągłe skanowanie z logowaniem wyników + opcjonalny GPS (UART)

#### 2.1.2 Przechwytywanie pakietów (Sniffing)
- **Promiscuous mode sniffer** — przechwytywanie wszystkich ramek 802.11 na wybranym kanale
- **Filtrowanie pakietów** — po typie (Management/Control/Data), podtypie (Beacon/Probe/Deauth/Auth/Assoc/Data), po MAC źródłowym/docelowym, po BSSID
- **Packet Monitor** — wizualizacja ruchu w czasie rzeczywistym (liczniki pakietów per typ, wykres intensywności)
- **Beacon Sniffer** — przechwytywanie i analiza beacon frames z pełnym dekodowaniem IE
- **Probe Request Sniffer** — zbieranie probe requests do mapowania urządzeń i ich preferowanych sieci
- **Deauth Sniffer** — detekcja ataków deautentykacji w sieci
- **EAPOL/PMKID Capture** — przechwytywanie 4-way handshake (EAPOL) i PMKID z pierwszego pakietu dla późniejszego offline crackingu
- **Raw Frame Capture** — zapis surowych ramek 802.11 bez filtrowania
- **Zapis do PCAP** — wszystkie powyższe sniffery z opcją zapisu do formatu PCAP na kartę SD, kompatybilnego z Wireshark
- **Channel State Information (CSI)** — zbieranie CSI z pakietów Wi-Fi 6 dla analizy kanału radiowego

#### 2.1.3 Ataki aktywne (testy zabezpieczeń)
- **Deauthentication Flood** — wysyłanie sfałszowanych ramek deauth do wszystkich klientów wybranego AP
- **Deauthentication Targeted** — deauth konkretnego klienta z konkretnego AP
- **Beacon Spam (z listy)** — generowanie setek fałszywych beaconów z listy SSID
- **Beacon Spam (losowe)** — generowanie beaconów z losowymi SSID
- **Beacon Spam (klonowanie)** — klonowanie istniejącego AP (same parametry, inny BSSID)
- **Probe Request Flood** — wysyłanie masowych probe request z losowymi SSID (zaciemnianie monitoringu)
- **Authentication Flood** — przeciążanie AP fałszywymi ramkami auth
- **Association Flood** — przeciążanie AP fałszywymi ramkami association
- **AP Clone** — tworzenie Soft-AP klonującego parametry prawdziwego AP
- **Evil Portal** — fałszywy captive portal z konfigurowalną stroną HTML (logowanie, phishing testowy)
- **Evil Twin** — pełne klonowanie AP + deauth oryginalnego + captive portal
- **Karma Attack** — AP odpowiadający na dowolne probe request (dla testowania zachowania klientów)
- **PMKID Attack** — wymuszenie wymiany PMKID z AP
- **Packet Injection** — wysyłanie dowolnych ramek 802.11 przez `esp_wifi_80211_tx()`

#### 2.1.4 Narzędzia Wi-Fi 6 specyficzne
- **Wi-Fi 6 AP Identification** — detekcja AP obsługujących ax, analiza HE Capabilities
- **TWT Analysis** — analiza parametrów Target Wake Time
- **BSS Coloring Detection** — identyfikacja BSS Color w ramkach
- **OFDMA/MU-MIMO Probing** — testowanie zachowania AP wobec wielu klientów

### 2.2 Moduł Bluetooth Low Energy (BLE)

#### 2.2.1 Skanowanie i rekonesans
- **BLE Device Scanner** — skanowanie urządzeń BLE (advertising), zbieranie: nazwa, MAC, RSSI, typ adresu (public/random), AD structures
- **Service Discovery** — łączenie z urządzeniem BLE i enumeracja Services, Characteristics, Descriptors (GATT)
- **Manufacturer Data Parser** — dekodowanie danych producenta z AD structures (Apple, Google, Samsung, Microsoft itp.)
- **iBeacon/Eddystone Scanner** — wykrywanie i dekodowanie beaconów
- **BLE Fingerprinting** — identyfikacja urządzenia przez analizę advertising data, GATT profilu, zachowania

#### 2.2.2 Przechwytywanie i analiza
- **BLE Advertisement Sniffer** — ciągłe zbieranie advertisement packets na wszystkich 3 kanałach adv (37, 38, 39)
- **GATT Read/Write Logger** — logowanie operacji na charakterystykach po połączeniu
- **Connection Parameter Analysis** — analiza parametrów połączenia (interval, latency, timeout)
- **Pairing Analysis** — monitorowanie procesu parowania, analiza metody (Just Works, Passkey, Numeric Comparison, OOB)
- **BLE PCAP Export** — zapis przechwyconych danych BLE do formatu analizowalnego offline

#### 2.2.3 Ataki aktywne (testy zabezpieczeń)
- **BLE Spam — Apple (Sour Apple)** — spoofing BLE advertisements imitujących urządzenia Apple (AirPods, AirTag itp.) powodujących notyfikacje
- **BLE Spam — Samsung** — spoofowanie advertisement dla urządzeń Samsung
- **BLE Spam — Google/Android** — spoofowanie advertisement Fast Pair
- **BLE Spam — Microsoft Swift Pair** — generowanie fałszywych BLE Swift Pair advertisements
- **BLE Spam — Universal** — wszystkie powyższe jednocześnie
- **BLE Beacon Spoofing** — tworzenie fałszywych iBeacon/Eddystone beaconów z konfigurowalnymi parametrami
- **BLE Flooding** — masowe wysyłanie advertisement packets z losowymi MAC (stress-test)
- **BLE GATT Fuzzing** — wysyłanie nieprawidłowych danych do characteristics (buffer overflow test, format string test)
- **BLE Denial of Service** — ciągłe żądania połączeń do urządzenia docelowego
- **BLE MAC Spoofing** — zmiana adresu MAC BLE na dowolny
- **BLE HID Injection (Rubber Ducky)** — emulacja klawiatury BLE, wysyłanie sekwencji klawiszy do sparowanego urządzenia
- **BLE Replay Attack** — przechwycenie i ponowne wysłanie advertisement/connection packets

#### 2.2.4 Narzędzia Bluetooth 5 specyficzne
- **Coded PHY Testing** — testowanie komunikacji Long Range (S=2, S=8)
- **2M PHY Testing** — testowanie szybkiej komunikacji
- **Extended Advertising** — tworzenie i analiza extended advertising PDU
- **Periodic Advertising** — monitorowanie periodic advertising

### 2.3 Moduł IEEE 802.15.4 — Zigbee

#### 2.3.1 Skanowanie i rekonesans
- **Network Discovery** — wykrywanie sieci Zigbee na kanałach 11–26 (2.4 GHz)
- **Channel Energy Scan** — pomiar energii na każdym kanale 802.15.4
- **Active Scan** — wysyłanie beacon request i zbieranie odpowiedzi od koordynatorów
- **PAN ID Discovery** — identyfikacja wszystkich PAN ID w zasięgu
- **Device Enumeration** — zbieranie adresów krótkich (16-bit) i rozszerzonych (64-bit EUI) urządzeń
- **Network Topology Mapping** — mapowanie relacji router/end-device/koordynator
- **Zigbee Profile Identification** — identyfikacja profilu (ZHA, ZLL, ZCL clusters)

#### 2.3.2 Przechwytywanie i analiza
- **802.15.4 Frame Sniffer** — przechwytywanie surowych ramek IEEE 802.15.4
- **Zigbee Packet Decoder** — dekodowanie warstw: MAC → NWK → APS → ZCL
- **Key Extraction** — przechwytywanie kluczy transportowych podczas procesu dołączania (join)
- **Trust Center Monitoring** — monitorowanie komunikacji z Trust Center
- **OTA Update Sniffer** — przechwytywanie ramek aktualizacji firmware OTA
- **ZCL Command Logger** — logowanie wszystkich komend ZCL (On/Off, Level, Color itp.)
- **PCAP Export 802.15.4** — zapis ramek 802.15.4 do PCAP

#### 2.3.3 Ataki aktywne (testy zabezpieczeń)
- **Replay Attack** — przechwycenie i ponowne wysłanie ramki 802.15.4
- **Touchlink Commissioning Attack** — nadużycie procesu Touchlink do:
  - Kradzieży urządzeń z sieci (factory reset + join do atakującego)
  - Identyfikacji urządzeń (wymuszona identyfikacja wizualna)
  - Zmiany kanału
- **Network Key Sniffing** — przechwycenie klucza sieciowego podczas procesu join (jeśli przesyłany niezaszyfrowanie lub z known Trust Center Link Key)
- **Frame Injection** — wstrzykiwanie surowych ramek 802.15.4
- **Jamming 802.15.4** — zakłócanie komunikacji na wybranym kanale (wysyłanie ciągłych ramek)
- **PAN Conflict** — tworzenie konfliktu PAN ID
- **Insecure Rejoin Attack** — wymuszenie rejoin bez szyfrowania
- **ZCL Command Injection** — wysyłanie spreparowanych komend ZCL do urządzeń (np. On/Off bez autoryzacji)
- **Denial of Service** — flood ramkami do koordynatora
- **Malformed Frame Fuzzing** — wysyłanie zniekształconych ramek 802.15.4 do testowania odporności stosu

#### 2.3.4 Narzędzia Zigbee 3.0 specyficzne
- **Install Code Verification** — testowanie procesu dołączania z Install Code
- **Green Power Proxy Testing** — analiza urządzeń Green Power
- **Zigbee Cluster Library Analysis** — pełne dekodowanie ZCL clusters

### 2.4 Moduł IEEE 802.15.4 — Thread

#### 2.4.1 Skanowanie i rekonesans
- **Thread Network Discovery** — wykrywanie sieci Thread (MLE Discovery)
- **Thread Network Data** — zbieranie Thread Network Data (prefiksy, trasy, serwisy)
- **Commissioner Discovery** — wykrywanie aktywnych Commissioner
- **Border Router Detection** — identyfikacja Border Routerów Thread
- **Thread Device Role Identification** — identyfikacja ról: Leader, Router, REED, SED, MED, FED

#### 2.4.2 Przechwytywanie i analiza
- **MLE Sniffer** — przechwytywanie wiadomości Mesh Link Establishment
- **802.15.4 Security Analysis** — analiza poziomu bezpieczeństwa ramek (Security Level, Key ID Mode)
- **CoAP Sniffer** — przechwytywanie wiadomości CoAP w sieci Thread
- **Thread Mesh Routing Analysis** — analiza tablicy routingu mesh
- **PCAP Export Thread** — zapis do PCAP z metadanymi Thread

#### 2.4.3 Ataki aktywne (testy zabezpieczeń)
- **MLE Frame Injection** — wstrzykiwanie ramek MLE
- **Network Partition** — próba podziału sieci Thread
- **Leader Impersonation** — próba ogłoszenia się liderem sieci
- **Denial of Service Thread** — flood ramkami 802.15.4 kierowanymi na adresy Thread
- **Commissioner Spoofing** — próba spoofingu Commissioner
- **Replay Attack Thread** — powtórzenie przechwyconych ramek Thread

#### 2.4.4 Narzędzia Thread 1.3 specyficzne
- **Service Registration Analysis** — monitorowanie SRP (Service Registration Protocol)
- **DNS-SD Discovery** — analiza odkrywania serwisów przez DNS-SD
- **Thread Border Router API Testing** — testowanie REST API Border Routerów

### 2.5 Moduł Cross-Protocol

#### 2.5.1 Matter Protocol Testing
- **Matter Device Discovery** — wykrywanie urządzeń Matter przez BLE i Wi-Fi
- **Commissioning Analysis** — analiza procesu komisjonowania Matter
- **Matter over Thread** — analiza komunikacji Matter przez Thread
- **Matter over Wi-Fi** — analiza komunikacji Matter przez Wi-Fi

#### 2.5.2 Testy koegzystencji
- **Multi-Radio Interference** — jednoczesna praca Wi-Fi + BLE + 802.15.4 i pomiar wzajemnych zakłóceń
- **2.4 GHz Spectrum Analysis** — wspólna analiza zajętości pasma 2.4 GHz przez wszystkie protokoły
- **Cross-Protocol Timing Attack** — wykorzystanie timingu jednego protokołu do ataku na inny

### 2.6 Moduł systemowy

#### 2.6.1 Interfejs użytkownika — CLI
- **Hierarchiczny system komend** — np. `wifi scan ap`, `ble scan`, `zigbee sniff start`
- **Autouzupełnianie** — TAB completion dla komend
- **Historia komend** — strzałki góra/dół
- **Kolorowe wyjście** — ANSI colors w terminalu
- **Pomoc kontekstowa** — `help`, `<komenda> --help`
- **Skryptowanie** — wykonywanie sekwencji komend z pliku na SD

#### 2.6.2 Interfejs użytkownika — Web UI (opcjonalny)
- **Dashboard** — status wszystkich modułów radiowych
- **Live Packet View** — podgląd pakietów w przeglądarce
- **Konfiguracja** — ustawienia przez formularz web
- **Pobieranie PCAP** — download plików z SD przez HTTP
- **WebSocket Stream** — real-time streaming danych do przeglądarki

#### 2.6.3 Interfejs użytkownika — TFT LCD 2.8" Touch (główny interfejs fizyczny)

Pełny graficzny interfejs użytkownika na wyświetlaczu TFT 320×240 z panelem dotykowym, umożliwiający kontrolę całej aplikacji SPECTRA bez podłączania komputera. Oparty na bibliotece **LVGL v9.x**.

**Ekrany nawigacyjne (hierarchia):**

```
┌─ Home Dashboard (główny ekran)
│   ├── Status modułów (Wi-Fi / BLE / Zigbee / Thread)
│   ├── Wskaźniki pamięci RAM/Flash/SD
│   ├── Czas pracy, temperatura chipu
│   └── GPS status (jeśli podłączony)
│
├─ Wi-Fi Module
│   ├── Wi-Fi Scanner
│   │   ├── Lista AP (SSID, RSSI bar, kanał, auth, vendor)
│   │   ├── Filtr: 2.4/5 GHz / otwarte / WPA
│   │   ├── Sortowanie: RSSI / kanał / SSID
│   │   └── Tap na AP → szczegóły + akcje (deauth / clone / sniff)
│   ├── Wi-Fi Sniffer
│   │   ├── Start/Stop + wybór kanału
│   │   ├── Live packet counter (typy ramek)
│   │   ├── Wykres RSSI w czasie (line chart)
│   │   └── Status zapisu PCAP (rozmiar pliku, czas)
│   ├── Wi-Fi Attacks
│   │   ├── Deauth — target selector (lista AP/STA) + start/stop
│   │   ├── Beacon Spam — tryb (list/random/clone) + start/stop
│   │   ├── Evil Portal — wybór template z SD + status (klienci, credentials)
│   │   ├── Evil Twin — target AP + auto-orchestration + status
│   │   ├── Karma — start/stop + lista probes
│   │   └── PMKID/Handshake — status capture
│   ├── Wi-Fi Detector
│   │   ├── Rogue AP alerts (Pineapple, Pwnagotchi)
│   │   └── Lista podejrzanych AP
│   └── Channel Occupancy
│       ├── Wykres słupkowy (1-14 + 5GHz channels)
│       └── Heatmap kanałów
│
├─ BLE Module
│   ├── BLE Scanner
│   │   ├── Lista urządzeń (nazwa, MAC, RSSI, vendor, typ)
│   │   ├── Filtr: iBeacon / Eddystone / Apple / Samsung / inne
│   │   ├── Tap na urządzenie → szczegóły + GATT discovery
│   │   └── RSSI proximity indicator
│   ├── GATT Explorer
│   │   ├── Drzewo: Services → Characteristics → Descriptors
│   │   ├── Read/Write/Notify toggle
│   │   └── Hex/ASCII value viewer
│   ├── BLE Attacks
│   │   ├── BLE Spam — wybór typu (Apple/Samsung/Google/MS/Universal)
│   │   ├── Beacon Spoof — konfiguracja iBeacon/Eddystone
│   │   ├── GATT Fuzzer — target + tryb + status
│   │   ├── HID Injection — wybór skryptu Ducky z SD
│   │   └── Connection Flood — target + start/stop
│   └── BLE Sniffer
│       ├── Advertisement monitor (live)
│       └── PCAP export status
│
├─ Zigbee Module
│   ├── Zigbee Scanner
│   │   ├── Lista sieci (PAN ID, kanał, koordynator, profil)
│   │   ├── Channel energy scan (wykres 11-26)
│   │   └── Tap na sieć → topologia (coordinator/router/end-device)
│   ├── Zigbee Sniffer
│   │   ├── Start/Stop + wybór kanału
│   │   ├── Dekodowane ramki (MAC/NWK/APS/ZCL) w scrollowanej liście
│   │   ├── Key extraction status
│   │   └── PCAP export status
│   └── Zigbee Attacks
│       ├── Touchlink (scan/identify/reset/steal)
│       ├── Replay attack
│       ├── Frame injection
│       ├── Jamming per channel
│       └── DoS coordinator
│
├─ Thread Module
│   ├── Thread Scanner
│   │   ├── Lista sieci Thread (Network Name, PAN, Channel, Leader)
│   │   ├── Topologia mesh (wizualizacja ról)
│   │   └── Border Router detection
│   ├── Thread Sniffer
│   │   ├── MLE frames monitor
│   │   ├── CoAP sniffer
│   │   └── PCAP export
│   └── Thread Attacks
│       ├── MLE injection
│       ├── Partition attack
│       └── Leader spoofing
│
├─ Cross-Protocol
│   ├── Matter Discovery
│   │   ├── Lista urządzeń Matter (BLE + mDNS)
│   │   └── Commissioning flow status
│   ├── Spectrum Analyzer
│   │   ├── Wykres 2.4 GHz (CSI + ED + RSSI) — fullscreen chart
│   │   └── Identyfikacja źródeł zakłóceń
│   └── Coexistence Test
│       ├── Konfiguracja testu (które moduły jednocześnie)
│       └── Wyniki (throughput, packet loss, latency)
│
├─ Wardriving
│   ├── Mapa? — prosta wizualizacja GPS track
│   ├── Live stats (AP count, unique SSIDs, distance)
│   ├── GPS status (fix, satellites, coordinates)
│   └── Start/Stop + wybór pliku CSV
│
├─ Files (SD Card Browser)
│   ├── Lista plików/folderów (PCAP, logi, skrypty, portal HTML)
│   ├── Info: rozmiar, data, typ
│   ├── Akcje: usuń, pobierz (przez Web UI)
│   └── Wolne miejsce na SD
│
└─ Settings
    ├── Wyświetlacz — jasność, timeout, orientacja, theme (dark/light)
    ├── Wi-Fi — domyślne pasmo, kanał, MAC policy
    ├── BLE — domyślne parametry skanowania
    ├── 802.15.4 — domyślny kanał
    ├── Storage — ścieżki zapisu, auto-rotacja logów
    ├── GPIO — konfiguracja pinów (LED, buzzer, GPS UART)
    ├── System — reboot, factory reset, OTA update, info
    └── About — wersja firmware, uptime, autor
```

**Komponenty GUI (LVGL):**
- **Navigation bar** — górny pasek z ikonami modułów + status bar (czas, bateria, GPS, SD)
- **Tab bar / Sidebar** — nawigacja między głównymi modułami (swipe lub tap)
- **Listy przewijalne** — dla wyników skanów (AP, BLE devices, Zigbee networks)
- **Wykresy** — LVGL lv_chart: RSSI timeline, channel occupancy, spectrum analyzer
- **Przyciski akcji** — Start/Stop z kolorowym feedbackiem (zielony=aktywny, czerwony=atak)
- **Dialogi potwierdzenia** — przed destrukcyjnymi akcjami (deauth, jamming, factory reset)
- **Keyboard on-screen** — do wpisywania SSID, filtrów, nazw plików
- **Toast / Snackbar** — powiadomienia o zdarzeniach (handshake captured!, rogue AP detected!)
- **Progress bar** — dla skanów, transferów, OTA
- **Spinner/Loading** — dla operacji asynchronicznych

**Optymalizacje wyświetlacza:**
- **Partial refresh** — LVGL dirty area rendering (nie odświeżamy całego ekranu)
- **DMA SPI transfer** — nieblokujący transfer pikseli do ILI9341
- **Double buffering** — 2× bufor (np. 2×10 linii = 2×320×10×2 = 12.8 KB) — **wewnętrzny SRAM DMA-capable** (nie PSRAM!); reszta obiektów LVGL w PSRAM
- **FPS target** — 25-30 FPS dla animacji, 10-15 FPS dla statycznych ekranów
- **Sleep mode** — wyłączenie podświetlenia + wstrzymanie LVGL po timeout
- **Priorytet task** — GUI task na niższym priorytecie niż radio tasks (Wi-Fi/BLE/802.15.4)

**Obsługa dotyku (XPT2046):**
- **Kalibracja** — 3-punktowa kalibracja przy pierwszym uruchomieniu (zapis do NVS)
- **Debouncing** — filtracja false touches (minimum press time, jitter filter)
- **Gestures** — tap, long press, swipe left/right (nawigacja ekranów)
- **Touch feedback** — krótki buzzer beep lub wizualne podświetlenie

#### 2.6.4 Storage Manager
- **SD Card** — główne miejsce zapisu PCAP, logów, konfiguracji, stron Evil Portal
- **SPIFFS/LittleFS** — przechowywanie plików web UI, konfiguracji domyślnej
- **NVS** — ustawienia persistent (ostatni profil, MAC, konfiguracja sieci)
- **Rotacja logów** — automatyczne usuwanie najstarszych plików przy zapełnieniu

#### 2.6.5 Packet Engine
- **PCAP Writer** — zapis pakietów w formacie PCAP (IEEE 802.11, IEEE 802.15.4, BLE)
- **Ring Buffer** — bufor cykliczny dla pakietów w RAM z konfigurowalnymi rozmiarem
- **Packet Filter Engine** — konfigurowalne filtry BPF-like
- **Statistics Engine** — zliczanie pakietów, przepływność, histogram RSSI
- **Timestamp Engine** — precyzyjne znaczniki czasu (RTC + mikrosekund timer)

#### 2.6.6 Narzędzia dodatkowe
- **MAC Randomization** — losowy MAC Wi-Fi/BLE przy każdym uruchomieniu
- **OTA Update** — aktualizacja firmware przez Wi-Fi
- **GPIO Control** — sterowanie LED statusowymi, buzzerem
- **GPS Integration** — odczyt pozycji z modułu GPS przez UART (wardriving)
- **Power Management** — tryby oszczędzania energii, odczyt napięcia baterii (ADC)
- **System Info** — wyświetlanie wersji firmware, wolnej pamięci, uptime, temperatury chipu

---

## 3. STRUKTURA PROJEKTU (KATALOGI I PLIKI)

```
spectra/
├── CMakeLists.txt                    # Główny CMakeLists
├── sdkconfig.defaults                # Domyślna konfiguracja ESP-IDF
├── partitions.csv                    # Tablica partycji (app, nvs, spiffs, ota)
├── main/
│   ├── CMakeLists.txt
│   ├── main.c                        # Entry point, inicjalizacja systemu
│   ├── spectra_config.h              # Globalna konfiguracja (#define)
│   └── version.h                     # Wersja firmware
├── components/
│   ├── cli/
│   │   ├── CMakeLists.txt
│   │   ├── cli_engine.h              # Parser komend, rejestracja, dispatch
│   │   ├── cli_engine.c
│   │   ├── cli_commands_wifi.h/.c    # Komendy Wi-Fi
│   │   ├── cli_commands_ble.h/.c     # Komendy BLE
│   │   ├── cli_commands_zigbee.h/.c  # Komendy Zigbee
│   │   ├── cli_commands_thread.h/.c  # Komendy Thread
│   │   ├── cli_commands_system.h/.c  # Komendy systemowe
│   │   └── cli_colors.h             # Definicje kolorów ANSI
│   ├── wifi_engine/
│   │   ├── CMakeLists.txt
│   │   ├── wifi_engine.h/.c          # Główny moduł Wi-Fi (init, deinit, switch band)
│   │   ├── wifi_scanner.h/.c         # Skanowanie AP i stacji
│   │   ├── wifi_sniffer.h/.c         # Promiscuous mode, callback, filtrowanie
│   │   ├── wifi_attacks.h/.c         # Deauth, beacon spam, probe flood
│   │   ├── wifi_evil_portal.h/.c     # Evil Portal (HTTP server + DNS + captive portal)
│   │   ├── wifi_evil_twin.h/.c       # Evil Twin (klonowanie + deauth + portal)
│   │   ├── wifi_karma.h/.c           # Karma Attack
│   │   ├── wifi_packet_builder.h/.c  # Budowanie surowych ramek 802.11
│   │   ├── wifi_deauth.h/.c          # Generowanie ramek deauth/disassoc
│   │   ├── wifi_beacon.h/.c          # Generowanie ramek beacon
│   │   ├── wifi_handshake.h/.c       # Przechwytywanie EAPOL/PMKID
│   │   ├── wifi_detector.h/.c        # Detekcja rogue AP, Pwnagotchi, Pineapple
│   │   ├── wifi_csi.h/.c             # Channel State Information
│   │   └── wifi_types.h              # Struktury danych (ap_info_t, station_t, itp.)
│   ├── ble_engine/
│   │   ├── CMakeLists.txt
│   │   ├── ble_engine.h/.c           # Główny moduł BLE (init, deinit)
│   │   ├── ble_scanner.h/.c          # Skanowanie advertising
│   │   ├── ble_gatt_client.h/.c      # GATT discovery, read/write
│   │   ├── ble_spam.h/.c             # BLE spam (Apple, Samsung, Google, MS, Universal)
│   │   ├── ble_beacon.h/.c           # iBeacon/Eddystone spoofing
│   │   ├── ble_fuzzer.h/.c           # GATT fuzzing
│   │   ├── ble_dos.h/.c              # BLE DoS attacks
│   │   ├── ble_hid.h/.c              # BLE HID injection (Rubber Ducky)
│   │   ├── ble_replay.h/.c           # Replay attacks
│   │   ├── ble_sniffer.h/.c          # Advertisement sniffer + logger
│   │   └── ble_types.h               # Struktury danych BLE
│   ├── zigbee_engine/
│   │   ├── CMakeLists.txt
│   │   ├── zigbee_engine.h/.c        # Główny moduł Zigbee (init, deinit)
│   │   ├── zigbee_scanner.h/.c       # Network discovery, channel scan
│   │   ├── zigbee_sniffer.h/.c       # 802.15.4 frame sniffer
│   │   ├── zigbee_decoder.h/.c       # Dekodowanie ramek ZB: MAC→NWK→APS→ZCL
│   │   ├── zigbee_attacks.h/.c       # Replay, touchlink, frame injection
│   │   ├── zigbee_touchlink.h/.c     # Touchlink commissioning attacks
│   │   ├── zigbee_key_extract.h/.c   # Key sniffing/extraction
│   │   ├── zigbee_fuzzer.h/.c        # Malformed frame fuzzing
│   │   ├── zigbee_inject.h/.c        # Raw frame injection
│   │   └── zigbee_types.h            # Struktury danych Zigbee
│   ├── thread_engine/
│   │   ├── CMakeLists.txt
│   │   ├── thread_engine.h/.c        # Główny moduł Thread (init, deinit)
│   │   ├── thread_scanner.h/.c       # Thread network discovery
│   │   ├── thread_sniffer.h/.c       # MLE sniffer, CoAP sniffer
│   │   ├── thread_attacks.h/.c       # MLE injection, partition, leader spoof
│   │   ├── thread_analyzer.h/.c      # Routing analysis, topology mapping
│   │   └── thread_types.h            # Struktury danych Thread
│   ├── packet_engine/
│   │   ├── CMakeLists.txt
│   │   ├── pcap_writer.h/.c          # Zapis do formatu PCAP
│   │   ├── packet_buffer.h/.c        # Ring buffer pakietów
│   │   ├── packet_filter.h/.c        # Silnik filtrów
│   │   ├── packet_stats.h/.c         # Statystyki pakietów
│   │   └── packet_types.h            # Wspólne typy pakietów
│   ├── storage/
│   │   ├── CMakeLists.txt
│   │   ├── sd_card.h/.c              # Obsługa karty SD (SPI mode)
│   │   ├── spiffs_mgr.h/.c           # SPIFFS/LittleFS management
│   │   ├── nvs_mgr.h/.c              # NVS storage
│   │   └── log_rotate.h/.c           # Rotacja logów
│   ├── web_ui/
│   │   ├── CMakeLists.txt
│   │   ├── web_server.h/.c           # HTTP server (esp_http_server)
│   │   ├── web_ws.h/.c               # WebSocket handler
│   │   ├── web_api.h/.c              # REST API endpoints
│   │   └── www/                      # Statyczne pliki HTML/CSS/JS (na SPIFFS)
│   │       ├── index.html
│   │       ├── dashboard.js
│   │       └── style.css
│   ├── tft_gui/
│   │   ├── CMakeLists.txt
│   │   ├── tft_driver.h/.c           # ILI9341 SPI init, DMA, backlight PWM
│   │   ├── touch_driver.h/.c         # XPT2046 SPI driver, kalibracja, debounce
│   │   ├── gui_main.h/.c             # LVGL init, tick, task handler, global navigation
│   │   ├── gui_theme.h/.c            # Definicja theme (dark/light), kolory, fonty
│   │   ├── gui_events.h/.c           # Event bus: GUI ↔ silniki radiowe (thread-safe)
│   │   ├── screens/
│   │   │   ├── scr_home.h/.c         # Home Dashboard
│   │   │   ├── scr_wifi.h/.c         # Wi-Fi — scanner, sniffer, attacks
│   │   │   ├── scr_wifi_detail.h/.c  # Wi-Fi — szczegóły AP + akcje
│   │   │   ├── scr_ble.h/.c          # BLE — scanner, GATT, attacks
│   │   │   ├── scr_ble_detail.h/.c   # BLE — szczegóły urządzenia + GATT tree
│   │   │   ├── scr_zigbee.h/.c       # Zigbee — scanner, sniffer, attacks
│   │   │   ├── scr_thread.h/.c       # Thread — scanner, sniffer, attacks
│   │   │   ├── scr_cross.h/.c        # Cross-Protocol — Matter, spectrum, coex
│   │   │   ├── scr_wardriving.h/.c   # Wardriving — GPS, live stats
│   │   │   ├── scr_files.h/.c        # SD Card file browser
│   │   │   ├── scr_settings.h/.c     # Ustawienia (display, radio, system)
│   │   │   └── scr_keyboard.h/.c     # On-screen keyboard (reusable)
│   │   ├── widgets/
│   │   │   ├── widget_statusbar.h/.c # Górny status bar (czas, bateria, GPS, SD)
│   │   │   ├── widget_navbar.h/.c    # Dolna nawigacja / tab bar
│   │   │   ├── widget_ap_list.h/.c   # Reusable lista AP z RSSI barami
│   │   │   ├── widget_ble_list.h/.c  # Reusable lista BLE devices
│   │   │   ├── widget_chart.h/.c     # Wrapper na lv_chart (RSSI, spectrum)
│   │   │   ├── widget_dialog.h/.c    # Dialogi potwierdzenia
│   │   │   └── widget_toast.h/.c     # Toast / snackbar notifications
│   │   └── assets/
│   │       ├── fonts/                # Skompilowane fonty LVGL (lv_font_conv)
│   │       │   ├── font_small.c      # 12px — listy, statusbar
│   │       │   ├── font_medium.c     # 16px — główny tekst
│   │       │   └── font_large.c      # 22px — nagłówki
│   │       └── icons/                # Skompilowane ikony (lv_img_conv)
│   │           ├── icon_wifi.c
│   │           ├── icon_ble.c
│   │           ├── icon_zigbee.c
│   │           ├── icon_thread.c
│   │           ├── icon_attack.c
│   │           ├── icon_sniff.c
│   │           ├── icon_files.c
│   │           └── icon_settings.c
│   ├── utils/
│   │   ├── CMakeLists.txt
│   │   ├── mac_utils.h/.c            # Parsowanie, formatowanie, randomizacja MAC
│   │   ├── oui_lookup.h/.c           # Baza OUI → producent
│   │   ├── time_utils.h/.c           # Timestamping, RTC
│   │   ├── gps_parser.h/.c           # NMEA parser (UART GPS)
│   │   ├── led_status.h/.c           # LED RGB / buzzer control
│   │   ├── power_mgmt.h/.c           # Battery ADC, sleep modes
│   │   └── ieee80211.h               # Definicje struktur 802.11 (header, IE, itp.)
│   └── cross_protocol/
│       ├── CMakeLists.txt
│       ├── matter_analyzer.h/.c      # Matter discovery i analiza
│       ├── coexistence_test.h/.c     # Testy koegzystencji multi-radio
│       └── spectrum_analyzer.h/.c    # Analiza widma 2.4 GHz
└── data/
    ├── evil_portal/
    │   └── index.html                # Domyślna strona Evil Portal
    ├── oui.csv                       # Baza OUI
    └── scripts/                      # Przykładowe skrypty CLI
        ├── full_scan.txt
        └── pentest_wifi.txt
```

---

## 4. FAZY IMPLEMENTACJI

---

### FAZA 1: FUNDAMENT SYSTEMOWY
**Czas: 3–4 tygodnie** (w tym 1.6: TFT/Touch/LVGL)

#### Faza 1.1: Inicjalizacja projektu
**Pliki:** `CMakeLists.txt`, `sdkconfig.defaults`, `partitions.csv`, `main/main.c`, `main/spectra_config.h`, `main/version.h`

**Szczegóły implementacji:**

1. **Utworzenie projektu ESP-IDF:**
   - `idf.py create-project spectra`
   - Konfiguracja `sdkconfig.defaults`:
     - `CONFIG_IDF_TARGET="esp32c5"`
     - `CONFIG_ESP_WIFI_ENABLE=y`
     - `CONFIG_BT_ENABLED=y`
     - `CONFIG_BT_NIMBLE_ENABLED=y`
     - `CONFIG_IEEE802154_ENABLED=y`
     - `CONFIG_FREERTOS_HZ=1000`
     - `CONFIG_ESP_TASK_WDT_TIMEOUT_S=30`
     - `CONFIG_PARTITION_TABLE_CUSTOM=y`
     - `CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y`
     - `CONFIG_SPIRAM_SUPPORT=y` (PSRAM)
     - `CONFIG_SPIRAM_USE_CAPS_ALLOC=y` — alokacja PSRAM przez `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
     - `CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL=64000` — rezerwuj 64 KB wewnętrznego SRAM dla DMA/ISR
     - `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP=y` — bufory Wi-Fi/LWIP w PSRAM
     - `CONFIG_ESP_WIFI_SOFTAP_SUPPORT=y`
     - `CONFIG_ESP_WIFI_NAN_ENABLE=n`
     - `CONFIG_LWIP_SO_RCVBUF=y`

2. **Tablica partycji (`partitions.csv`):**
   ```
   # Name,    Type, SubType, Offset,   Size,     Flags
   nvs,       data, nvs,     0x9000,   0x6000,
   phy_init,  data, phy,     0xf000,   0x1000,
   factory,   app,  factory, 0x10000,  0x300000,
   ota_0,     app,  ota_0,   0x310000, 0x300000,
   ota_1,     app,  ota_1,   0x610000, 0x300000,
   spiffs,    data, spiffs,  0x910000, 0x100000,
   coredump,  data, coredump,0xA10000, 0x10000,
   ```

3. **`main.c` — Entry point:**
   - `app_main()` → inicjalizacja NVS → inicjalizacja event loop → init storage → init CLI → print banner → start CLI task
   - Banner ASCII art z nazwą „SPECTRA", wersją, info o chipie

4. **`spectra_config.h`:**
   - Makra konfiguracyjne: `SPECTRA_SD_ENABLED`, `SPECTRA_GPS_ENABLED`, `SPECTRA_WEB_UI_ENABLED`
   - Rozmiary buforów: `SPECTRA_PACKET_BUFFER_SIZE 4096`
   - Piny: `SPECTRA_SD_CS_PIN`, `SPECTRA_GPS_TX_PIN`, `SPECTRA_GPS_RX_PIN`, `SPECTRA_LED_PIN`
   - **PSRAM helper macros:**
     ```c
     // Alokacja w PSRAM (fallback do SRAM jeśli PSRAM niedostępny)
     #define SPECTRA_MALLOC_PSRAM(size) \
         heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
     #define SPECTRA_CALLOC_PSRAM(n, size) \
         heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
     #define SPECTRA_REALLOC_PSRAM(ptr, size) \
         heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
     // Alokacja w wewnętrznym SRAM (DMA-capable, ISR-safe)
     #define SPECTRA_MALLOC_DMA(size) \
         heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)
     // Sprawdzenie dostępnego PSRAM
     #define SPECTRA_PSRAM_FREE() \
         heap_caps_get_free_size(MALLOC_CAP_SPIRAM)
     ```
   - **Progi pamięci:**
     - `SPECTRA_PSRAM_LOW_THRESHOLD (256 * 1024)` — 256 KB, ostrzeżenie
     - `SPECTRA_PSRAM_CRITICAL_THRESHOLD (64 * 1024)` — 64 KB, degradacja funkcji

#### Faza 1.2: Storage Manager
**Pliki:** `components/storage/*`

1. **`sd_card.c`:**
   - `sd_card_init()` — inicjalizacja SPI bus, montowanie FATFS, sprawdzenie wolnego miejsca
   - `sd_card_write_file()`, `sd_card_read_file()`, `sd_card_list_dir()`
   - `sd_card_get_free_space()` — odczyt wolnego miejsca
   - `sd_card_deinit()` — bezpieczne odmontowanie
   - Obsługa błędów: brak karty, karta pełna, błąd zapisu

2. **`spiffs_mgr.c`:**
   - `spiffs_init()` — montowanie partycji SPIFFS
   - `spiffs_read_file()`, `spiffs_write_file()`
   - `spiffs_list_dir()`

3. **`nvs_mgr.c`:**
   - `nvs_store_string()`, `nvs_load_string()`
   - `nvs_store_blob()`, `nvs_load_blob()`
   - `nvs_store_int()`, `nvs_load_int()`
   - Namespace: "spectra" dla ustawień aplikacji

4. **`log_rotate.c`:**
   - `log_rotate_check()` — sprawdzenie czy brakuje miejsca
   - `log_rotate_cleanup()` — usunięcie najstarszych plików PCAP
   - Konfiguracja: `LOG_ROTATE_MIN_FREE_MB 50`

#### Faza 1.3: CLI Engine
**Pliki:** `components/cli/cli_engine.h/.c`, `cli_colors.h`

1. **Architektura CLI:**
   - Task UART RX na priorytecie 5 (odrębny task FreeRTOS)
   - Line buffer 256 bajtów z obsługą backspace, strzałek, TAB
   - Ring buffer historii (16 ostatnich komend)
   - Prompt: `spectra> ` (zielony)

2. **System komend:**
   ```c
   typedef struct {
       const char *name;           // np. "scan"
       const char *help;           // opis
       const char *usage;          // np. "wifi scan ap [--band 2.4|5|both] [--passive]"
       esp_err_t (*handler)(int argc, char **argv);
       const cli_cmd_t *subcommands;  // podkomendy (NULL = liść)
       int subcmd_count;
   } cli_cmd_t;
   ```

3. **Rejestracja komend:**
   - `cli_register_command(parent_path, cmd)` — rejestracja w drzewie
   - `cli_dispatch(line)` — parsowanie linii i dispatch do handlera
   - Parsowanie argumentów: `--flag`, `--key value`, pozycyjne argumenty

4. **Komendy systemowe (`cli_commands_system.c`):**
   - `help` — lista wszystkich komend
   - `version` — wersja firmware
   - `sysinfo` — wolna RAM, uptime, temperatura
   - `reboot` — restart
   - `sd ls <path>` — lista plików SD
   - `sd cat <file>` — wyświetlenie pliku
   - `sd rm <file>` — usunięcie pliku
   - `set <key> <value>` — ustawienie konfiguracji
   - `get <key>` — odczyt konfiguracji
   - `script <file>` — wykonanie skryptu z SD

#### Faza 1.4: Packet Engine
**Pliki:** `components/packet_engine/*`

1. **`packet_buffer.c` — Ring Buffer:**
   - Alokacja w PSRAM (`SPECTRA_MALLOC_PSRAM()`) — bufor może mieć 32–128 KB w zależności od trybu
   - Fallback do wewnętrznego SRAM (mniejszy bufor 8 KB) jeśli PSRAM niedostępny
   - Nagłówek ring buffer i metadane w wewnętrznym SRAM (szybszy dostęp, ~128 bytes)
   - Lock-free SPSC (Single Producer Single Consumer) design
   - `packet_buffer_init(size)` — utworzenie bufora
   - `packet_buffer_push(pkt, len, timestamp, type)` — dodanie pakietu
   - `packet_buffer_pop(pkt_out)` — odczyt pakietu
   - `packet_buffer_flush()` — wyczyszczenie
   - `packet_buffer_stats()` — statystyki (fill level, dropped count)
   - Typ pakietu: enum `PKT_TYPE_WIFI`, `PKT_TYPE_BLE`, `PKT_TYPE_802154`

2. **`pcap_writer.c` — PCAP Writer:**
   - `pcap_open(filepath, link_type)` — utworzenie pliku PCAP z nagłówkiem
     - Link types: `DLT_IEEE802_11` (105), `DLT_IEEE802_15_4` (195), `DLT_BLUETOOTH_LE_LL` (251), `DLT_IEEE802_11_RADIO` (127, z radiotap)
   - `pcap_write_packet(handle, data, len, timestamp)` — zapis pakietu
   - `pcap_close(handle)` — zamknięcie pliku
   - Obsługa dużych plików: auto-split po 50 MB
   - Format: standardowy libpcap (magic 0xa1b2c3d4)

3. **`packet_filter.c` — Filtr pakietów:**
   - Konfiguracja filtrów:
     ```c
     typedef struct {
         uint8_t frame_type_mask;       // Management/Control/Data
         uint8_t frame_subtype_mask;    // Beacon/Probe/Deauth/...
         uint8_t src_mac[6];            // filtr MAC źródłowego (0 = any)
         uint8_t dst_mac[6];            // filtr MAC docelowego (0 = any)
         uint8_t bssid[6];             // filtr BSSID (0 = any)
         int8_t  rssi_min;             // minimalne RSSI
         bool    src_mac_enabled;
         bool    dst_mac_enabled;
         bool    bssid_enabled;
         bool    rssi_enabled;
     } packet_filter_t;
     ```
   - `packet_filter_match(filter, packet)` — sprawdzenie czy pakiet pasuje

4. **`packet_stats.c` — Statystyki:**
   - Liczniki per typ/podtyp ramki
   - Histogram RSSI (bins co 5 dBm)
   - Pakiety/sekundę (sliding window 1s)
   - Total bytes captured
   - `packet_stats_print()` — wydruk na CLI

#### Faza 1.5: Utility modules
**Pliki:** `components/utils/*`

1. **`mac_utils.c`:**
   - `mac_to_str(mac, buf)` — MAC bytes → "AA:BB:CC:DD:EE:FF"
   - `str_to_mac(str, mac)` — string → bytes
   - `mac_randomize(mac)` — generowanie losowego MAC (local bit ustawiony)
   - `mac_is_broadcast(mac)` — sprawdzenie
   - `mac_is_multicast(mac)`

2. **`oui_lookup.c`:**
   - Wczytanie skompresowanej bazy OUI z SPIFFS
   - `oui_lookup(mac)` → "Espressif Inc." / "Apple Inc." / "Unknown"
   - Baza: top ~5000 OUI (plik ~100 KB skompresowany)

3. **`time_utils.c`:**
   - `time_get_us()` — timestamp w mikrosekundach (esp_timer)
   - `time_get_iso8601(buf)` — timestamp ISO 8601
   - `time_set_from_gps(nmea_time)` — ustawienie RTC z GPS

4. **`ieee80211.h` — Struktury 802.11:**
   - `ieee80211_hdr` — nagłówek MAC (frame control, duration, addr1/2/3, seq_ctrl)
   - `ieee80211_mgmt_beacon` — beacon frame body
   - `ieee80211_ie` — Information Element (id, len, data)
   - `ieee80211_deauth` — deauth frame body
   - Makra: `IEEE80211_FC_TYPE_MGMT`, `IEEE80211_FC_STYPE_DEAUTH`, itd.

5. **`led_status.c`:**
   - `led_set_mode(LED_MODE_IDLE)` — niebieski pulsujący
   - `led_set_mode(LED_MODE_SCANNING)` — zielony migający
   - `led_set_mode(LED_MODE_ATTACKING)` — czerwony stały
   - `led_set_mode(LED_MODE_CAPTURING)` — żółty migający

#### Faza 1.6: TFT Display + Touch Driver + LVGL Init
**Pliki:** `components/tft_gui/tft_driver.c`, `touch_driver.c`, `gui_main.c`, `gui_theme.c`, `gui_events.c`

1. **`tft_driver.c` — Sterownik ILI9341 (SPI + DMA):**
   - `tft_init()`:
     - Konfiguracja SPI bus (`spi_bus_initialize()` z DMA channel)
     - Dodanie device ILI9341 (`spi_bus_add_device()`, CS pin, max 40 MHz dla stabilności)
     - Sekwencja inicjalizacyjna ILI9341:
       - Software reset → Sleep Out → Display Inversion ON
       - Pixel Format: 16-bit RGB565
       - Memory Access Control (orientacja: landscape 320×240)
       - Column/Page Address Set
       - Display ON
     - Konfiguracja DC pin (Data/Command) jako GPIO output
     - Konfiguracja RST pin + hardware reset sequence
   - `tft_set_backlight(uint8_t brightness)`:
     - PWM na pinie TFT_BL (`ledc_channel_config()`, 5 kHz, 8-bit resolution)
     - `brightness` 0-255 → duty cycle
   - `tft_flush_area(x1, y1, x2, y2, color_data)`:
     - Set Column Address (0x2A) + Set Page Address (0x2B)
     - Memory Write (0x2C) + DMA SPI transfer (`spi_device_queue_trans()`)
     - Callback po zakończeniu DMA → `lv_disp_flush_ready()`
   - `tft_sleep()` / `tft_wake()` — Display OFF/ON + backlight

2. **`touch_driver.c` — Sterownik XPT2046 (SPI):**
   - `touch_init()`:
     - Dodanie device XPT2046 na SPI bus (`spi_bus_add_device()`, osobny CS, ~2 MHz)
     - Opcjonalnie: konfiguracja TOUCH_IRQ pin jako GPIO input z interrupt
   - `touch_read_raw(uint16_t *x, uint16_t *y, bool *pressed)`:
     - Odczyt Z-pressure (0xB1 command) → wykrycie dotyku (threshold)
     - Odczyt X (0xD1 command), Y (0x91 command) — średnia z 4 próbek
     - Debouncing: minimum 2 kolejne odczyty w tym samym obszarze
   - `touch_calibrate()`:
     - 3-punktowa kalibracja (rogi + środek ekranu)
     - Wyświetlenie celowników na TFT
     - Obliczenie macierzy transformacji (affine transform)
     - Zapis współczynników kalibracji do NVS (`touch_cal_data`)
   - `touch_read_calibrated(lv_indev_data_t *data)`:
     - Odczyt raw → transformacja przez macierz kalibracji → koordynaty ekranowe
     - Mapowanie 4096×4096 → 320×240
     - Wypełnienie struktury LVGL (`data->point.x`, `.y`, `data->state`)

3. **`gui_main.c` — Inicjalizacja LVGL i główna pętla:**
   - `gui_init()`:
     - `lv_init()` — inicjalizacja biblioteki LVGL
     - Alokacja buforów wyświetlania:
       - **⚠ KRYTYCZNE:** Flush buffers MUSZĄ być w wewnętrznym SRAM (DMA-capable), NIE w PSRAM!
       - Double buffer: 2 × `LV_HOR_RES × 20 linii × 2 bytes` = 2 × 12.8 KB → `SPECTRA_MALLOC_DMA(12800)`
       - Alternatywnie: jeśli brak wewnętrznego SRAM, single buffer 1 × 12.8 KB
       - `lv_display_create(320, 240)`
       - `lv_display_set_buffers(disp, buf1, buf2, size, LV_DISPLAY_RENDER_MODE_PARTIAL)`
     - **LVGL heap allocator → PSRAM:**
       - `LV_MEM_CUSTOM=1` w `lv_conf.h`
       - Custom `lv_malloc` → `SPECTRA_MALLOC_PSRAM()`
       - Custom `lv_free` → `free()` (heap_caps_free działa automatycznie)
       - Cały widget tree, style, animacje, chart data → PSRAM
     - Rejestracja flush callback → `tft_flush_area()`
     - Rejestracja input device (touch):
       - `lv_indev_create()` typ `LV_INDEV_TYPE_POINTER`
       - Read callback → `touch_read_calibrated()`
     - Ustawienie theme → `gui_theme_apply()`
   - `gui_task(void *pvParameters)` — FreeRTOS task:
     - Priorytet: 3 (niższy niż radio tasks)
     - Stack: 8192 bytes
     - Pętla: `lv_timer_handler()` co 10-20 ms (`lv_tick_inc()`)
     - Obsługa sleep timeout (wyłączenie backlight po X sekund bez dotyku)
   - `gui_navigate_to(screen_id)` — nawigacja między ekranami z animacją slide
   - `gui_show_toast(msg, type)` — wyświetlenie powiadomienia (info/warning/error)
   - `gui_lock()` / `gui_unlock()` — mutex dla operacji GUI z innych tasks

4. **`gui_theme.c` — Definicja wyglądu:**
   - Theme dark (domyślny):
     - Background: `#1A1A2E` (ciemny granat)
     - Primary: `#16213E` (panele)
     - Accent: `#0F3460` (aktywne elementy)
     - Text: `#E0E0E0` (jasny szary)
     - Success: `#00C853`, Warning: `#FFD600`, Error: `#FF1744`, Attack: `#FF5722`
   - Theme light (opcjonalny):
     - Background: `#FAFAFA`, Text: `#212121`
   - Style definitions: `lv_style_t` dla przycisków, list, chart, statusbar
   - Font assignments: small (12px) → listy, medium (16px) → body, large (22px) → nagłówki

5. **`gui_events.c` — Event bus (GUI ↔ silniki radiowe):**
   - Thread-safe komunikacja przez FreeRTOS queue:
     - `gui_event_queue` — kolejka zdarzeń (128 elementów)
     - Typy zdarzeń:
       ```c
       typedef enum {
           GUI_EVT_WIFI_SCAN_RESULT,      // nowy AP znaleziony
           GUI_EVT_WIFI_SCAN_DONE,        // skan zakończony
           GUI_EVT_WIFI_SNIFFER_PACKET,   // pakiet przechwycony (summary)
           GUI_EVT_WIFI_HANDSHAKE,        // handshake/PMKID captured
           GUI_EVT_WIFI_ROGUE_DETECTED,   // rogue AP wykryty
           GUI_EVT_BLE_DEVICE_FOUND,      // nowe urządzenie BLE
           GUI_EVT_BLE_GATT_RESULT,       // wynik GATT discovery
           GUI_EVT_ZB_NETWORK_FOUND,      // sieć Zigbee
           GUI_EVT_ZB_FRAME_DECODED,      // zdekodowana ramka
           GUI_EVT_THREAD_NETWORK_FOUND,  // sieć Thread
           GUI_EVT_GPS_FIX,               // GPS fix update
           GUI_EVT_ATTACK_STATUS,         // zmiana statusu ataku
           GUI_EVT_SD_SPACE_LOW,          // mało miejsca na SD
           GUI_EVT_SYSTEM_ERROR,          // błąd systemowy
       } gui_event_type_t;
       ```
   - `gui_event_send(type, data, data_len)` — wysłanie z dowolnego task
   - `gui_event_process()` — odbiór w GUI task, dispatch do aktywnego ekranu
   - Kopiowanie danych (nie wskaźniki!) — bezpieczeństwo między tasks

---

### FAZA 2: SILNIK WI-FI
**Czas: 3–4 tygodnie**

#### Faza 2.1: Inicjalizacja i skanowanie Wi-Fi
**Pliki:** `wifi_engine.c`, `wifi_scanner.c`, `wifi_types.h`

1. **`wifi_engine.c`:**
   - `wifi_engine_init()`:
     - `esp_netif_init()`, `esp_event_loop_create_default()`
     - `esp_netif_create_default_wifi_sta()`, `esp_netif_create_default_wifi_ap()`
     - `esp_wifi_init(&wifi_init_config)` z `WIFI_INIT_CONFIG_DEFAULT()`
     - `esp_wifi_set_storage(WIFI_STORAGE_RAM)`
     - `esp_wifi_set_mode(WIFI_MODE_STA)` (domyślny)
   - `wifi_engine_set_band(band)`:
     - `WIFI_BAND_2G`, `WIFI_BAND_5G`, `WIFI_BAND_AUTO`
     - Użycie `esp_wifi_set_band_mode()`
   - `wifi_engine_set_channel(channel, bandwidth)`:
     - `esp_wifi_set_channel(primary, secondary)`
   - `wifi_engine_get_mac(mac_out)` / `wifi_engine_set_mac(mac)`
   - `wifi_engine_deinit()`

2. **`wifi_scanner.c`:**
   - **Skan AP:**
     - `wifi_scan_ap(config)` → `esp_wifi_scan_start()` z parametrami
     - Wynik: lista `wifi_ap_info_t` (SSID, BSSID, kanał, RSSI, authmode, 802.11 type)
     - Sortowanie po RSSI
     - Obsługa obu pasm: skan 2.4 GHz (ch 1-14) + 5 GHz (ch 36-165)
     - **PSRAM:** Lista wyników skanowania (`wifi_ap_info_t[]`, ~256 bytes/AP, max 128 AP = ~32 KB) alokowana w PSRAM przez `SPECTRA_CALLOC_PSRAM()`. Lista stacji (`wifi_station_info_t[]`, max 256 stacji × ~128 bytes = ~32 KB) również w PSRAM. Occupancy map kanałów (tablica int, ~200 bytes) — może zostać w wewnętrznym SRAM.
   - **Skan stacji:**
     - Tryb promiscuous, zbieranie probe request/response
     - Korelacja stacji z AP na podstawie BSSID
     - Lista `wifi_station_info_t` (MAC, RSSI, associated AP, probe SSIDs)
   - **Skan kanałów:**
     - Iteracja po kanałach, pomiar energii (czas na kanale: 100ms)
     - Wynik: tablica occupancy per kanał
   - **CLI komendy:**
     - `wifi scan ap` — skan obu pasm
     - `wifi scan ap --band 2.4` — tylko 2.4 GHz
     - `wifi scan ap --band 5` — tylko 5 GHz
     - `wifi scan sta` — skan stacji
     - `wifi scan channels` — mapa zajętości kanałów

#### Faza 2.2: Wi-Fi Sniffer
**Pliki:** `wifi_sniffer.c`, `wifi_packet_builder.c`

1. **`wifi_sniffer.c`:**
   - `wifi_sniffer_start(channel, filter, callback)`:
     - `esp_wifi_set_promiscuous(true)`
     - `esp_wifi_set_promiscuous_rx_cb(sniffer_callback)`
     - `esp_wifi_set_promiscuous_filter(&filter)` — typy ramek
     - `esp_wifi_set_promiscuous_ctrl_filter(&ctrl_filter)`
   - **Promiscuous callback:**
     ```c
     void sniffer_callback(void *buf, wifi_promiscuous_pkt_type_t type) {
         wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
         // Parsowanie nagłówka: pkt->rx_ctrl (RSSI, channel, sig_len, itp.)
         // Dane ramki: pkt->payload
         // Kolejkowanie do ring buffer lub bezpośredni zapis PCAP
         // WAŻNE: callback w kontekście Wi-Fi driver task — NIE blokować!
         // Użycie xQueueSend() do przekazania do task przetwarzania
     }
     ```
   - **Channel hopping task:**
     - Opcjonalny task przeskakujący między kanałami co N ms
     - Konfigurowalny czas na kanale (domyślnie 200ms)
     - Opcja: tylko wybrane kanały
   - `wifi_sniffer_stop()` — wyłączenie promiscuous mode
   - **Tryby sniffingu:**
     - `SNIFF_MODE_ALL` — wszystkie ramki
     - `SNIFF_MODE_BEACON` — tylko beacony
     - `SNIFF_MODE_PROBE_REQ` — tylko probe request
     - `SNIFF_MODE_DEAUTH` — tylko deauth/disassoc
     - `SNIFF_MODE_EAPOL` — tylko EAPOL (4-way handshake)
     - `SNIFF_MODE_DATA` — tylko ramki danych
   - **PSRAM:** Przechwycone pakiety kolejkowane do ring buffer w PSRAM (Faza 1.4). Przy intensywnym sniffingu (>500 pkt/s) bufor w PSRAM alokowany na 64–128 KB. Promiscuous callback kopiuje dane do tymczasowego bufora DMA-capable (wewnętrzny SRAM, `SPECTRA_MALLOC_DMA()`), następnie task przetwarzania przenosi do ring buffer w PSRAM.

2. **CLI komendy:**
   - `wifi sniff start [--channel N] [--filter beacon|probe|deauth|eapol|all] [--pcap filename]`
   - `wifi sniff stop`
   - `wifi sniff status` — ile pakietów przechwycono, aktualne statystyki
   - `wifi monitor [--channel N]` — live packet monitor (typy + liczniki)

#### Faza 2.3: Wi-Fi Ataki — Deauth & Beacon
**Pliki:** `wifi_attacks.c`, `wifi_deauth.c`, `wifi_beacon.c`

1. **`wifi_deauth.c`:**
   - `wifi_deauth_build(src_mac, dst_mac, bssid, reason_code, buffer)`:
     - Budowanie ramki deauth 802.11:
       - Frame Control: type=0 (mgmt), subtype=0xC (deauth)
       - Duration: 0
       - addr1=dst (target STA lub broadcast), addr2=src (spoofed AP), addr3=bssid
       - Reason code (np. 7 = Class 3 frame from nonassociated STA)
   - `wifi_deauth_flood(target_ap, reason_code)`:
     - Pętla: wysyłanie deauth do broadcast z BSSID = target AP
     - Użycie `esp_wifi_80211_tx(WIFI_IF_STA, buffer, len, false)`
     - Rate: ~100 pakietów/sekundę
   - `wifi_deauth_targeted(target_ap, target_sta, reason_code)`:
     - Deauth do konkretnego STA
     - Wysyłanie w obu kierunkach: AP→STA i STA→AP (spoofing obu stron)

2. **`wifi_beacon.c`:**
   - `wifi_beacon_build(ssid, bssid, channel, auth_type, buffer)`:
     - Frame Control: type=0 (mgmt), subtype=0x8 (beacon)
     - Fixed params: timestamp, beacon interval (100 TU), capability info
     - Tagged params: SSID IE, Supported Rates IE, DS Parameter Set IE, RSN IE
   - `wifi_beacon_spam_list(ssid_list, count)`:
     - Task generujący beacony dla każdego SSID z listy
     - Losowy BSSID dla każdego SSID
     - Wysyłanie ~10 beaconów/s per SSID
   - `wifi_beacon_spam_random(count)`:
     - Generowanie losowych SSID (8–32 znaków)
   - `wifi_beacon_clone(target_ap)`:
     - Kopiowanie parametrów prawdziwego AP (SSID, channel, capabilities)
     - Zmiana BSSID
   - **PSRAM:** Lista SSID dla beacon spam (`char[][33]`, max 1000 SSID = ~33 KB) alokowana w PSRAM. Pre-built beacon frames cache (max 100 × ~256 bytes = ~25 KB) w PSRAM — unikanie rebuildu ramki przy każdym wysyłaniu. Tablica klientów (lista znanych STA per AP, max 64 AP × 16 STA × ~10 bytes = ~10 KB) w PSRAM.

3. **CLI komendy:**
   - `wifi attack deauth --target <BSSID> [--sta <MAC>] [--reason N] [--count N]`
   - `wifi attack beacon-spam --mode list|random|clone [--ssid-file <file>] [--count N]`
   - `wifi attack probe-flood [--count N]`
   - `wifi attack auth-flood --target <BSSID> [--count N]`
   - `wifi attack stop` — zatrzymanie wszystkich ataków

#### Faza 2.4: Evil Portal / Evil Twin / Karma
**Pliki:** `wifi_evil_portal.c`, `wifi_evil_twin.c`, `wifi_karma.c`

1. **`wifi_evil_portal.c`:**
   - **Soft-AP setup:**
     - `esp_wifi_set_mode(WIFI_MODE_AP)` lub `WIFI_MODE_APSTA`
     - Konfiguracja: SSID, kanał, brak hasła (Open)
   - **DNS Hijacking:**
     - Prosty DNS server na porcie 53
     - Odpowiedź na WSZYSTKIE zapytania DNS adresem IP ESP32 (captive portal behavior)
   - **HTTP Server:**
     - `esp_http_server` na porcie 80
     - Serwowanie pliku `index.html` z karty SD
     - Endpoint POST `/login` — przechwytywanie credentials
     - Logowanie credentials: UART + SD card + wyświetlanie w CLI
     - **PSRAM:** Bufor HTTP response alokowany w PSRAM (`SPECTRA_MALLOC_PSRAM()`) — template HTML może mieć do 64 KB. Tablica przechwyconych credentials (`evil_portal_cred_t[]`, max 64 wpisów) przechowywana w PSRAM (~8 KB). Bufory TCP/IP (LWIP) automatycznie w PSRAM dzięki `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP`.
   - **Captive Portal Detection:**
     - Odpowiedź na `/generate_204`, `/hotspot-detect.html`, `/connecttest.txt`
     - Redirect na portal

2. **`wifi_evil_twin.c`:**
   - Orkiestracja: skan AP → wybór celu → klonowanie → deauth oryginalnego → evil portal
   - Jednoczesna praca: AP (klonowany) + deauth oryginalnego AP (w tle)
   - Automatyczne przełączanie kanału AP na kanał celu

3. **`wifi_karma.c`:**
   - Nasłuchiwanie probe request w promiscuous mode
   - Automatyczne tworzenie Soft-AP z SSID z probe request
   - Obsługa wielu SSID jednocześnie (rotacja)

4. **CLI komendy:**
   - `wifi evil-portal start --ssid <name> [--html <file>] [--channel N]`
   - `wifi evil-twin start --target <BSSID>`
   - `wifi karma start`
   - `wifi evil-portal stop` / `wifi evil-twin stop` / `wifi karma stop`
   - `wifi evil-portal creds` — wyświetlenie przechwyconych danych

#### Faza 2.5: EAPOL/PMKID Capture + Detector
**Pliki:** `wifi_handshake.c`, `wifi_detector.c`, `wifi_csi.c`

1. **`wifi_handshake.c`:**
   - Filtr EAPOL w promiscuous mode (EtherType 0x888E)
   - Detekcja 4-way handshake (msg 1→4)
   - Ekstrakcja PMKID z msg 1 (RSN IE, PMKID w Key Data)
   - Zapis do PCAP kompatybilnego z hashcat/aircrack-ng
   - Notyfikacja CLI przy przechwyceniu handshake
   - **PSRAM:** Bufor handshake state per AP (`handshake_state_t`, ~512 bytes × max 32 AP = 16 KB) alokowany w PSRAM. Pozwala śledzić wiele handshake'ów jednocześnie bez obciążania wewnętrznego SRAM.

2. **`wifi_detector.c`:**
   - **PineScan** — detekcja WiFi Pineapple:
     - Analiza vendor OUI (znane OUI pineapple)
     - Open AP od producenta, który normalnie nie tworzy open AP
   - **Pwnagotchi Detection:**
     - Parsowanie beacon payload (JSON w custom IE)
   - **Multi-SSID Detection:**
     - Wykrywanie wielu SSID z jednego MAC
   - **Deauth Detection:**
     - Zliczanie deauth ramek, alarm przy threshold

3. **`wifi_csi.c`:**
   - `esp_wifi_set_csi(true)`
   - `esp_wifi_set_csi_config(&csi_config)`
   - Callback CSI → zapis danych do pliku CSV na SD
   - Wizualizacja amplitudy subcarrierów w CLI (ASCII graph)
   - **PSRAM:** Bufor CSI data w PSRAM — każda próbka CSI to ~128 subcarrierów × 2 bytes (I/Q) = 256 bytes. Ring buffer na 512 próbek (~128 KB) alokowany przez `SPECTRA_MALLOC_PSRAM()` dla wizualizacji spectrum i analizy w czasie rzeczywistym.

4. **CLI komendy:**
   - `wifi capture handshake --target <BSSID> [--pcap <file>] [--deauth]`
   - `wifi detect start` — uruchomienie detekcji rogue AP
   - `wifi csi start --channel N [--file <csv>]`

---

### FAZA 3: SILNIK BLE
**Czas: 2–3 tygodnie**

#### Faza 3.1: Inicjalizacja i skanowanie BLE
**Pliki:** `ble_engine.c`, `ble_scanner.c`, `ble_types.h`

1. **`ble_engine.c`:**
   - Inicjalizacja NimBLE stack:
     - `nimble_port_init()`
     - `ble_hs_cfg` konfiguracja (security, bonding disabled dla testów)
     - `nimble_port_freertos_init(ble_host_task)`
   - `ble_engine_set_random_addr()` — ustawienie losowego adresu
   - `ble_engine_set_addr(addr)` — ustawienie konkretnego adresu (MAC spoofing)

2. **`ble_scanner.c`:**
   - **Passive scan:**
     - `ble_gap_disc()` z parametrami: passive, duration, filter_policy
     - Callback: zbieranie advertising data
   - **Active scan:**
     - Wysyłanie scan request, zbieranie scan response
   - **Parsowanie AD structures:**
     - Flags, Local Name, TX Power, Service UUIDs, Manufacturer Specific Data
     - Dekodowanie manufacturer data: Apple (company ID 0x004C), Google (0x00E0), Samsung, Microsoft
   - **Wynik:**
     ```c
     typedef struct {
         uint8_t addr[6];
         uint8_t addr_type;      // public/random/rpa
         int8_t  rssi;
         char    name[32];
         uint16_t company_id;
         char    manufacturer[32];
         uint8_t ad_data[64];
         uint8_t ad_len;
         uint32_t last_seen;     // timestamp
         uint32_t adv_count;     // ile razy widziany
     } ble_device_info_t;
     ```
   - Lista urządzeń z deduplication (po adresie)
   - **PSRAM:** Lista urządzeń BLE (`ble_device_info_t[]`) alokowana w PSRAM — każdy wpis ~160 bytes, max 256 urządzeń = ~40 KB. W gęstym środowisku BLE (centrum handlowe, konferencja) lista może rosnąć dynamicznie. Używaj `SPECTRA_REALLOC_PSRAM()` do rozszerzania tablicy. AD raw data (do 256 bytes per urządzenie) również w PSRAM.

3. **CLI komendy:**
   - `ble scan [--duration N] [--active] [--filter <name|mac>]`
   - `ble scan stop`
   - `ble devices` — lista wykrytych urządzeń

#### Faza 3.2: GATT Client + Sniffer
**Pliki:** `ble_gatt_client.c`, `ble_sniffer.c`

1. **`ble_gatt_client.c`:**
   - `ble_gatt_connect(addr, addr_type)` → `ble_gap_connect()`
   - **Service Discovery:**
     - `ble_gattc_disc_all_svcs()` — lista serwisów (UUID, handle range)
     - `ble_gattc_disc_all_chrs()` — lista characteristics per serwis
     - `ble_gattc_disc_all_dscs()` — lista descriptors per characteristic
   - **Read/Write:**
     - `ble_gattc_read()` — odczyt wartości characteristic
     - `ble_gattc_write_flat()` — zapis wartości
     - `ble_gattc_write_no_rsp_flat()` — zapis bez odpowiedzi
   - **Notifications/Indications:**
     - Subscribe do notifications (zapis 0x0001 na CCCD)
     - Logger: timestamp + handle + wartość
   - **PSRAM:** Drzewo GATT (services → characteristics → descriptors) alokowane w PSRAM — pełne discovery urządzenia z wieloma serwisami może zająć 4–16 KB. Bufor notification log (ring buffer, max 1024 wpisów × ~64 bytes = ~64 KB) w PSRAM. Wartości odczytane z characteristics cache'owane w PSRAM.

2. **`ble_sniffer.c`:**
   - Ciągły scan z logowaniem wszystkich advertisement PDU
   - Dekodowanie typów PDU: ADV_IND, ADV_DIRECT_IND, ADV_NONCONN_IND, SCAN_REQ, SCAN_RSP, CONNECT_IND
   - Extended advertising PDU (BT5): AUX_ADV_IND, AUX_SCAN_RSP, AUX_CONNECT_RSP
   - Zapis do PCAP (DLT_BLUETOOTH_LE_LL)
   - Tracking: śledzenie urządzenia po advertising data nawet przy zmianie MAC (RPA)
   - **PSRAM:** Log advertisement PDU w ring buffer PSRAM (32–64 KB). Tablica tracking fingerprints (hash AD data → MAC history, max 128 urządzeń) w PSRAM (~8 KB).

3. **CLI komendy:**
   - `ble connect <addr> [--type public|random]`
   - `ble gatt discover`
   - `ble gatt read <handle>`
   - `ble gatt write <handle> <hex_data>`
   - `ble gatt subscribe <handle>`
   - `ble sniff start [--pcap <file>]`
   - `ble sniff stop`

#### Faza 3.3: BLE Ataki
**Pliki:** `ble_spam.c`, `ble_beacon.c`, `ble_fuzzer.c`, `ble_dos.c`, `ble_hid.c`, `ble_replay.c`

1. **`ble_spam.c`:**
   - **Apple (Sour Apple):**
     - Advertising data imitujący AirPods/AirTag: company ID 0x004C
     - Rotation MAC co 200ms (nowe urządzenie)
     - Typ: ADV_NONCONN_IND
     - Payload: Apple Proximity Pairing, typ urządzenia (0x01=AirPods, 0x05=AirTag, itp.)
   - **Samsung:**
     - Company ID 0x0075
     - Payload: Samsung proprietary BLE advertisement
   - **Google Fast Pair:**
     - Company ID 0x00E0, Service UUID 0xFE2C
     - Model ID, TX Power
   - **Microsoft Swift Pair:**
     - Company ID 0x0006
     - Payload: Swift Pair specific data
   - **Universal:**
     - Task uruchamiający wszystkie powyższe jednocześnie, rotacja co 100ms

2. **`ble_beacon.c`:**
   - **iBeacon Spoofing:**
     - UUID, Major, Minor, TX Power — konfigurowalne
     - Advertising data format Apple iBeacon
   - **Eddystone Spoofing:**
     - Eddystone-URL, Eddystone-UID, Eddystone-TLM

3. **`ble_fuzzer.c`:**
   - Po połączeniu z urządzeniem:
     - Wysyłanie losowych danych do każdego writable characteristic
     - Wysyłanie oversized payloads (MTU overflow test)
     - Wysyłanie danych do read-only characteristics (write test)
     - Invalid handle access
   - Logowanie odpowiedzi i crashy urządzenia

4. **`ble_dos.c`:**
   - **Connection flood:**
     - Ciągłe connect → disconnect do target urządzenia
   - **Advertisement flood:**
     - Masowe advertising z losowymi MAC (zaśmiecanie skanera)

5. **`ble_hid.c`:**
   - Emulacja BLE HID Keyboard
   - GATT serwis: HID over GATT (HOGP)
   - Wysyłanie sekwencji klawiszy (payload z pliku na SD)
   - Ducky Script parser (prosty podzbiór)
   - **PSRAM:** Bufor Ducky Script po wczytaniu z SD (max 16 KB skrypt → PSRAM). Parsed command list (`hid_cmd_t[]`) w PSRAM. Bufor advertising data dla ble_spam (tablica 6 typów × 64 bytes payload w PSRAM, rotowane przez timer).

6. **`ble_replay.c`:**
   - Przechwycenie advertising data urządzenia
   - Retransmisja z tym samym payload ale innym MAC (test detekcji klonowania)

7. **CLI komendy:**
   - `ble spam apple|samsung|google|microsoft|all [--duration N]`
   - `ble spam stop`
   - `ble beacon ibeacon --uuid <UUID> --major N --minor N`
   - `ble beacon eddystone-url --url <URL>`
   - `ble fuzz <addr> [--type random|overflow|invalid]`
   - `ble dos <addr> [--mode connect-flood|adv-flood]`
   - `ble hid start <addr> --script <file>`
   - `ble replay <addr>`

---

### FAZA 4: SILNIK IEEE 802.15.4 (ZIGBEE + THREAD)
**Czas: 3–4 tygodnie**

#### Faza 4.1: Inicjalizacja 802.15.4 + Sniffer
**Pliki:** `zigbee_engine.c`, `zigbee_sniffer.c`, `zigbee_types.h`

1. **`zigbee_engine.c`:**
   - Inicjalizacja IEEE 802.15.4 radio:
     - `esp_ieee802154_enable()` — włączenie subsystemu 802.15.4
     - `esp_ieee802154_set_channel(channel)` — kanał 11–26
     - `esp_ieee802154_set_panid(panid)`
     - `esp_ieee802154_set_short_address(addr)`
     - `esp_ieee802154_set_extended_address(addr)`
   - Tryb pracy:
     - `esp_ieee802154_set_rx_when_idle(true)` — ciągły odbiór
     - `esp_ieee802154_receive()` — rozpoczęcie nasłuchu
   - Konfiguracja TX power: `esp_ieee802154_set_txpower(power)`

2. **`zigbee_sniffer.c`:**
   - **Raw frame sniffer:**
     - Callback `esp_ieee802154_receive_done()`:
       ```c
       void esp_ieee802154_receive_done(uint8_t *frame, esp_ieee802154_frame_info_t *frame_info) {
           // frame[0] = length
           // frame[1...] = PHY payload (MAC header + payload + FCS)
           // frame_info: channel, rssi, timestamp
           // Kolejkowanie do packet_buffer
       }
       ```
     - Zapis do PCAP (DLT_IEEE802_15_4 = 195)
   - **Channel hopping:**
     - Skanowanie kanałów 11–26 z konfigurowalnym czasem na kanale
   - **Energy Detection:**
     - `esp_ieee802154_energy_detect(duration)` per kanał
     - Callback `esp_ieee802154_energy_detect_done()`
   - **PSRAM:** Przechwycone ramki 802.15.4 buforowane w ring buffer PSRAM (32–64 KB). Ramki 802.15.4 mają max 127 bytes, ale przy szybkim channel hopping z dekodowaniem wielu warstw, bufor musi pomieścić ~500-1000 ramek. Wyniki energy detection (16 kanałów × historia) w PSRAM.

3. **CLI komendy:**
   - `zigbee sniff start [--channel N] [--pcap <file>]`
   - `zigbee sniff stop`
   - `zigbee scan energy` — energia na kanałach 11–26
   - `zigbee scan networks [--channel N]` — active scan (beacon request)

#### Faza 4.2: Zigbee Decoder + Scanner
**Pliki:** `zigbee_decoder.c`, `zigbee_scanner.c`

1. **`zigbee_decoder.c`:**
   - **MAC layer (IEEE 802.15.4):**
     - Frame Control: frame type (Beacon/Data/Ack/Command), security, pending, ack_req, PAN compression
     - Addressing: PAN ID, short/extended addresses
     - Security header: Security Level, Key ID Mode, Frame Counter, Key Source
   - **NWK layer (Zigbee):**
     - Frame Control: frame type, protocol version, discover route, security
     - Destination/Source address (16-bit network address)
     - NWK security: security level, key sequence, frame counter, key type
   - **APS layer:**
     - Frame Control: frame type (Data/Command/Ack), delivery mode, security
     - APS Cluster ID, Profile ID, Source/Destination endpoint
   - **ZCL layer:**
     - Frame Control: direction, disable default response
     - Manufacturer code (if present)
     - Sequence number, Command ID
     - Dekodowanie znanych komend: On/Off, Level Control, Color Control, Temperature Measurement
   - **Wydruk na CLI:**
     ```
     [CH:15] [RSSI:-45] MAC: Data, PAN:0x1234, Src:0xABCD → Dst:0x0000
       NWK: Data, Src:0xABCD → Dst:0x0001, Security:AES-CCM-128
       APS: Data, Cluster:0x0006 (On/Off), Ep:1→1, Profile:0x0104 (HA)
       ZCL: Cluster Specific, Cmd:0x01 (On)
     ```

2. **`zigbee_scanner.c`:**
   - **Active Scan:**
     - Wysyłanie MAC Beacon Request command na każdym kanale
     - Zbieranie Beacon frames z informacjami o sieci
   - **Network map:**
     - Lista: PAN ID, Extended PAN ID, Channel, Coordinator Address, Stack Profile, Permit Join
   - **Device tracking:**
     - Mapowanie short address → extended address (z obserwacji ramek)
     - Relacja parent-child
   - **PSRAM:** Network map (tablica `zigbee_network_t[]`, max 32 sieci × ~128 bytes = ~4 KB) i device tracking map (max 256 urządzeń × ~64 bytes = ~16 KB) alokowane w PSRAM. Bufor zdekodowanych ramek (cache ostatnich 512 ramek z pełnym decode, ~128 KB) w PSRAM — potrzebny do wyświetlania historii w GUI i CLI.

3. **CLI komendy:**
   - `zigbee scan networks`
   - `zigbee scan devices --channel N`
   - `zigbee decode on|off` — włączenie/wyłączenie dekodera w sniffer

#### Faza 4.3: Zigbee Ataki
**Pliki:** `zigbee_attacks.c`, `zigbee_touchlink.c`, `zigbee_key_extract.c`, `zigbee_fuzzer.c`, `zigbee_inject.c`

1. **`zigbee_inject.c`:**
   - `zigbee_inject_raw(frame, len)`:
     - `esp_ieee802154_transmit(frame, false)` (false = no CCA)
     - Callback `esp_ieee802154_transmit_done()` dla potwierdzenia
   - Budowanie surowych ramek 802.15.4:
     - MAC header builder
     - FCS calculation (CRC-16/CCITT — w HW)

2. **`zigbee_key_extract.c`:**
   - Nasłuchiwanie Transport Key command w APS wartstwie
   - Dekrypcja klucza sieciowego jeśli używany jest well-known Trust Center Link Key
     - Default TC Link Key: `5A:69:67:42:65:65:41:6C:6C:69:61:6E:63:65:30:39` ("ZigBeeAlliance09")
   - Logowanie przechwyconych kluczy z metadata (PAN, timestamp)

3. **`zigbee_touchlink.c`:**
   - Implementacja Touchlink initiator:
     - Scan Request → Scan Response
     - Identify Request (wymuszona identyfikacja urządzenia)
     - Reset to Factory New Request
     - Network Join Router/End Device Request
   - Konfigurowalny TX power (touchlink wymaga bliskiej odległości)

4. **`zigbee_fuzzer.c`:**
   - Generowanie zniekształconych ramek:
     - Invalid frame control bits
     - Truncated frames
     - Oversized frames (>127 bytes)
     - Invalid FCS
     - Malformed security headers
     - Invalid NWK/APS/ZCL payloads
   - Wysyłanie do wybranego urządzenia (short address)
   - Monitorowanie odpowiedzi (lub braku — crash)

5. **`zigbee_attacks.c`:**
   - **Replay:** przechwycenie + retransmisja ramki
   - **PAN Conflict:** ogłoszenie się koordynatorem z tym samym PAN ID
   - **Channel Jamming:** ciągłe wysyłanie ramek na kanale (carrier sense override)
   - **DoS Coordinator:** flood ramkami data/command do koordynatora
   - **PSRAM:** Bufor replay (tablica przechwyconych ramek, max 64 × 128 bytes = 8 KB) w PSRAM. Bufor fuzzera (generowane ramki testowe) w PSRAM. Lista Touchlink-capable urządzeń (max 32 × ~64 bytes) w PSRAM.

6. **CLI komendy:**
   - `zigbee inject raw <hex_frame>`
   - `zigbee attack replay --channel N [--count N]`
   - `zigbee attack touchlink scan`
   - `zigbee attack touchlink identify <addr>`
   - `zigbee attack touchlink reset <addr>`
   - `zigbee attack jam --channel N [--duration N]`
   - `zigbee attack keysniff --channel N [--pcap <file>]`
   - `zigbee fuzz <short_addr> --channel N [--mode random|truncated|oversized]`

#### Faza 4.4: Thread Engine
**Pliki:** `thread_engine.c`, `thread_scanner.c`, `thread_sniffer.c`, `thread_attacks.c`, `thread_analyzer.c`

1. **`thread_engine.c`:**
   - Wykorzystanie tego samego radio 802.15.4 co Zigbee (nie mogą działać jednocześnie!)
   - Mutex do przełączania między trybem Zigbee a Thread
   - Inicjalizacja OpenThread stack (esp_openthread):
     - `esp_openthread_init()`
     - Konfiguracja jako „sniffer" lub „detached" (nie dołączający do sieci)

2. **`thread_scanner.c`:**
   - **MLE Discovery:**
     - Wysyłanie MLE Discovery Request
     - Parsowanie MLE Discovery Response (Network Name, XPAN ID, channel, itp.)
   - **Active Scan:** przez OpenThread API
   - **Energy Scan:** przez OpenThread API
   - Wynik: lista sieci Thread z parametrami

3. **`thread_sniffer.c`:**
   - Tryb raw 802.15.4 sniffer z dekodowaniem Thread-specyficznym:
     - MLE messages: Link Request, Link Accept, Advertisement, Parent Request/Response
     - CoAP messages (port 61631): TMF commands
     - 6LoWPAN header decompression (basic)
   - Zapis do PCAP

4. **`thread_attacks.c`:**
   - **MLE Injection:** wysyłanie spreparowanych MLE wiadomości
   - **Partition Attack:** próba podziału sieci (fałszywe MLE Advertisement z innym Partition ID)
   - **Replay:** powtórzenie MLE/CoAP ramek
   - **DoS:** flood ramkami na Thread adresy

5. **`thread_analyzer.c`:**
   - Mapowanie topologii z przechwyconych MLE messages
   - Identyfikacja Leader, Router ID, RLOC16
   - Tablica routingu mesh (z MLE Route TLV)

6. **CLI komendy:**
   - `thread scan networks`
   - `thread sniff start [--channel N] [--pcap <file>]`
   - `thread sniff stop`
   - `thread attack mle-inject <type> [--target <addr>]`
   - `thread attack dos --channel N`
   - `thread analyze topology --channel N`

---

### FAZA 5: CROSS-PROTOCOL + WEB UI + GPS
**Czas: 5–7 tygodni** (w tym ~3–4 tygodnie na TFT GUI screens)

#### Faza 5.1: Cross-Protocol Module
**Pliki:** `cross_protocol/*`

1. **`matter_analyzer.c`:**
   - Matter device discovery:
     - BLE: szukanie serwisu Matter commissioning (UUID: 0xFFF6)
     - Wi-Fi: mDNS discovery (_matter._tcp)
   - Analiza commissioning flow (PASE/CASE)
   - Logowanie Matter payloads (nie dekodowanie — wystarczy zapis)

2. **`coexistence_test.c`:**
   - Test jednoczesnej pracy:
     - Wi-Fi scan + BLE scan → pomiar wpływu na throughput
     - 802.15.4 sniff + Wi-Fi promiscuous → pomiar packet loss
     - Wi-Fi TX (beacon spam) + 802.15.4 sniff → pomiar zakłóceń
   - Raport: CSV z timestampami i metrykami

3. **`spectrum_analyzer.c`:**
   - Użycie WiFi CSI + 802.15.4 Energy Detect + BLE RSSI
   - Mapa zajętości pasma 2.4 GHz (2400–2483.5 MHz)
   - ASCII wykres w CLI

#### Faza 5.2: Web UI
**Pliki:** `web_ui/*`

1. **`web_server.c`:**
   - `httpd_start()` na porcie 80 (kiedy SPECTRA jest w trybie AP)
   - Endpointy:
     - `GET /` → dashboard.html
     - `GET /api/status` → JSON ze statusem modułów
     - `GET /api/wifi/aps` → JSON z listą AP
     - `GET /api/ble/devices` → JSON z listą BLE devices
     - `GET /api/zigbee/networks` → JSON z listą sieci Zigbee
     - `POST /api/wifi/scan` → trigger skanu
     - `POST /api/wifi/attack/deauth` → trigger deauth
     - `GET /api/pcap/<filename>` → download pliku PCAP
     - `GET /api/sd/ls` → lista plików na SD

2. **`web_ws.c`:**
   - WebSocket endpoint `/ws`
   - Streaming w real-time:
     - Nowe pakiety (summary line)
     - Statystyki (co 1s)
     - Alerty (przechwycony handshake, wykryty rogue AP)

3. **Dashboard HTML/JS (www/):**
   - Single page application (vanilla JS, bez frameworków — oszczędność RAM)
   - Sekcje: Status, Wi-Fi, BLE, Zigbee/Thread, Files, Settings
   - Auto-refresh przez WebSocket
   - Responsywny layout
   - **PSRAM:** Pliki statyczne (HTML/JS/CSS) ładowane z SD card, ale chunked response buffer (4 KB) alokowany w PSRAM. WebSocket frame buffer (2 KB per połączenie, max 4 połączenia = 8 KB) w PSRAM. JSON serialization buffer (do 8 KB dla dużych list AP/BLE) w PSRAM — unikaj alokacji dużych buforów JSON w wewnętrznym SRAM.

#### Faza 5.3: GPS + Wardriving
**Pliki:** `utils/gps_parser.c`, dodatkowy kod w `wifi_scanner.c`

1. **`gps_parser.c`:**
   - UART RX task na konfigurowalnych pinach
   - Parser NMEA: $GPGGA (pozycja), $GPRMC (prędkość, kurs)
   - `gps_get_position()` → lat, lon, alt, fix_quality, satellites
   - `gps_get_time()` → UTC time

2. **Wardriving mode:**
   - Ciągły scan AP + zapis do CSV:
     ```
     MAC,SSID,AuthMode,Channel,RSSI,Latitude,Longitude,Altitude,Timestamp
     ```
   - Format kompatybilny z WiGLE
   - LED: zielony = GPS fix, czerwony = no fix
   - **PSRAM:** Bufor deduplikacji AP w wardriving (hash set BSSID, max 4096 unikalnych AP × ~12 bytes = ~48 KB) alokowany w PSRAM. Batch write buffer CSV (~8 KB w PSRAM) — gromadzi wiersze i flushuje na SD co 10 sekund (mniej operacji zapisu SD = mniejsze zużycie energii i dłuższa żywotność karty).

3. **CLI komendy:**
   - `gps status` — pozycja, fix, satelity
   - `wifi wardrive start [--file <csv>]`
   - `wifi wardrive stop`

---

### FAZA 6: INTERFEJS GRAFICZNY — TFT LCD TOUCH GUI (LVGL)
**Czas: 3–4 tygodnie**
**Zależności:** Faza 1.6 (drivery TFT/Touch/LVGL), Faza 2 (Wi-Fi API), Faza 3 (BLE API), Faza 4 (802.15.4 API), Faza 5.1–5.3 (Cross-Protocol, Web UI, GPS)

**Uwaga:** Poszczególne ekrany mogą być budowane inkrementalnie — `scr_wifi.c` już po Fazie 2, `scr_ble.c` po Fazie 3 itd. Faza 6 zakłada finalne scalenie, dopieszczenie i testy pełnego GUI.

**⚡ Strategia PSRAM dla GUI:**
GUI jest jednym z największych konsumentów pamięci w SPECTRA. Zasady alokacji:
- **LVGL display buffers** (2 × 12.8 KB) — **MUSZĄ być w wewnętrznym SRAM** (DMA-capable!), NIE w PSRAM. Użyj `SPECTRA_MALLOC_DMA()`
- **Drzewo widgetów LVGL** (`lv_obj_t` i pochodne) — automatycznie w PSRAM gdy `CONFIG_LV_ATTRIBUTE_MEM_ALIGN` skonfigurowane z PSRAM allocator. Złożony ekran (np. `scr_wifi` z tabelą 20 AP, wykresem, 4 tabami) = ~30-60 KB obiektów LVGL
- **Dane wykresów** (`widget_chart` series data) — tablice punktów (np. 60 samples × 4 bytes = 240 bytes per seria, ale pełny spectrum analyzer = 320 points × 4 bytes × 3 serie = ~4 KB) w PSRAM
- **Listy danych** (AP list, BLE device list, frame list) — dane źródłowe w PSRAM, widgety odwołują się przez wskaźniki
- **Fonty i ikony** (~100-120 KB) — w flash (XIP), nie w RAM; font cache LVGL (`LV_FONT_CACHE_DEF_SIZE`) w PSRAM
- **Animacje** — bufor animacji LVGL w PSRAM (`lv_anim_t` pool)
- **Screen cache** — nieaktywne ekrany mogą być cache'owane w PSRAM zamiast niszczone (szybsze przełączanie, koszt ~20-40 KB/ekran)
- **UWAGA:** SPI DMA transfer do ILI9341 wymaga buforów w wewnętrznym SRAM (DMA-capable). Flush buffer (320×20×2 = 12.8 KB) MUSI być w DRAM, nie w PSRAM! Użyj `SPECTRA_MALLOC_DMA()` dla flush buffers.
- **Degradacja:** Przy PSRAM free < 256 KB → wyłącz cache ekranów. Przy < 128 KB → zmniejsz display buffer (z 20 na 10 linii). Przy < 64 KB → wyłącz animacje, uprość widgety.

#### Faza 6.1: Szkielet nawigacji + Home Dashboard + Widgety bazowe
**Pliki:** `tft_gui/screens/scr_home.c`, `tft_gui/widgets/widget_statusbar.c`, `widget_navbar.c`, `widget_dialog.c`, `widget_toast.c`, `tft_gui/gui_main.c` (rozszerzenie)

1. **Szkielet nawigacji (`gui_main.c` — rozszerzenie):**
   - **Screen manager:**
     - Rejestr ekranów: tablica `gui_screen_t` (id, name, create_fn, destroy_fn, update_fn)
     - `gui_navigate_to(screen_id)` — animacja slide left/right (lv_scr_load_anim)
     - `gui_navigate_back()` — stos nawigacji (max 8 poziomów), animacja slide right
     - `gui_get_active_screen()` — aktualnie wyświetlany ekran
   - **Screen lifecycle:**
     ```c
     typedef struct {
         uint8_t id;
         const char *name;
         lv_obj_t* (*create)(void);     // tworzenie widgetów, bindowanie danych
         void (*destroy)(void);          // cleanup, zwolnienie pamięci
         void (*update)(gui_event_t *e); // obsługa eventów z silników radiowych
         void (*on_enter)(void);         // wywoływane przy nawigacji do ekranu
         void (*on_leave)(void);         // wywoływane przy nawigacji z ekranu
     } gui_screen_reg_t;
     ```
   - **Enum ekranów:**
     ```c
     typedef enum {
         SCR_HOME = 0,
         SCR_WIFI_MAIN,
         SCR_WIFI_SCANNER,
         SCR_WIFI_SCANNER_DETAIL,
         SCR_WIFI_SNIFFER,
         SCR_WIFI_ATTACKS,
         SCR_WIFI_EVIL_PORTAL,
         SCR_WIFI_DETECTOR,
         SCR_WIFI_CHANNELS,
         SCR_BLE_MAIN,
         SCR_BLE_SCANNER,
         SCR_BLE_SCANNER_DETAIL,
         SCR_BLE_GATT,
         SCR_BLE_ATTACKS,
         SCR_BLE_SNIFFER,
         SCR_ZIGBEE_MAIN,
         SCR_ZIGBEE_SCANNER,
         SCR_ZIGBEE_SNIFFER,
         SCR_ZIGBEE_ATTACKS,
         SCR_THREAD_MAIN,
         SCR_THREAD_SCANNER,
         SCR_THREAD_SNIFFER,
         SCR_THREAD_ATTACKS,
         SCR_CROSS_PROTOCOL,
         SCR_SPECTRUM_ANALYZER,
         SCR_WARDRIVING,
         SCR_FILES,
         SCR_SETTINGS,
         SCR_SETTINGS_DISPLAY,
         SCR_SETTINGS_RADIO,
         SCR_SETTINGS_SYSTEM,
         SCR_KEYBOARD,
         SCR_CALIBRATION,
         SCR_COUNT  // sentinel
     } screen_id_t;
     ```

2. **Status Bar (`widget_statusbar.c`):**
   - Górny pasek (320×24 px), stały na wszystkich ekranach
   - Zawartość (od lewej):
     - Ikona modułu (Wi-Fi / BLE / Zigbee / Thread / Home) — wskazuje aktywny ekran
     - Tytuł ekranu (np. "Wi-Fi Scanner", "BLE Attacks") — font_medium, biały
   - Zawartość (od prawej, ikony 16×16):
     - 📶 RSSI / sygnał (jeśli połączony)
     - 💾 SD card status (ikona: obecna / brak / zapis aktywny)
     - 📡 GPS fix status (ikona: no fix / 2D fix / 3D fix)
     - 🔋 RAM usage (mini bar: zielony >50%, żółty 25-50%, czerwony <25%)
     - ⏱ Uptime (HH:MM) — font_small
   - Aktualizacja: co 1 sekundę (`lv_timer_create()`, 1000 ms)
   - Implementacja: `lv_obj_t *statusbar_create(lv_obj_t *parent)`, `statusbar_update()`

3. **Navigation Bar (`widget_navbar.c`):**
   - Dolny pasek (320×40 px), stały na ekranach głównych
   - 6 przycisków (ikona + label 10px):
     - 🏠 Home | 📶 WiFi | 📱 BLE | 🔗 Zigbee | 🧵 Thread | ⚙️ More
   - „More" otwiera submenu: Cross-Protocol, Wardriving, Files, Settings
   - Aktywny przycisk podświetlony kolorem accent (`#0F3460`)
   - Implementacja: `lv_obj_t *navbar_create(lv_obj_t *parent)`, `navbar_set_active(screen_id)`
   - Obsługa touch: `lv_obj_add_event_cb()` → `gui_navigate_to(target_screen)`
   - Animacja: krótki highlight + haptic feedback (buzzer 20ms) jeśli włączony

4. **Home Dashboard (`scr_home.c`):**
   - **Layout (320×176 px, po odjęciu statusbar + navbar):**
     - Górna część (80 px) — 4 kafelki modułów w gridzie 2×2:
       ```
       ┌──────────┬──────────┐
       │ 📶 Wi-Fi │ 📱 BLE   │
       │  12 APs  │  8 devs  │
       │ ● Active │ ○ Idle   │
       ├──────────┼──────────┤
       │ 🔗Zigbee │ 🧵Thread │
       │  3 nets  │  1 net   │
       │ ○ Idle   │ ○ Idle   │
       └──────────┴──────────┘
       ```
     - Kafelek: tło panelowe (`#16213E`), ikona 24×24, nazwa modułu, statystyka (ile znaleziono), status (● Active / ○ Idle / ⚡ Attack)
     - Tap na kafelek → nawigacja do ekranu modułu
     - Kolor statusu: zielony=active, szary=idle, czerwony=attack, żółty=sniffing
   - Dolna część (96 px) — informacje systemowe:
     - **RAM:** progress bar (free/total) + wartość liczbowa
     - **Flash:** progress bar (used/total)
     - **SD:** wolne miejsce (np. "12.3 GB free")
     - **CPU temp:** wartość + ikona termometru
     - **Uptime:** dd:hh:mm:ss
     - **Active tasks:** lista aktywnych operacji (np. "📡 Sniffing CH6", "💥 Deauth: 1432 pkts")
   - **Event handling:**
     - `GUI_EVT_WIFI_SCAN_DONE` → aktualizacja kafelka Wi-Fi (AP count)
     - `GUI_EVT_BLE_DEVICE_FOUND` → aktualizacja kafelka BLE (device count)
     - `GUI_EVT_ZB_NETWORK_FOUND` → aktualizacja kafelka Zigbee
     - `GUI_EVT_ATTACK_STATUS` → zmiana statusu kafelka (Idle → Attack)
     - `GUI_EVT_SD_SPACE_LOW` → zmiana koloru paska SD na czerwony + toast

5. **Dialog potwierdzenia (`widget_dialog.c`):**
   - Modal overlay (ciemny tło 80% opacity)
   - Tytuł + treść + 2 przyciski (Cancel / Confirm)
   - Confirm w kolorze akcji: czerwony dla destrukcyjnych (deauth, factory reset), zielony dla normalnych
   - `dialog_show(title, msg, confirm_text, confirm_color, callback_fn)`
   - `dialog_dismiss()`

6. **Toast / Snackbar (`widget_toast.c`):**
   - Małe powiadomienie (280×36 px) wyświetlane u góry ekranu (pod statusbar)
   - Typy: INFO (niebieski), SUCCESS (zielony), WARNING (żółty), ERROR (czerwony), ATTACK (pomarańczowy)
   - Auto-hide po 3 sekundach (konfigurowalny)
   - Animacja: slide-in z góry, fade-out
   - Queue: max 3 oczekujące toasty, FIFO
   - `toast_show(msg, type, duration_ms)`
   - Przykłady:
     - "✓ Handshake captured!" (SUCCESS)
     - "⚠ Rogue AP detected!" (WARNING)
     - "💥 Deauth started: 00:1A:2B:..." (ATTACK)
     - "✗ SD card full!" (ERROR)

#### Faza 6.2: Ekrany Wi-Fi
**Pliki:** `tft_gui/screens/scr_wifi.c`, `scr_wifi_detail.c`, `tft_gui/widgets/widget_ap_list.c`, `widget_chart.c`

1. **Wi-Fi Main Menu (`scr_wifi.c` — widok menu):**
   - Lista opcji (lv_list) z ikonami:
     - 🔍 Scanner — "Scan AP / Stations"
     - 📡 Sniffer — "Capture packets"
     - 💥 Attacks — "Deauth / Beacon / Evil Twin"
     - 🛡 Detector — "Rogue AP detection"
     - 📊 Channels — "Channel occupancy map"
   - Tap → nawigacja do pod-ekranu

2. **Wi-Fi Scanner (`scr_wifi.c` — widok skanera):**
   - **Górna belka kontrolna (32 px):**
     - Przycisk "▶ Scan" / "⏹ Stop" (toggle)
     - Dropdown band filter: "All" / "2.4 GHz" / "5 GHz"
     - Dropdown sort: "RSSI ↓" / "Channel" / "SSID A-Z"
     - Label: "Found: 23 APs"
   - **Lista AP (`widget_ap_list.c`, 144 px, scrollowalna):**
     - Każdy element (40 px wysokości):
       ```
       ┌──────────────────────────────────────────┐
       │ 🔒 MyNetwork_5G          -42 dBm ████▌  │
       │    CH: 36  │  WPA3  │  Asus Inc.   2.4s  │
       └──────────────────────────────────────────┘
       ```
     - Ikona: 🔒 (WPA2/3), 🔓 (Open), 🔐 (WEP/WPA)
     - SSID (font_medium, bold, max 20 znaków + ellipsis)
     - RSSI wartość + bar graficzny (5 segmentów, kolor: zielony > -50, żółty > -70, czerwony > -80)
     - Kanał, typ auth, vendor (OUI), czas od ostatniego widzenia
     - Tap na AP → `scr_wifi_detail.c`
   - **Implementacja widget_ap_list:**
     - `lv_obj_t *ap_list_create(lv_obj_t *parent)`
     - `ap_list_update(wifi_ap_info_t *aps, int count)` — odświeżenie listy
     - Virtualized list (`lv_roller` lub custom) — max 64 AP bez problemów z pamięcią
     - Kolorowanie tła: delikatne podświetlenie AP z otwartą siecią (ostrzeżenie)

3. **Wi-Fi AP Detail (`scr_wifi_detail.c`):**
   - Pełne informacje o wybranym AP:
     - SSID, BSSID, Channel, Band, RSSI (live update), Auth mode
     - Vendor (OUI lookup), 802.11 type (b/g/n/ax), Hidden SSID flag
     - WPA3/WPA2 Enterprise detection
     - Beacon interval, Country code (jeśli IE obecne)
   - **RSSI wykres w czasie** (`widget_chart.c`):
     - `lv_chart` typu LINE, 60 punktów (1 min historii, update co 1s)
     - Zakres Y: -100 do -20 dBm
     - Kolor linii: gradient zielony→żółty→czerwony wg wartości
   - **Przyciski akcji (dolna część):**
     - [📡 Sniff] — start sniffer na kanale tego AP
     - [💥 Deauth] — deauth tego AP (dialog potwierdzenia!)
     - [👻 Clone] — beacon clone tego AP
     - [😈 Evil Twin] — evil twin tego AP (dialog potwierdzenia!)
     - [📋 Copy MAC] — kopiowanie BSSID do clipboard (internal)
   - Każdy przycisk akcji otwiera dialog potwierdzenia z parametrami:
     ```
     ┌─────────────────────────────────────┐
     │  ⚠ Deauth Attack                   │
     │                                      │
     │  Target: MyNetwork_5G               │
     │  BSSID:  AA:BB:CC:DD:EE:FF          │
     │  Channel: 36                         │
     │                                      │
     │  This will disconnect all clients.   │
     │                                      │
     │    [ Cancel ]    [ ▶ START ]         │
     └─────────────────────────────────────┘
     ```

4. **Wi-Fi Sniffer (`scr_wifi.c` — widok sniffera):**
   - **Kontrolki (32 px):**
     - Przycisk "▶ Start" / "⏹ Stop"
     - Dropdown channel: 1-14 (2.4G), 36-165 (5G), "Hop"
     - Dropdown filter: "All" / "Beacon" / "Probe" / "Deauth" / "EAPOL" / "Data"
     - Toggle "📁 PCAP" (zapis na SD on/off)
   - **Live stats panel (80 px):**
     - Liczniki per typ ramki (Management / Control / Data) — 3 kolumny z lv_label, update co 500ms
     - Packets/sec: wartość liczbowa + mini sparkline (lv_chart, 30 punktów)
     - PCAP file size: "capture_001.pcap — 2.3 MB"
     - Czas trwania: "00:05:23"
   - **Wykres RSSI w czasie (`widget_chart.c`, 64 px):**
     - `lv_chart` z widmem RSSI przechwyconych pakietów
     - Scatter plot lub histogram bins
   - **Event handling:**
     - `GUI_EVT_WIFI_SNIFFER_PACKET` → inkrementacja liczników, update wykresu
     - `GUI_EVT_WIFI_HANDSHAKE` → toast "✓ Handshake captured!" + highlight

5. **Wi-Fi Attacks (`scr_wifi.c` — widok ataków):**
   - Lista ataków z ikonami i statusem (lv_list):
     - 💥 **Deauth Flood** — [Idle] / [▶ Running: 1432 pkts sent]
       - Tap → dialog: wybór target AP (z listy skanów) + start/stop
     - 📡 **Beacon Spam** — [Idle] / [▶ Running: 50 SSIDs]
       - Tap → submenu: List mode / Random mode / Clone mode
       - List mode: wybór pliku SSID z SD
       - Clone mode: wybór AP do klonowania
     - 😈 **Evil Twin** — [Idle] / [▶ Active: 2 clients]
       - Tap → dialog: wybór target AP → auto-orchestration
       - Status: connected clients, captured credentials count
     - 🎭 **Evil Portal** — [Idle] / [▶ Active: portal.html]
       - Tap → dialog: wybór SSID + template HTML z SD
       - Status: connected clients, credentials captured
     - 🃏 **Karma** — [Idle] / [▶ Active: 12 probes answered]
       - Tap → start/stop
       - Status: lista odpowiedzianych probe requests
     - 🔑 **PMKID Capture** — [Idle] / [▶ Waiting on CH 6]
       - Tap → wybór target AP + start
       - Status: "Captured 0/1 PMKID"
   - **Każdy atak:**
     - Kolorowy indicator: szary=idle, zielony=ready, czerwony=running, żółty=paused
     - Real-time update statystyk z `GUI_EVT_ATTACK_STATUS`
     - Przycisk stop globalny: "⏹ Stop All Attacks"

6. **Wi-Fi Detector (`scr_wifi.c` — widok detektora):**
   - Przycisk "▶ Start Monitoring" / "⏹ Stop"
   - Lista wykrytych zagrożeń (scrollowalna):
     - Typ: 🍍 Pineapple / 👾 Pwnagotchi / 🔄 Multi-SSID / 💥 Deauth Flood
     - AP info: SSID, BSSID, kanał, RSSI, powód detekcji
     - Timestamp wykrycia
   - Alert toast przy nowym wykryciu
   - `GUI_EVT_WIFI_ROGUE_DETECTED` → dodanie do listy + toast

7. **Wi-Fi Channel Map (`scr_wifi.c` — widok kanałów):**
   - Wykres słupkowy (`lv_chart` typu BAR):
     - X: kanały 1-14 (2.4G) + 36-165 (5G) — dwie sekcje
     - Y: liczba AP na kanale / energia
     - Kolor słupka: gradient wg zajętości (zielony=wolny → czerwony=zatłoczony)
   - Tap na słupek → lista AP na tym kanale
   - Auto-refresh co 5 sekund (background scan)

#### Faza 6.3: Ekrany BLE
**Pliki:** `tft_gui/screens/scr_ble.c`, `scr_ble_detail.c`, `tft_gui/widgets/widget_ble_list.c`

1. **BLE Main Menu (`scr_ble.c` — widok menu):**
   - Lista opcji:
     - 🔍 Scanner — "Scan BLE devices"
     - 🌳 GATT Explorer — "Browse services & characteristics"
     - 💥 Attacks — "Spam / Fuzz / DoS / HID"
     - 📡 Sniffer — "Advertisement monitor"

2. **BLE Scanner (`scr_ble.c` — widok skanera):**
   - **Kontrolki:**
     - Przycisk "▶ Scan" / "⏹ Stop"
     - Dropdown filter: "All" / "Apple" / "Samsung" / "Google" / "iBeacon" / "Eddystone" / "Unknown"
     - Dropdown sort: "RSSI ↓" / "Name A-Z" / "Last seen"
     - Label: "Found: 15 devices"
   - **Lista urządzeń (`widget_ble_list.c`, scrollowalna):**
     - Każdy element (44 px):
       ```
       ┌──────────────────────────────────────────┐
       │ 🍎 AirPods Pro (Max)       -38 dBm █████ │
       │    AA:BB:CC:DD:EE:FF │ Apple │ Random     │
       │    iBeacon │ Connectable │ Seen: 2s ago   │
       └──────────────────────────────────────────┘
       ```
     - Ikona vendor: 🍎 Apple, 📱 Samsung, 🟢 Google, 🔵 Microsoft, 📶 iBeacon, ⬡ Eddystone, ❓ Unknown
     - Nazwa (lub "N/A"), RSSI + bar, MAC, vendor, addr type
     - Flagi: Connectable, iBeacon, Eddystone
     - Tap → `scr_ble_detail.c`
   - **Implementacja widget_ble_list:**
     - Deduplication by address (update existing entry)
     - Aging: usuwanie urządzeń nie widzianych > 30s (konfigurowalny)
     - Max 128 urządzeń w liście (ring buffer)

3. **BLE Device Detail (`scr_ble_detail.c`):**
   - Informacje szczegółowe:
     - Name, Address, Address Type, RSSI (live), Company ID, Manufacturer
     - AD Structures dekodowane: Flags, TX Power, Service UUIDs, Manufacturer Data (hex)
     - Advertising PDU type (ADV_IND, ADV_NONCONN_IND, itd.)
   - **RSSI wykres w czasie** (lv_chart, 60 punktów)
   - **Przyciski akcji:**
     - [🔗 Connect + GATT] — przejście do GATT Explorer
     - [💥 Spam Clone] — klonowanie advertising data
     - [🔨 Fuzz GATT] — start GATT fuzzer (po połączeniu)
     - [🎯 DoS] — connection flood do urządzenia
     - [⌨️ HID Inject] — jeśli HID service detected

4. **GATT Explorer (`scr_ble.c` — widok GATT):**
   - **Status połączenia:** "Connected to AA:BB:CC:DD:EE:FF" / "Disconnected"
   - **Przycisk "Connect"** (jeśli nie połączony) + target address
   - **Drzewo GATT (lv_treeview / nested lv_list):**
     ```
     ▼ Service: 0x180A (Device Information)
       ├─ Char: 0x2A29 (Manufacturer Name) [R]
       │   Value: "Apple Inc." (ASCII)
       ├─ Char: 0x2A24 (Model Number) [R]
       │   Value: "AirPods Pro" (ASCII)
       └─ Char: 0x2A26 (Firmware Revision) [R]
           Value: "5A377" (ASCII)
     ▼ Service: 0x180F (Battery Service)
       └─ Char: 0x2A19 (Battery Level) [R,N]
           Value: 0x5F (95%)
           └─ Desc: 0x2902 (CCCD) [R,W]
     ▼ Service: 0xFE2C (Unknown / Google Fast Pair)
       └─ Char: 0x1234 [R,W]
           Value: 0x0A 0x3B 0x... (HEX)
     ```
   - **Akcje per characteristic:**
     - [📖 Read] — odczyt wartości (hex + ASCII + decimal)
     - [✏️ Write] — otwarcie on-screen keyboard (hex input)
     - [🔔 Subscribe] — toggle notifications/indications
   - Wartości wyświetlane w: HEX / ASCII / Decimal (toggle)

5. **BLE Attacks (`scr_ble.c` — widok ataków):**
   - Lista ataków:
     - 🍎 **Apple Spam** — [Idle] / [▶ Running: 234 pkts/s]
     - 📱 **Samsung Spam** — [Idle] / [▶ Running]
     - 🟢 **Google Fast Pair Spam** — [Idle] / [▶ Running]
     - 🔵 **Microsoft Swift Pair Spam** — [Idle] / [▶ Running]
     - 🌀 **Universal Spam** — [Idle] / [▶ All running: 890 pkts/s]
     - 📡 **Beacon Spoof** — [Idle] / [▶ iBeacon: UUID...]
     - 🔨 **GATT Fuzzer** — [Idle] / [▶ Target: AA:BB:... Tested: 23/45 chars]
     - ⌨️ **HID Injection** — [Idle] / [▶ Script: payload.txt Line 12/30]
     - 🎯 **Connection Flood** — [Idle] / [▶ Target: AA:BB:... 156 conn/min]
   - Tap na atak → dialog konfiguracji + start
   - "⏹ Stop All" przycisk globalny

6. **BLE Sniffer (`scr_ble.c` — widok sniffera):**
   - Start/Stop + toggle PCAP
   - Live advertisement monitor:
     - Scrollowalna lista ostatnich 50 advertisement PDU
     - Każdy wpis: timestamp, MAC, RSSI, PDU type, krótki payload (hex, 16 bytes)
   - Statystyki: PDU/sec, unique MACs, total bytes

#### Faza 6.4: Ekrany Zigbee i Thread
**Pliki:** `tft_gui/screens/scr_zigbee.c`, `scr_thread.c`

1. **Zigbee Main Menu (`scr_zigbee.c` — widok menu):**
   - 🔍 Scanner — "Network discovery"
   - 📡 Sniffer — "802.15.4 frame capture"
   - 💥 Attacks — "Touchlink / Replay / Jam / Fuzz"

2. **Zigbee Scanner (`scr_zigbee.c` — widok skanera):**
   - **Kontrolki:**
     - Przycisk "▶ Scan Networks" — active scan (beacon request na kanałach 11-26)
     - Przycisk "📊 Energy Scan" — energy detection per channel
   - **Lista sieci:**
     - Każdy wpis (48 px):
       ```
       ┌──────────────────────────────────────────┐
       │ 🔗 PAN: 0x1A2B    Channel: 15   -52 dBm │
       │    Coordinator: 0x0000 (00:11:22:...)     │
       │    Profile: ZHA │ Permit Join: Yes │ 3 dev │
       └──────────────────────────────────────────┘
       ```
     - PAN ID, Extended PAN ID, Channel, RSSI, Coordinator addr
     - Stack Profile (ZB Pro / ZB), Permit Join status, device count
   - **Energy Scan wykres:**
     - Bar chart kanały 11-26, kolor wg energii
     - Overlay: znaczniki znalezionych sieci na odpowiednich kanałach
   - **Tap na sieć → szczegóły:**
     - Topologia: Coordinator → Routers → End Devices (prosta lista hierarchiczna)
     - Przechwycone klucze (jeśli key extraction aktywny)
     - Przyciski: [📡 Sniff this channel] [💥 Attacks...]

3. **Zigbee Sniffer (`scr_zigbee.c` — widok sniffera):**
   - **Kontrolki:**
     - Start/Stop, wybór kanału (11-26 / Hop), toggle PCAP, toggle decoder
   - **Dekodowane ramki (scrollowalna lista, kolorowana wg warstwy):**
     ```
     12:34:56.789 [CH15] -45dBm
       MAC: Data PAN:1A2B 0xABCD→0x0000
       NWK: Data 0xABCD→0x0001 [AES-CCM]
       ZCL: On/Off → Cmd: Toggle
     ```
     - Kolor: MAC=szary, NWK=niebieski, APS=zielony, ZCL=żółty
     - Tap na ramkę → pełne dekodowanie (hex dump + parsed fields)
   - **Statystyki:**
     - Frames/sec, frames by type (Beacon/Data/Ack/Command)
     - Key extraction status: "🔑 Keys captured: 2"
   - **Event handling:**
     - `GUI_EVT_ZB_FRAME_DECODED` → dodanie do listy

4. **Zigbee Attacks (`scr_zigbee.c` — widok ataków):**
   - Lista ataków:
     - 🔗 **Touchlink** → submenu:
       - Scan → lista urządzeń Touchlink w zasięgu
       - Identify → wymuszona identyfikacja (mruganie LED)
       - Factory Reset → reset urządzenia (dialog potwierdzenia!)
       - Steal → przejęcie urządzenia do swojej sieci
     - 🔄 **Replay Attack** — przechwycenie + retransmisja
     - 💉 **Frame Injection** — on-screen hex editor (basic) lub wczytanie z SD
     - 📻 **Channel Jamming** — wybór kanału + start/stop + wskaźnik mocy TX
     - 🔨 **Frame Fuzzing** — target address + tryb (random/truncated/oversized)
     - 🔑 **Key Sniffing** — pasywne nasłuchiwanie kluczy podczas join
     - 💣 **DoS Coordinator** — target PAN + flood

5. **Thread Main Menu (`scr_thread.c`):**
   - 🔍 Scanner — "Thread network discovery"
   - 📡 Sniffer — "MLE / CoAP capture"
   - 💥 Attacks — "MLE inject / Partition / Leader spoof"

6. **Thread Scanner (`scr_thread.c` — widok skanera):**
   - Lista sieci Thread:
     ```
     ┌──────────────────────────────────────────┐
     │ 🧵 "HomeThread"     Channel: 15  -48 dBm │
     │    XPAN: 0xDEAD...  PAN: 0x4567          │
     │    Leader: 0x0400 │ Routers: 3 │ BR: Yes  │
     └──────────────────────────────────────────┘
     ```
   - Network Name, Extended PAN ID, Channel, Leader RLOC16
   - Router count, Border Router presence
   - Tap → topologia mesh (lista: Leader → Routers → SEDs/MEDs/FEDs)

7. **Thread Sniffer (`scr_thread.c` — widok sniffera):**
   - Analogiczny do Zigbee Sniffer, ale z dekodowaniem:
     - MLE messages (Link Request, Advertisement, Parent Req/Resp)
     - CoAP messages (TMF commands)
     - 6LoWPAN header info
   - Kolorowanie: MLE=fioletowy, CoAP=cyjanowy, MAC=szary

8. **Thread Attacks (`scr_thread.c` — widok ataków):**
   - 💉 MLE Injection — typ wiadomości + target
   - 🔀 Partition Attack — fałszywy Partition ID
   - 👑 Leader Spoofing — ogłoszenie się liderem
   - 🔄 Replay — przechwycenie + retransmisja
   - 💣 DoS — flood na adresy Thread

#### Faza 6.5: Ekrany pomocnicze — Cross-Protocol, Wardriving, Files, Settings, Keyboard
**Pliki:** `tft_gui/screens/scr_cross.c`, `scr_wardriving.c`, `scr_files.c`, `scr_settings.c`, `scr_keyboard.c`

1. **Cross-Protocol (`scr_cross.c`):**
   - **Matter Discovery:**
     - Start scan (BLE + mDNS)
     - Lista urządzeń Matter: name, vendor, protocol (Thread/Wi-Fi), commissioning status
   - **Spectrum Analyzer:**
     - Fullscreen `lv_chart` (280×160 px):
       - X: częstotliwość 2400-2483 MHz (84 punkty)
       - Y: energia (dBm)
       - Overlay: kanały Wi-Fi (1-14), kanały Zigbee (11-26), BLE channels (37,38,39)
       - Kolorowe regiony: Wi-Fi=niebieski, Zigbee=zielony, BLE=fioletowy
     - Dane: CSI (Wi-Fi) + Energy Detect (802.15.4) + RSSI scan (BLE)
     - Update rate: co 500ms
   - **Coexistence Test:**
     - Checkboxy: Wi-Fi ☑, BLE ☑, 802.15.4 ☑
     - Przycisk "▶ Run Test" (30s duration)
     - Wynik: tabela (throughput, packet loss, latency per moduł)

2. **Wardriving (`scr_wardriving.c`):**
   - **GPS Status panel (40 px):**
     - Fix: No Fix 🔴 / 2D Fix 🟡 / 3D Fix 🟢
     - Coordinates: lat, lon (6 decimal places)
     - Satellites: N in use / N in view
     - Speed: km/h, Altitude: m
   - **Live Stats (80 px):**
     - Total APs found: N (unique SSIDs: M)
     - APs per minute: wykres sparkline
     - Open networks: N (podświetlone na czerwono)
     - Distance traveled: X.X km
     - File: "wardrive_001.csv — 156 KB"
   - **Kontrolki:**
     - [▶ Start Wardriving] / [⏹ Stop]
     - Toggle: "Only open networks" / "All networks"
   - **Mini mapa (opcjonalnie, jeśli RAM pozwoli):**
     - Canvas 160×80 z narysowanymi punktami (dot per AP na relatywnych pozycjach GPS)
     - Prosta wizualizacja track

3. **Files — SD Card Browser (`scr_files.c`):**
   - **Ścieżka (breadcrumbs):** `/sdcard/pcap/` — tap na segment = nawigacja
   - **Lista plików/folderów (scrollowalna):**
     ```
     📁 pcap/                     12 files
     📁 evil_portal/               3 files
     📁 scripts/                   2 files
     📁 wardrive/                  1 file
     📄 capture_001.pcap          2.3 MB    2024-01-15
     📄 capture_002.pcap          5.1 MB    2024-01-15
     📄 ble_log.txt               45 KB     2024-01-14
     ```
   - Ikony per typ: 📁 folder, 📄 PCAP, 📝 TXT/CSV, 🌐 HTML, 📜 script
   - **Tap na folder → wejście, tap na plik → dialog z info:**
     - Nazwa, rozmiar, data modyfikacji, typ
     - Przyciski: [🗑 Delete] (dialog potwierdzenia!) [📥 Download via Web] [📋 Info]
   - **Footer:** "SD Card: 12.3 GB free / 32 GB total" + progress bar
   - **Przycisk "🗑 Clear All PCAP"** — usunięcie wszystkich plików PCAP z potwierdzeniem

4. **Settings (`scr_settings.c`):**
   - **Lista kategorii (lv_list):**
     - 🖥 **Display** → `scr_settings_display`:
       - Brightness: slider 10-100%
       - Screen timeout: dropdown (30s / 1m / 5m / 10m / Never)
       - Orientation: "Landscape" / "Landscape Inverted"
       - Theme: "Dark" / "Light"
       - Touch recalibration: przycisk → `scr_calibration.c`
       - Touch sound: toggle on/off
     - 📻 **Radio** → `scr_settings_radio`:
       - Wi-Fi default band: "2.4 GHz" / "5 GHz" / "Both"
       - Wi-Fi MAC policy: "Random on boot" / "Fixed" / "Custom"
       - Custom MAC: text input (on-screen keyboard)
       - BLE scan duration default: slider 5-60s
       - 802.15.4 default channel: dropdown 11-26
       - TX Power: slider (low/medium/high/max)
     - 💾 **Storage** → submenu:
       - PCAP save path: "/sdcard/pcap/"
       - Auto-rotate logs: toggle + max size (100MB / 500MB / 1GB)
       - Auto-split PCAP: toggle + split size (10MB / 50MB / 100MB)
       - Format SD card: przycisk (dialog potwierdzenia DOUBLE — wpisz "FORMAT")
     - 🔌 **GPIO** → submenu:
       - LED mode: "Status" / "Off" / "Custom"
       - Buzzer: toggle on/off + volume (low/high)
       - GPS UART pins: TX/RX dropdown (lista GPIO)
     - ℹ️ **System** → `scr_settings_system`:
       - Firmware version: "SPECTRA v1.0.0"
       - ESP-IDF version
       - Chip info: model, revision, features
       - Free heap / Min free heap
       - Uptime
       - [🔄 Reboot] (dialog potwierdzenia)
       - [🏭 Factory Reset] (DOUBLE dialog potwierdzenia!)
       - [📡 OTA Update] — start Wi-Fi AP + OTA server
     - 📖 **About:**
       - Logo SPECTRA (ASCII lub bitmap 64×64)
       - Wersja, autor, licencja
       - "Security testing platform for ESP32-C5"
       - Disclaimer: "For authorized testing only"
   - **Każde ustawienie z auto-save do NVS** (`nvs_mgr` — natychmiast po zmianie)

5. **On-Screen Keyboard (`scr_keyboard.c`):**
   - Reusable komponent wywoływany z dowolnego ekranu
   - **Layout (320×120 px, dolna połowa ekranu):**
     - QWERTY keyboard z LVGL `lv_keyboard`
     - Tryby: ABC (lowercase/uppercase), 123 (numbers + symbols), HEX (0-9 A-F)
     - Przyciski specjalne: Backspace, Space, Enter, Cancel, Mode switch
   - **Text input field (320×32 px):**
     - `lv_textarea` z placeholderem
     - Max length konfigurowalny
   - **API:**
     - `keyboard_show(placeholder, mode, max_len, callback)` — wyświetlenie keyboardu
     - `callback(const char *text, bool confirmed)` — wynik (Enter = confirmed, Cancel = !confirmed)
     - Tryby: `KB_MODE_TEXT`, `KB_MODE_HEX`, `KB_MODE_NUMERIC`, `KB_MODE_MAC`, `KB_MODE_SSID`
   - **MAC address mode:** auto-formatowanie "AA:BB:CC:DD:EE:FF" z separatorami
   - **HEX mode:** auto-formatowanie "0x.." z walidacją

6. **Touch Calibration (`scr_calibration.c`):**
   - Fullscreen, bez statusbar/navbar
   - 3 celowniki (crosshair) wyświetlane sekwencyjnie:
     - Lewy-górny róg (20, 20)
     - Prawy-dolny róg (300, 220)
     - Środek (160, 120)
   - Instrukcja: "Touch the crosshair" (font_large, centered)
   - Po dotknięciu: animacja ✓ + przejście do następnego punktu
   - Obliczenie macierzy kalibracji → zapis do NVS
   - Po zakończeniu: toast "Calibration complete ✓" + powrót do Settings

#### Faza 6.6: Integracja GUI ↔ Silniki radiowe (Event Processing)
**Pliki:** `tft_gui/gui_events.c` (rozszerzenie), modyfikacja `scr_*.c`

1. **Event flow — architektura:**
   ```
   ┌─────────────┐     gui_event_queue (FreeRTOS)    ┌─────────────┐
   │ wifi_engine  │ ──── gui_event_send() ──────────► │             │
   │ ble_engine   │ ──── gui_event_send() ──────────► │  GUI Task   │
   │ zigbee_engine│ ──── gui_event_send() ──────────► │  (LVGL)     │
   │ thread_engine│ ──── gui_event_send() ──────────► │             │
   │ gps_parser   │ ──── gui_event_send() ──────────► │             │
   └─────────────┘                                     └──────┬──────┘
                                                              │
                                                   gui_event_process()
                                                              │
                                              ┌───────────────┼────────────────┐
                                              ▼               ▼                ▼
                                        scr_home.update  scr_wifi.update  scr_ble.update
                                        (update kafelki) (update listy)   (update listy)
   ```

2. **GUI → Engine (komendy od użytkownika):**
   - Touch event na przycisku → callback → wywołanie API silnika
   - Wywołania MUSZĄ być thread-safe:
     ```c
     // Przykład: użytkownik tapnął "Start Deauth" na ekranie Wi-Fi
     void on_deauth_start_btn(lv_event_t *e) {
         wifi_ap_info_t *target = lv_event_get_user_data(e);
         // Wywołanie w kontekście GUI task — silnik musi być thread-safe
         // Opcja A: bezpośrednie wywołanie (jeśli API jest thread-safe)
         wifi_deauth_flood(target->bssid, WIFI_REASON_UNSPECIFIED);
         // Opcja B: wysłanie komendy przez queue do silnika
         engine_cmd_t cmd = { .type = CMD_WIFI_DEAUTH, .target = *target };
         xQueueSend(engine_cmd_queue, &cmd, portMAX_DELAY);
         // Update UI
         gui_lock();
         lv_label_set_text(status_label, "▶ Deauth running...");
         gui_unlock();
     }
     ```
   - **engine_cmd_queue** — odwrotna kolejka (GUI → silniki):
     ```c
     typedef struct {
         engine_cmd_type_t type;     // CMD_WIFI_SCAN, CMD_WIFI_DEAUTH, CMD_BLE_SPAM_START, ...
         union {
             wifi_scan_config_t wifi_scan;
             wifi_deauth_config_t wifi_deauth;
             ble_spam_config_t ble_spam;
             zigbee_sniff_config_t zb_sniff;
             // ...
         } params;
     } engine_cmd_t;
     ```

3. **Throttling danych do GUI:**
   - Problem: sniffer może generować >1000 pakietów/s — GUI nie nadąży
   - Rozwiązanie: aggregacja w silniku, wysyłanie summary co 500ms:
     ```c
     typedef struct {
         uint32_t pkt_count_mgmt;
         uint32_t pkt_count_ctrl;
         uint32_t pkt_count_data;
         uint32_t pkt_per_sec;
         int8_t   avg_rssi;
         uint32_t pcap_file_size;
     } wifi_sniffer_summary_t;
     ```
   - Listy (AP, BLE devices): update batch co 1s (nie per-pakiet)
   - Wykresy: nowy punkt co 1s (nie per-pakiet)

4. **Memory management GUI (PSRAM-centric):**
   - Ekrany tworzone on-demand (`create()`) i niszczone przy nawigacji (`destroy()`)
   - Maksymalnie 2 ekrany w pamięci: aktualny + poprzedni (dla animacji back)
   - Listy z recyklingiem elementów (virtual list pattern) — obiekt `lv_obj_t` tylko dla widocznych wierszy, dane w tablicy PSRAM
   - **Alokacja LVGL w PSRAM:**
     - Skonfiguruj `lv_conf.h`: `LV_MEM_CUSTOM=1`, custom allocator → `SPECTRA_MALLOC_PSRAM()` / `SPECTRA_CALLOC_PSRAM()` / `free()`
     - Cały wewnętrzny heap LVGL (obiekty, style, drzewa) → PSRAM
     - **Wyjątek:** flush buffers (2 × 12.8 KB) → wewnętrzny SRAM DMA-capable (`SPECTRA_MALLOC_DMA()`)
   - Duże bufory wykresów (chart data) alokowane w PSRAM
   - Limit budżetowy: max 20 KB wewnętrzny SRAM (flush + DMA + krytyczne struktury) + 200 KB PSRAM per ekran
   - Monitoring: `heap_caps_get_free_size(MALLOC_CAP_SPIRAM)` + `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)` — logowanie co 10s do UART (debug)
   - **Degradacja przy niskim PSRAM:**
     - < 256 KB: wyłącz cache ekranów, niszcz ekrany natychmiast przy nawigacji
     - < 128 KB: zmniejsz display buffers (20 → 10 linii), wyłącz animacje slide
     - < 64 KB: uprość widgety (listy max 10 elementów, brak wykresów), toast SYSTEM_ERROR

---

### FAZA 7: HARDENING, TESTY, DOKUMENTACJA
**Czas: 2 tygodnie**

#### Faza 7.1: Hardening systemowy

1. **Zarządzanie pamięcią (PSRAM + wewnętrzny SRAM):**
   - **Monitoring heap:**
     - `heap_caps_get_free_size(MALLOC_CAP_SPIRAM)` — wolny PSRAM
     - `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)` — wolny wewnętrzny SRAM
     - `heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM)` — watermark PSRAM (najniższy poziom od startu)
     - `heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL)` — watermark SRAM
     - Logowanie co 30s do UART + opcjonalnie na SD (debug mode)
   - **Mapa alokacji PSRAM (typowy budżet dla 4 MB PSRAM):**
     - LVGL display buffers: 2 × 12.8 KB = **25.6 KB** (UWAGA: muszą być w **wewnętrznym SRAM** DMA-capable!)
     - LVGL widget tree + styles: ~**100-200 KB** PSRAM (zależy od aktywnego ekranu)
     - LVGL font cache: ~**16 KB** PSRAM
     - Packet ring buffer (Wi-Fi/BLE/802.15.4): **64-128 KB** PSRAM
     - Wi-Fi AP list (max 128 AP): ~**32 KB** PSRAM
     - BLE device list (max 256 urządzeń): ~**40 KB** PSRAM
     - Zigbee network + device map: ~**20 KB** PSRAM
     - Decoded frame cache (512 ramek): ~**128 KB** PSRAM
     - CSI data buffer: ~**128 KB** PSRAM
     - GATT tree cache: ~**16-64 KB** PSRAM
     - Handshake state buffer: ~**16 KB** PSRAM
     - Web UI buffers (HTTP + WebSocket): ~**24 KB** PSRAM
     - Evil Portal (HTML template + credentials): ~**72 KB** PSRAM
     - BLE sniffer log + tracking: ~**72 KB** PSRAM
     - **RAZEM typowe: ~750 KB – 1 MB PSRAM** (z zapasem ~3 MB przy 4 MB PSRAM)
   - **Wewnętrzny SRAM — zarezerwowane (nie alokuj w PSRAM!):**
     - DMA flush buffers (TFT ILI9341): 2 × 12.8 KB = 25.6 KB
     - DMA bufory SPI (SD card, touch XPT2046): ~4 KB
     - FreeRTOS task stacks (ISR-safe): gui_task 8 KB, radio tasks 4-6 KB each
     - Wi-Fi driver internal buffers: ~30-50 KB (zarządzane przez ESP-IDF)
     - BLE NimBLE stack: ~20-30 KB
     - LWIP TCP/IP stack: ~20 KB (częściowo w PSRAM z `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP`)
     - **Min. wolny SRAM:** utrzymuj ≥ 32 KB wolnego wewnętrznego SRAM (dla ISR, DMA, stack growth)
   - **Polityka alokacji:**
     - Wszystkie bufory > 1 KB → PSRAM (`SPECTRA_MALLOC_PSRAM()`)
     - Bufory DMA/ISR → wewnętrzny SRAM (`SPECTRA_MALLOC_DMA()`)
     - Małe struktury sterujące (< 256 bytes, często dostępne) → wewnętrzny SRAM (`malloc()`)
     - Tablice danych, listy, cache → zawsze PSRAM
   - **Graceful degradation:**
     - PSRAM free < 512 KB: log WARNING, wyłącz cache ekranów GUI
     - PSRAM free < 256 KB: zmniejsz ring buffer (128 KB → 32 KB), ogranicz listy (max 64 AP, 128 BLE)
     - PSRAM free < 128 KB: wyłącz wykresy, animacje, ogranicz listy do 32 elementów
     - PSRAM free < 64 KB: tryb awaryjny — tylko CLI, GUI minimalne (Home + statusbar)
     - Wewnętrzny SRAM free < 32 KB: log CRITICAL, wyłącz sniffera, zmniejsz task stacks
   - Stack overflow detection (FreeRTOS `configCHECK_FOR_STACK_OVERFLOW=2`)
   - `heap_caps_check_integrity_all(true)` — walidacja heap integrity (debug builds)

2. **Stabilność:**
   - Watchdog na wszystkich task'ach
   - Core dump do partycji flash przy crash
   - Automatyczny restart po crash z logiem przyczyny
   - Mutex/semaphore na współdzielonych zasobach (radio 802.15.4 Zigbee/Thread)

3. **Bezpieczeństwo firmware:**
   - Secure Boot v2 (opcjonalnie)
   - Flash Encryption (opcjonalnie)
   - OTA z weryfikacją podpisu

#### Faza 7.2: Testy

1. **Testy jednostkowe:**
   - Parsery pakietów (Wi-Fi, BLE, 802.15.4)
   - PCAP writer (poprawność formatu)
   - CLI parser (poprawność dispatch)
   - Filtr pakietów

2. **Testy integracyjne:**
   - Skan AP → wynik na CLI ✓
   - Sniffer → PCAP → Wireshark importuje poprawnie ✓
   - Evil Portal → klient łączy się → credentials przechwycone ✓
   - BLE scan → lista urządzeń → GATT discovery ✓
   - Zigbee sniffer → dekoder → poprawne warstwy ✓
   - Web UI → dashboard wyświetla dane ✓
   - **TFT GUI — nawigacja:** Home → Wi-Fi → Scanner → AP Detail → Back → Home ✓
   - **TFT GUI — Wi-Fi scan:** Start scan → lista AP na ekranie → tap AP → szczegóły ✓
   - **TFT GUI — atak:** Wi-Fi Attacks → Deauth → dialog potwierdzenia → start → status update ✓
   - **TFT GUI — BLE:** BLE Scanner → lista urządzeń → tap → GATT Explorer ✓
   - **TFT GUI — Zigbee sniffer:** Start → dekodowane ramki na ekranie → PCAP status ✓
   - **TFT GUI — Settings:** Zmiana brightness → natychmiastowy efekt → zapis do NVS ✓
   - **TFT GUI — Files:** SD browser → nawigacja folderów → info pliku → usunięcie ✓
   - **TFT GUI — Events:** Handshake captured → toast na aktywnym ekranie ✓
   - **TFT GUI — Keyboard:** On-screen keyboard → wpisanie SSID → potwierdzenie ✓
   - **TFT GUI — Kalibracja:** Touch calibration → 3 punkty → zapis do NVS → poprawne mapowanie ✓

3. **Testy obciążeniowe:**
   - Promiscuous mode przez 24h — brak memory leak
   - Beacon spam 1000 SSID — stabilność
   - Jednoczesny sniffer + zapis PCAP na SD — brak utraty pakietów
   - **TFT GUI — memory leak:** Nawigacja między ekranami 1000× → heap stable ± 2 KB
   - **TFT GUI — concurrent load:** Sniffer (1000 pkt/s) + GUI update → FPS ≥ 10, brak dropped events
   - **TFT GUI — SD browser:** 500 plików w folderze → scrollowanie płynne (≥15 FPS)
   - **TFT GUI — long run:** GUI aktywne 24h z rotating scan/sniff → brak crash, heap stable

#### Faza 7.3: Dokumentacja

1. **README.md** — przegląd projektu, budowanie, flashowanie
2. **COMMANDS.md** — pełna lista komend CLI z przykładami
3. **ARCHITECTURE.md** — diagram architektury, opis komponentów
4. **API.md** — opis REST API Web UI
5. **HARDWARE.md** — schemat połączeń (TFT ILI9341 + XPT2046, SD card, GPS, LED, UART)
6. **GUI.md** — mapa ekranów, nawigacja, opis widgetów, event flow, zrzuty ekranów
7. **LEGAL.md** — disclaimer: tylko do testów własnych urządzeń, zgodność z prawem

---

## 5. ZALEŻNOŚCI MIĘDZY FAZAMI

```
Faza 1 (Fundament) ─────────────────────────────────────┐
    ├── 1.1 Inicjalizacja projektu                       │
    ├── 1.2 Storage Manager                              │
    ├── 1.3 CLI Engine                                   │
    ├── 1.4 Packet Engine                                │
    ├── 1.5 Utils                                        │
    └── 1.6 TFT + Touch Driver + LVGL Init ◄── (1.2 NVS)│
         │                                               │
         ▼                                               │
Faza 2 (Wi-Fi) ──────────────────────────────┐          │
    ├── 2.1 Init + Scan ◄── (1.1, 1.3)      │          │
    ├── 2.2 Sniffer ◄── (1.4, 2.1)          │          │
    ├── 2.3 Deauth + Beacon ◄── (2.1)       │          │
    ├── 2.4 Evil Portal ◄── (1.2, 2.1)      │          │
    └── 2.5 EAPOL + Detector ◄── (2.2)      │          │
         │                                    │          │
         ▼                                    │          │
Faza 3 (BLE) ────────────────────────────┐   │          │
    ├── 3.1 Init + Scan ◄── (1.1, 1.3)  │   │          │
    ├── 3.2 GATT + Sniffer ◄── (3.1)    │   │          │
    └── 3.3 Attacks ◄── (3.1, 3.2)      │   │          │
         │                                │   │          │
         ▼                                │   │          │
Faza 4 (802.15.4) ───────────────────┐   │   │          │
    ├── 4.1 Init + Sniffer ◄── (1.4) │   │   │          │
    ├── 4.2 Decoder + Scanner ◄──(4.1)│   │   │          │
    ├── 4.3 Zigbee Attacks ◄── (4.2) │   │   │          │
    └── 4.4 Thread Engine ◄── (4.1)  │   │   │          │
         │                            │   │   │          │
         ▼                            ▼   ▼   ▼          │
Faza 5 (Cross + Web + GPS) ◄──── (2.x, 3.x, 4.x)       │
    ├── 5.1 Cross-Protocol                               │
    ├── 5.2 Web UI                                       │
    └── 5.3 GPS + Wardriving                             │
         │                                               │
         ▼                                               │
Faza 6 (TFT GUI Screens) ◄── (1.6, 2.x, 3.x, 4.x, 5.x)│
    ├── 6.1 Nawigacja + Home + Widgety ◄── (1.6)        │
    │        [screen manager, statusbar, navbar, toast]  │
    ├── 6.2 Ekrany Wi-Fi ◄── (2.x, 6.1)                │
    │        [scanner, detail, sniffer, attacks,         │
    │         detector, channels]                        │
    ├── 6.3 Ekrany BLE ◄── (3.x, 6.1)                  │
    │        [scanner, detail, GATT, attacks, sniffer]   │
    ├── 6.4 Ekrany Zigbee + Thread ◄── (4.x, 6.1)      │
    │        [scanner, sniffer+decoder, attacks]         │
    ├── 6.5 Ekrany pomocnicze ◄── (5.x, 6.1)           │
    │        [cross-protocol, spectrum, wardriving,      │
    │         files, settings, keyboard, calibration]    │
    └── 6.6 Integracja GUI ↔ Silniki ◄── (6.1–6.5)     │
              [event bus, throttling, memory mgmt,       │
               end-to-end testy GUI + radio]             │
         │                                               │
         ▼                                               │
Faza 7 (Hardening) ◄──────────────────── (ALL)           │
    ├── 7.1 Hardening                                    │
    ├── 7.2 Testy                                        │
    └── 7.3 Dokumentacja                                 │
```

**Uwaga:** Fazy 2, 3 i 4 mogą być realizowane częściowo równolegle, gdyż operują na osobnych modułach radiowych (z wyjątkiem Zigbee/Thread, które współdzielą radio 802.15.4).

**Uwaga TFT:** Faza 1.6 (drivery) może być realizowana równolegle z Fazami 2-4, ale pełne ekrany GUI (Faza 6) wymagają gotowych API silników radiowych. Poszczególne ekrany GUI mogą być budowane inkrementalnie — np. `scr_wifi.c` już po Fazie 2, `scr_ble.c` po Fazie 3 itd. Faza 6.6 (integracja) powinna być realizowana jako ostatnia przed Fazą 7.

**Ścieżka krytyczna:** 1 → 2 → 3 → 4 → 5 → 6 → 7 (sekwencyjnie ~28 tygodni)
**Ścieżka zoptymalizowana:** 1 → [2 ∥ 1.6] → [3 ∥ 6.2] → [4 ∥ 6.3] → [5 ∥ 6.4] → 6.5+6.6 → 7 (~24-25 tygodni)

---

## 6. KLUCZOWE API ESP-IDF

| Moduł | Kluczowe API |
|-------|-------------|
| Wi-Fi init | `esp_wifi_init()`, `esp_wifi_set_mode()`, `esp_wifi_start()` |
| Wi-Fi scan | `esp_wifi_scan_start()`, `esp_wifi_scan_get_ap_records()` |
| Wi-Fi promiscuous | `esp_wifi_set_promiscuous(true)`, `esp_wifi_set_promiscuous_rx_cb()` |
| Wi-Fi TX raw | `esp_wifi_80211_tx(ifx, buffer, len, en_sys_seq)` |
| Wi-Fi band | `esp_wifi_set_band_mode()`, `esp_wifi_set_bandwidths()` |
| Wi-Fi channel | `esp_wifi_set_channel()`, `esp_wifi_set_country()` |
| Wi-Fi CSI | `esp_wifi_set_csi(true)`, `esp_wifi_set_csi_config()` |
| Wi-Fi AP | `esp_wifi_set_config(WIFI_IF_AP, &ap_config)` |
| BLE init | `nimble_port_init()`, `ble_hs_cfg`, `nimble_port_freertos_init()` |
| BLE scan | `ble_gap_disc()`, `ble_gap_disc_cancel()` |
| BLE connect | `ble_gap_connect()`, `ble_gap_terminate()` |
| BLE GATT | `ble_gattc_disc_all_svcs()`, `ble_gattc_read()`, `ble_gattc_write_flat()` |
| BLE advertise | `ble_gap_adv_set_data()`, `ble_gap_adv_start()` |
| 802.15.4 init | `esp_ieee802154_enable()`, `esp_ieee802154_set_channel()` |
| 802.15.4 RX | `esp_ieee802154_receive()`, callback `esp_ieee802154_receive_done()` |
| 802.15.4 TX | `esp_ieee802154_transmit()`, callback `esp_ieee802154_transmit_done()` |
| 802.15.4 ED | `esp_ieee802154_energy_detect()` |
| OpenThread | `esp_openthread_init()`, `otInstanceGet()`, `otLinkActiveScan()` |
| HTTP server | `httpd_start()`, `httpd_register_uri_handler()` |
| SD card | `esp_vfs_fat_sdspi_mount()` |
| SPIFFS | `esp_vfs_spiffs_register()` |
| NVS | `nvs_open()`, `nvs_set_str()`, `nvs_get_str()` |
| GPIO | `gpio_set_direction()`, `gpio_set_level()` |
| UART | `uart_driver_install()`, `uart_read_bytes()` |
| Timer | `esp_timer_get_time()` (mikrosekundy) |
| SPI (TFT/Touch) | `spi_bus_initialize()`, `spi_bus_add_device()`, `spi_device_queue_trans()` |
| SPI DMA | `spi_device_transmit()` z flagą `SPI_TRANS_USE_TXDATA`, DMA channel config |
| PWM (backlight) | `ledc_timer_config()`, `ledc_channel_config()`, `ledc_set_duty()` |
| LVGL display | `lv_init()`, `lv_display_create()`, `lv_display_set_buffers()`, `lv_display_set_flush_cb()` |
| LVGL input | `lv_indev_create()`, `lv_indev_set_read_cb()`, `lv_indev_set_type()` |
| LVGL timer | `lv_timer_handler()`, `lv_tick_inc()`, `lv_timer_create()` |
| LVGL widgets | `lv_obj_create()`, `lv_label_create()`, `lv_btn_create()`, `lv_chart_create()`, `lv_list_create()` |
| LVGL navigation | `lv_scr_load_anim()`, `lv_obj_add_event_cb()`, `lv_event_get_code()` |

---

## 7. OGRANICZENIA I RYZYKA

1. **Radio 802.15.4 współdzielone** — Zigbee i Thread nie mogą działać jednocześnie; konieczne przełączanie trybu z mutexem.
2. **Wi-Fi dual-band ograniczenia** — ESP32-C5 może pracować na jednym kanale naraz; skanowanie obu pasm wymaga przełączania.
3. **Pamięć RAM** — 384 KB SRAM + PSRAM; jednoczesna praca wszystkich modułów może wymagać starannego zarządzania pamięcią.
4. **Promiscuous mode + AP** — tryb promiscuous może współistnieć z Soft-AP, ale na tym samym kanale; Evil Twin na innym kanale niż sniffer wymaga przełączania.
5. **Wi-Fi 5 GHz promiscuous** — obsługa promiscuous mode na 5 GHz zależy od wersji ESP-IDF; może wymagać dodatkowych patchów.
6. **BLE sniffer ograniczenia** — NimBLE nie daje dostępu do surowych PDU link layer; sniffer ograniczony do advertising data.
7. **Legalność** — Deauth, jamming, spoofing są nielegalne w większości jurysdykcji poza kontrolowanym laboratorium. Aplikacja MUSI zawierać disclaimer i być używana wyłącznie do testowania własnych urządzeń.
8. **ESP-IDF support C5** — ESP32-C5 jest stosunkowo nowym chipem; część API może być w fazie beta. Konieczne śledzenie release notes ESP-IDF.
9. **SPI bus sharing (TFT + Touch + SD)** — ILI9341, XPT2046 i SD card mogą współdzielić magistralę SPI (osobne CS), ale wymagają mutex'a na bus access. Jednoczesny zapis PCAP na SD i odświeżanie ekranu mogą powodować jitter na GUI.
10. **Pamięć LVGL** — LVGL wymaga minimum ~32 KB RAM na bufory wyświetlania + ~20-50 KB na widget tree. Przy złożonych ekranach (duże listy, wykresy) zużycie może przekroczyć 100 KB → konieczne umieszczenie buforów w PSRAM.
11. **FPS vs responsywność radio** — GUI task na niskim priorytecie (3) może powodować opóźnienia w odświeżaniu podczas intensywnych operacji radiowych. Target: 15-30 FPS w normalnym trybie, 5-10 FPS podczas aktywnego sniffingu/ataków.
12. **Fonty i ikony w flash** — Skompilowane assety LVGL (fonty + ikony) zajmują ~80-100 KB flash. Przy 16 MB flash to marginalny koszt, ale wpływa na czas bootowania (kopiowanie do cache).
13. **Kalibracja dotyku** — XPT2046 wymaga kalibracji przy pierwszym uruchomieniu i po zmianie orientacji ekranu. Źle skalibrowany dotyk uniemożliwia użycie GUI → konieczny fallback na CLI do resetu kalibracji.

---

## 8. SZACOWANY HARMONOGRAM

| Faza | Czas | Kamień milowy |
|------|------|---------------|
| Faza 1: Fundament (1.1–1.5) | Tydzień 1–3 | CLI działa, SD card, packet engine, PCAP writer |
| Faza 1.6: TFT + Touch + LVGL | Tydzień 3–4 | Wyświetlacz działa, dotyk skalibrowany, LVGL renderuje Home screen |
| Faza 2: Wi-Fi | Tydzień 5–8 | Pełne skanowanie, sniffing, deauth, evil portal, EAPOL capture |
| Faza 3: BLE | Tydzień 9–11 | BLE scan, GATT, spam, fuzzing, HID |
| Faza 4: 802.15.4 | Tydzień 12–15 | Zigbee sniffer/decoder/attacks, Thread sniffer/scanner |
| Faza 5.1–5.3: Cross + Web + GPS | Tydzień 16–18 | Web dashboard, GPS, wardriving, Matter |
| Faza 6.1: Nawigacja + Home + Widgety | Tydzień 19–20 | Screen manager, statusbar, navbar, home dashboard, toast/dialog |
| Faza 6.2: Ekrany Wi-Fi | Tydzień 20–21 | Scanner, AP detail, sniffer, attacks, detector, channel map |
| Faza 6.3: Ekrany BLE | Tydzień 21–22 | Scanner, GATT explorer, attacks, sniffer |
| Faza 6.4: Ekrany Zigbee + Thread | Tydzień 22–23 | Scanner, sniffer z dekoderem, attacks (oba moduły) |
| Faza 6.5: Ekrany pomocnicze | Tydzień 23–24 | Cross-protocol, wardriving, files, settings, keyboard, kalibracja |
| Faza 6.6: Integracja GUI ↔ Silniki | Tydzień 24–25 | Event bus, throttling, testy end-to-end GUI + radio |
| Faza 7: Hardening | Tydzień 26–28 | Testy, stabilizacja, dokumentacja |

**Łącznie: ~28 tygodni (7 miesięcy) dla jednego developera.**

**Uwaga:** Faza 1.6 (TFT driver) może być realizowana równolegle z końcówką Fazy 1. Poszczególne ekrany GUI (Faza 6) mogą być budowane inkrementalnie w miarę gotowości silników radiowych — np. `scr_wifi.c` można zacząć już po Fazie 2, dodając kolejne ekrany w tle podczas prac nad Fazami 3-5.3. Przy takim podejściu efektywny czas może się skrócić o ~2-3 tygodnie (GUI budowane równolegle z silnikami Faz 3-5).

---

*Dokument wygenerowany jako plan implementacji platformy SPECTRA do testów bezpieczeństwa IoT na ESP32-C5. Wszystkie funkcjonalności przeznaczone wyłącznie do użytku w kontrolowanym środowisku laboratoryjnym na własnych urządzeniach.*