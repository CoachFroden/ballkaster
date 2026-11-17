# Ballkaster

ESP32-styrt ballkaster med to 775 DC-motorer via IBT-2 og steppermotor via ULN2003A og stepperdriver.

## Kontrollpanel

Den nye metallisk blå kontrollappen ligger i `index.html` (kopi i `web/index.html`). Foreløpig er den en **demo**: knappene styrer ikke fysisk utstyr.

For nettvisning kan GitHub Pages aktiveres fra Settings → Pages → Deploy from a branch → `main` → `/ (root)`.

Når GitHub Pages er aktivert, blir adressen `https://coachfroden.github.io/ballkaster/`.

## Planlagt ESP32-integrasjon

ESP32 skal opprette eget Wi-Fi-nettverk. Kontrollappen serveres lokalt på `http://192.168.4.1` uten internett. Før reell motordrift kreves blant annet fysisk nødstopp, tilkoblingsovervåking og kalibrert vinkelreferanse.

## Motorpinner

- Motor 1: RPWM GPIO26, LPWM GPIO27, EN GPIO17
- Motor 2: RPWM GPIO14, LPWM GPIO13, EN GPIO16
- Stepper via ULN2003A: STEP GPIO4, DIR GPIO22, EN GPIO23
