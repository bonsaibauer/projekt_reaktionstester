# Block Dodge - Codeuebersicht (`Data/app/main.c`)

## Ziel dieser Uebersicht
Diese Datei erklaert den Aufbau von `Data/app/main.c` so, dass man den Code schnell lesen, einordnen und gezielt anpassen kann.

## So liest man den Code am besten (Reihenfolge)
1. `main()` lesen: Welche States gibt es und wie springt das Programm zwischen Menues und Spiel.
2. `ShowHighscoreStartScreen()` und `EnterName()` lesen: Das erklaert den kompletten UI-Flow vor jedem Spielstart.
3. `PlayGame()` lesen: Kernlogik fuer Gameplay, Schwierigkeit und Spawn.
4. `Input()` lesen: Wie Links/Rechts im Spiel verarbeitet wird.
5. `Init_HW()` und `UpdateHighscore()` lesen: Hardware-Setup und persistente Speicherung.

## Datei-Aufbau (von oben nach unten)
- Konfiguration: Farben, Fahrzeuggroesse, Anzahl Hindernis-Slots, Button-Bits.
- Globale Daten:
- `lx[]` Spur-X-Positionen.
- `obs[]` aktive Hindernisse.
- `cur_lane`, `s1_old`, `s2_old` fuer Fahrspur und Flankenerkennung.
- `flashHighscore` als persistenter Datensatz im InfoD-Flash.
- Basisfunktionen:
- `Init_HW()`, `UpdateHighscore()`, `Rect()`, `DrawCar()`, `WaitForRelease()`.
- Input-Funktionen:
- `ReadJoystickY()`, `ReadJoystickX()`, `Input()`, `Wait()`.
- UI-Funktionen:
- `ShowHighscoreStartScreen()`, `EnterName()`.
- Gameplay:
- `PlayGame()`.
- Steuerzentrale:
- `main()`.

## Zentrale Daten und Bedeutung
- `flashHighscore.name[3]`, `flashHighscore.score`:
- Ein gemeinsamer Highscore fuer Single und Multi.
- `cur_lane`:
- Aktuelle Spur des Spielerfahrzeugs (`0..4`).
- `obs[MAX_OB]`:
- Hindernisse mit `active, lane, x, y, spd`.
- `state` in `main()`:
- `0` Hauptmenue.
- `1` Singleplayer.
- `2` Multiplayer.

## Menue- und State-Flow
### `state 0` (Hauptmenue)
- Zeigt `SINGLE` und `MULTI`.
- Auswahl per Up/Down oder Joystick Y.
- Start per `START` oder Joystick rechts (`joy_x > 2600`).

### `state 1` (Singleplayer)
- Erst gemeinsamer Highscore-Vorschirm ueber `ShowHighscoreStartScreen()`.
- Bei `Start` folgt `EnterName()`, danach `PlayGame()`.
- Bei neuem Rekord wird `UpdateHighscore()` aufgerufen.
- Wenn in der Namenseingabe `Cancel` gewaehlt wird, Ruecksprung ins Hauptmenue.

### `state 2` (Multiplayer)
- Erst derselbe Highscore-Vorschirm wie in Singleplayer.
- Danach Screen `NUMBER OF PLAYERS` mit Auswahl `2/3/4`.
- Fuer jeden Spieler: `EnterName()` dann `PlayGame()`.
- Bei `Cancel` in der Namenseingabe eines Spielers: kompletter Ruecksprung ins Hauptmenue.
- Ergebnisse werden sortiert angezeigt; bester Score aktualisiert bei Bedarf den gemeinsamen Highscore.

## Eingabelogik kompakt
- Im Spiel (`Input()`):
- Links: `BACK` oder Joystick X `< 1500`.
- Rechts: `START` oder Joystick X `> 2600`.
- Deadzone: `1800..2300` (damit Joystick nicht dauernd ausloest).
- In Menues:
- Vertikal mit Up/Down oder Joystick Y (`>3072` hoch, `<1024` runter).
- Start/Back je nach Screen ueber Button und teilweise Joystick X.

## Namenseingabe (`EnterName`)
- 2 Zeichen, plus finaler Start-Confirm.
- Anzeige unten:
- Kein Zeichen bestaetigt: `Cancel     Ok`.
- Nach 1 Zeichen: `Backspace  Ok`.
- Nach 2 Zeichen: `Backspace  Start`.
- Verhalten links:
- Bei noch keinem bestaetigten Zeichen: `Cancel` (`return 0`).
- Danach: `Backspace` (ein Zeichen zurueck).

## Gameplay-Kern (`PlayGame`)
- Startet mit Tutorial (`Right >`, `Left <`, dann `Ready ?`).
- Danach Endlos-Loop bis Crash:
- Input lesen.
- Hindernisse bewegen/zeichnen/entfernen.
- Kollision pruefen.
- Neue Hindernisse spawnen.
- Auf Framerate ueber `Wait(start, ticks)` synchronisieren.

## Schwierigkeit (aktueller Stand)
- Startwerte:
- `max_active = 3`
- `min_speed = 2`
- `ticks = 1900`
- Progression:
- `score > 5` -> `max_active = 3`
- `score > 15` -> `max_active = 4`
- `score > 35` -> `max_active = 5`, `min_speed = 3`
- Alle 3 Punkte wird `ticks` um `300` reduziert (bis Minimum-Grenze).
- Spawn-Chance:
- `40 + score/10`, gedeckelt bei `59`.

## Wo aendere ich was?
- Menue-Texte/Screen-Flow: `main()`, `ShowHighscoreStartScreen()`, `EnterName()`.
- Steuerung: `Input()` und Menue-Schleifen in `main()`.
- Schwierigkeit: Startwerte und Skalierung in `PlayGame()`.
- Highscore-Verhalten: `UpdateHighscore()` und Aufrufe in `state 1`/`state 2`.
