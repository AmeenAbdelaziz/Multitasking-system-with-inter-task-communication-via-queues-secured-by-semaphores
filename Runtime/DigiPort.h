/*! \brief DigiPort - verschiedene Klassen zur Ansteuerung von digitalen Ports

  

	\version   1.0
	\date      9.2.2018
	\author    Bomarius
	\copyright Bomarius
	\file      DigiPort.h
*/


#ifndef DIGIPORT_H_
#define DIGIPORT_H_


#include <avr/io.h>
#include <avr/interrupt.h>
#include "Basics.h"
#include "Timer.h"

/*! \cond */
/* Konstanten zur Port-Konfiguration
 * Diese Konstanten sind nur zur internen Nutzung!
 */
#define SET_PULL_UPS     ((uint8_t)(0xFF))
#define SET_NO_PULL_UPS  ((uint8_t)(0x00))
/*! \endcond */

/*! \ingroup group1
  * @{
  */

//===============================================================================
//============================  DigiPortRaw  ====================================
//===============================================================================
/*! \brief Klasse zur Abstraktion digitaler 8-Bit Ports. 

	Pro Port darf hoechstens ein DigiPort Objekt instanziiert werden. Der Versuch
	ein zweites Objekt zu instanziieren wird ignoriert.
    Beim Lesen vom DigiPortRaw Objekt werden nur Zustaende der Port-Register, aber
    keine Flanken erkannt (dafuer ist die Klasse DigiPortIRPT zu verwenden).
    Die Lese-Routinen arbeiten immer direkt auf dem Port-Register, d.h. lesen den
    momentanen (zufaelligen) Port-Wert ohne Entprellung.
 */
class DigiPortRaw {
/*! \cond */
protected:
	volatile uint8_t* const base;    // die Basisadresse des Ports (=PINx)
    uint8_t  direction;              // die Kommunikationsrichtung der Pins (=DDRx)
	                                 // die Pins können individuell in oder oder sein
	uint8_t  mode;                   // active low oder active high Logik; gilt
	                                 // dann gemeinsam für alle Pins des Ports

   // diese Konstanten werden benutzt, um die jeweils addressierten Ein- oder
   // Ausgabepins zu selektieren:
   #define IN_MASK  ((uint8_t)(~direction))
   #define OUT_MASK (direction)
/*! \endcond */
public:

    /*! \brief Erzeuge ein Port Objekt am durch p_handle spezifizierten Port.
	
	    \param [in] p_handle Eine Port-Bezeichnung gemaess Definition in Basics.h.
		\param [in] dir Bitmaske zur Spezifikation der Kommunikationsrichtung. Mittels
		           SET_IN_PORT oder SET_OUT_PORT koennen alle 8 Bit einheitlich
				   gesetzt werden. Bitweise individuelle Konfiguration ist durch
				   Angabe eines Bitmusters moeglich und wird Bit-individuell bei
				   den Lese- und Schreibeoperationen beachtet: nur die fuer Ausgabe
				   konfigurierten Bits werden geschrieben; nur die fuer Eingabe
				   konfigurierten Bits werden gelesen.
		\param [in] m Auswahl von active high oder active low Betriebsart. Dies bestimmt die
		         Betriebsart an den Pins des Ports. Alle Parameter und Funktionsrueckgabewerte
				 sind immer in active high Logik codiert.
	 */
	DigiPortRaw(uint8_t p_handle, uint8_t dir, uint8_t m = SET_ACTIVE_LOW);
	
    /*! \brief  Gib data auf den Ausgabepins des Ports aus.
		\param [in] data Auszugebenedes Bitmuster in active-high Logik. Bitpositionen, die
				    beim Port zur Eingabe konfiguriert sind, werden dabei ignoriert.
	*/
    inline void write(uint8_t data) { 
		*(base + OUT_OFFSET) = (uint8_t)(OUT_MASK & ((mode==SET_ACTIVE_HIGH) ? data : (uint8_t)(~data)));
    };

    /*! \brief Schalte die durch bits spezifizierten Bits des Ports ein.
		\param [in] bits Bitmuster in active-high Logik. Default 0xFF. Bitpositionen, die
		            beim Port zur Eingabe konfiguriert sind, werden dabei ignoriert.
	*/
	inline void on (uint8_t bits=0xFF) { 
		if (mode==SET_ACTIVE_HIGH)
	   		*(base + OUT_OFFSET) |= (uint8_t)(OUT_MASK & bits);
		else
			*(base + OUT_OFFSET) &= (uint8_t)(OUT_MASK & (uint8_t)(~bits));											   
    };
	
    /*! \brief Schalte die durch bits spezifizierten Bits des Ports aus.
		\param [in] bits Bitmuster in active-high Logik.  Default 0xFF. Bitpositionen, die
		            beim Port zur Eingabe konfiguriert sind, werden dabei ignoriert.
	*/
    inline void off (uint8_t bits=0xFF) { 
		if (mode==SET_ACTIVE_HIGH)
			*(base + OUT_OFFSET) &= (uint8_t)(OUT_MASK & (uint8_t)(~bits));
		else
			*(base + OUT_OFFSET) |= (uint8_t)(OUT_MASK & bits);
	};
	 
    /*! \brief Invertiere die durch bits spezifizierten Bits des Ports.
		\param [in] bits Bitmuster in active-high Logik. Default 0xFF. Bitpositionen, die
		            beim Port zur Eingabe konfiguriert sind, werden dabei ignoriert.
	*/
	inline void toggle (uint8_t bits=0xFF) {
		*(base + OUT_OFFSET) ^= (uint8_t)(OUT_MASK & bits);
	};

    /*! \brief Gib den aktuellen Port-Wert in active high Logik zurueck, ohne zu warten
	           und ohne zu entprellen.
	    \retval uint_8-Wert Der aktuelle Bitcode des Ports. Nicht zur Eingabe konfigurierte
		                    Bits und Bits, die durch die mask ausgeblendet sind liefern 0.
		\param [in] mask Bitmuster in active-high Logik zur Auswahl der zu lesenden Bitpositionen.
		            Falls die Maske Bitpositionen die zur Ausgabe konfiguriert sind enthaelt,
					werden diese mit 0 besetzt ausgegeben. Default ist 0xFF.
		
		Wenn mask mehr als ein 1-Bit hat, muss aus dem Rueckgabewert die erkannte Taste
		ermittelt werden. Die Reihenfolge in der auf gesetzte Bits getestet werden bestimmt
		dabei die Vorrang-Reihenfolge der Tasten (falls mehrere Tasten gleichzeitig gedrueckt
		werden).
					
		Sofern der Rueckgabewert komplementiert werden muss ist zu beachten, dass der Operator
		"~" einen 16 Bit Wert als Ergebnis hat und dann Vergleiche mit 8 Bit Werten nicht
		wie erwartet funktionieren. Daher ist ein Typecast zu verwenden, bspw. wie folgt:
		\verbatim
		switch ((uint8_t)(~keys.read_raw())){
			case 0b11111110: ...
			case 0b11111101: ...	
		}
		oder man vermeidet Komplementierung:
		switch (keys.read_raw()) {
			case 0b00000001: ...
			case 0b00000010: ...
		}
		\endverbatim
			  
	*/
    //inline uint8_t read_raw(uint8_t mask=0xFF) {
		//return IN_MASK & mask & ((mode==SET_ACTIVE_HIGH) ? (*(base + IN_OFFSET)) : ~(*(base + IN_OFFSET)));
	//};
	
	inline uint8_t read_raw(uint8_t mask=0xFF) {
		uint8_t ret_val;
		CRITICAL_SECTION {
		 ret_val = IN_MASK & mask & ((mode==SET_ACTIVE_HIGH) ? (*(base + IN_OFFSET)) : ~(*(base + IN_OFFSET)));
		}
		return ret_val;
	};
 
    /*! \brief Warte (mittels busy wait), dass einer der mit mask spezifizierten Bits
	           gesetzt wird. Typischerweise zum Lesen von Tasten verwendet. Rueckgabewert in
			   active high Logik.
	    \retval uint_8-Wert Der aktuelle Bitcode des Ports. Nicht zur Eingabe konfigurierte
			   	    Bits und Bits, die durch die mask ausgeblendet sind liefern 0.
		\param [in] mask Bitmuster in active-high Logik zur Auswahl der zu lesenden Bitpositionen.
		            Falls die Maske Bitpositionen, die zur Ausgabe konfiguriert sind enthaelt,
		            werden diese mit 0 besetzt ausgegeben. Ein gesetztes Bit in mask fuehrt
					zur Beruecksichtigung der entsprechenden Taste. Default ist 0xFF.

		 Wenn mask mehr als ein 1-Bit hat, muss aus dem Rueckgabewert die erkannte Taste 
		 ermittelt werden. Die Reihenfolge in der auf gesetzte Bits getestet werden bestimmt
		 dabei die Vorrang-Reihenfolge der Tasten (falls mehrere Tasten gleichzeitig gedrueckt
		 werden).
	 */	
	uint8_t read_busy_wait(uint8_t mask=0xFF);

    /*! \brief Warte (mittels Task-Wechsel), dass einer der mit mask spezifizierten Bits
	           gesetzt wird. Typischerweise zum Lesen von Tasten verwendet. Rueckgabewert in
			   active high Logik.
	    \retval uint_8-Wert Der aktuelle Bitcode des Ports. Nicht zur Eingabe konfigurierte
			   	    Bits und Bits, die durch die mask ausgeblendet sind liefern 0.
		\param [in] mask Bitmuster in active-high Logik zur Auswahl der zu lesenden Bitpositionen.
		            Falls die Maske Bitpositionen, die zur Ausgabe konfiguriert sind enthaelt,
		            werden diese mit 0 besetzt ausgegeben. Ein gesetztes Bit in mask fuehrt
					zur Beruecksichtigung der entsprechenden Taste. Default ist 0xFF.

		 Wenn mask mehr als ein 1-Bit hat, muss aus dem Rueckgabewert die erkannte Taste 
		 ermittelt werden. Die Reihenfolge in der auf gesetzte Bits getestet werden bestimmt
		 dabei die Vorrang-Reihenfolge der Tasten (falls mehrere Tasten gleichzeitig gedrueckt
		 werden).
		 
		 Diese Funktion ist nur im Multi-Tasking Kontext verwendbar und vermeidet busy
		 waiting eines durch die Leseanforderung blockierten Tasks.
	 */	

    uint8_t read_blocking(uint8_t mask=0xFF);

};

//===============================================================================
//===========================  DigiPortIRPT  ====================================
//===============================================================================

/*! \brief DigiPort mit Interrupt-gesteuertem (flankengesteuertem) Lesen.

    \verbatim
    DigiPortIRPT bietet interruptbasierte Funktionen fuer einen Teil derjenigen
	Ports, welche IRQs generieren koennen. Dies sind (beim MEGA 2560):
      PORT B  Pins 0 .. 7    Pin Change Interrupt (Flankenerkennung, PCINT0)
      PORT K  Pins 0 .. 7    Pin Change Interrupt (Flankenerkennung, PCINT2)
      PORT J  Pins 0 .. 6    Pin Change Interrupt (Flankenerkennung, PCINT1)
    Folgende sind beim MEGA 2560 vorhanden aber deren Nutzung ist in der Klasse 
	bislang *nicht* implementiert:
      PORT E  Pin  0         Pin Change Interrupt (Flankenerkennung, PCINT1)
      PORT D  Pins 0 .. 3    ext. Interrupt (Flanken & Status, INT0 .. INT3)
      PORT E  Pins 4 .. 7    ext. Interrupt (Flanken & Status, INT4 .. INT7)
 
    Die Pin Change Interrupts (ISRs PCINT0 bis PCINT2) koennen nur Flanken
    erkennen und werden gerufen sobald einer der bis zu 8 ueberwachten Pins 
    eine Signalflanke zeigt. Beachte: bei Port J kann Pin 7 keinen IRQ
	erzeugen!
	
	Das DigiPortIRPT Objekt speichert bei einem Interrupt die aufgetretene Flanke
	bis die Information abgerufen und dadurch geloescht wird. Der Interrupt
	wird also sicher erkannt - das Anwendungsprogramm kann das Ereignis
	abfragen, wenn es bereit ist.
	Muss auf einen Interrupt ohne Verzoegerung in der Applikation reagiert werden,
	kann diese eine Call-Back Routine uebergeben, die von der Interrupt Routine
	des DigiPortIRPT Objektes gerufen wird. Darin kann dann abgefragt werden,
	was den Interrupt ausgeloest hat. Diese Routine muss so kurz als moeglich
	sein, da alle anderen Interrupts waehrend ihrer Ausfuehrung blockiert sind.	
	\endverbatim
	*/
	
// Innerhalb der ISR muss der den IRQ ausloesende Pin ermittelt
// werden. Durch PCMSK0 bis PCMSK2 werden die Pins als IRQ Quellen
// konfiguriert.
// Bei den externen Interrupts (ISRs INT0 bis INT7) ist eine ISR einem Pin
// zugeordnet und kann außerdem (je nach Konfiguration in EICRA und EICRB)
// auf fallende Flanke, auf steigende Flanke und auf Zustand auslösen. Mit
// EIMSK werden die Pins als IRQ Quellen konfiguriert. Wir verwenden hier
// nur die Flanken-IRQs. Port D (Pin 0:3) und Port E (Pin 4:7) verhalten
// sich dann wie die Ports B, K und J.


class DigiPortIRPT : public DigiPortRaw {
	private:
		CBF_t cbf;
	
/*! \cond */
protected:
    volatile uint8_t rising_edge_detected;   // steigende Flanke je Pin
	volatile uint8_t falling_edge_detected;  // fallende Flanke je Pin
	volatile uint8_t old_state;              // letzter Status der Pins
/*! \endcond */
public:


    /*! \brief Erzeuge ein Port Objekt am durch p_handle spezifizierten Port.
	
	    \param [in] p_handle Eine Port-Bezeichnung gemaess Definition in Basics.h.
		\param [in] dir Bitmaske zur Spezifikation der Kommunikationsrichtung. Mittels
		           SET_IN_PORT oder SET_OUT_PORT koennen alle 8 Bit einheitlich
				   gesetzt werden. Bitweise individuelle Konfiguration ist durch
				   Angabe eines Bitmusters moeglich und wird Bit-individuell bei
				   den Lese- und Schreibeoperationen beachtet: nur die fuer Ausgabe
				   konfigurierten Bits werden geschrieben; nur die fuer Eingabe
				   konfigurierten Bits werden gelesen.
		\param [in] m Auswahl von active high oder active low Betriebsart. Dies bestimmt die
		         Betriebsart an den Pins des Ports. Alle Parameter und Funktionsrueckgabewerte
				 sind immer in active high Logik codiert.
		\param [in] cbf optionale call-back Funktion der Applikation, die bei einen
		          DigiPortIRPT Event zu rufen ist; sie ist vom Typ void cbf(void). In der
				  Callback Funktion muss mittels falling_edge oder rising_edge der Grund
				  erfragt werden. Beachte: hier \a nicht mit read_raw() arbeiten!
	 */
    DigiPortIRPT(uint8_t p_handle, uint8_t dir, uint8_t m = SET_ACTIVE_LOW, CBF_t cbf = NULL);							
	
    /*! \brief Warte auf eine steigende Flanke an einem der selektierten Pins.
		\retval uint_8-Wert Bitcode zur Anzeige derjenigen Pins, an denen Flanken 
		                    festgestellt wurden. Nicht zur Eingabe konfigurierte
		                    Bits und Bits, die durch die mask ausgeblendet sind liefern 0.
	    \param [in] maske Selektiert in active high Logik diejenigen Pins, an denen
		             auf eine steigende Flanke gewartet werden soll.
					 
		Die Funktion verwendet busy waiting und kehrt zurueck, sobald an mindestens
		einem der selektierten Pins eine steigende Flanke erkannt wurde. Der Funktionswert
		spezifiziert in active high Logik diejenigen Pins mit erkannter Flanke.
	*/
	inline uint8_t wait_rising_edge  (uint8_t mask=0xFF) {
		uint8_t result;
		while(!(result = (uint8_t)(mask & rising_edge_detected)));
		rising_edge_detected &= (uint8_t)(~mask);
		return result;
	};
	
    /*! \brief Warte auf eine fallende Flanke an einem der selektierten Pins.
	    \retval uint_8-Wert Bitcode zur Anzeige derjenigen Pins, an denen Flanken
			festgestellt wurden. Nicht zur Eingabe konfigurierte
			Bits und Bits, die durch die mask ausgeblendet sind liefern 0.
	    \param [in] maske Selektiert in active high Logik diejenigen Pins, an denen
		             auf eine fallende Flanke gewartet werden soll.
					 
		Die Funktion verwendet busy waiting und kehrt zurueck, sobald an mindestens
		einem der selektierten Pins eine fallende Flanke erkannt wurde. Der Funktionswert
		spezifiziert in active high Logik diejenigen Pins mit erkannter Flanke.
	*/
	inline uint8_t wait_falling_edge (uint8_t mask=0xFF) {
		uint8_t result;
		while(!(result = (uint8_t)(mask & falling_edge_detected)));
		falling_edge_detected &= (uint8_t)(~mask);
		return result;
	};

    /*! \brief Teste ohne zu warten auf fallende Flanke an einem der ueberwachten Pins.
		\retval uint_8-Wert Bitcode zur Anzeige derjenigen Pins, an denen Flanken
			festgestellt wurden. Nicht zur Eingabe konfigurierte
			Bits und Bits, die durch die mask ausgeblendet sind liefern 0.
	    \param [in] maske Selektiert in active high Logik diejenigen Pins, an denen
		             auf eine fallende Flanke getestet werden soll.
					 
		Der Funktionswert spezifiziert in active high Logik diejenigen Pins mit
		erkannter Flanke.
	*/
	inline uint8_t falling_edge (uint8_t mask=0xFF) {
		uint8_t result = (uint8_t)(falling_edge_detected & mask);
		falling_edge_detected &= (uint8_t)(~mask);
		return result;
	};
	
    /*! \brief Teste ohne zu warten auf steigende Flanke an einem der ueberwachten Pins.
		\retval uint_8-Wert Bitcode zur Anzeige derjenigen Pins, an denen Flanken
			festgestellt wurden. Nicht zur Eingabe konfigurierte
			Bits und Bits, die durch die mask ausgeblendet sind liefern 0.
	    \param [in] maske Selektiert in active high Logik diejenigen Pins, an denen
		             auf eine steigende Flanke getestet werden soll.
					 
		Der Funktionswert spezifiziert in active high Logik diejenigen Pins mit
		erkannter Flanke.
	*/
	inline uint8_t rising_edge  (uint8_t mask=0xFF) {
		uint8_t result = (uint8_t)(rising_edge_detected & mask);
		rising_edge_detected &= (uint8_t)(~mask);
		return result;
	};

/*! \cond */
    // nur zur internen Nutzung. Notify Funktion, die von der ISR gerufen wird.		
	virtual inline void notify() {
		uint8_t new_state = this->read_raw();
		rising_edge_detected  |= (uint8_t)((uint8_t)(~new_state) & old_state);
		falling_edge_detected |= (uint8_t)((uint8_t)(~old_state) & new_state);
		old_state = new_state;
		if (NULL != cbf) cbf();
	};
/*! \endcond */
};


/*! @}*/
	
#endif /* DIGIPORT_H_ */