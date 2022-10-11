/*! \brief Modul zur Druckausgabe auf LCD Displays.

	\version   1.0
	\file      Print.h
	\date      10.03.2018
	\author    bomarius
	\copyright bomarius
*/

/*! \brief Modul zur Druckausgabe auf einem zweizeiligen LCD-Display ueber den Port C.

	\defgroup group5 Standard-Ausgabe
	
	Das Modul Print stellt einfache Ausgabefunktionen auf einem zweizeiligen LCD-Display
	bereit, welches an Port C des STK 600 angeschlossen ist. Zur Verwendung von Print
	muss das Header-File Print.h inkludiert werden. Da Print C++ Code verwendet muss
	das Projekt als C++ Executable Projekt angelegt werden - ein C-Executable ist nicht
	moeglich. Dies heisst nicht, dass C++ Features benutzt werden muessen - die Applikation
	kann in C geschrieben sein, wird lediglich als C++ uebersetzt und gebunden.
	 
	Die mittels der 3 Ausgabefunktionen PRINT_STRING, PRINT_NUMBER, PRINT_CHAR gemachten
	Ausgaben werden auf dem Display als nach oben rollender Text dargestellt. Freie
	Positionierung der Schreibmarke auf dem Display ist nicht moeglich - lediglich "\n" 
	wird interpretiert.
	
	Parameter fuer PRINT_STRING() muessen entweder als Arrays im \a SRAM liegen oder als 
	\a literal-Konstanten (in doppelten Hochkommas wie im Verwendungsbeispiel) angegeben werden.
	
	Mittels der Funktionen SCROLL_UP() und SCROLL_DOWN() kann das Display ueber dem
	gepufferten Text verschoben werden. Da der Puffer nur 10 Zeilen speichern kann fuehrt
	jede weitere Ausgabezeile zum Verlust der jeweils aeltesten Zeile. Es kann also nur um
	acht Zeilen (bei einem zweizeiligen Display) zurueck gescrollt werden. Beim Aufruf
	einer PRINT_xx() Funktion wird automatisch ans Pufferende gescrollt, um die neue
	Ausgabe sichtbar zu machen.

	\verbatim
	Verwendungsbeispiel
	
	#include <avr/io.h>
	#include "Print.h"

	int main (void){
		
		PRINT_STRING("Hallo Welt !! \n");
		PRINT_NUMBER(234);
		PRINT_CHAR('\n');
		PRINT_STRING("noch eine dritte Zeile\n");
		SCROLL_DOWN(1);  // mach die erste Zeile wieder sichtbar
		...
		SCROLL_UP(1);    // zeige wieder das Ausgabeende
		
		while(1) {}
	}

	\endverbatim

   @{

*/


#ifndef PRINT_H_
#define PRINT_H_

#include <stddef.h>
#include "Basics.h"
#include "LCD.h"

/*! \cond */
#define USE_DISPLAY          LCDS stdout (PC, LCD_Type_24x2)
/*! \endcond */

/*! \brief Gib den Buchstaben ch auf dem Display aus. */
#define PRINT_CHAR(ch)		 {stdout.write_char(ch); stdout.write_char(FLUSH);}

/*! \brief Gib den String s auf dem Display aus. s muss die Adresse eines null-terminierten Strings im SRAM sein.*/
#define PRINT_STRING(s)      {stdout.write_SRAM_text(s); stdout.write_char(FLUSH);}

/*! \brief Gib die Zahl n in 3 Druckpositionen rechtsbuendig auf dem Display aus. */
#define PRINT_NUMBER(n)      {stdout.write_SRAM_text(LCD::itoa(NULL, n, 3)); stdout.write_char(FLUSH);}

/*! \brief	Verschiebe Text um maximal n Zeilen nach oben auf dem Display. */
#define SCROLL_UP(n)         {stdout.scroll_up(n);}

/*! \brief Verschiebe Text um maximal n Zeilen nach unten auf dem Display. */
#define SCROLL_DOWN(n)       {stdout.scroll_down(n);}

/*! \cond */
USE_DISPLAY;
/*! \endcond */

/*! @}  Ende group5 Standard-Ausgabe */

#endif /* PRINT_H_ */