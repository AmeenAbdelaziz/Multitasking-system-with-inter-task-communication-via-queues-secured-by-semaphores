#ifndef STEPPERCONTROLLER_H_
#define STEPPERCONTROLLER_H_

/*!
	\version 3.1 
	\file StepperController.h
	\date 11.7.2019
	\author bomarius
*/

/*! \brief Modul zur Anteuerung von Schrittmotoren.

    Ansteuerung von bis zu vier Schrittmotoren mit Kontroller L297 und L298 dual full 
	bridge Treiber oder DM542 Microstep Driver.
	Software-seitige Ansteuerung erfolgt durch Steuerkurven, die aus Schritten 
	(Streckenabschnitten mit jeweils konstanter Geschwindigkeit) zusammengesetzt sind.
	
	Version 3.1:
		- Ansteuerung mehrerer Motoren und mit Interrupt-Sicherheitsabschaltung
		- Steuerkurven von 256 auf 65436 Schritte pro Abschnitt geaendert
		- Umdrehungszaehler eingefuehrt
	
	
	\defgroup group14 Stepper
	@{
*/

#include "Basics.h"
#include "Timer.h"


/*! \cond */
// Definition of Port Bits for L297/298 and DM542 MicroStep Driver 
// Bits INT2BIT thru INT5BIT and HOMBIT are input bits
// Note that DM542 MicroStep Driver only uses CLKBIT and DIRBIT, hence cannot signal a home-position
// and Microstepping is set using micro-switches on the driver board.

#define CLKBIT  0     // [out] clock bit (active low); rising edge advances motor one step
#define DIRBIT  1     // [out] rotation direction bit (1 == CW, 0 == CCW)
#define INT2BIT 2     // [in]  INT2 signal (Port D) (ABORT_A)
#define INT3BIT 3     // [in]  INT3 signal (Port D) (ABORT_B)
#define INT4BIT 4     // [in]  INT4 signal (Port E) (ABORT_A)
#define INT5BIT 5     // [in]  INT5 signal (Port E) (ABORT_B)
#define HOMBIT  6     // [in] L297/298: home position indicator (active low)

// not connected / function not used:
#define MODBIT  5     // [out] L297/298: stepping mode selection: half (=1) / full (=0)
#define RESBIT  7     // [out] L297/298: reset bit (active low); restores controller to its home position
/*! \endcond */

/*! \brief Langsamste Geschwindigkeit. */
#define LOW_SPEED   0

/*! \brief Hoechste Geschwindigkeit. */
#define TOP_SPEED   16 

/*! \brief Drehrichtungen des Motors. */
enum dir_t {
	CCW,			/*!< Drehrichtung gegen den Uhrzeigersinn - die tasaechliche Richtung haengt von der Polung des Motors ab. */
	CW	            /*!< Drehrichtung im Uhrzeigersinn - die tasaechliche Richtung haengt von der Polung des Motors ab. */
};

/*! \brief Ausfuehrungsmodus einer Steuerkurve */
enum curve_exec_mode_t {
	ONCE,   /*!< Alle Segmente der Kurve werden einmal abgefahren und der Motor stoppt dann. */
	REPEAT  /*!< Alle Segmente der Kurve werden endlos wiederholt abgefahren, bis der Motor
	             durch abort() oder safety_abort() gestoppt wird. */
};

/*! \cond */
/* Nur intern genutzt - Maximale Anzahl der StepperController Instanzen */
#define SC_OBJ_CNT_MAX  4
/*! \endcond */

/*! \brief Eine Steuerkurve (StepperCurve) besteht aus mindestens einem StepperCurveSegment
           Objekt, welches Geschwindigkeit und Anzahl der Schritte, die der Motor mit dieser 
		   Geschwindigkeit ausfuehren soll, beinhalt. Im allgemeinen ist die Steuerkurve
		   eine Aneinanderreihung (ein Array) von solchen Segmenten. */
class StepperCurveSegment {
public:
	uint16_t steps;		  //!< Anzahl der Motorschritte
	uint8_t  speed_index; //!< Index in die speed_vector Table der Klasse StepperController
	StepperCurveSegment(uint16_t st, uint8_t si):steps(st),speed_index(si){};
};


/*! \brief Klasse zur Darstellung einer Steuerkurve bestehend aus einem Vektor von
           Abschnitten unterschiedlicher Geschwindigkeiten (StepperCurveSegment Objekte).
		   Alle Abschnitte einer Steuerkurve haben die gleiche Drehrichtung und werden
		   von der Methode run() des StepperController Objekts komplett ausgefuehrt, 
		   sofern kein vorzeitiger Abbruch durch abort() oder safety_abort() passiert.

	\verbatim
	Verwendungsbeispiel
	Erzeugung der Kurve curve_1 bestehend aus 19 Kurvenabschnitten, die rechtsdrehend einmalig
	ausgefuehrt werden soll:
	
	#define CURVE_LEN  19
	StepperCurveSegment curve_segments[CURVE_LEN] =
		{{10,0}, {20,2}, {40,4}, {40,6}, {40,8}, {40,10}, {40,12}, {40,14}, {40,15}, 
		 {1000,16},
		 {40,15}, {40,14}, {40,12}, {40,10}, {40,8}, {40,6}, {40,4}, {20,2}, {10,0}};

	StepperCurve curve_1(curve_segments, CURVE_LEN, CLOCKWISE, ONCE);

	\endverbatim
*/

class StepperCurve {
private:
	StepperCurveSegment* s_v;
	uint8_t  len;		  // number of s_v entries
	uint8_t  act_entry;   // current index to s_v
	uint16_t rem_steps;   // remaining steps in current s_v entry
	dir_t    dir;		  // rotation direction for this curve
	bool     mode;        // endless repeat or one-shot execution of this curve

public:

    /*! \brief Konstruktor zur Erzeugung einer kompletten Steuerkurve.
		\param [in] sv  Zeiger auf einen Vektor von Geschwindigkeitsabschnitten.
		\param [in] len Anzahl der Vektorelemente in sv.
		\param [in] dir Drehrichtung fuer das Abfahren der Kurve (nur CW oder CCW machen hier Sinn).
		\param [in] mode Endlose Wiederholung der Kurve, oder einmaliges Abfahren.
	*/
	StepperCurve(StepperCurveSegment* sv, uint8_t len, dir_t dir=CW, curve_exec_mode_t mode=ONCE);

	/*! \brief Hole den Geschwindigkeitsindex fuer den naechsten Motorschritt.
		\param [out] ind Der Geschwindigkeitsindex fuer den naechsten Schritt
		                 des Motors.
		\retval true   Naechster Schritt ist ausgewaehlt und der zugehoerige
		               Geschwindigkeitsindex ist in ind abgelegt.
		\retval false  Die Kure ist fertig abgefahren; Motor wurde gestoppt;
		               ind wurde nicht veraendert.
	*/	
	bool next_speed_index(volatile uint8_t*ind);
	
	/*! \brief Frage die aktuelle Drehrichtung der Kurve ab.
		\retval  CW
		\retval  CCW
	 */
	dir_t get_dir(){return dir;};
		
	/*! \brief Setze die aktuelle Drehrichtung der Kurve.
		\param [in] dir Die neue Drehrichtung der Kurve (nur CW oder CCW machen hier Sinn).
	 */	
	void set_dir(dir_t dir) {this->dir = dir;};

	/*! \brief Setze den Ausfuehrungsmodus der Kurve (ONCE, REPEAT).
		\param [in] dir Der Modus (einmalig oder endlose Wiederholung).
	 */	
	void set_mode(curve_exec_mode_t mode) {this->mode = mode;};
	
		
	/*! \brief Bereite die Kurve fuer die erste oder eine erneute Abspielung vor. */
	void reset() {act_entry = 0; rem_steps = s_v[0].steps;};

};


/*!  \brief Klasse zur Ansteuerung von Schrittmotoren.

     Der StepperController benoetigt pro Objekt einen DigitalPort, den er selbst
	 konfiguriert und ansteuert (also ohne DigiPort zu verwenden) sowie ein 
	 16 Bit Timer-Objekt, welches er selbst instanziiert. Der ATMega2560 hat 4
	 solcher Timer, folglich koennen maximal 4 StepperController Objekte 
	 gleichzeitig instanziiert werden (Objekte der Subklasse
	 SafeStepperController zaehlen dabeit natuerlich mit).

	 
	 \verbatim
	 Verwendungsbeispiel: Erzeugt den StepperController smot am Port D 
	 und erzeugt zusaetzlich das Timer-Objekt an TC1 zur Steuerung von smot.
	 Der dritte Parameter (hier 200) gibt die Anzahl der Schritte des Motors
	 fuer eine volle Achsenumdrehung an:
	 
	 StepperController smot(PD, TC1, 200);
	 
	 \endverbatim
*/


class StepperController {

/*! \cond */
protected:
    static const uint16_t    speed_vector[];           // Geschwindigkeitentabelle
	static uint8_t           sc_obj_cnt;               // Anzahl der bereits instanziierten Objekte
    static StepperController *sc_obj[SC_OBJ_CNT_MAX];  // Zeiger auf die StepperController Objekte

	Timer16             stepper_timer;  // Timer, um das Clock-Signal des Motors zu erzeugen
	StepperCurveSegment sv;             // Step-Vektor der Laenge 1 fuer die Instanzenvariable sc
	StepperCurve        sc;             // spezielle Kurve fuer step(), step_to(), und calibrate()
	volatile uint8_t*   port;           // PORTx Ausgabe-Register-Adresse des Ports
	volatile bool       running;        // running == true, stopped == false
	volatile dir_t      dir;            // aktuelle Drehrichtung
	const uint16_t      steps_per_rev;  // Anzahl Schritte pro Umdrehung der Motorachse
	
	StepperCurve*       curve;          // aktuelle Steuerkurve
	volatile uint8_t    speed_index;    // aktueller Geschwindigkeitsindex			
	volatile uint16_t   pos;            // aktuelle Position der Motorachse (0 .. steps_per_rev-1) zaehlt in
	                                    // CW Richtung hoch; der Wert 0 ist die zufaellige Position beim Einschalten
	volatile int16_t    revs;		    // Anzahl der vollen Umdrehungen der Motorachse; der Wert 0 ist die
									    // zufaellige Position beim Einschalten, wird in CW hochgezaehlt
	
	uint8_t            cntrl;           // Kontrollwort fuer den naechsten Schritt
	
	// Send an active low signal for at least 0.5µs in order to have the
	// stepper controller read the current control word from the stepper port
	void enable();
	
	// Berechne die naechste Motorposition
	void advance_pos();

    // Setze die Drehrichtung	
    void set_direction(dir_t dir);
	
	// Die statischen call-back Routinen zum Weiterschalten der Motoren. Muessen vom Konstruktor 
	// beim zugehoerigen Timer registriert werden. 
	// Bilden den call auf die Instanz und deren notify() Methode ab.
	static void notify_0() {if (NULL != sc_obj[0]) sc_obj[0]->notify();};
	static void notify_1() {if (NULL != sc_obj[1]) sc_obj[1]->notify();};
	static void notify_2() {if (NULL != sc_obj[2]) sc_obj[2]->notify();};
	static void notify_3() {if (NULL != sc_obj[3]) sc_obj[3]->notify();};

	// Die Instanzen-Methode zum Weiterschalten des Motors. Hierin wird die aktuelle Kurve abgearbeitet.
	void notify();

/*! \endcond */
	
public:

	/*!  \brief Erzeuge das Stepper Controller Objekt.
		 \param [in] p_handle Eine Port-Bezeichnung gemaess Definition in Basics.h.
		 \param [in] t_handle  Ein handle eines Timer16 Objekts, welches exklusiv zur Motorsteuerung genutzt werden soll.
		 \param [in] spr Die Anzahl der Motorschritte pro Umdrehung der Motorachse; default ist 200
		 
		 Es wird ein Timer Objekt und ein StepperController Objekt erzeugt und mit der 
		 langsamsten Geschwindigkeitseinstellung, Drehrichtung CLOCKWISE und als gestoppt konfiguriert.
		 Achsenposition und Umdrehungszaehler werden auf null gesetzt.
	*/
	StepperController (uint8_t p_handle, uint8_t t_handle, uint16_t spr);
	
	///*!  \brief Suche die Null-Position und kalibriere internen Schrittzaehler. Achtung: nur bei
				//L297/298 Controller und bei angeschlossenem Null-Durchgangssensor anwendbar.
			//\retval true  Die Null-Position wurde gefunden.
			//\retval false Die Null-Position konnte nicht ermittelt werden; das StepperController Objekt hat eine
						//zufaellige Null-Position, die nicht mit der Home-Position im L297/298 Controller uebereinstimmt.
	//
			//Der Schrittmotor wird solange gedreht, bis die Lichtschranke die Home-Position
			//meldet. Falls die Meldung nach einer vollen Umdrehungen nicht erfolgt ist,
			//wechselt die Drehrichtung und der Motor laeuft eine weitere volle Umdrehung.
			//Sollte die Kalibrierung dann immer noch keine Home Position gefunden haben, meldet sie
			//false. Dann muss die Lichtschranke ueberprueft und ggf. neu justiert werden.
			//Wird bei Aufruf von auto_calibrate() eine Steuerkurve abgefahren,so wird diese gestoppt.
	//*/
	//bool auto_calibrate();
	
	
	/*!  \brief Stoppe den Motor und loesche die aktuelle Steuerkurve. */
	void abort ();

	/*!  \brief Bewege den Motor um die angegebene Zahl Schritte. Falls gerade eine Steuerkurve 
	            abgefahren wird, so wird diese zuvor gestoppt.
		
		 \param [in] steps Die Anzahl der Schritte.
		 \param [in] dir Die gewuenschte Drehrichtung (CW oder CCW).	
	*/
	void step(uint16_t steps=1, dir_t dir=CW);
	
	/*!  \brief Laufe mit mittlerer Geschwindigkeit zur Schrittposition pos in
	            der angegebenen Drehrichtung. Falls gerade eine Steuerkurve
				abgefahren wird, so wird diese gestoppt.
	     \param [in] pos Die Zielposition.
		 \param [in] dir Die Drehrichtung.
		 
		 Die Zielposition ist relativ zur Home-Position zu zaehlen. Beim Einschalten
		 wird die aktuelle Position als Home angenommen. Erst durch ein erfolgreiches
		 calibrate() ist die Home Position mit dem Lichtschrankensignal synchronisiert.
	*/	
	void step_to (uint16_t pos=0, dir_t dir=CW);
	
	
	/*!  \brief Frage die zur Zeit eingestellte Geschwindigkeitsstufe ab. */
	uint8_t get_speed() {return speed_index;};
		
	/*!  \brief Frage die zur Zeit eingestellte Drehrichtung ab. */		
	dir_t get_direction() {return dir;}	

	/*!  \brief Frage die aktuelle Achsenstellung ab. */
	uint16_t get_pos() {return pos;}

	/*!  \brief Frage die aktuelle Umdrehungsanzahl ab. */
	int16_t get_revs() {return revs;}


    /*!  \brief Fahre die Steuerkurve ab.
	     \param [in] curve Zeiger auf ein StepperCurve Objekt, das die Steuerkurve enthaelt.
	 */
	void run(StepperCurve* curve);
	
	 /*!  \brief Pruefe ohne zu warten, ob der Motor zur Zeit eine Fahrkurve abarbeitet.
	 */
	bool is_running(){return running;};

	/*!  \brief  Warte blockierend bis der Motor seine aktuelle Kurve beendet hat.
	*/
	void wait(){while(true==running);}

};

/*! \brief Auswahl-Optionen fuer die Not-Stop Interrupts.
*/
enum interrupt_control_t {
	any_edge,      /*!< Jede Flanke loest einen IRQ aus. */
	rising_edge,   /*!< Die steigende Flanke loest IRQ aus.*/
	falling_edge,  /*!< Die fallende Flanke loest IRQ aus. */
	low_level      /*!< Low Level loest staendig IRQ aus. Sollte nicht genutzt werden, da im 
	                    Falle des dauerhaften Not-Stopps beim Initialisieren das System durch 
						Dauer-IRQ haengt. */
};

/*! \brief Arten des Not-Stopps.
*/
enum safety_status_t {
	not_aborted,         /*!< Es liegt kein Not-Stop an. */
	a_aborted,           /*!< Not-Stop A ist ausgeloest. */
	b_aborted            /*!< Not-Stop B ist ausgeloest. */
};

/*! \brief Unterscheidung der Not-Stop Schalter. !!! PORT E muss noch gedreht werden !!!!!!!*/
enum abort_switch_t {
	ABORT_A_SWITCH, /*!< Not-Stop Schalter unten (INT2 bzw. INT5 an Port D). */
	ABORT_B_SWITCH  /*!< Not-Stop Schalter oben  (INT3 bzw. INT4 an Port E). */
	};


/*!  \brief Klasse zur Ansteuerung von Schrittmotoren mit Interrupt-gesteuerter
            Sicherheitsabschaltung. Zur Anwendung kommen die externen Interrupts
			INT2 - INT5. Daher sind nur die Ports D und E verwendbar. Pinbelegung:
			PD2 und PD3 sind INT2 und INT3; PE4 und PE5 sind INT4 und INT5.
*/

class SafeStepperController : public StepperController {
private:
	safety_status_t sas;

public:
	/*!  \brief Erzeuge das Stepper Controller Objekt.
		 \param [in] p_handle Eine Port-Bezeichnung gemaess Definition in Basics.h. Erlaubt sind PD und PE.
		 \param [in] t_handle  Ein handle eines Timer16 Objekts, welches exklusiv zur Motorsteuerung genutzt werden soll.
		 \param [in] spr Die Anzahl der Motorschritte pro Umdrehung der Motorachse; default ist 200
		 \param [in] ict Bestimmt, welche Flanke des Interruptsignals genutzt werden soll; deafult ist FALLING_EDGE.
		 
		 Es wird ein Timer Objekt und ein SafeStepperController Objekt erzeugt und mit der 
		 langsamsten Geschwindigkeitseinstellung, Drehrichtung CW und als gestoppt konfiguriert.
         Es duerfen nur die Ports PD oder PE verwendet werden, da diese die notwendigen 
		 Interrupts (INT2 bis INT5) fuer die Sicherheits-/Endabschaltung generieren.		 
	*/
	SafeStepperController (uint8_t p_handle, uint8_t t_handle, uint16_t spr, interrupt_control_t ict);


	/*! \cond
	    \brief Die ISR zum sofortigen Anhalten des Motors. Die aktuelle Drehrichtung wird verriegelt.
		\param [in] swt Kennung des ausloesenden Not-Stop Schalters.
		\endcond
	*/	
	void safety_abort(abort_switch_t swt) {
		// In dieser Methode muss Entprellung des Not-Stop Schalters implementiert werden. Dies geschieht durch
		// den Parameter swt der von der eigentlichen ISR gesetzt wird. Solange wir nicht versuchen in Richtung
		// des durch swt angegebenen Not-Stopps weiter zu fahren ignorieren wir das Not-Stop Signal, denn es 
		// ist im Grunde nur Prellen. Dies ermöglicht insbesondere das Herausfahren aus einer Not-Stop Situation.
		
		// Die Fahrtrichtung CCW wird durch ABORT_A_SWITCH gestoppt.
		// Die Fahrtrichtung CW  wird durch ABORT_B_SWITCH gestoppt.
		
		if ((CW == dir) && (ABORT_B_SWITCH == swt)) {
			this->abort();
			sas = b_aborted;
		} else if ((CCW == dir) && (ABORT_A_SWITCH == swt)){
			this->abort();
			sas = a_aborted;
		}
		// else: wir ignorieren den Interrupt
	};	

	/*!  \brief Fahre die Steuerkurve ab. Falls die Kurve in eine gesperrte Drehrichtung
	            zeigt, tue nichts.
	     \param [in] curve Zeiger auf ein StepperCurve Objekt, das die Steuerkurve enthaelt.
	 */
	bool run(StepperCurve* curve);
	
	/*!  \brief Fahre endlos in die angegebene Richtung bis ein abort() oder safety_abort() passiert.
		 \detail Setzt voraus, dass andere Programmteile (nebenlaeufig oder per Interrupt) den Lauf
		         tatsaechlich mittels abort() oder safety_abort() stoppen. Wenn eine gesperrte
				 Laufrichtung angefordert wird, passiert beim Aufruf run() nichts.
	*/
	bool run_endless(dir_t dir);

	
	/*!	\brief	Teste  die Not-Stop Schalter Position und aktualisiere ggf. den abort Status.
	            <B>Wichtig: diese Routine muss vor jedem Fahrbefehl aufgerufen werden, um nicht
				wider einen aktuell bestehenden Not-Stop zu fahren.<b>
				
		\details Teste, ob aktuell ein statischer Not-Stop vorliegt und verrigele die entsprechende
		         Fahrtrichtung. Dies passiert z.B., wenn beim Einschalten der Motor bereits am 
				 Not-Stop steht oder ein Not-Stop Schalter von Hand (dauerhaft) betätigt wird,
				 und nun eine Fahrt gestartet werden soll.
				
		\retval a_aborted     Eine Fahrt in Richtung \a CW darf nicht gestartet werden.
		\retval b_aborted     Eine Fahrt in Richtung \a CCW darf nicht gestartet werden.
		\retval not_aborted   Eine Fahrt in beliebige Richtung darf gestartet werden.
	*/	
	safety_status_t abort_status ();
};

/*! @}  Ende group14 Stepper */

#endif /* STEPPERCONTROLLER_H_ */