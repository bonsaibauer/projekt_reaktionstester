#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include "ST7735.h"

// KONFIGURATION
#define C_BLK 0x000000
#define C_WHT 0xFFFFFF
#define C_GRN 0x00FF00
#define C_RED 0x0000FF
#define C_YEL 0x00FFFF
#define C_BLU 0xFF0000
#define C_GRY 0x555555

#define CAR_Y 100
#define CAR_W 18
#define CAR_H 18
#define MAX_OB 8

#define BTN_START BIT1
#define BTN_BACK  BIT1
#define BTN_UP    BIT0
#define BTN_DOWN  BIT7

// GLOBALE VARIABLEN
const int lx[] = {4, 29, 54, 79, 104};

typedef struct { int active, lane, x, y, spd; } Obstacle;
Obstacle obs[MAX_OB];
int cur_lane = 2, s1_old = 1, s2_old = 1;

typedef struct {
    char name[3];
    unsigned int score;
} HighscoreEntry;
volatile HighscoreEntry __attribute__((section(".infoD"))) flashHighscore = {"AA", 0};

// HARDWARE-INITIALISIERUNG
void Init_HW() {
    WDTCTL = WDTPW | WDTHOLD;
    UCSCTL3 |= SELREF_2; UCSCTL4 |= SELA_2;
    __bis_SR_register(SCG0);
    UCSCTL0 = 0; UCSCTL1 = DCORSEL_7; UCSCTL2 = FLLD_1 + 380;
    __bic_SR_register(SCG0);
    TA0CTL = TASSEL_2 + ID_3 + MC_2;

    P1DIR &= ~BTN_START; P1REN |= BTN_START; P1OUT |= BTN_START;
    P2DIR &= ~BTN_BACK;  P2REN |= BTN_BACK;  P2OUT |= BTN_BACK;
    P4DIR &= ~BTN_UP;    P4REN |= BTN_UP;    P4OUT |= BTN_UP;
    P3DIR &= ~BTN_DOWN;  P3REN |= BTN_DOWN;  P3OUT |= BTN_DOWN;

    P6SEL |= BIT5 | BIT3;
    ADC12CTL0 = ADC12ON | ADC12SHT0_2;
    ADC12CTL1 = ADC12SHP;
    ADC12CTL0 |= ADC12ENC;

    ST7735_interface_init();
    ST7735_display_init();
}

void UpdateHighscore(char* name, int score) {
    FCTL3 = FWKEY;
    FCTL1 = FWKEY + ERASE;
    *(unsigned int*)&flashHighscore = 0;
    FCTL1 = FWKEY + WRT;
    volatile HighscoreEntry* ptr = &flashHighscore;
    ((volatile char*)ptr->name)[0] = name[0];
    ((volatile char*)ptr->name)[1] = name[1];
    ((volatile char*)ptr->name)[2] = '\0';
    ((volatile unsigned int*)&ptr->score)[0] = score;
    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;
}

// GRAFIK-HELFER
void Rect(int x, int y, int w, int h, unsigned long c) {
    if (x >= 0 && x < 128) draw(x, y, w, h, c);
}

void DrawCar(int l, unsigned long c) {
    Rect(lx[l], CAR_Y, CAR_W, CAR_H, c);
}

void WaitForRelease() {
    while (!(P1IN & BTN_START));
    __delay_cycles(2000000);
}

// INPUT-HANDLING
int ReadJoystickY() {
    ADC12CTL0 &= ~ADC12ENC;
    ADC12MCTL0 = ADC12INCH_3;
    ADC12CTL0 |= ADC12ENC | ADC12SC;
    while (!(ADC12IFG & ADC12IFG0));
    int val = ADC12MEM0;
    ADC12IFG &= ~ADC12IFG0;
    return val;
}

int ReadJoystickX() {
    ADC12CTL0 &= ~ADC12ENC;
    ADC12MCTL0 = ADC12INCH_5;
    ADC12CTL0 |= ADC12ENC | ADC12SC;
    while (!(ADC12IFG & ADC12IFG0));
    int val = ADC12MEM0;
    ADC12IFG &= ~ADC12IFG0;
    return val;
}

void Input() {
    static int moved = 0;
    int sl = (P2IN & BTN_BACK), sr = (P1IN & BTN_START);
    int new_lane = cur_lane;

    if (!sl && s1_old && cur_lane > 0) new_lane = cur_lane - 1;
    if (!sr && s2_old && cur_lane < 4) new_lane = cur_lane + 1;

    int joy_x = ReadJoystickX();
    if (joy_x < 1500 && !moved && cur_lane > 0) { new_lane = cur_lane - 1; moved = 1; }
    else if (joy_x > 2600 && !moved && cur_lane < 4) { new_lane = cur_lane + 1; moved = 1; }
    else if (joy_x >= 1800 && joy_x <= 2300) moved = 0;

    if (new_lane != cur_lane) {
        DrawCar(cur_lane, C_BLK);
        cur_lane = new_lane;
        DrawCar(cur_lane, C_BLU);
    }
    s1_old = sl; s2_old = sr;
}

void Wait(unsigned int start, unsigned int dur) {
    while ((TA0R - start) < dur) Input();
}

int ShowHighscoreStartScreen() {
    char b[20];
    drawTextLine(1, 2, "BLOCK DODGE", C_YEL, C_BLK);
    if (flashHighscore.score > 0) {
        drawTextLine(3, 2, "Highscore", C_GRN, C_BLK);
        sprintf(b, "%s: %u", flashHighscore.name, flashHighscore.score);
        drawTextLine(5, 2, b, C_GRN, C_BLK);
    } else drawTextLine(5, 2, "No Record", C_GRY, C_BLK);
    Rect(0, 100, 128, 1, C_WHT);
    drawTextLine(9, 1, "Back       Start", C_YEL, C_BLK);
    WaitForRelease();
    while (1) {
        int joy_x = ReadJoystickX();
        if (!(P2IN & BTN_BACK) || joy_x < 1500) return 0;
        if (!(P1IN & BTN_START) || joy_x > 2600) return 1;
    }
}

// NAMEN-EINGABE
int IsNameTaken(char* name, char usedNames[][3], int usedCount) {
    int i;
    if (flashHighscore.score > 0 && name[0] == flashHighscore.name[0] && name[1] == flashHighscore.name[1]) return 1;
    if (!usedNames) return 0;
    for (i = 0; i < usedCount; i++) if (name[0] == usedNames[i][0] && name[1] == usedNames[i][1]) return 1;
    return 0;
}

int EnterName(int playerNum, char* name, char usedNames[][3], int usedCount) {
retry:
    name[0] = 'A'; name[1] = 'A'; name[2] = '\0';
    int letterIdx = 0, charSel = 0;
    int up_old = 1, down_old = 1, start_old = 1, back_old = 1;

    Rect(0, 0, 128, 128, C_BLK);
    char buf[20];
    if (playerNum > 0) {
        sprintf(buf, "Player %d", playerNum);
        drawTextLine(1, 2, "ENTER NAME", C_YEL, C_BLK);
        drawTextLine(2.5, 2, buf, C_WHT, C_BLK);
    } else {
        drawTextLine(1, 2, "ENTER NAME", C_YEL, C_BLK);
    }
    Rect(0, 100, 128, 1, C_WHT);
    drawTextLine(7, 13, "_ _", C_WHT, C_BLK);

    while (letterIdx < 3) {
        drawTextLine(9, 1, letterIdx == 0 ? "Cancel     Ok   " : (letterIdx == 1 ? "Backspace  Ok   " : "Backspace  Start"), C_YEL, C_BLK);

        char prev = (charSel == 0) ? 'Z' : ('A' + charSel - 1);
        char curr = 'A' + charSel;
        char next = (charSel == 25) ? 'A' : ('A' + charSel + 1);

        char prevStr[2] = {prev, '\0'};
        char currStr[4]; currStr[0] = '>'; currStr[1] = ' '; currStr[2] = curr; currStr[3] = '\0';
        char nextStr[2] = {next, '\0'};

        drawTextLine(4.5, 4, "  ", C_BLK, C_BLK);
        drawTextLine(4.5, 4, prevStr, C_GRY, C_BLK);
        drawTextLine(5.5, 3, "    ", C_BLK, C_BLK);
        drawTextLine(5.5, 3, currStr, C_GRN, C_BLK);
        drawTextLine(6.5, 4, "  ", C_BLK, C_BLK);
        drawTextLine(6.5, 4, nextStr, C_GRY, C_BLK);

        int waiting = 1;
        while (waiting) {
            int joy_y = ReadJoystickY();
            int joy_x = ReadJoystickX();
            int up = (P4IN & BTN_UP), down = (P3IN & BTN_DOWN), start = (P1IN & BTN_START), back = (P2IN & BTN_BACK);

            if (letterIdx < 2 && ((!up && up_old) || joy_y > 3072)) {
                charSel = (charSel == 0) ? 25 : charSel - 1;
                name[letterIdx] = 'A' + charSel;
                __delay_cycles(150000);
                waiting = 0;
            }
            if (letterIdx < 2 && ((!down && down_old) || joy_y < 1024)) {
                charSel = (charSel == 25) ? 0 : charSel + 1;
                name[letterIdx] = 'A' + charSel;
                __delay_cycles(150000);
                waiting = 0;
            }
            if (((!start && start_old) || joy_x > 2600)) {
                if (letterIdx < 2) {
                    letterIdx++;
                    if (letterIdx < 2) charSel = 0;
                    char display[10];
                    if (letterIdx == 1) sprintf(display, "%c _", name[0]);
                    else if (letterIdx == 2) sprintf(display, "%c %c", name[0], name[1]);
                    drawTextLine(7, 13, "    ", C_BLK, C_BLK);
                    drawTextLine(7, 13, display, C_WHT, C_BLK);
                } else {
                    letterIdx++;
                }
                __delay_cycles(200000);
                waiting = 0;
            }
            if (((!back && back_old) || joy_x < 1500)) {
                if (letterIdx == 0) {
                    __delay_cycles(200000);
                    return 0;
                }
                letterIdx--;
                charSel = name[letterIdx] - 'A';
                char display[10];
                if (letterIdx == 0) sprintf(display, "_ _");
                else sprintf(display, "%c _", name[0]);
                drawTextLine(7, 13, "    ", C_BLK, C_BLK);
                drawTextLine(7, 13, display, C_WHT, C_BLK);
                __delay_cycles(200000);
                waiting = 0;
            }
            up_old = up; down_old = down; start_old = start; back_old = back;
        }
    }
    if (IsNameTaken(name, usedNames, usedCount)) {
        drawTextLine(9, 1, "Name taken     ", C_RED, C_BLK);
        __delay_cycles(12000000);
        goto retry;
    }
    __delay_cycles(2000000);
    return 1;
}

// SPIELLOGIK
int PlayGame(int pNum, int isMulti, char* playerName) {
    if (isMulti) {
        Rect(0, 0, 128, 128, C_BLK);
        char readyMsg[20]; sprintf(readyMsg, "It's %s's turn!", playerName);
        drawTextLine(5, 2, readyMsg, C_WHT, C_BLK);
        __delay_cycles(20000000);
    }

    Rect(0, 0, 128, 128, C_BLK);
    int k; for (k = 1; k < 5; k++) Rect(k * 25, 0, 1, 128, C_GRY);
    cur_lane = 2; DrawCar(cur_lane, C_BLU);
    s1_old = 1; s2_old = 1;

    Rect(0, 0, 128, 15, C_BLK);
    drawTextLine(0.1, 6, "Right >", C_GRN, C_BLK);
    while (cur_lane < 3) Input();

    Rect(0, 0, 128, 15, C_BLK);
    drawTextLine(0.1, 6, "Left  <", C_GRN, C_BLK);
    while (cur_lane > 2) Input();

    drawTextLine(0.1, 6, "Ready ?", C_WHT, C_BLK);
    __delay_cycles(20000000);

    Rect(0, 0, 128, 15, C_BLK);
    for (k = 1; k < 5; k++) Rect(k * 25, 0, 1, 15, C_GRY);
    DrawCar(cur_lane, C_BLU);

    srand(TA0R);
    int i; for (i = 0; i < MAX_OB; i++) obs[i].active = 0;

    int score = 0, max_active = 3, min_speed = 2;
    unsigned int ticks = 1900;
    const int TARGET_LOAD = 6;
    static int last_speed_score = 0;

    while (1) {
        unsigned int start = TA0R;
        int active = 0, crash = 0, draw_count = 0;
        Input();

        if (score > 5) max_active = 3;
        if (score > 15) max_active = 4;
        if (score > 35) { max_active = 5; min_speed = 3; }
        if (score > last_speed_score && score % 3 == 0 && ticks > 400) {
            ticks -= 300; last_speed_score = score;
        }

        for (i = 0; i < MAX_OB; i++) {
            if (obs[i].active) {
                if (obs[i].y >= -18) { Rect(obs[i].x, obs[i].y, CAR_W, obs[i].spd, C_BLK); draw_count++; }
                obs[i].y += obs[i].spd;
                if (obs[i].y > -18) { Rect(obs[i].x, obs[i].y, CAR_W, CAR_H, C_RED); draw_count++; }
                if (obs[i].lane == cur_lane && obs[i].y + CAR_H >= CAR_Y && obs[i].y <= CAR_Y + CAR_H) crash = 1;
                if (obs[i].y > 128) { obs[i].active = 0; score++; } else active++;
            }
        }

        int target_draws = TARGET_LOAD * 2;
        while (draw_count < target_draws) { Rect(0, 130, CAR_W, CAR_H, C_BLK); draw_count++; }
        DrawCar(cur_lane, C_BLU);

        if (crash) {
            Rect(0, 0, 128, 128, C_RED);
            drawTextLine(4, 3, "GAME OVER", C_WHT, C_RED);
            char b[20]; sprintf(b, "Score: %d", score); drawTextLine(6, 3, b, C_WHT, C_RED);
            __delay_cycles(40000000);
            return score;
        }

        int spawn_chance = 40 + (score / 10);
        if (spawn_chance > 59) spawn_chance = 59;

        if (active < max_active && (rand() % 100) < spawn_chance) {
            for (i = 0; i < MAX_OB; i++) if (!obs[i].active) {
                int l, free, attempts = 0, j;
                do {
                    l = rand() % 5; free = 1;
                    for (j = 0; j < MAX_OB; j++) if (obs[j].active && obs[j].lane == l && obs[j].y < 40) { free = 0; break; }
                    if (free) {
                        int blocked_lanes = 0;
                        for (j = 0; j < MAX_OB; j++) if (obs[j].active && obs[j].y >= -25 && obs[j].y < 35) blocked_lanes++;
                        if (blocked_lanes >= 4) free = 0;
                    }
                    attempts++;
                } while (!free && attempts < 15);
                if (free) {
                    obs[i].active = 1; obs[i].lane = l; obs[i].x = lx[l];
                    obs[i].y = -18 - (rand() % 35); obs[i].spd = min_speed + (rand() % 2);
                    break;
                }
            }
        }
        Wait(start, ticks);
    }
}

// MAIN LOOP
void main(void) {
    Init_HW();
    if (flashHighscore.score == 0xFFFF) {
        char initName[3] = "  ";
        UpdateHighscore(initName, 0);
    }

    int state = 0, mainSel = 0, pCount = 2;
    char b[20], singlePlayerName[3] = "AA";

    while (1) {
        WaitForRelease();
        Rect(0, 0, 128, 128, C_BLK);

        if (state == 0) {
            drawTextLine(1, 2, "BLOCK DODGE", C_YEL, C_BLK);
            Rect(0, 100, 128, 1, C_WHT);
            drawTextLine(9, 12.5, "Start", C_YEL, C_BLK);
            while (state == 0) {
                drawTextLine(4, 3, mainSel == 0 ? ">  SINGLE" : "  SINGLE", mainSel == 0 ? C_GRN : C_GRY, C_BLK);
                drawTextLine(6, 3, mainSel == 1 ? ">  MULTI " : "  MULTI ", mainSel == 1 ? C_GRN : C_GRY, C_BLK);
                int joy_y = ReadJoystickY();
                int joy_x = ReadJoystickX();
                if (!(P4IN & BTN_UP) || joy_y > 3072) { mainSel = 0; __delay_cycles(150000); }
                if (!(P3IN & BTN_DOWN) || joy_y < 1024) { mainSel = 1; __delay_cycles(150000); }
                if (!(P1IN & BTN_START) || joy_x > 2600) state = (mainSel == 0) ? 1 : 2;
            }
        }
        else if (state == 1) {
            if (!ShowHighscoreStartScreen()) { state = 0; continue; }
            if (!EnterName(0, singlePlayerName, 0, 0)) { state = 0; continue; }
            int score = PlayGame(0, 0, singlePlayerName);
            Rect(0, 0, 128, 128, C_BLK);
            sprintf(b, "Score: %d", score); drawTextLine(5, 3, b, C_WHT, C_BLK);
            if (score > flashHighscore.score) {
                drawTextLine(7, 3, "NEW RECORD!", C_GRN, C_BLK);
                UpdateHighscore(singlePlayerName, score);
            }
            __delay_cycles(20000000);
        }
        else if (state == 2) {
            if (!ShowHighscoreStartScreen()) { state = 0; continue; }

            Rect(0, 0, 128, 128, C_BLK);
            drawTextLine(1, 1, "NUMBER OF PLAYERS", C_YEL, C_BLK);
            Rect(0, 100, 128, 1, C_WHT);
            drawTextLine(9, 1, "Back       Start", C_YEL, C_BLK);
            WaitForRelease();
            int multiSel = 0;
            while (state == 2) {
                drawTextLine(3, 3, multiSel == 0 ? ">  2 PLAYERS" : "  2 PLAYERS", multiSel == 0 ? C_GRN : C_GRY, C_BLK);
                drawTextLine(5, 3, multiSel == 1 ? ">  3 PLAYERS" : "  3 PLAYERS", multiSel == 1 ? C_GRN : C_GRY, C_BLK);
                drawTextLine(7, 3, multiSel == 2 ? ">  4 PLAYERS" : "  4 PLAYERS", multiSel == 2 ? C_GRN : C_GRY, C_BLK);
                int joy_y = ReadJoystickY();
                int joy_x = ReadJoystickX();
                if (!(P4IN & BTN_UP) || joy_y > 3072) { multiSel--; if (multiSel < 0) multiSel = 2; __delay_cycles(1500000); }
                if (!(P3IN & BTN_DOWN) || joy_y < 1024) { multiSel++; if (multiSel > 2) multiSel = 0; __delay_cycles(1500000); }
                if (!(P2IN & BTN_BACK) || joy_x < 1500) state = 0;
                if (!(P1IN & BTN_START) || joy_x > 2600) {
                    pCount = multiSel + 2;
                    int scores[5], ids[5], i, j;
                    char names[5][3];
                    for (i = 1; i <= pCount; i++) if (!EnterName(i, names[i], names + 1, i - 1)) { state = 0; break; }
                    if (state == 0) break;
                    for (i = 1; i <= pCount; i++) { scores[i] = PlayGame(i, 1, names[i]); ids[i] = i; }
                    for (i = 1; i < pCount; i++) for (j = 1; j <= pCount - i; j++)
                        if (scores[j] < scores[j + 1]) {
                            int t = scores[j]; scores[j] = scores[j + 1]; scores[j + 1] = t;
                            t = ids[j]; ids[j] = ids[j + 1]; ids[j + 1] = t;
                        }
                    int newRecord = 0;
                    if (scores[1] > flashHighscore.score) {
                        UpdateHighscore(names[ids[1]], scores[1]);
                        newRecord = 1;
                    }
                    Rect(0, 0, 128, 128, C_BLK);
                    drawTextLine(1, 1, "RESULTS", C_YEL, C_BLK);
                    for (i = 1; i <= pCount; i++) {
                        sprintf(b, "%d. %s: %d", i, names[ids[i]], scores[i]);
                        drawTextLine(2 + i, 1, b, i == 1 ? C_GRN : C_WHT, C_BLK);
                    }
                    if (newRecord) drawTextLine(7, 1, "NEW RECORD!", C_GRN, C_BLK);
                    Rect(0, 100, 128, 1, C_WHT);
                    drawTextLine(9, 1, "Back       Start", C_YEL, C_BLK);
                    WaitForRelease();
                    while (1) {
                        int joy_x_end = ReadJoystickX();
                        if (!(P1IN & BTN_START) || joy_x_end > 2600) break;
                        if (!(P2IN & BTN_BACK) || joy_x_end < 1500) break;
                    }
                    state = 0;
                }
            }
        }
    }
}
