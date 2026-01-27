#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include "ST7735.h"

// --- KONFIGURATION & FARBEN ---
#define C_BLK 0x000000
#define C_WHT 0xFFFFFF
#define C_GRN 0x00FF00
#define C_RED 0xFF0000
#define C_YEL 0x00FFFF
#define C_BLU 0x0000FF
#define C_GRY 0x555555

#define CAR_Y 100
#define CAR_W 18
#define CAR_H 18
#define MAX_OB 8
#define NAME_LEN 4
#define HIGHSCORE_MAGIC 0xA55A

// --- BUTTON MAPPING ---
#define BTN_START BIT1
#define BTN_BACK  BIT1
#define BTN_UP    BIT0
#define BTN_DOWN  BIT7

const int lx[] = {4, 29, 54, 79, 104}; // Spuren-Koordinaten

// --- STRUKTUREN & GLOBALE VARIABLEN ---
typedef struct { int active, lane, x, y, spd; } Obstacle;
typedef struct { char name[NAME_LEN]; unsigned int score; } ScoreEntry;
typedef struct { unsigned int magic; ScoreEntry entries[3]; } HighscoreData;

Obstacle obs[MAX_OB];
int cur_lane = 2, s1_old = 1, s2_old = 1;

// Flash Speicher Segment für Highscore Tabelle (Top 3)
volatile HighscoreData __attribute__((section(".infoD"))) flashScores;

// --- HARDWARE & SPEICHER FUNKTIONEN ---

void SaveHighscores(HighscoreData *src) {
    unsigned int *dst = (unsigned int *)&flashScores;
    unsigned int *s = (unsigned int *)src;
    unsigned int words = sizeof(HighscoreData) / 2;

    FCTL3 = FWKEY;
    FCTL1 = FWKEY + ERASE;
    *dst = 0; // Start Erase
    FCTL1 = FWKEY + WRT;
    unsigned int i;
    for (i = 0; i < words; i++) dst[i] = s[i];
    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;
}

void ResetHighscores() {
    HighscoreData tmp;
    tmp.magic = HIGHSCORE_MAGIC;
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < NAME_LEN; j++) tmp.entries[i].name[j] = 'A';
        tmp.entries[i].score = 0;
    }
    SaveHighscores(&tmp);
}

void EnsureHighscores() {
    if (flashScores.magic != HIGHSCORE_MAGIC) ResetHighscores();
}

void InsertScore(HighscoreData *data, char name[NAME_LEN], unsigned int score) {
    int i, j;
    for (i = 0; i < 3; i++) {
        if (score > data->entries[i].score) {
            for (j = 2; j > i; j--) data->entries[j] = data->entries[j - 1];
            for (j = 0; j < NAME_LEN; j++) data->entries[i].name[j] = name[j];
            data->entries[i].score = score;
            break;
        }
    }
}

void UpdateHighscoreTable(char name[NAME_LEN], unsigned int score) {
    EnsureHighscores();
    HighscoreData tmp = flashScores;
    InsertScore(&tmp, name, score);
    SaveHighscores(&tmp);
}

void Init_HW() {
    WDTCTL = WDTPW | WDTHOLD;
    UCSCTL3 |= SELREF_2;
    UCSCTL4 |= SELA_2;
    __bis_SR_register(SCG0);
    UCSCTL0 = 0;
    UCSCTL1 = DCORSEL_7; // Hoher Takt für Display
    UCSCTL2 = FLLD_1 + 380;
    __bic_SR_register(SCG0);

    TA0CTL = TASSEL_2 + ID_3 + MC_2;

    // Inputs konfigurieren (Pull-Ups etc. hardwareabhängig)
    P1DIR &= ~BTN_START; P1REN |= BTN_START; P1OUT |= BTN_START;
    P2DIR &= ~BTN_BACK;  P2REN |= BTN_BACK;  P2OUT |= BTN_BACK;
    P4DIR &= ~BTN_UP;    P4REN |= BTN_UP;    P4OUT |= BTN_UP;
    P3DIR &= ~BTN_DOWN;  P3REN |= BTN_DOWN;  P3OUT |= BTN_DOWN;

    ST7735_interface_init();
    ST7735_display_init();
}

// --- GRAFIK & INPUT HELFER ---

void Rect(int x, int y, int w, int h, unsigned long c) {
    if (x >= 0 && x < 128) draw(x, y, w, h, c);
}

void DrawCar(int l, unsigned long c) {
    Rect(lx[l], CAR_Y, CAR_W, CAR_H, c);
}

void WaitForRelease() {
    while (!(P1IN & BTN_START));
    __delay_cycles(2000000); // Warten bis Taste losgelassen
}

void Input() {
    int sl = (P2IN & BTN_BACK), sr = (P1IN & BTN_START);
    // Links Steuern
    if (!sl && s1_old && cur_lane > 0) {
        DrawCar(cur_lane, C_BLK);
        cur_lane--;
        DrawCar(cur_lane, C_BLU);
    }
    // Rechts Steuern
    if (!sr && s2_old && cur_lane < 4) {
        DrawCar(cur_lane, C_BLK);
        cur_lane++;
        DrawCar(cur_lane, C_BLU);
    }
    s1_old = sl;
    s2_old = sr;
}

// Wartet eine definierte Zeit und prüft dabei Inputs
void Wait(unsigned int start, unsigned int dur) {
    while ((TA0R - start) < dur) Input();
}

int SameName(const char *a, const char *b) {
    int i;
    for (i = 0; i < NAME_LEN; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

int NameTaken(char name[NAME_LEN + 1], char used[][NAME_LEN + 1], int usedCount) {
    EnsureHighscores();
    int i;
    for (i = 0; i < 3; i++) {
        if (SameName(name, flashScores.entries[i].name)) return 1;
    }
    if (used != 0) {
        for (i = 0; i < usedCount; i++) {
            if (SameName(name, used[i])) return 1;
        }
    }
    return 0;
}

// --- NAMENS-EINGABE & HIGHSCORE ANZEIGE ---

void EnterName(char *dest, int playerNum, char used[][NAME_LEN + 1], int usedCount) {
    WaitForRelease();
    while (1) {
        Rect(0, 0, 128, 128, C_BLK);
        char name[NAME_LEN] = {'A', 'A', 'A', 'A'};
        int idx = 0;
        char title[20];
        sprintf(title, "P%d NAME", playerNum);
        drawTextLine(2, 2, title, C_YEL, C_BLK);
        drawTextLine(9, 1, "Up/Down  Start>Next", C_GRN, C_BLK);

        int up_old = 1, down_old = 1, start_old = 1, back_old = 1;
        while (idx < NAME_LEN) {
            char buf[20];
            sprintf(buf, "%c %c %c %c", name[0], name[1], name[2], name[3]);
            drawTextLine(5, 2, buf, C_WHT, C_BLK);
            char arrow[9] = "        ";
            arrow[idx * 2] = '^';
            arrow[8] = 0;
            drawTextLine(6, 2, arrow, C_GRN, C_BLK);

            int up = (P4IN & BTN_UP);
            int down = (P3IN & BTN_DOWN);
            int start = (P1IN & BTN_START);
            int back = (P2IN & BTN_BACK);

            if (!up && up_old)   { name[idx] = (name[idx] == 'Z') ? 'A' : (name[idx] + 1); __delay_cycles(80000); }
            if (!down && down_old) { name[idx] = (name[idx] == 'A') ? 'Z' : (name[idx] - 1); __delay_cycles(80000); }
            if (!start && start_old) { idx++; __delay_cycles(80000); }
            if (!back && back_old && idx > 0) { idx--; __delay_cycles(80000); }

            up_old = up;
            down_old = down;
            start_old = start;
            back_old = back;
        }

        int i;
        for (i = 0; i < NAME_LEN; i++) dest[i] = name[i];
        dest[NAME_LEN] = '\0';

        if (!NameTaken(dest, used, usedCount)) break;

        drawTextLine(7, 2, "NAME EXISTS", C_RED, C_BLK);
        __delay_cycles(4000000);
        drawTextLine(7, 2, "           ", C_BLK, C_BLK); // Hinweis entfernen
        WaitForRelease();
    }
    WaitForRelease();
}

void DrawHighscoreList(int line) {
    EnsureHighscores();
    int i;
    char buf[20];
    for (i = 0; i < 3; i++) {
        if (flashScores.entries[i].score == 0) {
            drawTextLine(line + i, 2, "                ", C_BLK, C_BLK);
        } else {
            sprintf(buf, "%d. %c%c%c%c: %u",
                    i + 1,
                    flashScores.entries[i].name[0],
                    flashScores.entries[i].name[1],
                    flashScores.entries[i].name[2],
                    flashScores.entries[i].name[3],
                    flashScores.entries[i].score);
            drawTextLine(line + i, 2, buf, (i == 0) ? C_GRN : C_WHT, C_BLK);
        }
    }
}

// --- HAUPTSPIEL LOGIK ---

int PlayGame(int pNum, int isMulti, char name[NAME_LEN]) {
    char nameStr[NAME_LEN + 1];
    int i;
    for (i = 0; i < NAME_LEN; i++) nameStr[i] = name[i];
    nameStr[NAME_LEN] = '\0';

    // 1. Spieler Anzeige
    Rect(0, 0, 128, 128, C_BLK);
    char buf[20];
    sprintf(buf, "Player %d", pNum);
    drawTextLine(3, 3, buf, C_YEL, C_BLK);
    sprintf(buf, "%s", nameStr);
    drawTextLine(5, 3, buf, C_WHT, C_BLK);
    __delay_cycles(8000000);

    // 2. Spielfeld Init
    Rect(0, 0, 128, 128, C_BLK);
    int k;
    for (k = 1; k < 5; k++) Rect(k * 25, 0, 1, 128, C_GRY);
    cur_lane = 2;
    DrawCar(cur_lane, C_BLU);
    s1_old = 1;
    s2_old = 1;

    // 3. Tutorial Sequenz
    drawTextLine(0.1, 6, "Right >", C_GRN, C_BLK);
    while (cur_lane < 3) Input();

    Rect(0, 0, 128, 15, C_BLK);
    for (k = 1; k < 5; k++) Rect(k * 25, 0, 1, 15, C_GRY);
    drawTextLine(0.1, 6, "Left  <", C_GRN, C_BLK);
    while (cur_lane > 2) Input();

    Rect(0, 0, 128, 15, C_BLK);
    for (k = 1; k < 5; k++) Rect(k * 25, 0, 1, 15, C_GRY);
    drawTextLine(0.1, 6, "Ready ?", C_WHT, C_BLK);
    __delay_cycles(20000000); // Bereit-Nachricht anzeigen

    // Tutorial Cleanup
    Rect(0, 0, 128, 15, C_BLK);
    for (k = 1; k < 5; k++) Rect(k * 25, 0, 1, 15, C_GRY);
    DrawCar(cur_lane, C_BLU);

    // --- GAME START ---
    srand(TA0R);
    for (i = 0; i < MAX_OB; i++) obs[i].active = 0;

    int score = 0;
    unsigned int ticks = 8000; // weiter beschleunigt
    int max_active = 2;
    int min_speed = 2;
    const int TARGET_LOAD = 6; // Konstante Last für stabile Framerate
    static int last_speed_score = 0;

    while (1) {
        unsigned int start = TA0R;
        int active = 0, crash = 0;
        int draw_count = 0;

        Input();

        // Schwierigkeitskurve
        if (score > 10) max_active = 3;
        if (score > 25) max_active = 4;
        if (score > 50) max_active = 5;
        if (score > 100) max_active = 6;

        // Geschwindigkeit erhöhen
        // Alle 5 Punkte wird ticks um 800 reduziert (bis min 3000)
        if (score > last_speed_score && score % 5 == 0 && ticks > 5000) {
            ticks -= 1000;
            last_speed_score = score;
        }

        for (i = 0; i < MAX_OB; i++) {
            if (obs[i].active) {
                // Spur löschen
                if (obs[i].y >= -18) {
                    Rect(obs[i].x, obs[i].y, CAR_W, obs[i].spd, C_BLK);
                    draw_count++;
                }

                obs[i].y += obs[i].spd;

                // Block neu zeichnen
                if (obs[i].y > -18) {
                    Rect(obs[i].x, obs[i].y, CAR_W, CAR_H, C_RED);
                    draw_count++;
                }

                // Kollisionserkennung
                if (obs[i].lane == cur_lane && obs[i].y + CAR_H >= CAR_Y && obs[i].y <= CAR_Y + CAR_H) crash = 1;

                // Score Zählen oder aktiv bleiben
                if (obs[i].y > 128) {
                    obs[i].active = 0;
                    score++;
                } else {
                    active++;
                }
            }
        }

        // Dummy Load: Erzwingt konstante Rechenzeit durch unsichtbares Malen
        int target_draws = TARGET_LOAD * 2;
        while (draw_count < target_draws) {
            Rect(0, 130, CAR_W, CAR_H, C_BLK);
            draw_count++;
        }

        DrawCar(cur_lane, C_BLU);

        if (crash) {
            Rect(0, 0, 128, 128, C_RED);
            drawTextLine(4, 3, "GAME OVER", C_WHT, C_RED);
            char b[20]; sprintf(b, "Score: %d", score); drawTextLine(6, 3, b, C_WHT, C_RED);
            __delay_cycles(40000000); // Game Over anzeigen
            return score;
        }

        // Neue Hindernisse spawnen
        if (active < max_active && (rand() % 100) < 40) {
            for (i = 0; i < MAX_OB; i++) {
                if (!obs[i].active) {
                    int l = rand() % 5;
                    int free = 1;
                    int j;
                    // Prüfen ob Lane frei ist
                    for (j = 0; j < MAX_OB; j++) {
                        if (obs[j].active && obs[j].lane == l && obs[j].y < 45) free = 0;
                    }
                    if (free) {
                        obs[i].active = 1;
                        obs[i].lane = l;
                        obs[i].x = lx[l];
                        obs[i].y = -18 - (rand() % 30);
                        obs[i].spd = min_speed + (rand() % 3);
                        break;
                    }
                }
            }
        }
        Wait(start, ticks);
    }
}

// --- MAIN LOOP ---

void main(void) {
    Init_HW();
    EnsureHighscores();

    int state = 0; // 0=Main, 1=Single, 2=Multi
    int mainSel = 0;
    int pCount = 2;
    char b[20];

    while (1) {
        WaitForRelease();
        Rect(0, 0, 128, 128, C_BLK);

        // --- HAUPTMENÜ ---
        if (state == 0) {
            drawTextLine(1, 2, "BLOCK DODGE", C_YEL, C_BLK);
            Rect(0, 100, 128, 1, C_WHT);
            drawTextLine(9, 12.5, "Start", C_YEL, C_BLK);

            int up_old = 1, down_old = 1, start_old = 1;
            while (state == 0) {
                // Menü-Visualisierung
                drawTextLine(4, 3, (mainSel == 0) ? ">  SINGLE"    : "   SINGLE",    (mainSel == 0) ? C_GRN : C_WHT, C_BLK);
                drawTextLine(6, 3, (mainSel == 1) ? ">  MULTI "    : "   MULTI ",    (mainSel == 1) ? C_GRN : C_WHT, C_BLK);

                // Steuerung
                int up = (P4IN & BTN_UP);
                int down = (P3IN & BTN_DOWN);
                int start = (P1IN & BTN_START);

                // Flanke statt Pegel auswerten, damit jeder Klick nur einen Schritt macht
                if (!up && up_old)   { mainSel--; if (mainSel < 0) mainSel = 1; __delay_cycles(60000); }
                if (!down && down_old) { mainSel++; if (mainSel > 1) mainSel = 0; __delay_cycles(60000); }

                // Auswahl bestätigen
                if (!start && start_old) state = (mainSel == 0) ? 1 : 2;

                up_old = up;
                down_old = down;
                start_old = start;
            }
        }
        // --- SINGLE PLAYER ---
        else if (state == 1) {
            drawTextLine(1, 2, "BLOCK DODGE", C_YEL, C_BLK);
            drawTextLine(3, 2, "HIGHSCORE", C_GRN, C_BLK);
            DrawHighscoreList(4);

            Rect(0, 100, 128, 1, C_WHT);
            drawTextLine(9, 1, "Back       Start", C_YEL, C_BLK);
            WaitForRelease();

            while (1) {
                if (!(P2IN & BTN_BACK)) { state = 0; break; }
                if (!(P1IN & BTN_START)) {
                    char pName[NAME_LEN + 1];
                    EnterName(pName, 1, 0, 0);
                    int score = PlayGame(1, 0, pName); // 0 = Singleplayer Modus
                    Rect(0, 0, 128, 128, C_BLK);
                    sprintf(b, "%c%c%c%c: %d", pName[0], pName[1], pName[2], pName[3], score);
                    drawTextLine(5, 3, b, C_WHT, C_BLK);
                    UpdateHighscoreTable(pName, score);
                    drawTextLine(7, 3, "HIGHSCORE", C_GRN, C_BLK);
                    DrawHighscoreList(8);
                    __delay_cycles(20000000); // Ergebnis anzeigen
                    break;
                }
            }
        }
        // --- MULTI PLAYER ---
        else if (state == 2) {
            drawTextLine(1, 1, "NUMBER OF PLAYERS", C_YEL, C_BLK);
            Rect(0, 100, 128, 1, C_WHT);
            drawTextLine(9, 1, "Back       Start", C_YEL, C_BLK);
            int multiSel = 0;

            int up_old = 1, down_old = 1, start_old = 1, back_old = 1;
            while (state == 2) {
                // Spieleranzahl Auswahl
                drawTextLine(3, 3, (multiSel == 0) ? ">  2 PLAYERS" : "   2 PLAYERS", (multiSel == 0) ? C_GRN : C_WHT, C_BLK);
                drawTextLine(5, 3, (multiSel == 1) ? ">  3 PLAYERS" : "   3 PLAYERS", (multiSel == 1) ? C_GRN : C_WHT, C_BLK);
                drawTextLine(7, 3, (multiSel == 2) ? ">  4 PLAYERS" : "   4 PLAYERS", (multiSel == 2) ? C_GRN : C_WHT, C_BLK);

                int up = (P4IN & BTN_UP);
                int down = (P3IN & BTN_DOWN);
                int start = (P1IN & BTN_START);
                int back = (P2IN & BTN_BACK);

                if (!up && up_old)   { multiSel--; if (multiSel < 0) multiSel = 2; __delay_cycles(60000); }
                if (!down && down_old) { multiSel++; if (multiSel > 2) multiSel = 0; __delay_cycles(60000); }
                if (!back && back_old) state = 0;

                // Turnier Start
                if (!start && start_old) {
                    pCount = multiSel + 2;
                    int scores[4], i, j, k;
                    char names[4][NAME_LEN + 1];

                    // Namen erfassen und Spiele nacheinander ausführen
                    for (i = 0; i < pCount; i++) {
                        EnterName(names[i], i + 1, names, i);
                        scores[i] = PlayGame(i + 1, 1, names[i]); // 1 = Multiplayer Modus (mit Anzeige)
                        UpdateHighscoreTable(names[i], scores[i]);
                    }

                    // Bubble Sort für Ranking
                    for (i = 0; i < pCount - 1; i++) {
                        for (j = 0; j < pCount - i - 1; j++) {
                            if (scores[j] < scores[j + 1]) {
                                int t = scores[j];
                                scores[j] = scores[j + 1];
                                scores[j + 1] = t;
                                char tmp[NAME_LEN + 1];
                                for (k = 0; k <= NAME_LEN; k++) tmp[k] = names[j][k];
                                for (k = 0; k <= NAME_LEN; k++) names[j][k] = names[j + 1][k];
                                for (k = 0; k <= NAME_LEN; k++) names[j + 1][k] = tmp[k];
                            }
                        }
                    }

                    // Ergebnisliste anzeigen
                    Rect(0, 0, 128, 128, C_BLK);
                    drawTextLine(1, 1, "RESULTS", C_YEL, C_BLK);
                    for (i = 0; i < pCount; i++) {
                        sprintf(b, "%d. %s: %d", i + 1, names[i], scores[i]);
                        drawTextLine(2 + i, 1, b, (i == 0) ? C_GRN : C_WHT, C_BLK);
                    }

                    Rect(0, 100, 128, 1, C_WHT);
                    drawTextLine(9, 11, "Highscore", C_YEL, C_BLK);

                    // Ergebnisse bestätigen, dann Highscore separat anzeigen
                    WaitForRelease();
                    while ((P1IN & BTN_START)); // warten bis Start gedrückt wird

                    // Highscore-Screen
                    Rect(0, 0, 128, 128, C_BLK);
                    drawTextLine(1, 1, "HIGHSCORE", C_GRN, C_BLK);
                    DrawHighscoreList(3);
                    Rect(0, 100, 128, 1, C_WHT);
                    drawTextLine(9, 11, "Finish", C_YEL, C_BLK);
                    WaitForRelease();
                    while ((P1IN & BTN_START));
                    state = 0;
                }

                up_old = up;
                down_old = down;
                start_old = start;
                back_old = back;
            }
        }
    }
}
