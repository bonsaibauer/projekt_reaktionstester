# Block Dodge – Codeübersicht (Data/app/main.c)

## Aufbau
- **Hardware-Init (`Init_HW`)**: Takt, Timer0 (TA0), Button-Pullups, Display-Init (ST7735). Timer0 läuft frei und dient als Zeitbasis.
- **Persistenz**: Highscore-Struktur (`flashScores`) liegt im InfoD-Flash-Segment. Funktionen `SaveHighscores`, `ResetHighscores`, `EnsureHighscores` sichern Top-3.
- **Eingabe**: Vier Tasten (Start/Back/Up/Down) werden als Pegel gelesen. `Input()` wertet Flanken aus und verschiebt das Auto (`cur_lane`) zwischen 5 Spuren.
- **Grafik**: `Rect` und `DrawCar` zeichnen Rechtecke. Spielfeldbreite 128px, 5 Spuren mit x-Koordinaten aus `lx[]`.
- **Gameplay (`PlayGame`)**: Tutorial-Sequenz, dann Endlosschleife mit Obstacle-Update, Kollision, Scoring und adaptiver Spawn-Rate.
- **Menüs**: Hauptmenü (Single/Multi), Namenseingabe, Highscore-Anzeige, Turnier-Flow für 2–4 Spieler.

## Feste Werte (wichtige Defines)
- Farben: `C_BLK`, `C_WHT`, `C_GRN`, `C_RED`, `C_YEL`, `C_BLU`, `C_GRY`.
- Auto: Y-Position `CAR_Y = 100`, Breite/Höhe `CAR_W = CAR_H = 18`.
- Spuren: `lx[] = {4, 29, 54, 79, 104}` (5 Lanes).
- Obstacle-Limits: `MAX_OB = 8`.
- Name: `NAME_LEN = 4`.
- Highscore-Marker: `HIGHSCORE_MAGIC = 0xA55A`.
- Buttons (Ports): `BTN_START = BIT1 (P1)`, `BTN_BACK = BIT1 (P2)`, `BTN_UP = BIT0 (P4)`, `BTN_DOWN = BIT7 (P3)`.

## Spielgeschwindigkeit
- Start-Delay: `ticks = 2000` Timer-Takte pro Frame (kleiner = schneller).
- Beschleunigung: Alle 5 Punkte wird `ticks` um `200` verringert, solange `ticks > 200`. Minimalwert somit `200`.
- Hindernis-Geschwindigkeit: `min_speed = 2`, tatsächliche Block-Geschwindigkeit `spd = min_speed + rand()%3` (also 2–4 px/FW).
- Ziel-Draw-Load: `TARGET_LOAD = 6` sorgt mit Dummy-Zeichnen für konstante Rechenzeit je Frame.

## Spiellogik in Kurzform
1. Tutorial zwingt den Spieler einmal nach rechts und wieder nach links.
2. Hauptschleife:
   - Eingabe lesen (`Input`), Auto neu zeichnen.
   - Schwierigkeit: `max_active` steigt bei Scores >10/25/50/100.
   - Hindernisse: Bis zu `max_active` gleichzeitig; Spawn-Chance 40%. Spawn nur, wenn Spur im oberen Bereich frei ist.
   - Bewegung: Jedes aktive Hindernis wird nach unten verschoben und neu gezeichnet; Kollision prüft Rechtecküberlappung mit Spieler-Auto.
   - Scoring: Wenn Block unterhalb 128px ist, Score++ und Slot wird frei.
   - Frame-Timing: `Wait(start, ticks)` hält die Schleife auf die gewünschte Dauer und pollt weiter Eingaben.
3. Crash: Roter Screen, Score-Anzeige, Rückkehr mit Score-Wert.

## Highscore-/Multiplayer-Flow
- **Single**: Name eingeben → Spiel → Score anzeigen → Highscore aktualisieren (`UpdateHighscoreTable`).
- **Multi (2–4 Spieler)**: Namen nacheinander → jeder spielt → Scores werden per Bubble-Sort absteigend gelistet → optional Highscore-Screen.

## Wichtige Variablen
- `cur_lane`: aktuelle Spur des Autos (0–4).
- `obs[MAX_OB]`: Hindernis-Array mit `active`, `lane`, `x`, `y`, `spd`.
- `score`: aktuelle Punktezahl im Lauf.
- `last_speed_score`: merkt sich letzte Schwelle für Geschwindigkeitsreduktion.

## Anpassungstipps
- Schneller/langsamer Start: `ticks`-Startwert anpassen.
- Härtere Beschleunigung: Schrittgröße (`200`) ändern oder Schwellen (alle 5 Punkte) anpassen.
- Mehr/Weniger Verkehr: Startwert und Stufen in `max_active` ändern oder Spawn-Prozentsatz (40%) anpassen.
- Höhere Blockgeschwindigkeit: `min_speed` anheben.
