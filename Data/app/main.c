#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include "ST7735.h"

const int lx[] = {4, 29, 54, 79, 104}; 
#define PLAYER_Y 100
#define BLOCK_SIZE 18 
#define MAX_OB 8

#define C_BLU 0x0000FF
#define C_RED 0xFF0000
#define C_WHT 0xFFFFFF
#define C_GRN 0x00FF00

#define BTN_LEFT  0
#define BTN_UP    1
#define BTN_DOWN  2
#define BTN_RIGHT 3

typedef struct { 
    int active, lane, x, y, spd;
} Obstacle;

int cur_lane = 2;
Obstacle obs[MAX_OB];
int player_scores[4];
int num_players = 1;

void Init_HW() {
    WDTCTL = WDTPW | WDTHOLD;
    UCSCTL3 |= SELREF_2; 
    UCSCTL4 |= SELA_2;
    __bis_SR_register(SCG0); 
    UCSCTL0 = 0; 
    UCSCTL1 = DCORSEL_7; 
    UCSCTL2 = FLLD_1 + 380; 
    __bic_SR_register(SCG0);
    
    TA0CTL = TASSEL_2 + ID_3 + MC_2; 

    ST7735_interface_init(); 
    ST7735_display_init();  
    
    P2DIR &= ~BIT1; P2REN |= BIT1; P2OUT |= BIT1;
    P1DIR &= ~BIT1; P1REN |= BIT1; P1OUT |= BIT1;
    P4DIR &= ~BIT0; P4REN |= BIT0; P4OUT |= BIT0;
    P3DIR &= ~BIT7; P3REN |= BIT7; P3OUT |= BIT7;
}

unsigned char getBtn(int i) {
    if(i == BTN_LEFT)  return !(P2IN & BIT1);
    if(i == BTN_UP)    return !(P4IN & BIT0);
    if(i == BTN_DOWN)  return !(P3IN & BIT7);
    if(i == BTN_RIGHT) return !(P1IN & BIT1);
    return 0;
}

void delay_ms(unsigned int ms) { 
    while(ms--) __delay_cycles(1000); 
}

void Rect(int x, int y, int w, int h, unsigned long c) {
    
    if (x >= 128 || y >= 128 || x + w <= 0 || y + h <= 0) return;
    
    int x_draw = x, y_draw = y;
    if(x_draw < 0) x_draw = 0;
    if(y_draw < 0) y_draw = 0;
    
    draw(x_draw, y_draw, w, h, c);
}

void DrawPlayer(int color) { 
    Rect(lx[cur_lane], PLAYER_Y, BLOCK_SIZE, BLOCK_SIZE, color); 
}

int CheckAndMovePlayer() {
    static int last_state_L = 0;
    static int last_state_R = 0;
    
    int current_L = getBtn(BTN_LEFT);
    int current_R = getBtn(BTN_RIGHT);
    int moved = 0;

    if (current_L && !last_state_L) {
        if(cur_lane > 0) {
            DrawPlayer(0); 
            cur_lane--; 
            DrawPlayer(C_BLU); 
            moved = 1;
        }
    }
    
    if (current_R && !last_state_R) {
        if(cur_lane < 4) {
            DrawPlayer(0); 
            cur_lane++; 
            DrawPlayer(C_BLU); 
            moved = 1;
        }
    }

    last_state_L = current_L;
    last_state_R = current_R;
    return moved;
}

void RunTutorial() {
    
    Rect(0, 0, 128, 128,0);
    cur_lane = 2;
    DrawPlayer(C_BLU);
    drawTextLine(4, 4, "right >", C_GRN,0);

    while(cur_lane < 3) {
        CheckAndMovePlayer();
        delay_ms(20);
    }

    Rect(0, 30, 128, 50, 0);
    
    drawTextLine(4, 4, "< left", C_GRN, 0);
    
    while(cur_lane != 2) {
        CheckAndMovePlayer();
        delay_ms(20);
    }
    
    Rect(0, 30, 128, 50, 0);
    drawTextLine(5, 5, "Ready?", C_GRN, 0);
    delay_ms(20000);
    
    Rect(0, 0, 128, 128, 0);
}

int PlayGameRound(int playerNum) {
    
    Rect(0,0,128,128, 0);
    char buffer[20];
    sprintf(buffer, "Spieler %d", playerNum);
    drawTextLine(5, 4, buffer, C_WHT, 0);
    delay_ms(20000);

    RunTutorial();

    DrawPlayer(C_BLU); 
    int i;
    for(i=0; i<MAX_OB; i++) obs[i].active = 0;
    
    int survival_time = 0;
    int base_speed = 3;
    srand(TA0R); 
    
    while(1) {
        unsigned int start = TA0R;
        
        survival_time++;
        
        if(survival_time % 100 == 0 && base_speed < 12) {
            base_speed++;
        }
        
        CheckAndMovePlayer();
        
        int active_blocks = 0;
        
        for(i=0; i<MAX_OB; i++) {
            if(obs[i].active) {
                
                Rect(obs[i].x, obs[i].y, BLOCK_SIZE, obs[i].spd, 0);
                obs[i].y += obs[i].spd;

                
                if(obs[i].lane == cur_lane && 
                   obs[i].y + BLOCK_SIZE >= PLAYER_Y && 
                   obs[i].y <= PLAYER_Y + BLOCK_SIZE) {
                      
                      Rect(0, 50, 128, 30, C_RED);
                      drawTextLine(5, 5, "Game Over", C_WHT, C_RED);
                      delay_ms(20000);
                      return survival_time; 
                }
                
                if(obs[i].y > 128) {
                    obs[i].active = 0;
                } else {
                    Rect(obs[i].x, obs[i].y, BLOCK_SIZE, BLOCK_SIZE, C_RED);
                    active_blocks++;
                }
            }
        }
        
        DrawPlayer(C_BLU); 
        
        if(active_blocks < 4 && (rand() % 100) < 15) {
            for(i=0; i<MAX_OB; i++) {
                if(!obs[i].active) {
                    obs[i].active = 1;
                    obs[i].lane = rand() % 5;
                    obs[i].x = lx[obs[i].lane];
                    obs[i].y = -BLOCK_SIZE;
                    obs[i].spd = base_speed + (rand() % 3); 
                    break;
                }
            }
        }

        while((TA0R - start) < 30000) {
            CheckAndMovePlayer();
        }
    }
}
void SelectPlayers() {
    Rect(0,0,128,128,0);
    drawTextLine(2, 1, "Player Count", C_WHT, 0);
    drawTextLine(12, 1, "set right", C_GRN, 0);
    
    char buf[10];
    int selected = 0, last_U = 0, last_D = 0, last_R = 0;

    num_players=1;

 while(!selected) {
        
        sprintf(buf, "< %d >", num_players);
        drawTextLine(6, 6, buf, C_GRN, 0);

        int b_up   = getBtn(BTN_UP);
        int b_down = getBtn(BTN_DOWN);
        int b_conf = getBtn(BTN_RIGHT); 

        if (b_up && !last_U) {
            if (num_players < 5) num_players++;
        }
        if (b_down && !last_D) {
            if (num_players > 1) num_players--;
        }
        if (b_conf && !last_R) {
            selected = 1;
        }

        last_U = b_up;
        last_D = b_down;
        last_R = b_conf;

        delay_ms(20);
    }
    delay_ms(10000);
}

void Scoreboard() {
    Rect(0,0,128,128, 0);
    drawTextLine(1, 1, "Ranking", C_WHT, 0);
    
    typedef struct {
        int id, score;
    } PlayerResult;

    PlayerResult results[4];

    for(int i=0; i < num_players; i++) {
        results[i].id = i + 1;
        results[i].score = player_scores[i];
    }

    for(int step = 0; step < num_players - 1; ++step) {
        for(int k = 0; k < num_players - step - 1; ++k) {
            if(results[k].score < results[k+1].score) {
                PlayerResult temp = results[k];
                results[k] = results[k+1];
                results[k+1] = temp;
            }
        }
    }
    
    char buffer[20];
    for(int i=0; i < num_players; i++) {

        sprintf(buffer, "P%d: %d", results[i].id, results[i].score); 
        unsigned long color = (i == 0) ? C_GRN : C_WHT;
        drawTextLine(3 + (i*2), 2, buffer, color, 0);
    }
    
    drawTextLine(13, 1, "Restart right", C_RED, 0);

    while(getBtn(BTN_RIGHT));
    while(!getBtn(BTN_RIGHT));
}

void main(void) {
    Init_HW();

    Rect(0,0,128,128,0);
    drawTextLine(5,2,"BLOCK DODGE",C_GRN,0);
    drawTextLine(8,2,"Start right",C_WHT,0);
    
    while(!getBtn(BTN_RIGHT));
    
    while (1) {

        SelectPlayers();
        
        for(int p = 0; p < num_players; p++) {
            player_scores[p] = PlayGameRound(p + 1);
        }
        Scoreboard();
    }
}