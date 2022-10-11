/*! \brief Modul fuer die Ansteuerung von LCDs

	\version   3.0  (single controller mit ready-flag, dual controller mit delay)
	\date      8.7.2019
	\author    Bomarius
	\copyright Bomarius
	\file      LCD.h
*/

/*! \brief Modul fuer die Ansteuerung von LCDs

	\defgroup group10 LCD

    Das LCD Modul kann eine Reihe von LCD Displays ansteuern. Diese unterscheiden sich
	nach Anzahl und Laenge der Zeilen des Displays. Da die internen Controller von LCD Displays
	nur eine begrenzte Speichermoeglichkeit haben, kann ein Controller nur Displays bis zu einer
	maximalen Anzahl Zeichen ansteuern. Groessere Displays besitzen daher 2 Controller, die 
	jeweils 1/2 des Displays ansteuern und wechselweise von der Treibersoftware angesprochen
	werden muessen. Das LCD Modul kann ein- sowie doppel-Controller ansteuern.
	
	Bei einfach-Controller Displays wird das ready-flag des Display Controllers vor jeder
	Operation abgefragt (busy-waiting) und nur so lange gewartet, bis der Controller bereit
	ist das naechste Kommando auszufuehren. Dies verringert im Vergleich zur
	Verwendung von Delay Funktionen die Wartezeit auf das unbedingt notwendige Mass,
	funktioniert auch bei Displays, die sich im Timing unterscheiden und ist vor allem im
	Multi-Tasking Betrieb von Vorteil.
	
	Bei doppel-Controller Displays steht aufgrund begrenzter Pin-Zahl (8) das ready-flag Flag nicht
	zur Verfuegung und es muessen die Standard-Wartezeiten gemaess Datenblatt (mittels delay-Funktionen)
	eingehalten werden. Nach jeder Operation des Controllers wird die jeweils noetige Standard-Wartezeit
	abgewartet, damit der Controller im Falle eines sofort darauf folgenden Kommandos auch sicher
	bereit ist. Dies erhoeht den Zeitverbrauch der  LCD-Ansteuerung ggue. einfach-Controller Displays.
	
	
	Die Ansteuerung des LCD Controllers ist durch Vewendung von kritischen Abschnitten \ref group3 
	auf der Ebene einzelner Befehle (z.B. Schreiben eines Buchstabens) mult-tasking sicher. 
	Allerdings sollte ein LCD nicht von mehreren Tasks gleichzeitig benutzt werden,
	da keine Sicherung beim Schreiben laengerer Sequenzen (z.B. Strings oder Zahlen)
	eingebaut ist. In diesem Fall muss mittels \a Semaphor synchroniert auf das 
	gemeinsame Display zugegriffen werden.

	\verbatim
	 Anschlussbelegung der Pins (single Controller)
	 
	  LCD     |LCD_PORT|
	  --------+--------+------------------------------------------------------------
	  DB0-DB7 | P0-P3  | Daten (in zwei 4-Bit Paketen nacheinander uebertragen)
	  E       | P4     | von enable() gepulst 
	  RW      | P5     | 0 = schreiben, 1 = lesen
	  RS      | P6     | von write_char() auf 1 und command() auf 0 gesetzt
	  ready   | P7     | ready Anzeige des Controllers

	 Anschlussbelegung der Pins (doppel-Controller)
	 
	 LCD     |LCD_PORT|
	 --------+--------+------------------------------------------------------------
	 DB0-DB7 | P0-P3  | Daten (in zwei 4-Bit Paketen nacheinander uebertragen)
	 E1      | P4     | von enable() gepulst (erster Controller)
	 RW      | P5     | 0 = schreiben, 1 = lesen
	 RS      | P6     | von write_char() auf 1 und command() auf 0 gesetzt
	 E2      | P7     | von enable() gepulst (zweiter Controller)

     Statt dem read-flag muss hier Pin 7 den strobe-Puls des zweiten Controllers uebertragen.
	  
	\endverbatim

	@{

*/

#ifndef LCD_H_
#define LCD_H_


//================================================================
//                            LCD
//================================================================

/*! \cond
 
     Einstellungen des Displaycontrollers via command
     Display On/Off Control Schema:  0b00001DCB
     display on (D=1);  display off (D=0)
     cursor on (C=1);   cursor off (C=0)
     blink on (B=1);    blink off (B=0)
	 
	 Wrapping ist keine Option des Display Controllers sondern wird 
	 im Treiber umgesetzt. Bei LDCS ist wrapping immer eingeschaltet.
	 
     \endcond
 */

/*!  \brief Makro zum Einschalten des Displays (Parameter zum Konstruktor)*/
#define DISPLAY_ON   (0b00001100)
/*!  \brief Makro zum Ausschalten des Displays (Parameter zum Konstruktor)*/
#define DISPLAY_OFF  (0b00001000)
/*!  \brief Makro zum Einschalten des Cursors (Parameter zum Konstruktor)*/
#define CURSOR_ON    (0b00001010)
/*!  \brief Makro zum Ausschalten des Cursors (Parameter zum Konstruktor)*/
#define CURSOR_OFF   (0b00001000)
/*!  \brief Makro zum Einschalten des Blinkes der Schreibposition (Parameter zum Konstruktor)*/
#define BLINK_ON     (0b00001001)
/*!  \brief Makro zum Ausschalten des Blinkes der Schreibposition (Parameter zum Konstruktor)*/
#define BLINK_OFF    (0b00001000)
/*!  \brief Makro zum Einschalten des Zeilenumbruchs (Parameter zum Konstruktor)*/
#define WRAPPING_ON  (0b10000000)
/*!  \brief Makro zum Ausschalten des Zeilenumbruchs (Parameter zum Konstruktor)*/
#define WRAPPING_OFF (0b00000000)

/*!  \brief Makro zur Definition der Default-Konfiguration eines LCD Controllers (Parameter zum Konstruktor). */
#define LCD_DISPLAY_CONTROL_DEFAULT  (uint8_t)(DISPLAY_ON | CURSOR_ON | BLINK_ON | WRAPPING_ON)

/*!  \brief Makro zur Definition der Default-Konfiguration eines LCDS Controllers (Parameter zum Konstruktor). */
#define LCDS_DISPLAY_CONTROL_DEFAULT  (uint8_t)(DISPLAY_ON | CURSOR_OFF | BLINK_OFF | WRAPPING_ON)


/*! \brief Aufzaehlungstyp fuer die unterstuetzten Display-Typen 

	Der zu verwendende Display-Typ muss als Parameter beim Konstruktor angegeben werden.

    \cond
    Die Zahlenwerte dienen gleichzeitig als Indexwert für das LCDType Array, duerfen
    also nicht veraendert werden, ohne das Array in LCD.cpp anzupassen.
	\endcond
*/

enum LCD_Type_t {
	LCD_Type_20x1 = 0, /*!< 20 Zeichen, einzeilig, single-Controller  */
	LCD_Type_24x1 = 1, /*!< 24 Zeichen, einzeilig, single-Controller  */
	LCD_Type_40x1 = 2, /*!< 40 Zeichen, einzeilig, single-Controller  */
	LCD_Type_16x2 = 3, /*!< 16 Zeichen, zweizeilig, single-Controller */
	LCD_Type_20x2 = 4, /*!< 20 Zeichen, zweizeilig, single-Controller */
	LCD_Type_24x2 = 5, /*!< 24 Zeichen, zweizeilig, single-Controller */
	LCD_Type_40x2 = 6, /*!< 40 Zeichen, zweizeilig, single-Controller */
	LCD_Type_16x4 = 7, /*!< 16 Zeichen, vierzeilig, single-Controller */
	LCD_Type_20x4 = 8, /*!< 20 Zeichen, vierzeilig, single-Controller */
	LCD_Type_40x4 = 9  /*!< 40 Zeichen, vierzeilig, dual Controller.  */
	};


/*! \cond */
// Descriptor for LCD configurations. The LCD_Type_xxx Makros are used
// as index to the array of lcdTypes. The array adr specifies the
// LCD data RAM address of the first char in each line. We assume
// LCDs do not have more than 4 lines and do not have more than 62
// chars per line (technical limit given by the setpos command).
struct LCDType {
	uint8_t visNumOfLines;   // number of lines visible on LCD
	uint8_t visNumOfChars;   // number of chars visible per line
	uint8_t adr[4];	         // offsets of lines into LCD Data RAM
};

extern const LCDType lcdTypes[10];

/*! \endcond */


/*! \brief Klasse fuer einfache LCD-Ansteuerung ohne Pufferung, mit Umbruch
           (wrap) am rechten Displayrand und am am Ende der letzten Zeile.
	
	Die Klasse kann verschiedene LCD Typen ansteuern. Insbesondere LCD Displays
	mit einem oder zwei Controllern (grosse Displays). Wenn zwei Controller
	verbaut sind, wird aufgrund der aktuellen Cursor Position ermittelt,
	welcher Controller anzusprechen ist - dies wird durch die Klasse abstrahiert.
*/
class LCD {
	/*! \cond */

private: 
    uint8_t display_control;
	bool wrapping;				// automatischer Zeilenumbruch 
	
	void advance_cursor();
	void enable ();
	void command (uint8_t cmd);
	void init (uint8_t cntrl);	
	 
	void wait_ready();
			
protected:
	
	volatile uint8_t* const port; // PORTx Ausgabe-Register-Adresse des Ports
	uint8_t  lcdt;		          // Der LCD Typ der aktuell angesteuert wird
	const LCDType* lcdType;       // Zeiger auf die LCD description fuer schnellern Zugriff
	uint8_t  curr_row;            // Cursor Position: current row
	uint8_t  curr_col;            // Cursor Position: current column
	uint8_t  enable_pin;          // Strobe Pin (enable fuer den LCD Controller)
	/*! \endcond */	
	
public:

    /*! \cond */	
	static char* itoa (char* buf, int16_t n, const uint8_t lim, const char fill=' ');
	/*! \endcond */


	/*! \brief Erzeuge ein LCD Objekt am digitalen Port p_handle.
		\param [in] p_handle Eine Port-Bezeichnung gemaess Definition in Basics.h
		\param [in] lcdt Spezifiziert die Art des LCDs mittels Werten des enums LCD_Type_t
		\param [in] d_cntrl Bestimmt Sichtbarkeit eines Cursors und Blinken der Schreibposition; Default: beide an.
		
		Grundeinstellung des LCDS erfolgt gemaess Makro LCD_DISPLAY_CONTROL_DEFAULT und
		kann jederzeit mit den entsprechenden on/off Methoden veraendert werden.
		Text-Umbruch ist eingeschaltet, so dass am Zeilenende auf die naechste Zeile
		und am Display-Ende auf den Display-Beginn gesprungen wird. 
	 */
	LCD (uint8_t p_handle, LCD_Type_t lcdt, uint8_t d_cntrl = LCD_DISPLAY_CONTROL_DEFAULT);
	
	/*! \cond */
	// Hilfsfunktion zur Ausgabe der Descriptor-Werte eines LCDs.	
	void describe();
	/*! \endcond */	

	/*! \brief Setze die Cursor Position fuer die naechste Ausgabeoperation.
		\param [in] row  Die Zeile auf dem LCD Display (Zaehlung beginnt bei null).
		\param [in] col  Die Spalte auf dem LCD Display (Zaehlung beginnt bei null).
		\retval true Positionierung war erfolgreich.
		\retval false Angeforderte Position beim gewaehlten Display Typ nicht moeglich.
		
		Wenn die angeforderte Position nicht gesetzt werden kann, wird die aktuelle
		Cursor Position nicht veraendert.
	*/
	bool set_pos (uint8_t row, uint8_t col);
	
	/*! \brief Schreibe den Buchstaben ch an der aktuellen Cursor Position und ruecke die
			   Schreibeposition um eine Position weiter. Sonderzeichen, wie "\n" werden
			   \a nicht interpretiert.
		\param [in] ch Druckbarer Buchstabe im ASCII Code.
		
		Wenn wrapping eingeschaltet ist (Default) wird bei Erreichen des sichtbaren
		Zeilenendes die Schreibeposition auf den sichtbaren Beginn der naechsten Zeile
		gesetzt. Wenn das Ende der letzten Zeile erreicht ist, wird der Cursor auf die
		Home Position gesetzt. Wenn wrapping ausgeschaltet ist, werden Zeichen, die
		ueber das Zeileende hinaus gehen wuerden, auf der letzten Zeichenposition der
		Zeile (uebereinander) geschrieben. In beiden Faellen werden niemals Zeichen
		in den nicht angezeigten Speicherbereich des LCD Controllers geschrieben.
	*/
	void write_char(const char ch);
	
	/*! \brief Schreibe einen C-String, der an der Adresse text im SRAM liegt.
		\param [in] text Die Adresse des null-terminierten C-Strings im SRAM Speicher.
		\param [in] max Die maximale Anzahl zu schreibender Zeichen. Bei max = 0 (default) schreiben bis zum Erreichen des Null-Zeichens.
	
		Die Schreibeposition wird um die Anzahl der Zeichen im String 
		(ohne das Null-Zeichen) weitergerueckt. Bezueglich wrapping siehe die
		Beschreibung zu write_char().
	*/
	void write_SRAM_text (const char* text, uint8_t max=0);
	
	/*! \brief Schreibe einen C-String, der an der Adresse text im FLASH liegt.
		\param [in] text Die Adresse des null-terminierten C-Strings im FLASH Speicher.
		\param [in] max Die maximale Anzahl zu schreibender Zeichen. Bei max = 0 (default) schreiben bis zum Erreichen des Null-Zeichens.
	
		Die Schreibeposition wird um die Anzahl der Zeichen im String
		(ohne das Null-Zeichen) weitergerueckt.  Bezueglich wrapping siehe die
		Beschreibung zu write_char().
	*/
	void write_FLASH_text (const char* text, uint8_t max=0) ;
	
	/*! \brief Schreibe die Zahl n auf lim Positionen rechtsbuendig und fuelle fuehrende Stellen mit fill.
		\param [in] n    Die positive oder negative 16 Bit Zahl, die als String ausgegeben werden soll.
		\param [in] lim  Die zu verwendende Druckbreite. Es werden genau lim Positionen verwendet. Wenn die
					Zahl zu gross ist, werden die hoeherwertigen Stellen abgeschnitten! Hinweis: Maximal 6
					Positionen werden fuer eine vorzeichenbehaftete 16 Bit Zahl benoetigt.
		\param [in] fill Falls fuehrende Stellen keine Ziffern haben, fuelle mit diesem Zeichen auf.
		
		Die Schreibeposition wird um die Anzahl der Zeichen lim weitergerueckt. 
		Bezueglich wrapping siehe die Beschreibung zu write_char().
	*/
	void write_number(int16_t n, const uint8_t lim = 3, const char fill=' ');
	
	/*! \brief Loesche die Anzeige und setze die Curosr Position auf das erste Zeichen 
			   links oben auf dem Display (Home Position). */
	void clear();
	
	/*! \brief  Setze die Scheibeposition auf das erste Zeichen links oben auf dem Display
				(Home Position) ohne den Inhalt des Displays zu veraendern. */
	void home();
	
	/*! \brief  Schalte das Dsiplay ein. */
	void screen_on()  {display_control |=  0b00000100; this->command(display_control);}
		
	/*! \brief Schalte das Display aus. */
	void screen_off() {display_control &= ~0b00000100; this->command(display_control);}
		
	/*! \brief Schalte den Schreibcursor sichtbar. */
	void cursor_on()  {display_control |=  0b00000010; this->command(display_control);}
		
	/*! \brief Schalte den Cursor unsichtbar. Verwendet, um das stoerende Rechteck
           des Cursors aus der Anzeige zu entfernen. */
	void cursor_off() {display_control &= ~0b00000010; this->command(display_control);}
		
	/*! \brief Zeige den Cursor als blinkenden Block. */
	void blink_on()   {display_control |=  0b00000001; this->command(display_control);}
		
	/*! \brief Zeige den Cursor als statischen Block. */
	void blink_off()  {display_control &= ~0b00000001; this->command(display_control);}

	/*! \brief Schalte Umbruch am Zeilen- und Display-Ende ein. */		
	void wrapping_on()  {wrapping = true;}
		
	/*! \brief Schalte Umbruch am Zeilen- und Display-Ende aus. */		
	void wrapping_off() {wrapping = false;}
};


//================================================================
//                            LCDS
//================================================================

/*! \brief Makro zur Definition eines Sonderzeichens mit dem die sofortige Ausgabe
    von bislang gepuffertem, aber noch nicht angezeigtem Text auf das
	Display erzwungen werden kann. (Nur von LCDS Objekten interpretiert.)
*/
#define FLUSH ('\x16')  // das ASCII Zeichen "SYN"


/*! \cond */
// Da nur statische Allokation moeglich ist setzen wir hier die Puffergroesse fuer
// die Objekte vom Typ LCDS, unabhaengig von der tatsaechlichen Displaygroesse
#define LCD_BUFFER_SIZE   (250)
/*! \endcond */

/*! \brief  Klasse fuer gepufferte und scroll-bare Ausgabe auf LCDs.

    Ein LCDS Objekt erweitert ein LCD Objekt um einen mehrzeiligen Textpuffer.
	Die Anzahl der Zeilen sowie die Anzahl der Zeichen pro Zeile im Puffer 
	haengen von der sichtbaren Zeilen- und Zeichenzahl auf dem verwendeten
	Display ab. Es wir immer die volle Displaybreite benutzt. Geschriebener 
	Text wird als Stream betrachtet, der am
	Zeilenende automatisch umbricht. Das Zeichen "\n" wird erkannt und als
	Zeilenumbruch interpretiert, aber nicht gespeichert. Der Textpuffer hat
	mehr Zeilen als das jeweils verwendete Display anzeigen kann und kann
	daher auf und ab gerollt werden. Liegt die Schreibeposition momentan
	ausserhalb des Displays, so verschiebt die naechste Schreibeoperation
	den Displayinhalt so, dass der neue Text sichtbar ist.
*/

class LCDS: private LCD {

private:
/*! \cond */
    // the text buffer; holds lines of text of length visNumOfChars (from
	// LCDType descriptor) plus one extra char '\0' at each line's end
	char buffer[LCD_BUFFER_SIZE];

	uint8_t  numOfLines;     // number of lines that fit into the buffer
	uint8_t  curPrintLine;   // current buffer line-no. to be written to 
	uint8_t  curPrintPos;    // next free char pos within a buffer line
	uint8_t  curOffset;      // offset from curPrintLine when scrolling
	                         // a negative offset:
							 // a positive offset: 

    // as the last buffer line is full (or a linefeed is issued on the
	// last line) the buffer contents is moved up by one line. Contents
	// of first buffer line gets lost. New last line is initialized to '\0'	
	void buffer_roll_up();
	
	// refresh the LCD display. Usually the lines at curPrintLine 
	// position (bottom lines the of buffer) are displayed, unless 
	// curOffset forces to display text from different buffer position.
	void refresh();
/*! \endcond */

public:
	/*! \brief Erzeuge ein LCDS Objekt vom Typ lcdt am digitalen Port p_handle.

		\param [in] p_handle Eine Port-Bezeichnung gemaess Definition in Basics.h
		\param [in] lcdt lcdt Spezifiziert die Art des LCDs mittels Werten des enums LCD_Type_t.
		\param [in] d_cntrl Bestimmt Sichtbarkeit eines Cursors und Blinken der Schreibposition; Default: beide aus.
	
		Berechne die notwendige Puffergroesse anhand des LCD Typs und initialisiere 
		den Puffer sowie das LCD. Der LCD Typ wird durch eines der LCD_Type_xxx
		Makros bestimmt.
	*/
	LCDS (uint8_t p_handle, LCD_Type_t lcdt, uint8_t d_cntrl = LCDS_DISPLAY_CONTROL_DEFAULT);

	/*! \brief Loesche den Puffer und die Anzeige und setze die Schreibposition auf 
	           das erste Zeichen (links oben auf dem Display). */
	void clear();
	
	/*! \brief Schreibe den Buchstaben ch in den Puffer. Falls das Pufferende
	           erreicht ist, rolle den Inhalt um eine Zeile nach oben. 

        \param [in] ch Druckbarer Buchstabe im ASCII Code, oder FLUSH oder "\n".
			   
		Beim Rollen am Pufferende geht der Inhalt der obersten Zeile verloren.
		Interpretiert linefeed. Verschiebt die Anzeige ggf. so, dass der neue Text
		sichtbar wird.
	*/
	void write_char(const char ch);
	
	/*! \brief Schreibe den Text text in den Puffer. Falls dabei das Pufferende
	           erreicht wird, rolle den Inhalt nach oben. 

        \param [in] text Adresse eines null-terminierten C-Strings im SRAM. Darf
		            FLUSH oder "\n" beinhalten.
		\param [in] max Die maximale Anzahl zu schreibender Zeichen. Bei max = 0
		                (default) schreiben bis zum Erreichen des Null-Zeichens.

		Beim Rollen am Pufferende geht der Inhalt der obersten Zeile verloren.
		Interpretiert linefeed. Verschiebt die Anzeige ggf. so, dass der neue Text
		sichtbar wird.
	*/
	void write_SRAM_text (const char* text, uint8_t max=0);
	
	/*! \brief Schreibe den Text text in den Puffer. Falls dabei das Pufferende
	           erreicht wird, rolle den Inhalt nach oben. 

        \param [in] text Adresse eines null-terminierten C-Strings im FLASH. Darf
                    FLUSH oder "\n" beinhalten.
		\param [in] max Die maximale Anzahl zu schreibender Zeichen. Bei max = 0
		                (default) schreiben bis zum Erreichen des Null-Zeichens.

		Beim Rollen am Pufferende geht der Inhalt der obersten Zeile verloren.
		Interpretiert linefeed. Verschiebt die Anzeige ggf. so, dass der neue Text
		sichtbar wird.
	*/
	void write_FLASH_text (const char* text, uint8_t max=0) ;
	
    /*! \brief Verschiebe den Displayinhalt um n Zeilen nach oben (zeige mehr Text 
	           vom Ende des Puffers).
		\param [in] n Anzahl der Zeilen, um die (maximal) verschoben wird. Wenn weniger
		       als n ungezeigte Zeilen im Puffer vorhanden sind, erscheint die
			   letzte Puffer-Zeile auf der untersten Displayzeile.
	*/
	void scroll_up (uint8_t n=1);
	
    /*! \brief Verschiebe den Displayinhalt um n Zeilen nach unten (Zeige mehr Text
	           vom Anfang des Puffers).
		\param [in] n Anzahl der Zeilen, um die (maximal) verschoben wird. Wenn weniger
		       als n ungezeigte Zeilen im Puffer vorhanden sind, erscheint die
			   erste Puffer-Zeile auf der obersten Displayzeile.
	*/
	void scroll_down (uint8_t n=1);
	
	/*! \brief  Schalte das Dsiplay ein. */
	using LCD::screen_on;
		
	/*! \brief Schalte das Display aus. */
	using LCD::screen_off;
		
	/*! \brief Schalte den Schreibcursor sichtbar. */
	using LCD::cursor_on;
		
	/*! \brief Schalte den Cursor unsichtbar. Verwendet, um das stoerende Rechteck
           des Cursors aus der Anzeige zu entfernen. */
	using LCD::cursor_off;
		
	/*! \brief Zeige den Cursor als blinkenden Block. */
	using LCD::blink_on;
		
	/*! \brief Zeige den Cursor als statischen Block. */
	using LCD::blink_off;
};

/* @}   end group 10 LCD */

#endif /* LCD_H_ */