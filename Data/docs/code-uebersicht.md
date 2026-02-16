# Block Dodge - Spielueberblick und Einstellungen

## Was bietet das Spiel?
- Einfache Arcade-Idee: Hindernissen ausweichen und moeglichst lange ueberleben.
- Singleplayer-Modus.
- Multiplayer-Modus mit 2, 3 oder 4 Spielern (nacheinander, danach Rangliste).
- Persistenter Highscore im Flash (bleibt nach Neustart erhalten).
- Namenseingabe vor jedem Lauf.
- Name-Duplikat-Schutz:
- Highscore-Name kann nicht erneut verwendet werden (wenn bereits ein Record existiert).
- Im Multiplayer koennen Spieler nicht denselben Namen waehlen.

## Wie laeuft das Spiel ab?
1. Hauptmenue: `SINGLE` oder `MULTI` waehlen.
2. Highscore-Startscreen anzeigen.
3. Namen eingeben.
4. Spiel starten.
5. Bei Kollision: Score anzeigen, ggf. Highscore aktualisieren.
6. In Multi: Alle Spieler spielen, dann sortierte Ergebnisliste.

## Steuerung
- Im Menue: Up/Down oder Joystick Y.
- Bestaetigen: Start-Taste oder Joystick nach rechts.
- Zurueck: Back-Taste oder Joystick nach links.
- Im Spiel: Spurwechsel links/rechts mit Back/Start oder Joystick X.

## Was kann man im Code einstellen?
Alle Anpassungen sind in `Data/app/main.c`.

- Farben:
- `C_BLK`, `C_WHT`, `C_GRN`, `C_RED`, `C_YEL`, `C_BLU`, `C_GRY`
- Wichtiger Hinweis: Wegen Display-Farbreihenfolge sind `C_RED` und `C_BLU` bereits passend gemappt.

- Spielfeld/Fahrzeug:
- `CAR_Y`, `CAR_W`, `CAR_H`
- Spurpositionen in `lx[]`
- Anzahl Hindernis-Slots: `MAX_OB`

- Schwierigkeitsgrad:
- Startwerte in `PlayGame()`:
- `max_active`, `min_speed`, `ticks`
- Progression:
- Schwellen bei `score > 5`, `> 15`, `> 35`
- Spawn-Formel:
- `spawn_chance = 40 + (score / 10)` mit Deckel `59`

- Menue und Modi:
- Hauptmenue-Flow in `main()` ueber `state`
- Multiplayer-Anzahl ueber `multiSel` (`2/3/4 Players`)

- Namenseingabe:
- UI und Eingabelogik in `EnterName(...)`
- Name hat aktuell 2 Buchstaben
- Meldung bei Konflikt: `Name taken`

- Highscore:
- Struktur: `flashHighscore` (`name`, `score`)
- Schreiben in Flash: `UpdateHighscore(...)`

## Was kann der aktuelle Code noch nicht?
- Kein Pause-Menue im laufenden Spiel.
- Keine frei waehlbare Schwierigkeit im Menue.
- Keine Soundeffekte/Musik.
- Keine erweiterten Namen (nur 2 Zeichen, nur A-Z).
- Keine getrennten Highscore-Listen pro Modus oder pro Spieler.
- Keine Runtime-Settings (Aenderungen nur im Code).

## Wichtige Funktionen (kurz)
- `main()`: Menue-Logik und kompletter Programmablauf.
- `ShowHighscoreStartScreen()`: Startscreen mit Highscore-Anzeige.
- `EnterName(...)`: Namenseingabe plus Duplikatpruefung.
- `PlayGame(...)`: Gameplay, Kollision, Spawn, Difficulty.
- `Input()`: Eingabe waehrend des Spiels.
- `UpdateHighscore(...)`: Highscore ins Flash speichern.
