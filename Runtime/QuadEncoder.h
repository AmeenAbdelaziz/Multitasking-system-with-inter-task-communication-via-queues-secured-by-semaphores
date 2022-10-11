#ifndef QUADENCODER_H_
#define QUADENCODER_H_

/*!
	\version   1.0
	\date      9.2.2018
	\author    Bomarius
	\copyright Bomarius
	\file      QuadEncoder.h
*/

/*! \brief Modul zum Betrieb eines Quadratur-Encoders mit Anzeige-LEDs und Tastenfunktion.
		   
	Die Treiber-Klasse QuadEncoder basiert auf der Flankenerkennung an digitalen Ports
	wie sie durch die Basisklasse DigiPortIRPT aus dem Modul Ports implementiert wird.   

	\defgroup group12 QuadEncoder
	@{
*/

#include "Basics.h"
#include "DigiPort.h"

/*! \cond */
// Pin-Belegung des Drehgeber-Bausteins (mit Taster und 3 Signal-LEDs)
#define PIN_CHAN_A      (0)
#define PIN_CHAN_B      (1)
#define PIN_KEY         (3)
#define PIN_LOCKED_LED  (4)
#define PIN_MAX_LED     (2)
#define PIN_MIN_LED     (5)
// Zu der Pin-Belegung passende Port-Konfiguration
#define QUAD_ENC_DDR_CONF      ((1<<PIN_LOCKED_LED) | (1<<PIN_MAX_LED) | (1<<PIN_MIN_LED))
/*! \endcond */

// Konstanten zum Schalten der LEDs:
/*! Bitposition der roten LED (Maximalwert). */
#define QUAD_ENC_MAX_LED       (1<<PIN_MAX_LED)
/*! Bitposition der gelben LED (Minimalwert). */
#define QUAD_ENC_MIN_LED       (1<<PIN_MIN_LED)
/*! Bitposition der gruenen LED (Taste wurde gedrueckt). */
#define QUAD_ENC_LOCKED_LED    (1<<PIN_LOCKED_LED)
/*! Bitposition aller LEDs gemeinsam. */
#define QUAD_ENC_ALL_LEDS      (QUAD_ENC_LOCKED_LED | QUAD_ENC_MAX_LED | QUAD_ENC_MIN_LED)

//=================================================================================
//==============================  QuadEncoder  ====================================
//=================================================================================
/*! \brief Klasse  zur Abstraktion von Quadratur-Encodern.

    Ein QuadEncoder Objekt kann nur an Ports mit Interrupterzeugung fuer Flanken
	betrieben werden. Dies sind die Ports B, K und J des ATMEGA 2560. Dabei werden 
	die Quadratur-Signale (Rechtecksignale, Kanaele A und B) sowie der Taster
	mithilfe von IRPTs gelesen und im Objekt gespeichert. Es werden drei Signal-LEDS
	angesteuert. Die LOCKED_LED (gruen) leuchtet, nachdem der Taster gedrueckt wurde
	und so der aktuelle eingestellte Wert des Drehgebers "gelockt" (fixiert) wurde. Die LEDs 
	MAX (rot) bzw. MIN (gelb) leuchten, sobald das jeweilige Ende des konfigurierten
	Einstellbereichs erreicht ist. Weitere Drehung in dieser Richtung bewirkt dann
	keine Wertaenderung mehr.
	Der Drehgeber muss mittels einer der start() Methoden aktiviert werden, um Werte
	abfragen zu koennen. Wenn er nicht mehr gebraucht wird, kann er mit stop()
	deaktiviert werden.
	
	\verbatim
	1. Verwendungsbeispiel: Lesen des Drehgebers quenc bis Taste gedrueckt wird
	
	quenc.start(1, 11, 5); // stelle Wertebereich 1 bis 11 ein und setzte 5 als aktuellen 
	                       // Wert und starte die Funktion des Drehgebers
	while (! quenc.new_locked_value_available()) {
		if (quenc.new_value_available()) {
    		val = (uint8_t)quenc.get_unlocked_value();
		    // val verarbeiten
		}
	};
	quenc.stop();  // Stoppe den Drehgeber und loesche LEDs des Drehgebers, die evtl. noch an sind
	
	
	2. Verwendungsbeispiel: Lesen des Drehgebers quenc mit Unterscheidung zwischen fixierten und nicht fixierten Werten
	
	quenc.start(1, 11, 5); // stelle Wertebereich 1 bis 11 ein und setzte 5 als aktuellen
	                       // Wert und starte die Funktion des Drehgebers
	while (1) {
		if (quenc.new_locked_value_available()) {
			val = (uint8_t)quenc.get_locked_value();
			// val verarbeiten
			// falls der Knopfdruck die Eingabe beenden soll hier ein break einfuegen	
		} else if (quenc.new_value_available()) {
			val = (uint8_t)quenc.get_unlocked_value();
			// val verarbeiten
		}
	};
	quenc.stop();  // Stoppe den Drehgeber und loesche LEDs des Drehgebers, die evtl. noch an sind
	
	\endverbatim
 */

class QuadEncoder : protected DigiPortIRPT {
/*! \cond */
private:
    static int8_t sub_state_table [4][4];

    using DigiPortRaw::base;

    volatile bool      is_active;        // ist der Drehgeber zur Zeit aktiviert?
	volatile bool      new_locked_value; // hat sich seit letztem Lesen der Wert geändert?
	volatile bool      new_curr_value;   // hat sich der Einstellwert geändert
	int16_t            min;              // untere Einstellgrenze
	int16_t            max;              // obere Einstellgrenze
	volatile int16_t   locked_value;     // letzter fixierter Zählerstand
	volatile int16_t   curr_value;       // aktueller Zählerstand
	uint8_t            curr_gray_value;  // aktuell gelesener Gray-Code des Encoders
	uint8_t            last_gray_value;  // alter Gray-Code des Encoders
/*! \endcond */

public:

	/*! \brief Erzeuge das Quad-Encoder-Objekt am durch das handle gegebenen Port (B,K,J).
	
		\param [in] p_handle Eine Port-Bezeichnung gemaess Definition in Basics.h.
	 
	    Das Objekt kapselt den 2-Kanal Encoder, den Taster des Encoders sowie 
	    drei Anzeige-LEDS und steuert diese ueber den gewaehlten Port an.
		Die Grenzen des Einstellbereichs sowie der Startwert muessen spaeter ueber eine
		start() Methode noch gesetzt werden. Initial werden die Grenzen 0 und 9999 
		sowie der Startwert 4999 eingestellt.
	*/
	QuadEncoder (uint8_t p_handle);
	
	/*! \brief Schalte die durch mask selektierten LEDs an.
	    \param [in] mask Bitposition(en) der einzuschaltetenden LEDs. Angabe in active high Logik.
		
		Zur Bildung der Maske sollten die Makros QUAD_ENC_MAX_LED, QUAD_ENC_MIN_LED, QUAD_ENC_LOCKED_LED
		und QUAD_ENC_ALL_LEDS verwendet werden. 
	*/
	inline void on (uint8_t mask = QUAD_ENC_ALL_LEDS) {*(base + OUT_OFFSET) &= ~mask;};
		
	/*! \brief Schalte die durch mask selektierten LEDs aus.
	    \param [in] mask Bitposition(en) der auszuschaltetenden LEDs. Angabe in active high Logik.
		
		Zur Bildung der Maske sollten die Makros QUAD_ENC_MAX_LED, QUAD_ENC_MIN_LED, QUAD_ENC_LOCKED_LED
		und QUAD_ENC_ALL_LEDS verwendet werden. 
	*/
	inline void off (uint8_t mask = QUAD_ENC_ALL_LEDS) {*(base + OUT_OFFSET) |=  mask;};
		
	/*! \brief Invertiere die durch mask selektierten LEDs.
	    \param [in] mask Bitposition(en) der zu invertierenden LEDs. Angabe in active high Logik.
		
		Zur Bildung der Maske sollten die Makros QUAD_ENC_MAX_LED, QUAD_ENC_MIN_LED, QUAD_ENC_LOCKED_LED
		und QUAD_ENC_ALL_LEDS verwendet werden. 
	*/
	inline void toggle (uint8_t mask = QUAD_ENC_ALL_LEDS) {*(base + OUT_OFFSET) ^=  mask;};

	/*! \brief Gib den aktuell konfigurierten Minimalwert des Einstellbereichs zurueck.*/
	inline int16_t get_min () {return min;};
		
	/*! \brief Gib den aktuell konfigurierten Maximalwert des Einstellbereichs zurueck.*/
	inline int16_t get_max () {return max;};
	
	/*! \brief Setze den momentanen Wert des Drehgebers. 
	    \param [in] v Neuer Wert des Drehgebers, sofern erfolgreich gesetzt.
		
		Beachte dass der Wert nur gesetzt wird, wenn er innerhalb der aktuell eingestellten
		Grenzen liegt. Eine Abfrage von new_value_avalaible() unmittelbar danach meldet 
		dann einen neuen verfuegbaren Wert, als ob eine Eingabe am Drehgeber erfolgt waere.
	*/					
	inline void set_value(int16_t v) {
		if (v <= max && v >= min) {new_curr_value = true; curr_value = v;};};
		
	/*! \brief Starte die Funktion des Drehgebers und setze dazu den momentanen Wert des 
	           Drehgebers auf den Wert value und die neuen Einstellgrenzen auf min und max.
		\param [in] min  Neue untere Grenze.
		\param [in] max	 Neue obere Grenze.	    
		\param [in] value  Neuer aktueller nicht fixierter Wert.
					   
		Wenn min >= max ist, werden die alten Grenzwerte beibehalten. Der Wert value wird
		zum neuen aktuellen Wert, sofern er innernhalb der neuen Grenzen liegt. Sollte value
		ausserhalb liegen, wird als neuer Wert von value die Mitte des Einstellbereichs genommen.
		Wenn sofort nach start() nach new_value_available() oder new_locked_value_available()
		abgefragt wird, wird false gemeldet, solange noch keine Dreheingabe erfolgt ist.
	*/
    void start (int16_t min, int16_t max, int16_t value);

	/*! \brief Starte die Funktion des Drehgebers und setze dazu den momentanen Wert des 
	           Drehgebers auf den Wert value.  Diese Funktion ist eine Kurzform von
			   void start(min, max, value) und fuer erneute Aktivierung bei Beibehaltung
			   der Grenzen gedacht.
			   
		\param [in] value  Neuer aktueller nicht fixierter Wert.
					   
		Die alten Grenzwerte werden beibehalten. Der Wert value wird
		zum neuen aktuellen Wert, sofern er innernhalb der Grenzen liegt. Sollte value
		ausserhalb liegen, wird als neuer Wert von value die Mitte des Einstellbereichs genommen.
		Wenn sofort nach start() nach new_value_available() oder new_locked_value_available()
		abgefragt wird, wird false gemeldet, solange noch keine Dreheingabe erfolgt ist.
	*/
	
	inline void start(int16_t value) {start (0, 0, value);};
	
	/*! \brief Stoppe die Funktion des Drehgebers. Seine LEDs werden geloescht und er reagiert
	           weder auf Drehen noch auf Tastendruck. Einstellgrenzen bleiben erhalten.
	*/
	inline void stop (void) {off(); is_active = false;};

	/*! \brief Stelle fest, ob am Drehgeber ein bislang noch nicht mittels get_value()
	           oder get_unlocked_value() abgeholter Wert vorliegt.*/
	inline bool new_value_available () {return new_curr_value;};
		
	/*! \brief Gib den letzten nicht fixierten Einstellwert des Drehgebers zurueck.
	
	    Beachte: wenn get_unlocked_value() (oder get_value()) zu selten aufgerufen wird, gehen der Applikation 
		moeglicherweise Einstellwerte verloren. Es wird immer der zuletzt detektierte Wert
		geliefert, bei sehr rascher Aufruffolge auch wiederholt der gleiche Wert.
		Ein ggf. frueher fixierter Wert, der noch nicht mittels get_locked_value() geholt
		wurde, bleibt gespeichert.
	*/
	inline int16_t get_unlocked_value() {new_curr_value = false; return curr_value;};

	/*! \brief Stelle fest, ob am Drehgeber ein bislang noch nicht mittels get_locked_value()
	           abgeholter mittels Tastendruck fixierter Wert vorliegt.*/
	inline bool new_locked_value_available() {return new_locked_value;};
		
	/*! \brief Gib den letzten fixierten Einstellwert des Drehgebers zurueck.
	
	    Beachte: wenn zwischenzeitlich der Drehgeber bewegt wird, ist der fixierte
		Wert nicht der zuletzt detektierte Wert, sondern der Wert zum Zeitpunkt des
		Tastendrucks.
	*/		
	inline int16_t get_locked_value () {
		new_locked_value = false;
		off(QUAD_ENC_LOCKED_LED); 
		return locked_value;
	};
	
	/*! \brief Gib den letzten Einstellwert des Drehgebers zurueck.
	
	    Beachte: wenn get_value() (oder get_unlocked_value()) zu selten aufgerufen wird, gehen der Applikation 
		moeglicherweise Einstellwerte verloren. Es wird immer der zuletzt detektierte Wert
		geliefert, bei sehr rascher Aufruffolge auch wiederholt der gleiche Wert.
		Ein ggf. frueher fixierter Wert, der noch nicht mittels get_locked_value() geholt
		wurde, wird jetzt geloescht.
	*/
	inline int16_t get_value() {
		new_curr_value = false;
		new_locked_value = false; 
		off(QUAD_ENC_LOCKED_LED); 
		return curr_value;
	};


/*! \cond */
	// nur zur internen Verwendung:
	void notify();
/*! \endcond */
};

/*! @}  Ende group12 QuadEncoder */


#endif /* QUADENCODER_H_ */