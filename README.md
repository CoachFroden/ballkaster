# Ballkaster

Prototype for ESP32-styrt ballkaster.

## Servo-test

Første test styrer én SG90 på **GPIO 18 / D18** fra mobil eller PC.

### Kobling
- SG90 signal (oransje/gul) → D18
- SG90 +5 V (rød) → separat regulert 5 V
- SG90 GND (brun/svart) → 5 V GND
- ESP32 GND → samme GND
- ESP32 → PC via USB-C ved programmering

Ikke koble servoen til 12 V.

### Arduino
Installer:
- ESP32 by Espressif Systems
- ESP32Servo

Velg:
- Board: ESP32 Dev Module
- Upload speed: 115200

Åpne:
`firmware/ballkaster_servo_test/ballkaster_servo_test.ino`

Last opp til ESP32.

### Bruk appen
Etter oppstart lager ESP32 sitt eget Wi-Fi:
- Nettverk: **Ballkaster-Test**
- Passord: **balltest**

Koble telefon/PC til dette Wi-Fi-nettet og åpne **http://192.168.4.1** i nettleseren.

Appen har:
- vinkel 0–180°
- hastighet 1–100 %
- hurtigvalg 0° / 90° / 180°
- stoppknapp
- beregnet nåværende posisjon

> Merk: SG90 gir ikke faktisk posisjonsfeedback. Visningen er ESP32-ens beregnede posisjon basert på kommandoene den har sendt.
