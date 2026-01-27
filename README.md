# Projekt 1: Reaktionszeitmessung

## Einordnung
Projekt im Rahmen der Vorlesung **Interface Synthesis** (DHBW Mannheim).

## Bewertung
- 17 Prozentpunkte der Gesamtbewertung

## Abgabe
- **Die Abgabe umfasst den Quellcode.**

## Projektbeschreibung (laut Folien)
- Verwenden Sie zumindest die Knöpfe, LEDs und das Display zur Bedienung.
- Eine Reaktionsbestimmung nutzt mehrere Messungen, um Glückstreffer auszuschließen.
- Betrug und Fehlbedienung ist auszuschließen.
- Es wird über die besten Zeiten der Spieler und/oder der letzten Reaktionsbestimmung informiert.
- Die Aufgabe ist vage formuliert, um viel Spielraum zu geben.
- Ziel ist ein intuitives Spiel, dessen Anwendung Spaß macht und auf verständlichem Quellcode basiert,
  welcher einfach zu erweitern ist.

## Bewertungskriterien (notwendig für 1.0)
- Einfache, intuitive und konsistente Bedienung
- System lässt sich nicht austricksen
- Keine Bugs
- Hohe Qualität des Quellcodes

## Extras
- Extras sind erwünscht.
- Abzüge lassen sich durch Extras kompensieren, z. B.:
  - Persistente Wertespeicherung
  - Mehrspielermodus
  - Eingabe des Benutzernamens für Highscore

## Termin
- Vorstellung und Abgabe: **19.02.2026**

## Projekt ausführen

### Schnellstart (Simulation)
1. `MSPSIMPortable.exe` im Projektwurzelverzeichnis doppelklicken. Es öffnet sich eine vorkonfigurierte portable VS‑Code‑Instanz.
2. In VS Code `View → Appearance → Secondary Side Bar` aktivieren, damit die vorbereitete Simulationsleiste rechts sichtbar ist.
3. Im Explorer `Data/app/main.c` (oder die gewünschte Versuchsversion) öffnen.
4. Im rechten Bereich auf den Reiter/Knopf **Simulierung** gehen und dort die Simulation starten.

### Auf echter Hardware (BoostXL‑EDUMKII)
1. Educational BoosterPack MKII (Modell **BoostXL‑EDUMKII** von Texas Instruments) auf das MSP430‑LaunchPad stecken. USB zuerst verbinden, damit die Stromversorgung steht, dann das BoosterPack aufstecken.
2. `MSPSIMPortable.exe` starten, Secondary Side Bar wie oben einblenden und `Data/app/main.c` öffnen.
3. Im rechten **Simulierung**‑Panel den Flash/Run‑Button nutzen, um die Firmware auf das Board zu laden. Warten, bis das Flashen abgeschlossen ist, erst dann das Board trennen.
4. Der BoosterPack‑Aufbau zur Orientierung:

   ![Educational BoosterPack MKII](../image/device.jpg)
