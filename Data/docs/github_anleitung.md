# Git-Verwaltung und lokale Ausfuehrung

## Repository herunterladen

Um das Projekt lokal herunterzuladen, klone das Git-Repository:

```bash
git clone https://github.com/bonsaibauer/projekt_reaktionstester.git
cd dhbw_project_dip
```

## Standard-Git-Befehle

### Status pruefen

```bash
git status
```

### Aenderungen hinzufuegen

```bash
git add .
# oder einzelne Datei:
# git add pfad/zur/datei
```

### Commit erstellen

```bash
git commit -m "Beschreibung der Aenderung"
```

### Aenderungen zum Remote pushen

```bash
git push origin main
```

### Branch erstellen & wechseln

```bash
git checkout -b feature/mein-branch
```

### Aenderungen vom Remote laden

```bash
git pull origin main
```

## Pull Request erstellen

1. Aenderungen pushen  
2. Auf GitHub das Repository oeffnen:  
   [https://github.com/bonsaibauer/dhbw_project_dip ](https://github.com/bonsaibauer/projekt_reaktionstester) 
3. Einen **Pull Request (PR)** fuer deinen Branch erstellen  
4. Aenderungen und Zweck kurz beschreiben

