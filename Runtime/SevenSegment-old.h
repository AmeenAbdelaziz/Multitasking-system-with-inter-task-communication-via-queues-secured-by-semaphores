#ifndef SEVENSEGMENT_H_
#define SEVENSEGMENT_H_
/*! 
  	\version   1.0
	\date      9.2.2018
	\author    Bomarius
	\copyright Bomarius
	\file      SevenSegment.h
*/

/*! \brief Modul zur Ansteuerung von Sieben-Segment Anzeigen

	\defgroup group13 SevenSegment
	@{
*/

#include "Basics.h"
#include "DigiPort.h"

//=================================================================================
//==============================  SevenSegment  ===================================
//=================================================================================

/*! \cond */
// nur zur internen Nutzung:
#define BLANK8  (uint8_t) (0xFF)
/*! \endcond */

/*! \brief Die Klasse SevenSegment implementiert die Ansteuerung von 7-Segment-Anzeigen.

     Die Ansteuerung kann eine Zahl mit bis zu 4 Ziffern (0 .. 9999), die auf bis zu
	 zwei 7-Segment Karten mit je zwei Ziffern pro Karte abbilden. Dazu werden 1 oder 2 
	 Objekte vom Typ DigiPortRaw verwendet, die zuvor als Ausgabeports, ACTIVE_HIGH
	 erzeugt werden muessen. 
 */

class SevenSegment {

/*! \cond */
private:
	uint8_t      num_of_pos;   // Anzahl Ziffernpositionen
	uint16_t     last_value;   // zuletzt angezeigte Zahl
	DigiPortRaw* portl;       // Anzeige von 2 Ziffern (niederwertig)
	DigiPortRaw* porth;       // optionale Anzeige zusätzlicher 2 Ziffern (höherwertig)
/*! \endcond */
	
public:

	/*! \brief Erzeuge ein SiebenSegment Objekt, dass eine Anzeige von bis zu vier
	           Ziffern ansteuern kann.
		\param [in] pl Die Adresse eines zuvor erzeugten DigiPortRaw Objektes im Modus
		          active high. Ueber dieses Objekt werden die Einer- und, sofern 
				  verwendet, die Zehner-Stelle angesteuert.
		\param [in] ph Die Adresse eines zuvor erzeugten DigiPortRaw Objektes im Modus
		          active high. Ueber dieses Objekt werden die Hunderter- und, sofern
		          verwendet, die Tausender-Stelle angesteuert. Optional.
		\param [in] pos Die tatsaechliche Anzahl auszugebender Stellen. Bei 1 oder 2 
		           Stellen genuegt das Objekt pl. Bei 3 oder 4 Stellen muss das Objekt
				   ph angegeben sein.
	 */
	SevenSegment(DigiPortRaw* pl, DigiPortRaw* ph=NULL, uint8_t pos=2);

    /*! \brief 	Gib den Wert value auf der SiebenSegment-Anzeige aus.

	    Wenn value nicht in die konfigurierte Stellenzahl passt, werden nur die
		niederwertigen Stellen angezeigt. */
	void write (uint16_t value);

    /*! \brief Loesche die SiebenSegment-Anzeige. */	
	void blank (void) {
		portl->write(BLANK8);
		if (porth != NULL)
		   porth->write(BLANK8);
	};
	
	/*! \brief Gib den letzten mit write() geschriebenen Wert zurueck. */
	inline uint16_t read () {return last_value;};
};



///* ===========================================================================
 //* Die Klasse SevenSegmentShifter implementiert die Ansteuerung von 6-stelligen
 //* 7-Segment-LCD-Anzeigen mittels serieller Ansteuerung über einen Port. 
 //*/
//
//
//#define DP_1		(0b01000000)
//#define DP_2		(0b00100000)
//#define DP_3		(0b00010000)
//#define DP_4		(0b00001000)
//#define DP_5		(0b00000100)
//#define COL_1		(0b00000010)
//#define COL_2		(0b00000001)
//#define ALL_BULLETS (0b01111111)
//
//class SevenSegmentShifter {
//private:
	//volatile uint8_t* const base;   // base address of port
	//uint8_t digits[6];				// the six digits, counted from left to right
	//uint8_t bullets;                // bit-map of the decimal points and colons
	//
//public:
	//SevenSegmentShifter(uint8_t p_handle);
	//
	//// blank out the display and delete digits stored
	//void blank ();
	//
	//// show previously loaded digits on display
	//void show_digits();
	//
	//// load a single digit, but do not display yet
	//void set_digit (uint8_t d, uint8_t index);
//
    //// load a pair of digits, but do not display yet
	//void set_digit_pair (uint8_t d, uint8_t index);
	//
	//// switch decimal points and colons on. data is a bit vector built using 
	//// DP_1 to COL_2 constants. Do not display yet.
	//void bullets_on (uint8_t data);
	//
	//// switch decimal points and colons off. data is a bit vector built using 
	//// DP_1 to COL_2 constants. Do not display yet.
	//void bullets_off (uint8_t data);
	//
	//// toggle decimal points and colons. data is a bit vector built using
	//// DP_1 to COL_2 constants. Do not display yet.
	//void bullets_toggle (uint8_t data);
	//
	//// display the current setting of decimal points and colons on display
	//void show_bullets ();
//};
//
//


/*! @}  Ende group13 SevenSegment */

#endif /* SEVENSEGMENT_H_ */