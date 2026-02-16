# Block Dodge - Codeübersicht (`Data/app/main.c`)

## Aufbau
- **Hardware-Init (`Init_HW`)**: Stoppt den Watchdog, konfiguriert Clock/FLL, startet `TA0` im Continuous-Mode, setzt Button-Pullups, aktiviert ADC12 und initialisiert das ST7735-Display.
- **Persistenz**: Ein einzelner Highscore (`flashHighscore`) liegt im InfoD-Flash (`.infoD`) als `{ name[3], score }`.
- **Eingabe**: Tasten plus Joystick (ADC X/Y). `Input()` verarbeitet Flanken für Tasten und eine Deadzone-Logik fuer den Joystick.
- **Rendering**: `Rect()` als Rechteck-Helfer, `DrawCar()` zeichnet das Spieler-Auto in einer von 5 Spuren.
- **Spielkern (`PlayGame`)**: Tutorial (rechts/links), dann Frame-Loop mit Obstacle-Update, Kollision, Spawn und Difficulty-Scaling.
- **State-Machine in `main()`**: Hauptmenue (`state 0`), Singleplayer (`state 1`), Multiplayer (`state 2`).

## Wichtige Konstanten
- Farben: `C_BLK`, `C_WHT`, `C_GRN`, `C_RED`, `C_YEL`, `C_BLU`, `C_GRY`.
- Auto: `CAR_Y = 100`, `CAR_W = 18`, `CAR_H = 18`.
- Spuren: `lx[] = {4, 29, 54, 79, 104}`.
- Hindernis-Slots: `MAX_OB = 8`.
- Buttons:
- `BTN_START = BIT1` auf Port `P1`.
- `BTN_BACK = BIT1` auf Port `P2`.
- `BTN_UP = BIT0` auf Port `P4`.
- `BTN_DOWN = BIT7` auf Port `P3`.

## Eingabe und Steuerung
- **Tasten links/rechts waehrend Spiel**: `BACK` (P2.1) bewegt nach links, `START` (P1.1) nach rechts.
- **Joystick X im Spiel**:
- `< 1500` bewegt nach links.
- `> 2600` bewegt nach rechts.
- Zwischen `1800..2300` wird das "moved"-Latch zurueckgesetzt (Deadzone).
- **Joystick Y in Menues/Namenseingabe**:
- `> 3072` entspricht "hoch".
- `< 1024` entspricht "runter".

## Gameplay (`PlayGame`)
1. Optionaler Multiplayer-Hinweis: `"It's <Name>'s turn!"`.
2. Tutorial: einmal nach rechts, einmal nach links bewegen.
3. Spielstart mit:
- `score = 0`
- `max_active = 2`
- `min_speed = 2`
- `ticks = 2600` (Frame-Dauer in Timer-Ticks)
4. Pro Frame:
- Eingabe lesen (`Input()`).
- Aktive Hindernisse loeschen/verschieben/neu zeichnen.
- Kollision pruefen (Spur + Y-Ueberlappung mit Spielerfahrzeug).
- Bei `y > 128`: Hindernis entfernen und `score++`.
- Spawn pruefen, wenn `active < max_active`.
- Timing mit `Wait(start, ticks)` (pollt dabei weiter Eingaben).

## Difficulty-Scaling
- `max_active` steigt mit dem Score:
- `> 5` -> `3`
- `> 15` -> `4`
- `> 35` -> `5` und `min_speed = 3`
- Geschwindigkeitserhoehung:
- Wenn `score % 3 == 0` und `ticks > 400`, dann `ticks -= 300`.
- Spawn-Chance:
- `spawn_chance = 30 + score/10`, gedeckelt auf `55`.
- Spawn-Speed:
- `spd = min_speed + rand()%2` (also 2-3, spaeter 3-4).

## Spawn-Regeln
- Spawn nur in freier Spur im oberen Bereich (`kein aktives Obstacle in gleicher Spur mit y < 40`).
- Zusaetzliche Blockade: Wenn bereits `>= 4` Hindernisse im Bereich `-25 <= y < 35` aktiv sind, wird Spawn verworfen.
- Maximal `15` Spawn-Versuche pro Spawn-Ereignis.

## Menue- und Modus-Flow
- **`state 0` (Hauptmenue)**:
- Auswahl `SINGLE` oder `MULTI` per Joystick Y / Up/Down.
- `START` wechselt in den gewaehlten Modus.
- `BACK` setzt den gespeicherten Highscore auf Name `"  "` und Score `0`.
- **`state 1` (Singleplayer)**:
- Zeigt aktuellen Highscore (oder `No Record`).
- Bei `START`: Namenseingabe (`2` Zeichen) -> Spiel -> Score-Anzeige -> ggf. neuer Highscore via `UpdateHighscore()`.
- **`state 2` (Multiplayer)**:
- Auswahl 2/3/4 Spieler.
- Jeder Spieler gibt 2 Zeichen Namen ein und spielt nacheinander.
- Ergebnisse werden per Bubble-Sort absteigend sortiert und angezeigt.

## Highscore-Implementierung
- Nur **ein** gespeicherter Rekord (kein Top-3-Board).
- Flash wird in `UpdateHighscore()` geloescht und direkt neu beschrieben.
- Initialisierung in `main()`:
- Wenn `flashHighscore.score == 0xFFFF`, wird auf leeren Namen und Score `0` gesetzt.

## Relevante globale Variablen
- `cur_lane`: Aktuelle Spieler-Spur (`0..4`).
- `obs[MAX_OB]`: Hindernisliste mit `{ active, lane, x, y, spd }`.
- `s1_old`, `s2_old`: Flankenerkennung fuer linke/rechte Taste.
- `flashHighscore`: persistenter Einzel-Highscore im Flash.

## Hinweise fuer Anpassungen
- Starttempo: `ticks` in `PlayGame()` aendern.
- Progression: Schwellen fuer `max_active` oder Schrittweite bei `ticks -= 300` aendern.
- Verkehrsaufkommen: `spawn_chance` oder `MAX_OB` anpassen.
- Schwierigkeitsspitze: `min_speed` und Spawn-Regeln im oberen Bereich justieren.
