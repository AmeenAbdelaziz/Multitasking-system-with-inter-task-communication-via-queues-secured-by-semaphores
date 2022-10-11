/*! \brief Modul fuer simple Zusicherungskontrolle und Fehleranzeige auf dem jeweils
           verfuegbaren Ausgabemedium.

		\version   1.0
		\date      9.2.2018
		\author    Bomarius
		\copyright Bomarius
		\file      ErrorHandler.h

		\todo LCD und LCDS als Ausgabemedium testen.
		\todo Verwendungsbeispiel geben
*/

/*! \brief Modul fuer simple Zusicherungskontrolle und Fehleranzeige auf dem jeweils
           verfuegbaren Ausgabemedium.

    \defgroup group9 ErrorHandler

	Das Modul verwendet LEDs oder, wenn verfuegbar ein LCD, um Fehler anzuzeigen.
	Im Fehlerfall uebernimmt es das Ausgabemedium und zeigt Fehlernummern (LED) oder
	Texte (LCD, LCDS) an und stoppt die Programmausfuehrung.
	Die Funktionalitaet diese Moduls ist als drei Makros implementiert und wird nach Abschluss
	des Testens deaktiviert. Um die Funktionalitaet des Moduls \a einzuschalten muss im
	Applikationsprogramm \a vor der Zeile mit #include ErrorHandler.h eines bzw. beide der
	folgenden Makros definiert werden: ERR_CHECK_ON, ASSERT_ON. Mit ASSERT_ON werden Zusicherungen
	(Assertions) aktiviert, die Bedingungen zur Laufzeit pruefen und melden, wenn sie \a nicht eingehalten
	werden. Mit ERR_CECK_ON werden Fehlerchecks aktiviert, welche Fehlersituationen anhand
	von \a erfuellten Bedingungen feststellen und melden. Dabei wird unterschieden nach
	Erreichbarkeit von Stellen im Code und nach allgemeinen Fehleranzeigen.
	Die Menge der Fehlertexte und zugehoeriger Fehlercodes kann durch Hinzufuegen
	derselben in der Datei ErrorHandler.h erweitert werden. (Siehe Hinweise dazu in ErrorHandler.h) 
	Dann muss diese veraenderte Version im Applikationsprojekt (als eigene Datei des Projekts)
	angesiedelt sein (diese ersetzt dann die Systemdatei ErrorHandler.h).
	Die Implementierungsdatei ErrorHandler.cpp bleibt unveraendert.

    @{

 */


#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

#include <avr/pgmspace.h>

// Dummy - Dient nur der Dokumentation des Makros ERR_CHECK_ON. Definition des Makros
// muss bei Bedarf im Applikationsprogramm erfolgen, um Error Checks einzuschalten.
#ifdef __CERTAINLY_NOT_DEFINED__
/*! \brief Aktivierung der Auswertung von Fehlerbedingungen. */
#define ERR_CHECK_ON
#endif

// Dummy - Dienst nur der Dokumentation des Makros ASSERT_ON. Definition des Makros
// muss bei Bedarf im Applikationsprogramm erfolgen, um Assertions einzuschalten.
#ifdef __CERTAINLY_NOT_DEFINED__
/*! \brief Aktivierung der Auswertung von Assertions. */
#define ASSERT_ON
#endif

/*! \brief Art des verfuegbaren Ausgabemediums. */
typedef enum {
	RAWLEDS,       /*!< Es stehen LEDs am digitalen Port zur Verfuegung. */
	DIGIPORTLED,   /*!< Es stehen LEDs ueber ein DigiPort Objekt zur Veruegung. */
	RAWLCD,        /*!< Es steht ein LCD ueber ein einfaches LCD Objekt zur Verfuegung. */
	LCDS,          /*!< Es steht ein LCD mit Textpufferung zur Verfuegung. */
	UNDEF          /*!< Es gibt keine Ausgabemoeglichkeit. */
	} ErrHandle_t;

#if defined ERR_CHECK_ON || defined ASSERT_ON

/*! \cond */

    // Zum Hinzufuegen neuer Fehlercodes muessen 3 Dinge getan werden:
	// 1.  ein neuer Fehlercode ist zu vergeben; dieser dient als Index in errlist[]
	// 2.  der Textstring ist anzulegen
	// 3. der Zeiger auf den Textstring ist in errlist[] einzufügen

    // Die Fehlercode Konstanten dienen als Index in das errlist Array
    // Weitere Codes koennen als aufsteigende, zusammenhaengende Nummernfolge
	// hinzugefuegt werden.
	#define ANY_ERR               (1)  // Allgemeiner Fehler
	#define TASK_INSERT_ERR       (2)
	#define SEMAPHOR_RELEASE_ERR  (3)
	#define QUEUE_OVF_ERR         (4)
	#define CODE_NOT_REACHED_ERR  (5)  // Programmzweig erreicht, der nie erreicht werden darf
	#define CALLED                (6)  // Fuer Prozedur-Traces
	#define RETURNING             (7)  // Fuer Prozedur-Traces
	#define LCD_POS_ERR           (8)  // LCD set_pos mit nicht erlaubter Position gerufen

    // nach vorgegebenem Schema koennen weitere Strings angelegt werden.
	// Beachte, dass sie noch in errlist registriert werden muessen.
	static const char err1[] PROGMEM = "Genaral error!";
	static const char err2[] PROGMEM = "task_insert() failed!";
	static const char err3[] PROGMEM = "Semaphor: not owner!";
	static const char err4[] PROGMEM = "Queue: overflow!";
	static const char err5[] PROGMEM = "Dead code reached!";
	static const char err6[] PROGMEM = "called!";
	static const char err7[] PROGMEM = "returning!";
	static const char err8[] PROGMEM = "LCD: set_pos() failed!"


    // Registriere weitere Fehlerstrings am Ende der Liste, so dass die Fehlerkonstanten als 
	// Indexwert zum jeweiligen String dienen.
	const char * const errlist[] PROGMEM =
	{
		NULL, err1, err2, err3, err4, err5, err6, err7, err8
	};

	void ERR_HANDLE (const char * text, uint8_t errno);
	void ERRINIT (ErrHandle_t eht, void* ptr);

/*! \endcond */

	#ifdef ERR_CHECK_ON
	
/*! \brief  Mittels Makro NOTREACHED kann ein Zweig im Code markiert werden, von dem
            angenommen wird, dass er NICHT erreicht werden soll. Sollte das Programm
			doch in den Zweig laufen, erfolgt die Ausgabe von text gefolgt vom
		    Fehlertext "Dead code reached!". 
			
	\param [in] text Ein C-String, der vor dem Standard-Text mit der Nummer
	            CODE_NOT_REACHED_ERR ausgegeben wird. Diser String sollte die
				Position im Code beschreiben. */	
		#define NOTREACHED(text)   ERR_HANDLE(text, CODE_NOT_REACHED_ERR)
		
/*! \brief  Mittels Makro ERRCHK kann eine Bedingung geprueft werden und falls diese
            erfuellt ist eine Fehlermeldung ausgegeben werden.
			
	\param [in] expr  Die Bedingung, die eintreten muss, damit der Fehler errno angezeigt wird.
	\param [in] errno Die Nummer der Fehlermeldung, die anzuzeigen ist. Vor den Fehlertext wird die
	             expr selbst als String ausgegeben.
*/	
		#define ERRCHK(expr,errno) do {if(expr) ERR_HANDLE(#expr, errno);} while(0)
    #else
	    #define ERRCHK(expr,errno) expr  // expr must be left in the code!
		#define NOTREACHED(a)
	#endif
	
	#ifdef ASSERT_ON
/*! \brief  Mittels Makro ASSERT kann eine Bedingung geprueft werden und falls diese
            NICHT erfuellt ist wird die Fehlermeldung errno ausgegeben.
			
	\param [in] expr  Die Bedingung, die eintreten muss, damit der Fehler NICHT angezeigt wird.
	\param [in] errno Die Nummer der Fehlermeldung, die anzuzeigen ist. Davor wird die
	             expr als String ausgegeben.
*/	
		#define ASSERT(expr,errno) do {if(!(expr)) ERR_HANDLE(#expr, errno);} while(0)
    #else
	    #define ASSERT(expr,errno)	//do nothing; we expect expr to not have side effects
	#endif

#else

	#define ERRCHK(expr,errno) expr  // expr must be left in the code, we need its intended effect!
    //compile all other macros to nothing; we expect all parameters a and b to not have side effects!!
    #define ERRINIT(a,b)
    #define NOTREACHED(a)
	#define ASSERT(a,b)
	 
#endif

/*! @} */  // end group9 ErrorHandler

#endif /* ERRORHANDLER_H_ */