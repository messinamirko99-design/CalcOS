![Bruce Main Menu](./media/pictures/bruce_banner.jpg)

# :shark: Bruce — BARTOS Remix

This is a community remix of [Bruce firmware](https://github.com/pr3y/Bruce) adding two new board variants designed to run **without a built-in TFT LCD display**, making Bruce accessible on bare ESP32 boards and custom hardware.

> **Upstream:** Bruce v1.14 by [@pr3y](https://github.com/pr3y) and contributors.
> **Remix by:** [@lol753](https://github.com/lol753)

---

## :new: New Board Variants

### BARTOS
A hardware-first variant with a 2-button + I2C 16×2 LCD interface.

| Component | GPIO |
|-----------|------|
| LCD SDA | 21 |
| LCD SCL | 22 |
| SCROLL button (next) | this button is on the gpio0 and if you are using a esp32 dev kit its the second button after reboot |
| SELECT button (confirm/back) | 26 → GND |

- **I2C auto-detect** — scans `0x27` → `0x3F` → `0x20` → `0x38` at boot
- **Backlight timeout** — LCD backlight turns off after 30s of inactivity, any key wakes it
- **Marquee scroll** — labels longer than 16 chars scroll automatically on the LCD
- **SCROLL** = next item / **SELECT short press** = confirm / **SELECT long press (>700ms)** = back/ESC
- Text input: serial console when available, char picker as fallback

### CYD-Console
A headless variant for the CYD-2432S028 (or any ESP32) with serial console navigation only. No display required.

| Key | Action |
|-----|--------|
| `w` / `a` / `↑` / `←` | Previous |
| `s` / `d` / `↓` / `→` | Next |
| `Enter` / `Space` | Select |
| `q` / `Backspace` / `ESC` | Back |

---

## :building_construction: How to install

### Flash with ESP32 Flash Download Tool

Use these three files from the release (or your own build output):

| File | Address |
|------|---------|
| `bootloader.bin` | `0x1000` |
| `partitions.bin` | `0x8000` |
| `firmware.bin` | `0x10000` |

1. Open the tool → select chip **ESP32**
2. Add the 3 files and set addresses as above ✅
3. SPI Speed: 40MHz · SPI Mode: DIO · Flash Size: 4MB
4. Hold **BOOT**, click **START**, release after ~2s
5. Press **EN/RST** to reboot into the firmware

### Flash with esptool.py

```sh
esptool.py --port /dev/ttyACM0 --baud 921600 \
  write_flash \
  0x1000  bootloader.bin \
  0x8000  partitions.bin \
  0x10000 firmware.bin
```

### Build from source

```sh
pio run -e BARTOS --target upload
# or
pio run -e CYD-Console --target upload
```

Build output lands at `.pio/build/<env>/firmware.bin`.

---

## :keyboard: Discord Server (upstream)

Contact the Bruce team in their [Discord Server](https://discord.gg/WJ9XF9czVT).

## :bookmark_tabs: Wiki

For features and usage, see the [upstream Bruce wiki](https://github.com/pr3y/Bruce/wiki).

---

## :computer: List of Features

<details>
  <summary><h2>WiFi</h2></summary>

- [x] Connect to WiFi
- [x] WiFi AP
- [x] Disconnect WiFi
- [x] [WiFi Atks](https://github.com/pr3y/Bruce/wiki/WiFi#wifi-atks)
  - [x] [Beacon Spam](https://github.com/pr3y/Bruce/wiki/WiFi#beacon-spam)
  - [x] [Target Atk](https://github.com/pr3y/Bruce/wiki/WiFi#target-atk)
    - [x] Information
    - [x] Target Deauth
    - [x] EvilPortal + Deauth
  - [x] Deauth Flood (More than one target)
- [x] [Wardriving](https://github.com/pr3y/Bruce/wiki/Wardriving)
- [x] [TelNet](https://github.com/pr3y/Bruce/wiki/WiFi#telnet)
- [x] [SSH](https://github.com/pr3y/Bruce/wiki/WiFi#ssh)
- [x] [RAW Sniffer](https://github.com/pr3y/Bruce/wiki/WiFi#raw-sniffer)
- [x] [TCP Client](https://github.com/pr3y/Bruce/wiki/WiFi#tcp-client)
- [x] [TCP Listener](https://github.com/pr3y/Bruce/wiki/WiFi#tcp-listener)
- [x] [Evil Portal](https://github.com/pr3y/Bruce/wiki/WiFi#evil-portal) — captured credentials shown live on LCD
- [x] [Scan Hosts](https://github.com/pr3y/Bruce/wiki/WiFi#evil-portal) (with TCP Port scanning)
- [x] [Responder](https://github.com/BruceDevices/firmware/wiki/WiFi#responder)
- [x] [Arp Spoofing](https://github.com/BruceDevices/firmware/wiki/WiFi#arp-spoofing)
- [x] [Arp Poisoning](https://github.com/BruceDevices/firmware/wiki/WiFi#arp-poisoning)
- [x] [Wireguard Tunneling](https://github.com/pr3y/Bruce/wiki/WiFi#wireguard-tunneling)
- [x] Brucegotchi
  - [x] Pwnagotchi friend
  - [x] Pwngrid spam faces & names

</details>

<details>
  <summary><h2>BLE</h2></summary>

- [X] [BLE Scan](https://github.com/pr3y/Bruce/wiki/BLE#ble-scan)
- [X] Bad BLE - Run Ducky scripts
- [X] iOS Spam
- [X] Windows Spam
- [X] Samsung Spam
- [X] Android Spam
- [X] Spam All
</details>

<details>
  <summary><h2>RF</h2></summary>

- [x] Scan/Copy
- [x] [Custom SubGhz](https://github.com/pr3y/Bruce/wiki/RF#replay-payloads-like-flipper)
- [x] Spectrum
- [x] Jammer Full
- [x] Jammer Intermittent
- [x] Config (TX Pin, RX Pin, Module, Frequency)
- [x] Replay
</details>

<details>
  <summary><h2>RFID</h2></summary>

- [x] Read tag / Read 125kHz
- [x] Clone tag
- [x] Write NDEF records
- [x] Amiibolink / Chameleon
- [x] Write / Erase / Save / Load
- [x] Config (PN532, PN532Killer)
</details>

<details>
  <summary><h2>IR</h2></summary>

- [x] TV-B-Gone
- [x] IR Receiver
- [x] Custom IR (NEC, NECext, SIRC, Samsung32, RC5, RC6…) — file browser navigable on LCD
- [x] Config (TX Pin, RX Pin)
</details>

<details>
  <summary><h2>FM</h2></summary>

- [x] Broadcast standard / reserved / stop
</details>

<details>
  <summary><h2>NRF24</h2></summary>

- [X] NRF24 Jammer
- [X] 2.4G Spectrum
</details>

<details>
  <summary><h2>Scripts</h2></summary>

- [X] [JavaScript Interpreter](https://github.com/pr3y/Bruce/wiki/Interpreter)
</details>

<details>
  <summary><h2>Others</h2></summary>

- [X] Mic Spectrum
- [X] QRCodes (Custom, PIX)
- [x] SD Card Mngr — file browser shown on LCD with folder + filename
- [x] LittleFS Mngr
- [x] WebUI — IP and credentials shown on LCD when active
- [x] Megalodon
- [x] BADUsb
- [x] iButton
- [x] LED Control
</details>

<details>
  <summary><h2>Clock</h2></summary>

- [X] RTC Support
- [X] NTP time adjust
- [X] Manual adjust
</details>

<details>
  <summary><h2>Connect (ESPNOW)</h2></summary>

- [X] Send / Receive File
- [X] Send / Receive Commands
</details>

<details>
  <summary><h2>Config</h2></summary>

- [x] Brightness (LCD backlight on/off)
- [x] Dim Time (30s default, configurable via `BACKLIGHT_TIMEOUT_MS`)
- [X] UI Color
- [x] Boot Sound on/off
- [x] Clock / Sleep / Restart
</details>

---

## Board Compatibility

| Device | CC1101 | NRF24 | FM | PN532 | Mic | BadUSB | Speaker |
| --- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **BARTOS** (this remix) | :ok: | :ok: | :ok: | :ok: | :x: | :ok:¹ | :x: |
| **CYD-Console** (this remix) | :ok: | :ok: | :ok: | :ok: | :x: | :ok:¹ | :x: |
| [JCZN CYD‑2432S028](https://www.aliexpress.us/item/3256804774970998.html) (upstream) | :ok: | :ok: | :ok: | :ok: | :x: | :ok:¹ | :x: |
| [M5Stack Cardputer](https://shop.m5stack.com/products/m5stack-cardputer-kit-w-m5stamps) | :ok: | :ok: | :ok: | :ok: | :ok: | :ok: | NS4168 |
| [Lilygo T‑Deck](https://lilygo.cc/products/t-deck) | :ok: | :x: | :x: | :x: | :x: | :ok: | :x: |
| [Lilygo T‑Embed CC1101](https://lilygo.cc/products/t-embed-cc1101) | :ok: | :ok: | :ok: | :ok: | :ok: | :ok: | :ok: |

For the full upstream device table see [Bruce's README](https://github.com/pr3y/Bruce).

¹ BadUSB on CYD/bare ESP32: see [wiki](https://github.com/pr3y/Bruce/wiki/Others#badusb)

---

## :sparkles: Why this remix?

Bruce is an incredible toolkit, but it assumes a TFT display is always present. This remix opens it up to bare ESP32 boards, DIY builds, and anyone who wants to run Bruce on a custom PCB with just a cheap I2C LCD and two buttons — or even headless over serial.

---

## :clap: Acknowledgements

**Upstream Bruce team:**
+ [@pr3y](https://github.com/pr3y) — original Bruce firmware
+ [@bmorcelli](https://github.com/bmorcelli) for new core and device ports
+ [@IncursioHack](https://github.com/IncursioHack) for RF and RFID modules
+ [@Luidiblu](https://github.com/Luidiblu) for logo and UI design
+ [@eadmaster](https://github.com/eadmaster) for many features
+ [@rennancockles](https://github.com/rennancockles) for RFID refactoring
+ [@7h30th3r0n3](https://github.com/7h30th3r0n3) for WiFi attacks
+ [@Tawank](https://github.com/Tawank) for interpreter refactoring
+ Everyone who contributed — thanks :heart:

**This remix:**
+ [@lol753](https://github.com/lol753) — BARTOS & CYD-Console board variants, I2C LCD driver, 2-button navigation, marquee scroll, backlight timeout, I2C auto-detect

---

## :construction: Disclaimer

Bruce is a tool for cyber offensive and red team operations, distributed under the AGPL. Intended for legal and authorized security testing only. Use of this software for malicious or unauthorized activities is strictly prohibited. The developers assume no liability for misuse. Use at your own risk.
