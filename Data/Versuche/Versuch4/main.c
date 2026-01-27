#include <msp430.h>
#include "ST7735.h"
//Willkommen in der Zeichensalat
//Der verwendete Zeichensatz (font.h) verwendet erweiterte ASCII Kodierung nach der 
//amerikanische Codepage CP850. Eigentlich sollte der über die Einstellung -fexec-charset=CP850 
//die Strings utf-8 nach Codepage 850 konvertieren, dass wird aber vom msp gcc nicht unterstützt. 
//Die Datei als Codepage 850 zu speichern funktioniert wird aber von VSCode beim Speichern 
//überschrieben. Lösung Hexcodes für Sonderzeichen verwenden.
//Freiwillige Aufgabe: "Zurück" im Menü auf dem LCD korrekt anzeigen
const char* Texte[]={"Einstellungen", "Telefonbuch", "Anrufe", "Klingelton", "Lautst\x84rke", "Seppi", "Karl Heinz","Carlo", "24.03.21", "Tutut","BeepBeep", "0 %","50 %","100 %","0162-123235","0163-143235","07243-123665","07212-213665","Zurück"};
int Select[][4]= { {0,1,2,-1}, {3,4,18,-1}, {5,6,18,-1},{7,8,18,-1}};

signed Page;signed PagePos;

void PrintMenue(){
    drawTextLine(1, 0, (char*) Texte[Select[Page][PagePos]], 0x000000L,0xFFFFFFL);
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;
	ST7735_interface_init();
	ST7735_display_init();
    drawTextLine(0, 0, "Auswahl", 0xFF00FFL,0x000000L);
	for (int i = 4; i < 11; i++)
		drawTextLine(i, 0, "", 0x000000L, 0xFFFFFFL);
    PrintMenue();

	__bis_SR_register(LPM4_bits + GIE); // Globale Interruptfreigabe + Energiesparen
}