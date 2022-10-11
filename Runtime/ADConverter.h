/*!	 \brief ADConverter - Modul zu Ansteuerung von Analog-Digital-Wandlern (ADC).
	 \version   1.0
	 \date      9.2.2018
	 \author    Bomarius
	 \copyright Bomarius
	 \file      ADConverter.h
*/


//=================================================================================
//===============================  ADConverter  ===================================
//=================================================================================

/*! \brief Modul zu Ansteuerung von Analog-Digital-Wandlern (ADC). 

  \defgroup group6 ADC
  
  Ein ADConverter-Objekt kann jeweils einen ADC-Kanal abstrahieren.
  Die Implementierung ist mehrkanalfaehig. Als Betriebsmodi sind entweder polling 
  oder Interrupt-Betrieb einstellbar (fuer alle Kanaele einheitlich). Je Kanal
  kann eine eigene Mittelwertbildung konfiguriert werden.
  
  Der ADConverter abstrahiert die Analog-Digital Converter des ATMega2560.
  Erlaubte Ports sind F und K. Jeder Pin dieser Ports ist ein ADC Kanal.
  Die Kanaele werden "single-ended" betrieben und liefern jeweils einen 10 Bit
  Wert. Die Bezugsspannungen sind 0 und 5V.
  Welcher Kanal genutzt werden soll, legt Parameter chan des Konstruktors fest.
  Die ADC Kanaele koennen (jeweils alle) entweder free-running (kontinuierliche
  Wandlung und Nutzung des ADC Interrupts) oder im Mode polling (Wandlung auf
  Nachfrage) betrieben werden. Die Einstellung dieses Modus erfolgt durch
  Setzen der Konstanten ADC_MODE (zur Uebersetzzeit!).
  Bei Erzeugung eines ADC Kanal Objekts muss neben dem Kanal auch angegeben werden,
  ob die Werte durch Mehrfachlesen und Mittelwertbildung geglaettet werden sollen.
  Der default Wert hierfuer ist 1 (einmalige Wandlung liefert Wert). Insbesondere
  im polling Modus fuehrt eine hohe Anzahl Mehrfachwerte zu entsprechend langen
  Wartezeiten beim Abfragen des betroffenen ADC Objekts. Insofern sollte bei
  notwendiger Glaettung kontinuierliche Wandlung in Betracht gezogen werden.
  An Port F identifiziert chan die AD Kanaele 0 bis 7 und fuer Port K die
  Kanaele 8 bis 15. Der chan-Wert muss aus den define-Konstanten AD_CAHN_xx gebildet werden.
  Bei Nutzung der internen Referenzspannung (fuer ADC noetig) muessen die Jumper ARFE1 und
  AREF0 auf dem STK600 entfernt werden, sonst blinkt die LED rot (VREF shortcut).
  Zitat aus STK600 user-guide; page 71: 6.3.2. Using the Internal Voltage Reference
  "If the Atmel AVR's internal voltage reference is used, the AREF0/AREF1 jumper
  must be removed."
 
 
 \version   1.0
 \date      9.2.2018
 \author    Bomarius
 \copyright Bomarius
 \file      ADConverter.h

 
  @{
  
 */

#ifndef ADCONVERTER_H_
#define ADCONVERTER_H_

#include <avr/io.h>

/*! \def AD_CHAN_0 
    \brief entspricht Pin 0 Port F */
#define   AD_CHAN_0    ((uint8_t)(0b00000000))
/*! \def AD_CHAN_1 
    \brief entspricht Pin 1 Port F */
#define   AD_CHAN_1    ((uint8_t)(0b00000001))
/*! \def AD_CHAN_2 
    \brief entspricht Pin 2 Port F */
#define   AD_CHAN_2    ((uint8_t)(0b00000010))
/*! \def AD_CHAN_3 
    \brief entspricht Pin 3 Port F */
#define   AD_CHAN_3    ((uint8_t)(0b00000011))
/*! \def AD_CHAN_4 
    \brief entspricht Pin 4 Port F */
#define   AD_CHAN_4    ((uint8_t)(0b00000100))
/*! \def AD_CHAN_5 
    \brief entspricht Pin 5 Port F */
#define   AD_CHAN_5    ((uint8_t)(0b00000101))
/*! \def AD_CHAN_6 
    \brief entspricht Pin 6 Port F */
#define   AD_CHAN_6    ((uint8_t)(0b00000110))
/*! \def AD_CHAN_7 
    \brief entspricht Pin 7 Port F */
#define   AD_CHAN_7    ((uint8_t)(0b00000111))
/*! \def AD_CHAN_8 
    \brief entspricht Pin 0 Port K */
#define   AD_CHAN_8    ((uint8_t)(0b00100000))
/*! \def AD_CHAN_9 
    \brief entspricht Pin 1 Port K */
#define   AD_CHAN_9    ((uint8_t)(0b00100001))
/*! \def AD_CHAN_10 
    \brief entspricht Pin 2 Port K */
#define   AD_CHAN_10   ((uint8_t)(0b00100010))
/*! \def AD_CHAN_11 
    \brief entspricht Pin 3 Port K */
#define   AD_CHAN_11   ((uint8_t)(0b00100011))
/*! \def AD_CHAN_12 
    \brief entspricht Pin 4 Port K */
#define   AD_CHAN_12   ((uint8_t)(0b00100100))
/*! \def AD_CHAN_13 
    \brief entspricht Pin 5 Port K */
#define   AD_CHAN_13   ((uint8_t)(0b00100101))
/*! \def AD_CHAN_14 
    \brief entspricht Pin 6 Port K */
#define   AD_CHAN_14   ((uint8_t)(0b00100110))
/*! \def AD_CHAN_15 
    \brief entspricht Pin 7 Port K */
#define   AD_CHAN_15   ((uint8_t)(0b00100111))

/*!  \def FREE_RUNNING
     \brief Makro zum Setzen des Interrupt-getriebenen Betriebsmodus. Alle konfigurierten Kanaele 
	  werden staendig gewandelt. Der jeweils letzte Wert wird in ADConverter Objekt zur Abholung
	  mittels \a get_value() gespeichert.
*/
#define FREE_RUNNING (0)

/*! \def POLLING
    \brief Makro zum Setzen des Polling Betriebsmodus. Ein Kanal wird nur bei Datenabruf gewandelt.
	Die Abruffunktion \a get_value() triggert die Wandlung und wartet auf das Wandlungsergebnis.
*/
#define POLLING (1)

/*! \def ADC_MODE
    \brief Durch Zuweisung von \a POLLING oder \a FREE_RUNNING wird mittels Makro ADC_MODE die
	Betriebsart gewaehlt.
*/

// Hier die Art der Datenakquisition fuer den ADC Wandler einstellen:
// nur explizite Wertabfrage fuehrt zur Wandlung; Rufer wartet:
#define ADC_MODE POLLING
// Werte werden im Interruptbetrieb permanent abgelesen und gespeichert:
//#define ADC_MODE  FREE_RUNNING


/*! \brief Typ der Kennlinien-Funktion, die beim Erzeugen eines ADC-Objektes uebergeben werden 
           kann, damit das ADC Objekt die gelesenen Sensorwerte automatisch gemaess der Kennlinie
		   korrigiert zurueckliefert. 
*/
typedef uint16_t (*cft)(uint16_t);

/*! \brief ADConverter - Klasse zur Abstraktion des ADC.*/
class ADConverter {
	
private:
	static const uint8_t  mode;          // FREEE_RUNNING (kontinuierliche Wandlung) oder
	                                     // Wandlung auf Nachfrage (POLLING)

	static ADConverter* adc_object[16];  // Liste der instanziierten ADC Objekte
	static uint8_t inst_cnt;             // Anzahl der erzeugten Instanzen

    uint8_t chan;               // Kanalnummer des ADC Objekts.

	volatile uint16_t value;	// jeweils letzter gewandelter Wert des Kanals bzw.
	                            // Summe der Werte im Averaging Betriebsmodus, der noch nicht
								// gemittelt und abgeholt wurde. 

	volatile uint16_t future;   // Bei FREE_RUNNING genutzt: hier wird der naechste gewandelte 
	                            // Wert gebildet, der bei Ende der Wandlung value ersetzt;
								// zwischenzeitlich kann der vorige Wert aus value abgeholt werden.

	volatile bool new_value;	// Flag zur Signalsierung, ob ein noch nicht abgeholter Wert vorliegt.

	const uint8_t averaging;    // Liefere Mittelwerte aus Mehrfachmessungen (averaging > 1)
		                        // oder nur einfach gemessene Werte (averaging = 1, default)

	uint8_t avg_cnt;            // Zaehler fuer den Averaging Betriebsmodus.
	
    cft convert;			    // Kanalspezifische Kennlinienfunktion; Default: Identitaet


	// ISR zur Behandlung der ADC Interrupts; nur verwendet, falls der ADC im Modus
	// FREE_RUNNING betrieben wird. Hinterlegt den gewandelten und ggf gemittelten
	// Wert beim zustaendigen ADC Objekt. Beliefert die Objekte reihum mit neuen Werten.
	// Schaltet die benutzten Kanaele des ADC reihum durch.
	static void adc_isr () __asm__ ("__vector_29") __attribute__ ((__signal__, __used__));


public:

    /*! \brief  Liefere rohen aktuellen Wert des ADC Objektes (Identitaetsfunktion).
		\retval unveraenderter Sensorwert
		\param [in] adc_val der rohe Sensorwert
		
		Diese Funktion ist Default, wenn beim Konstruktor keine andere Funktion angegeben wird.
	*/ 
    static uint16_t identity (uint16_t adc_val) {return adc_val;};
    
    /*! \brief  Liefere korrigierten Wert des ADC Objektes (Korrekturkurve fuer Temperatursensor LM335).
		\retval korrigierter Sensorwert
		\param [in] adc_val der rohe Sensorwert
		
		Diese Funktion ist fuer den Temperatursensor LM335 verwendbar.
	*/ 
    static uint16_t temp_convert (uint16_t adc_val) {return adc_val*0.48828 - 273.15;};
    
    /*! \brief  Liefere korrigierten Wert des ADC Objektes (Korrekturkurve fuer Feuchtesensor HIH 4010-001).
		\retval korrigierter Sensorwert
		\param [in] adc_val der rohe Sensorwert
		
		Diese Funktion ist fuer den Feuchtesensor HIH 4010-001 verwendbar.
	*/ 
	static uint16_t hum_convert (uint16_t adc_val) {return adc_val*0.1631 - 26.753;};


    /*! \fn ADConverter(uint8_t chan, uint8_t avg = 1)
	    \brief Konstruktor: Erzeuge ein ADConverter Objekt fuer den Kanal \a chan.
		\param [in] chan Spezifikation des ADC Kanals. Wird durch eines der Makros AD_CHAN_xx definiert.
		       Beachte: Die Kanaele AD_CHAN_0 bis AD_CHAN_7 gehoeren zum Port F und die Kanaele AD_CHAN_8 
			   bis AD_CHAN_15 zum Port K. Die Verkabelung am Board muss entsprechend hergestellt werden.
			   
	    \param [in] avg Spezifiziert fuer den gewaehlten Kanal die die Anzahl der Wandlungen, die zu
		       einem geglaetteten Wert gemittelt werden sollen. Maximaler Wert ist 63.
			   Default ist 1 (keine Glaettung).
		\param [in] conv Funktionszeiger auf Umwandlungsfunktion von ADC-Wandlungstufen in Zielwerte;
		       Implementierung der sensorspezifischen Kennlinie; Default: Identitaetsfunktion.
	*/
	ADConverter(uint8_t chan, uint8_t avg = 1, cft conv = identity);

    /*! \brief  Liefere den aktuellen Wert des ADC Objektes als Funktionswert.
		\retval uint16-Wert Der zuletzt vom ADC gelieferte (FREE_RUNNING) oder
		        erfragte (POLLING) Wert. Wertebereich ist 0 bis 1023 im Falle der
				Identitaetsfunktion, sonst der Wertebereich der angegebenen
		        spezifischen Konversionsfunktion.
		
	    Hole den neuesten Wert vom ADC Objekt ab. Im Polling Modus wird von
		get_value() immer eine neue
	    Konversion angestossen und das Ende abgewartet. Je nach Wahl des averaging
	    Wertes kann das Warten auf den naechsten Wert einige Zeit dauern! Im Modus
	    FREE_RUNNING kann vor Aufruf von get_value() mittels value_available() 
	    nichtblockierend geprueft werden, ob ein neuer (ggf. gemittelter) Wert 
	    bereits vorliegt. Unmittelbar nach Aufruf von get_value() liefert value_avalaible() false.
		Die Konversionsfunktion wird auf den fertig gemittelten Wert angewendet.
	*/
	uint16_t get_value ();

    /*! \brief Testet auf Verfuegbarkeit eines neuen noch nicht mittels get_value() gelesenen Wertes.
		\retval true  Im Modus FREE_RUNNING, falls eine Mittelung abgeschlossen ist und ein neuer 
		              Wert vorliegt. Im Modus POLLING immer true.
		\retval false Im Modus FREE_RUNNING, falls noch kein neuer Wert vorliegt. Im Modus POLLING
		              kommt false niemals vor.
		
	    Im Modus FREE_RUNNING kann die Verfuegbarkeit eines neuen gewandelten
	    Wertes abgefragt werden. Im Polling Modus liefert die Funktion immer
	    true - ist also unnoetig sie aufzurufen, da get_value() in jedem Fall immer eine neue
	    komplette Konversion vornimmt.
	*/
	bool value_available() {return (FREE_RUNNING == mode) ? new_value : true;};
};

/*! @} */

#endif /* ADCONVERTER_H_ */