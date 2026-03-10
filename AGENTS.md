# AGENTS.md

## Ziel
Dieses Repository stellt eine ESPHome-`external_component` fuer den DS2482 bereit
(Temperatursensoren und DS2413-Binary-Sensoren ueber 1-Wire).

## Projektstruktur
- `components/ds2482/__init__.py`:
  Einstieg der External Component, Namespace/Codegen-Definition, DS2482-Hub-Schema.
- `components/ds2482/ds2482.h`:
  Zentrale Hub-Klasse (`DS2482Component`) und 1-Wire-Methoden-Schnittstellen.
- `components/ds2482/ds2482.cpp`:
  Implementierung des DS2482-Hubs (I2C-Zugriff, Kanalwahl, 1-Wire-Befehle, Search, Scan-Logik).
- `components/ds2482/sensor.py`:
  YAML-Schema und Codegen fuer `sensor: - platform: ds2482` (Temperatur).
- `components/ds2482/ds2482_sensor.h`:
  C++-Klasse fuer Temperatursensor-Entitaet.
- `components/ds2482/ds2482_sensor.cpp`:
  Temperaturmessung (Convert T, Scratchpad lesen, CRC pruefen, Plausibilitaet).
- `components/ds2482/binary_sensor.py`:
  YAML-Schema und Codegen fuer `binary_sensor: - platform: ds2482` (DS2413).
- `components/ds2482/ds2482_binary_sensor.h`:
  C++-Klasse fuer DS2413-Binary-Sensor-Entitaet (PIO A/B, Invertierung).
- `components/ds2482/ds2482_binary_sensor.cpp`:
  DS2413-Leseablauf, Antwortvalidierung (Nibble-Check), Status-Publish und Diagnose-Logs.

## YAML-Einbindung (lokal)
```yaml
external_components:
  - source:
      type: local
      path: /config/esphome/components
    components: [ds2482]
```

## Hinweise fuer Aenderungen
- Bereits englische Kommentare im Quellcode bitte auf Englisch belassen.
- Neue Kommentare kurz und konsistent zum Dateikontext schreiben.
- Bei DS2413 immer das Antwortformat pruefen:
  Low-Nibble = Daten, High-Nibble = invertiertes Low-Nibble.
- Nach Anpassungen an `external_components` immer Clean Build durchfuehren:
  `esphome clean <datei>.yaml`

## Typische Fehlerbilder
- `Platform not found: 'binary_sensor.ds2482'`:
  `binary_sensor.py` fehlt im lokalen Komponentordner oder falscher `path`.
- `invalid DS2413 response`:
  falscher Kanal/Adresse, Leitungsproblem oder ungueltiges Antwortbyte.
