# KitchenScanner (ESP8266) – Barcode Scanner für Küche & Inventar (MQTT)

Ein ESP8266-basierter Barcode-Scanner für die Küche:
Scannen → per MQTT als **IN** (hinzufügen) oder **OUT** (entfernen) ans Smart Home / Inventarsystem schicken.
Mit **VL53L0X ToF** als „Hand davor = Trigger/Hold“, **OLED** für Status/Text und **WS2812 Ring** für Feedback.

---

## Features

* **Barcode-Scan via UART** (SoftwareSerial)
* **2 Modi:** IN / OUT (per Taster umschaltbar)
* **MQTT Topics (fix):**

  * `scanner/in`  (Scan als IN)
  * `scanner/out` (Scan als OUT)
  * `scanner/display` (Textanzeige auf OLED)
  * `scanner/status` (online/offline via LWT)
* **OLED (SH1106 128x64)**: zentrierter Text + Ring-Grafik
* **WS2812B Ring (12 LEDs)**: Idle-Farbe je Modus, Runner bei Hold, Flash beim Scan
* **Web-UI Setup** (WLAN / MQTT / Rolle, Reboot, Factory Reset)
* **LittleFS-Konfiguration** (`/config.json`) – keine Credentials im Code

---

## Hardware

Getestete / vorgesehene Komponenten:

* ESP8266 (z. B. **D1 Mini** oder Clone)
* Barcode-Scanner mit **UART/TTL Output (9600 Baud)**
  *(kein reiner USB-Keyboard-Scanner)*
* **VL53L0X** Time-of-Flight Sensor (I2C)
* **OLED SH1106 128x64** (I2C)
* **WS2812B LED Ring** (12 LEDs)
* 1× Taster für Modusumschaltung (IN / OUT)
* 5 V Versorgung für Scanner & LED-Ring

---

## Pinout (Default im Code)

> Bezeichnungen entsprechen dem D1‑Mini‑Layout (`Dx`)

| Funktion                              | Pin        |
| ------------------------------------- | ---------- |
| ToF SDA                               | D2 (GPIO4) |
| ToF SCL                               | D1 (GPIO5) |
| Trigger Output                        | D5         |
| Scanner RX (ESP empfängt vom Scanner) | D6         |
| Mode Button (INPUT_PULLUP)            | D7         |
| WS2812 Data                           | D4         |

**Hinweise:**

* Scanner TX → ESP RX (D6), **GND unbedingt verbinden**
* WS2812 Ring möglichst mit **5 V** betreiben, GND mit ESP verbinden
* Optional: Level-Shifter für WS2812 (3.3 V → 5 V)

---

## MQTT

### Topics

* `scanner/in` – Scan als Eingang (JSON)
* `scanner/out` – Scan als Ausgang (JSON)
* `scanner/display` – Text für OLED
* `scanner/status` – `online` / `offline` (retained, LWT)

### Payload (Beispiel)

```json
{
  "ean": "4001234567890",
  "qty": 1,
  "action": "in",
  "source": "eingang",
  "uptime_ms": 1234567
}
```

**Felder:**

* `action`: `in` oder `out`
* `source`: `eingang` oder `ausgang`
* `uptime_ms`: Systemlaufzeit (`millis()`)

---

## Inbetriebnahme (Quickstart)

### 1) Flashen

Sketch kompilieren und auf den ESP8266 flashen.
LittleFS wird automatisch für die Konfiguration verwendet.

### 2) Erststart – AP Setup

Ist kein WLAN konfiguriert, startet das Gerät im Access-Point-Modus:

* **SSID:** `KitchenScanner-Setup`
* **Passwort:** *(leer)*

Die IP-Adresse wird im seriellen Monitor ausgegeben (typisch `192.168.4.1`).

### 3) Web UI

Im Browser die IP öffnen:

* `/` – Status
* `/wifi` – WLAN-Einstellungen
* `/mqtt` – MQTT & Default-Rolle
* `/reboot` – Neustart
* `/factory` – Factory Reset

Nach dem Speichern erfolgt ein automatischer Neustart.

---

## OLED Steuerung via MQTT

Das OLED zeigt den Text aus dem Topic `scanner/display` an.

Beispiel:

```bash
mosquitto_pub -t scanner/display -m "Milch (2x)"
```

Leere oder ungültige Nachrichten werden als `READY` interpretiert.

---

## IN / OUT Modus

* Umschaltung per Taster (D7)
* Idle-Farbe des LED-Rings zeigt aktuellen Modus
* Runner-Animation bei aktivem ToF-Hold
* Kurzer Flash beim erfolgreichen Scan

---

## ToF / Trigger-Hold Logik

* **Near** (< ca. 120 mm): Trigger HOLD ON (`D5 HIGH`)
* **Far**  (> ca. 170 mm): Trigger HOLD OFF (`D5 LOW`)
* **Timeout** (~6 s): automatisches Re-Arm
* Nach einem Scan wird auf „Objekt weg“ gewartet (Anti-Doppelscan)

---

## Konfiguration

Die Konfiguration wird auf dem ESP unter folgendem Pfad gespeichert:

* `/config.json` (LittleFS)

Beispiel:

```json
{
  "wifi_ssid": "MeinWLAN",
  "wifi_pass": "********",
  "mqtt_host": "192.168.0.10",
  "mqtt_port": 1883,
  "mqtt_user": "",
  "mqtt_pass": "",
  "role": "eingang"
}
```

---

## Troubleshooting

* **MQTT verbindet nicht:** Host/Port prüfen, seriellen Monitor (`115200`) nutzen
* **Scanner liefert Müll:** Baudrate, Pegel, GND prüfen
* **WS2812 flackert:** stabile 5 V, ggf. 330 Ω Datenwiderstand + 1000 µF Puffer
* **OLED bleibt leer:** SH1106 vs. SSD1306 prüfen, I2C-Adresse kontrollieren
* **ToF nicht erkannt:** SDA/SCL, Pullups, Kabellänge prüfen

---

## Roadmap / Ideen

* Optional: MQTT Discovery (Home Assistant)
* Konfigurierbarer Base-Topic
* Alternative Scanner-Modi
* UI-Verbesserungen

---

## Lizenz

Noch festzulegen (empfohlen: **MIT License**)
