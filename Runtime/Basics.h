#ifndef BASICS_H_
#define BASICS_H_

/*! \mainpage Runtime Dokumentation

	\version   3
	\date      22.9.2021
	\author    Bomarius
	\copyright Bomarius

	# Vorbemerkung
	Die Module, die unter dem Begriff "Runtime" zur Verfuegung gestellt werden, sind
	ausschliesslich fuer die Nutzung im Kontext der Lehre gedacht. Anderweitige 
	Nutzung, inbesondere gewerblicher Art, ist untersagt.
	
	# Verwendung von Runtime
	Die Module von Runtime werden in eine gemeinsame Bibliothek libruntime.a uebersetzt
	und gebunden. Es genuegt also diese eine Bibliothek zum Projekt des Applikationsprogramms hinzu zu binden. Die 
	Header Dateien der Module stehen alle in einem gemeinsamen Verzeichnis, welches
	dem Praeprozessor als Such-Verzeichnis bekannt gemacht werden muss. Diese beiden
	Schritte sind fuer jedes Projekt, welches Runtime verwenden soll, notwendig.
	Sie sollten die Dateinen von Runtime \a nicht kopieren - nur anbinden.
	
	Die Module sind in C++ geschrieben und erfordern daher ein Anwendungsprogramm, 
	welches als C++ Projekt angelegt ist, in das sie eingebunden werden. Bestimmte Module 
	sind aber auch in "C-Manier" zu verwenden (also ohne, dass man eigene Objekte erzeugen muss),
	bspw. Print, ErrorHandler, Soft-Reset, OSKernel. Alle anderen erfordern in
	jedem Fall den Umgang im Applikationsprogramm mit selbst erzeugten Objekten.
	
	Applikationsprogramme sind fuer den ATMega2560 und das STK600 Board zu entwickeln. Auf 
	dieser Hardware plus den Peripherie-Komponenten aus dem Labor (Sensoren, Displays, 
	Keypads, Drehgeber) funktioniert die Runtime Software. Andere Prozessoren oder
	Boards werden nicht unterstuetzt. Beachten Sie die standardisierte Portbelegung.
	
	# Liste der Module
	Runtime beinhaltet verschiedene Arten von Modulen; Auflistung erfolgt jeweils in alpahbetischer Reihenfolge:
	
	## Grundlegendes Modul
	- Basics - beinhaltet Definitionen, die von anderen Modulen und dem Applikationsprogramm selbst 
	           verwendet werden. Muss daher immer inkludiert werden.
	
	## Treibermodule
	- ADC - Analog-Digital-Wandler
	- DigiPort - Verschiedene Treiber zur Ansteuerung von digitalen Ports.
	- LCD - Verschiedene Treiber zur Ansteuerung von LCD Displays.
	- QuadEncoder - Treiber zum Auslesen eine Drehgebers (Quadratur-Encoder).
	- Stepper - verschiedene Treiber zur Ansteuerung von Schrittmotoren.
	- Timer - verschiedene Timer zur Ansteuerung der 8 und 16 Bit Timer und ein DelayHandler.
	- SevenSegment - Treiber zur Ansteuerung von mehrstelligen 7-Segment Anzeigen.

	## Multi-Tasking Betriebssystem-Kern und Zusatzmodule
	- BinarySemaphor - Binaeres Semaphor zur Koordination von Tasks und fuer wechselseitigen Ausschluss.
	- BoundedQueue - Einfache FiFo Warteschlange zur Kommunikation zwischen Tasks.
	- Kritische Abschnitte - Makros, um kritische Programmabschnitte zu bilden.
	- OSKernel - Der Multi-Tasking Betriebssystem-Kern und Zusatzfunktionen rund um die Task-Steuerung.
	- Soft-Reset - Funktion zum Reset des ATMega Prozessors per Software.
	
	## Verschiedene nuetzliche Module
	- ErrorHandler - Ein simpler Fehler-Melder, der unterschiedliche Ausgabemedien nutzen kann.
	- Print - Eine primitive Standard-Ausgabe aufbauend auf dem LCD Modul.

	\cond
	
    Zuordnung der Modulgruppen in Doxygen:
	group	Modulname
		1	Ports
		2	Timer
		3	Kritische Abschnitte
		4	Soft-Reset
		5	Standard-Ausgabe
		6	ADC
		7	Semaphore
		8	BoundedQueue
		9	ErrorHandler
		10	LCD
		11	OSKernel
		12	QuadEncoder
		13	SevenSegment
		14	Stepper
		
	\endcond
*/


//=================================================================================
//=================================  BASICS  ======================================
//=================================================================================

/*! \brief Basics - Modul mit grundlegenden Makro-Definitionen, die von anderen 
           Runtime Modulen und von Applikationsmodulen verwendet werden. Sollte
		   immer zuerst inkludiert werden.

	\version   1.1    SoftReset hinzugefügt
	\date      9.2.2018
	\author    Bomarius
	\copyright Bomarius
	\file      Basics.h
*/

#include <avr/io.h>

/*****************************************************************************
              Definitionen zur Nutzung von digitalen Ports:
 *****************************************************************************/

/*! 
	\brief Modul zur Nutzung Digitaler Ports.
	
	\defgroup group1 Ports

   Die Port Handles (PA ... PL) dienen als aktuelle Parameter zur Spezifikation eines Ports.
   Diese Konstanten (und nur diese) sind zur Benennung eines Ports zu verwenden, wenn dieser
   zu einer Nutzung konfiguriert werden soll. Anwendungsfaelle: Konstruktoren
   der Klassen aus den Modulen DigiPort oder LCD.
   Die Nutzung der Port Handles wird kontrolliert damit eine versehentliche
   mehrfache Nutzung des gleichen Ports fuer verschiedene Zwecke ausgeschlossen
   wird. Die SET_XX Makros dienen darueber hinaus zur Vereinfachung der Konfiguation von Ports.
   
   @{

 */
/*! \brief  Handle fuer Port A. */
#define PA   ((uint8_t)(0))
/*! \brief  Handle fuer Port B. */
#define PB   ((uint8_t)(1))
/*! \brief  Handle fuer Port C. */
#define PC   ((uint8_t)(2))
/*! \brief  Handle fuer Port D. */
#define PD   ((uint8_t)(3))
/*! \brief  Handle fuer Port E. */
#define PE   ((uint8_t)(4))
/*! \brief  Handle fuer Port F. */
#define PF   ((uint8_t)(5))
/*! \brief  Handle fuer Port H. */
#define PG   ((uint8_t)(6))
/*! \brief  Handle fuer Port H. */
#define PH   ((uint8_t)(7))
/*! \brief  Handle fuer Port J. */
#define PJ   ((uint8_t)(8))
/*! \brief  Handle fuer Port K. */
#define PK   ((uint8_t)(9))
/*! \brief  Handle fuer Port L. */
#define PL   ((uint8_t)(10))

/*  ----- Funktion und Array sind NUR ZUR INTERNEN NUTZUNG! -----------------
 *  Setze Handle in reale Adresse um; registriere Nutzung des Ports.
 *  Bei wiederholter Nutzung eines Handles liefere null (=FEHLER).
 */
/*! \cond */
extern volatile uint8_t* const port_handle_to_address(uint8_t p_handle);
extern volatile uint8_t* const port_vect[];
/*! \endcond */

/* Konstanten zur Konfiguration digitaler Ports. Zweiter bzw. dritter 
 * Parameter in Konstruktoren. Falls NICHT alle 8 Pins identisch
 * konfiguriert werden sollen, sind geeignete eigene Masken zu definieren.
 */
/*! \brief  Konfiguriere alle Pins des Ports fuer Input. */
#define SET_IN_PORT      ((uint8_t)(0x00))
/*! \brief  Konfiguriere alle Pins des Ports fuer Output. */
#define SET_OUT_PORT     ((uint8_t)(0xFF))
/*! \brief  Konfiguriere alle Pins des Ports als active low. Default-Einstellung beim ATMega.  */
#define SET_ACTIVE_LOW   ((uint8_t)(0x00))
/*! \brief  Konfiguriere alle Pins des Ports als active high. */
#define SET_ACTIVE_HIGH  ((uint8_t)(0xFF))

/*  Konstanten zum Loeschen und Setzen aller LEDS
 */
/*! \brief  Konfiguriere alle LEDs an einem active low Port als ausgeschaltet. */
#define SET_LEDS_OFF     ((uint8_t)(0xFF))
/*! \brief  Konfiguriere alle LEDs an einem active low Port als eingeschaltet. */
#define SET_LEDS_ON      ((uint8_t)(0x00))

/* ---- Diese Konstanten sind NUR ZUR INTERNEN NUTZUNG! -----------
 * Die je 3 Register zur Steuerung eines digitalen Ports liegen auf
 * konsekutiven Adressen. PINx liegt auf der niedersten Adresse, danach
 * folgt DDRx dann PORTx. Die folgenden Offset-Konstanten dienen der
 * Bestimmung der Registeradressen ausgehend von einer Basisadresse
 * des jeweiligen Ports.
 */
/*! \cond */
#define IN_OFFSET   ((uint8_t)(0))
#define DDR_OFFSET  ((uint8_t)(1))
#define OUT_OFFSET  ((uint8_t)(2))
/*! \endcond */

/*! @} */  // Ende group Ports

/*****************************************************************************
              Definitionen zur Nutzung der Timer/Counter:
 *****************************************************************************/
/*! 
	\brief Modul zur Nutzung der Timer/Counter.
	
	\defgroup group2 Timer

    Timer Handles (TC0 ... TC5) dienen als aktuelle Parameter zur Spezifikation eines Timer/Counters (TC).
    Diese Konstanten (und nur diese) sind zur Benennung eines Timers zu verwenden, wenn dieser
    zu einer Nutzung konfiguriert werden soll. Anwendungsfaelle: Konstruktoren
    der Klassen aus dem Modul Timer.
    Die Nutzung der Timer Handles wird kontrolliert damit eine versehentliche
    mehrfache Nutzung des gleichen Timers fuer verschiedene Zwecke ausgeschlossen
    wird.
	
   @{
	   
*/

/*! \brief  Handle fuer den 8-Bit Timer/Counter 0. */
#define TC0   ((uint8_t)(0))
/*! \brief  Handle fuer den 16-Bit Timer/Counter 1. */
#define TC1   ((uint8_t)(1))
/*! \brief  Handle fuer den 8-Bit Timer/Counter 2. */
#define TC2   ((uint8_t)(2)) 
/*! \brief  Handle fuer den 16-Bit Timer/Counter 3. */
#define TC3   ((uint8_t)(3))
/*! \brief  Handle fuer den 16-Bit Timer/Counter 4. */
#define TC4   ((uint8_t)(4))
/*! \brief  Handle fuer den 16-Bit Timer/Counter 5. */
#define TC5   ((uint8_t)(5))

/*  ----- Funktion tc_handle_to_address und Array tc_vect sind NUR ZUR INTERNEN NUTZUNG! -----------------
 *  Setze Handle in reale Adresse um; registriere Nutzung des Timers.
 *  Bei wiederholter Nutzung eines Handles liefere null (=FEHLER).
 */
/*! \cond */
extern volatile uint8_t* const tc_handle_to_address(uint8_t tc_handle);
extern volatile uint8_t* const tc_vect[];
/*! \endcond */

/*! @} */  // Ende group Timer

/*****************************************************************************
    Definitionen für Kritische Abschnitte (im Kontext von Multi-Tasking):
 *****************************************************************************/

#include <util/atomic.h>

/*! 
	\brief Modul fuer die Bildung Kritischer Abschnitte (im Kontext 
	       von Multi-Tasking und Interrupt Service Routinen).
	
	\defgroup group3 Kritische Abschnitte

  Das Makro CRITICAL_SECTION dient zusammen mit einem {...} Block der Klammerung
  von Anweisungen in sogenannten kritischen Abschnitten (nicht unterbrechbare 
  Abfolgen von Anweisungen, auch atomare Blöcke genannt).
  Kritische Abschnitte koennen geschachtelt werden, da der Abschnitt jeweils den
  Zustand der globalen Interrupt-Maske vor Beginn des Blocks speichert und beim 
  Verlassen des Blocks restauriert.
  Es ist auch moeglich, einen Abschnitt vorzeitig mit BREAK_CRITICAL_SECTION zu
  verlassen. Das hat die gleiche Wirkung bezueglich der Restaurierung der
  globalen Interrupt-Maske. Aber Vorsicht: es wird \a break verwendet, daher ist
  BREAK_CRITICAL_SECTION \a nicht in Schleifen oder Switch-Statements, die innerhalb
  des kritischen Abschnitts liegen, anwendbar.
  \verbatim
  Verwendungsbeispiel:
  ...
  CRITICAL_SECTION {       // Beginn atomarer Block
	  ...
	  if (...)
	     BREAK_CRITICAL_SECTION; // Bedingter vorzeitiger Ausstieg
	  ...
  }                       // Ende atomarer Block
  
  Negativbeispiel (so geht es nicht:)
  ...
  CRITICAL_SECTION {       // Beginn atomarer Block
	  ...
	  while (...) {
		  BREAK_CRITICAL_SECTION; // hier wuerde nur die while Schleife beendet!
		  ...
	  }
  }                           // Ende atomarer Block
  \endverbatim
	
   @{
	   
*/

/*! \brief Beginn eines kritischen Abschnitts. 

    Siehe Verwendungsbeispiel.
*/
#define CRITICAL_SECTION ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

/*! \brief Verlaesst einen kritischen Abschnitt vorzeitig mittels \a break. 

    Siehe Verwendungsbeispiel.
*/
#define BREAK_CRITICAL_SECTION break

/*!  @}  */  // Ende group Kritische Abschnitte

/*****************************************************************************
    Definition der Software-Reset Funktion:
 *****************************************************************************/

/*! 
	\brief Modul fuer die Software-Reset Funktion.
	
	\defgroup group4 SoftReset

   Das STK600 Board hat keine Funktion zum Ausschalten per Software. Man kann
   allerdings einen Software-Reset simulieren, indem man mittels Watchdog Timer
   einen Watchdog Interrupt (der als RESET Interrupt abgebildet ist) ausloest.
   Dazu dient die Prozedur soft_reset().
   Mittels soft-reset() wird das Programm nach Ablauf einer einstellbaren
   Wartezeit neu gestartet. Allerdings muss ein Programm, welches auf diese Weise
   durch Watchdog time-out neu gestartet wird, den Watchdog als erstes ausschalten,
   da er über den RESET hinaus eingeschaltet bleibt, und so sofort wieder zu
   einem RESET fuehren wuerde. Die zum Ausschalten des Watchdogs notwendige
   Software wird mit diesem Modul automatisch eingebunden.

   @{
	   
 */

#include <avr/wdt.h>

/*! \cond */
//-------------------------------------------------------------------------
// Nach einem Watchdog timeout bleibt der Watchdog aktiv und wird nach dem
// Reset einen weiteren Watchdog timeout innerhalb 15ms nach Neustart ausloesen.
// Daher muss der Watchdog so frueh als moeglich nach dem Programmstart
// ausgeschaltet werden, sonst ensteht eine endlose RESET-Schleife. 
// Zu diesem Zweck wird der Aufruf von wdt_off() in die frueheste init-Sektion
// eingebaut - dieser Code wird unmittelbar nach RESET ausgefuehrt.

static void wdt_off (void) __attribute__((section(".init0"),naked, used));
static void wdt_off (void) {MCUSR = 0; wdt_disable();}
/*! \endcond */

/*! \brief  Soft-Reset timeout nach 15ms*/
#define RESET_15MS   WDTO_15MS
/*! \brief  Soft-Reset timeout nach 30ms*/
#define RESET_30MS   WDTO_30MS
/*! \brief  Soft-Reset timeout nach 60ms*/
#define RESET_60MS   WDTO_60MS
/*! \brief  Soft-Reset timeout nach 120ms*/
#define RESET_120MS  WDTO_120MS
/*! \brief  Soft-Reset timeout nach 250ms*/
#define RESET_250MS  WDTO_250MS
/*! \brief  Soft-Reset timeout nach 500ms*/
#define RESET_500MS  WDTO_500MS
/*! \brief  Soft-Reset timeout nach 1 s*/
#define RESET_1S     WDTO_1S
/*! \brief  Soft-Reset timeout nach 2 s*/
#define RESET_2S     WDTO_2S
/*! \brief  Soft-Reset timeout nach 4 s*/
#define RESET_4S     WDTO_4S
/*! \brief  Soft-Reset timeout nach 8 s*/
#define RESET_8S     WDTO_8S

/*! 
	\brief Software-Reset Funktion zum zwangsweisen Neustart des Programms.
	
	\param [in] delay Gibt an, nach wie vielen milli-Sekunden oder Sekunden ein Software 
	                  Reset ausgeloest werden soll. Darf nur Werte, die durch die Konstanten
					  RESET_15MS bis RESET_8S gegeben sind annehmen.
	
	Schaltet den Watchdog ein und stellt die per delay angegebene Verzoegerung ein. Diese ist
	nur ein ungefaehrer Wert. Nach Ablauf des delays erfolgt ein RESET Interrupt.
*/

inline void soft_reset (uint8_t delay) {wdt_enable(delay);};

/*!  @}  */  // Ende group SoftReset

#endif /* BASICS_H_ */