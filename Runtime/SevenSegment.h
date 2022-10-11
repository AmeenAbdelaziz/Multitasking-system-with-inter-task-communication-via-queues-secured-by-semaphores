#ifndef SEVENSEGMENT_H_
#define SEVENSEGMENT_H_
/*! 
  	\version   1.0
	\date      9.9.2021
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

/*! \brief Die Klasse SevenSegment implementiert die Ansteuerung von 4 stelligen 7-Segment-Anzeigen.

     Die Ansteuerung kann eine vorzeichenlose Zahl mit bis zu 4 Ziffern (also den Wertebereich 
	 0 .. 9999), auf einem 4-stelligen 7-Segment Display abbilden. Dazu werden 2 
	 Objekte vom Typ DigiPortRaw verwendet, die zuvor als Ausgabeports, ACTIVE_HIGH
	 erzeugt werden muessen. 
 */

class SevenSegment {

/*! \cond */
private:
	uint8_t      num_of_pos;   // Anzahl Ziffernpositionen
	DigiPortRaw* portl;        // Anzeige von 2 Ziffern (einer und zehner)
	DigiPortRaw* porth;        // Anzeige von 2 Ziffern (hunderter und tausender)
/*! \endcond */
	
public:

	/*! \brief Erzeuge ein SiebenSegment Objekt, dass eine Anzeige mit vier
	           Ziffern ansteuern kann.
		\param [in] pl Port an dem die beiden niederwertige 
		          Stellen (Einer- und Zehner-Stelle) angezeigt werden.
		\param [in] ph Port an dem die beidenhöherwertigen
                  Stellen (Hunderter- und Tausender-Stelle) angezeigt werden.
		\param [in] pos Die tatsaechliche Anzahl auszugebender Stellen, ggf.
		            mit fuehrenden Nullen. Wenn keine fuehrenden Nullen gewuenscht
					sind den Parameter weglassen oder auf 1 setzen.
	 */
	SevenSegment(DigiPortRaw* pl, DigiPortRaw* ph, uint8_t pos=1);

    /*! \brief 	Gib den Wert value auf der SiebenSegment-Anzeige aus.

	    Wenn value nicht in die Stellenzahl passt, werden nur die
		vier niederwertigen Stellen angezeigt. Je nach Konfiguraton
		beim Erzeugen des Objekts mit fuehrenden Nullen fuellen */
	void write (uint16_t value);

    /*! \brief Loesche die SiebenSegment-Anzeige. */	
	void blank (void) {
		portl->write(BLANK8);
		porth->write(BLANK8);
	};
};


/*! @}  Ende group13 SevenSegment */

#endif /* SEVENSEGMENT_H_ */