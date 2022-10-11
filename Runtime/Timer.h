//=================================================================================
//=================================  TIMER  =======================================
//=================================================================================

/*! \brief Das Modul Timer stellt C++ Klassen fuer die Kapselung von 8 und 16 Bit
           Timern zur Verfuegung. 
		  	
	\defgroup group2 Timer	   
	
	\verbatim	   
    Das Modul Timer hat folgende Klassenhierarchie:
 
              Timer (abstrakt)
              /   \      
         Timer16  Timer8
                      | 
                      OSTimer (interne Nutzung durch Kernel Modul (TC0))
			                			  
    \endverbatim
   Timer8 und Timer16 haben (bis auf Wertebereiche) gleiche Funktion:
   sie koennen als einmal, mehrfach wiederholender oder endlos wiederholender
   Timer konfiguriert werden, der bei jedem Ablauf eine callback-Funktion(en)
   der Applikation aufruft - diese werden dem Konstruktor uebergeben.
   Die callback-Funktionen werden im Kontext einer ISR gerufen, muessen also so
   kurz als moeglich sein, da waehrend ihrer Abarbeitung Interrupts ausgeschaltet sind!
   Die notwendigen ISRs werden automatisch registiert und mit den
   Timer-Objekten verbunden. 
   Die Klasse OSTimer ist speziell fuer die Verwendung durch den OSKernel 
   vorgesehen und verwendet den Overflow IRPT. Der OSKernel instanziiert 
   den TC0 als OSTimer Objekt. Daher darf TC0 nicht von der Applikation 
   verwendet werden, wenn gleichzeitig das Modul OSKernel verwendet wird !!
   Die Konstruktoren werden jeweils unter Angabe des zu verbindenden Hardware
   Timers aufgerufen: bei Timer8 bzw OSTimer sind das TC0 oder TC2 (8 Bit)
   bei Timer16 sind das TC1, TC3, TC4 und TC5 (16 Bit).
   
   Bei Timerarten koennen mit Aufloesung in Mikro- oder Milli-Sekundenbereich
   gestartet werden (\c start_us() bzw. \c start_ms()). Es ergeben sich folgende Zeitbereiche:
   \verbatim	
        Timer8    start_us()  --> 1 ..   127 Mikrosekunden
                  start_ms()  --> 1 ..    17 Millisekunden
        Timer16   start_us()  --> 1 .. 32768 Mikrosekunden = 32,7 Millisekunden
                  start_ms()  --> 1 ..  4369 Millisekunden = 4,37 Sekunden
    \endverbatim	
   
 	\version   3.0
 	\date      9.9.2021
 	\author    Bomarius
 	\copyright Bomarius  
    \file Timer.h 
	
	@{
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <stddef.h>
#include "Basics.h"

#include "LCD.h"

// Typdefinition fuer die Callback Funktion der Applikation, welche bei
// Eintritt des Timerereignisses vom Timer-Objekt aufzurufen ist.
/*! \brief Typ der call-back Funktionen, die bei den Timer-Events gerufen werden 
           sollen. Siehe Konstruktoren der Timer Klassen. */
typedef void (*CBF_t) (void);

// Offsets fuer die Timer Register relativ zum A Kontrollregister
/*! \cond */
#define TCCRA  0	// valid for 8 and 16 Bit
#define TCCRB  1    // valid for 8 and 16 Bit
/*! \endcond */

//=======================================================================
/*! \brief  Die Klasse Timer ist die abstrakte Basisklasse fuer alle 8 und 16 Bit
            Timer. 
 */
class Timer {

protected:
    volatile uint8_t* const tccra;  // Adressen der jeweiligen.. 
	volatile uint8_t* const tccrb;  // ..Timer Kontroll-Register
    uint8_t  repeats;  // Anzahl der (verbleibenden) Compare-A Ereignisse
	bool     running;  // Status des Timers
	CBF_t    cbf_A;    // Callback Prozedur der Applikation fuer Comapre A
	CBF_t    cbf_B;    // Callback Prozedur der Applikation fuer Compare B
	CBF_t    cbf_C;    // Callback Prozedur der Applikation fuer Compare C (nur 16 Bit Timer)
	CBF_t    cbf_O;    // Callback Prozedur der Applikation fuer Overflow

public:
	/*! \brief Konstruktor zur Erzeugung eines Objekts fuer den durch das Handle spezifizierten Timer.
		\param [in] tc_handle Muss als aktuellen Parameter einen der Werte TC0 bis TC5 erhalten.
	*/
    Timer (uint8_t tc_handle) : 
	    tccra(tc_handle_to_address(tc_handle)), 
		tccrb(tc_vect[tc_handle] + TCCRB) {};
	
	/*! \brief Stoppe den Timer. */	
    void stop ();

/*! \cond */		
	/*! \brief  Starte den Timer fuer eine neue Runde.
		\detail Diese Methode ist pure virtual und muss von der abgeleiteten Klasse
		        implementiert werden. Siehe dort.
	 */
	virtual void reset () = 0;
	
	/*! \brief  Call-back fuer den Capture A Event.
		\detail Diese Methode ist pure virtual und muss von der abgeleiteten Klasse
		        implementiert werden. Siehe dort.
	 */
    virtual void notify_A () = 0;
	
	/*! \brief  Call-back fuer den Capture B Event.
		\detail Diese Methode ist pure virtual und muss von der abgeleiteten Klasse
		        implementiert werden. Siehe dort.
	 */
	virtual void notify_B () = 0;
	
	/*! \brief  Call-back fuer den Capture C Event.
		\detail Diese Methode ist pure virtual und muss von der abgeleiteten Klasse
		        implementiert werden. Siehe dort.
	 */
	virtual void notify_C () = 0;
	
	/*! \brief  Call-back fuer den Overflow Event.
		\detail Diese Methode ist pure virtual und muss von der abgeleiteten Klasse
		        implementiert werden. Siehe dort.
	 */
	virtual void notify_O () = 0;
	
/*! \endcond */
	
	/*! \brief Teste, ob Timer aktuell laeuft oder gestoppt ist. */
	inline bool is_running() {return running;};
};

//====================================================================

/*! \brief Die Klasse Timer8 abstrahiert die 8-Bit Timer des ATMega.
*/

class Timer8 : public Timer {
protected:
	uint8_t tcnt_init;   // tcnt Initialwert
public:
	/*! \brief Erzeuge ein 8-Bit Timer Objekt. 
	   \param [in] tc_handle  Stellt den Bezug zum Hardware Timer mittels Timer Handle 
	                     her. Erlaubte Werte sind TC0 und TC2.
	   \param [in] cbA  Zeiger auf die parameterlose globale callback-Funktion,
	               die bei Eintritt des Capture A Events gerufen werden
				   soll. Notwendiger Parameter.
		\param [in] cbB Zeiger auf die parameterlose globale callback-Funktion,
		           die bei Eintritt des Capture B Events gerufen werden
		           soll. Optionaler Parameter. Default ist Null und bedeutet,
				   dieser Event wird nicht verwendet.		  
	   	\param [in] cb0 Zeiger auf die parameterlose globale callback-Funktion,
	   	           die bei Eintritt des Overflow Events gerufen werden
	   	           soll. Optionaler Parameter. Default ist Null und bedeutet,
	   	           dieser Event wird nicht verwendet.
	*/
	Timer8(uint8_t tc_handle, CBF_t cbA, CBF_t cbB = NULL, CBF_t cbO = NULL);


    /*! \brief (obsolet - nutze start_ms) Starte den Timer  in Aufloesung Millisekunden
	           fuer rpt Runden bezogen auf den hoechsten konfigurierten
	           event (A oder B);  bei rpt=255 endlos, mindestens aber 
			   einmal (auch bei rpt = 0).
			   
		Diese Methode ist nur fuer Abwaertskompatibilitaet angegeben und
		eigentlich obsolet Es sollte start_ms() genutzt werden.
		
		Fuer 8 Bit Timer muessen millisecs Werte kleiner-gleich 17 sein!!
		Die Werte fuer A und B capture muessen aufsteigend sein und werden nur verwendet,
		wenn bei der Instanziierung entsprechende CBFs registriert wurden.  Der Wert
		fuer millisecs_CNT dient zum Setzen eines Startwerts fuer den Counter.
	 */
    inline void start(unsigned int millisecs_A, unsigned int millisecs_B=0,
                      unsigned int millisecs_CNT=0, uint8_t rpt=255) {
        start_ms(millisecs_A,millisecs_B,millisecs_CNT,rpt);
	}
	
    /*! \brief Starte den Timer in Aufloesung Millisekunden fuer rpt Runden bezogen 
	           auf den hoechsten konfigurierten event (A oder B);  bei rpt=255 endlos, 
			   mindestens aber einmal (auch bei rpt = 0).
			   
		Fuer 8 Bit Timer muessen millisecs Werte kleiner-gleich 17 sein!!
		Die Werte fuer A und B capture muessen aufsteigend sein und werden nur verwendet,
		wenn bei der Instanziierung entsprechende CBFs registriert wurden.  Der Wert
		fuer millisecs_CNT dient zum Setzen eines Startwerts fuer den Counter.
	 */
	void start_ms(unsigned int millisecs_A, unsigned int millisecs_B=0, 
	              unsigned int millisecs_CNT=0, uint8_t rpt=255);

    /*! \brief Starte den Timer in Aufloesung Mikrosekunden fuer rpt Runden bezogen 
	           auf den hoechsten konfigurierten event (A oder B);  bei rpt=255 endlos, 
			   mindestens aber einmal (auch bei rpt = 0).
			   
		Fuer 8 Bit Timer muessen microsecs Werte kleiner-gleich 127 sein!!
		Die Werte fuer A und B capture muessen aufsteigend sein und werden nur verwendet,
		wenn bei der Instanziierung entsprechende CBFs registriert wurden.  Der Wert
		fuer microsecs_CNT dient zum Setzen eines Startwerts fuer den Counter.
	 */
	void start_us(unsigned int microsecs_A, unsigned int microsecs_B=0, 
	              unsigned int microsecs_CNT=0, uint8_t rpt=255);



    /*! \cond  */
	// Wird bei Compare-A IRPT gerufen, um dem Timer Objekt den Eintritt des
    // Ereignisses mitzuteilen. Sie ruft ihrerseits die CBF_A der Appliaktion.
    inline void notify_A () {
	    if (cbf_A != NULL) cbf_A();
        if (cbf_B == NULL) this->reset();};

	// Wird bei Compare-B IRPT gerufen, um dem Timer Objekt den Eintritt des
	// Ereignisses mitzuteilen. Sie ruft ihrerseits die CBF_B der Appliaktion.
	inline void notify_B () {
		if (cbf_B != NULL) cbf_B();
	    if (cbf_O == NULL) this->reset();};

    inline void notify_C () {}; // Dummy, nie benutzt bei 8 Bit Timern
		
	// Wird bei Overflow IRPT gerufen, um dem Timer Objekt den Eintritt des
    // Ereignisses mitzuteilen. Sie ruft ihrerseits die CBF_O der Appliaktion
    inline void notify_O () {
	    if (cbf_O != NULL) cbf_O();
        this->reset();};		
	   
	// Starte den Timer fuer eine neue Runde.
	inline void reset () {
		if (repeats > 1){
			if (repeats != 255) repeats--;
			if (tccra == tc_vect[0]) TCNT0 = tcnt_init;
			else /* TC2 */           TCNT2 = tcnt_init;
		}
		else
		    this->stop();
	};
		
	// gib den momentanen Zaehlerstand zurueck.
	inline uint8_t counter_value() {return (tccra==tc_vect[0]) ? TCNT0 : TCNT2;}
	
	/*! \endcond */	
 };

//====================================================================

/*! \cond */
class OSTimer : public Timer8 {
private:
	
public:
	// Erzeuge spezielles 8-Bit Timer Objekt fuer den OSKernel.
	// Erlaubte handle sind sind TC0 oder TC2.
	OSTimer (uint8_t tc_handle); 

    // Starte den Timer endlos. Mit fester time-slice kleiner-gleich 17 ms.
	void start (unsigned int millisecs);
	
	// Loese das Timer Ereignis zum fruehest moeglichen Zeitpunkt aus und
	// gib die abgelaufenen ticks seit letztem Start zurück.
	uint8_t force_timeout ();
		
	// Starte den Timer neu (mache Änderungen in der Timer Konfiguration,
	// die durch force-timeout() gemacht werden, wieder rueckgaengig
	inline void reset() {
		CRITICAL_SECTION {
			if (tccra == tc_vect[0]) { // TC0
				*tccrb |= (volatile uint8_t) ((1 << CS00) | (1 << CS02) );
				TCNT0 = tcnt_init;
			}
			else {// TC2
				// der Prescaler fuer TC2 hat andere codes als die anderen Timer
				*tccrb |= (volatile uint8_t) ((1 << CS00) | (1 << CS01) | (1 << CS02) );
				TCNT2 = tcnt_init;
			}
		}
	}
};		
/*! \endcond */

//====================================================================

/*! \brief Die Klasse Timer16 abstrahiert die 16-Bit Timer des ATMega.
*/
class Timer16 : public Timer {
private:
	unsigned int tcnt_init;   // tcnt Startwert
public:
	/*! \brief Erzeuge ein 16-Bit Timer Objekt. 
	   \param [in] tc_handle  Stellt den Bezug zum Hardware Timer mittels Timer Handle her.
	                     Erlaubte Werte sind TC1, TC3, TC4 oder TC5.
	   \param [in] cbA  Zeiger auf die parameterlose globale callback-Funktion,
	               die bei Eintritt des Capture A Events gerufen werden
				   soll. Notwendiger Parameter.
		\param [in] cbB Zeiger auf die parameterlose globale callback-Funktion,
		           die bei Eintritt des Capture B Events gerufen werden
		           soll. Optionaler Parameter. Default ist Null und bedeutet,
				   dieser Event wird nicht verwendet.
		\param [in] cbC Zeiger auf die parameterlose globale callback-Funktion,
		           die bei Eintritt des Capture C Events gerufen werden
		           soll. Optionaler Parameter. Default ist Null und bedeutet,
		           dieser Event wird nicht verwendet.		  
	   	\param [in] cb0 Zeiger auf die parameterlose globale callback-Funktion,
	   	           die bei Eintritt des Overflow Events gerufen werden
	   	           soll. Optionaler Parameter. Default ist Null und bedeutet,
	   	           dieser Event wird nicht verwendet.
	*/

	Timer16(uint8_t tc_handle, CBF_t cbA, CBF_t cbB = NULL, 
	        CBF_t cbC = NULL, CBF_t cbO = NULL);


    /*! \brief (obsolet - nutze start_ms) Starte den Timer in Aufloesung Millisekunden 
	           fuer rpt Runden bezogen 
	           auf den hoechsten konfigurierten Event (A, B oder C); bei rpt=255 endlos, 
			   mindestens aber einmal (auch bei rpt = 0).

         Diese Methode ist nur fuer Abwaertskompatibilitaet angegeben und
		 eigentlich obsolet Es sollte start_ms() genutzt werden.
		   
	     Die Werte fuer die millisecs muessen kleiner-gleich 4369 (~4,37 sec) sein.
	     Die Werte fuer A, B und C capture muessen aufsteigend sein und werden nur verwendet,
	     wenn bei der Instanziierung entsprechende CBFs registriert wurden.  Der Wert
         fuer millisecs_CNT dient zum Setzen eines Startwerts fuer den Counter.
		 */
	inline void start(unsigned int millisecs_A, unsigned int millisecs_B=0, unsigned int millisecs_C=0,
	                  unsigned int millisecs_CNT=0, uint8_t rpt=255) {
		start_ms(millisecs_A, millisecs_B, millisecs_C, millisecs_CNT, rpt);		   
	}

    /*! \brief Starte den Timer in Aufloesung Millisekunden fuer rpt Runden bezogen
	           auf den hoechsten konfigurierten event; bei rpt=255 endlos, mindestens
			   aber einmal (auch bei rpt = 0).

 	     Die Werte fuer die millisecs muessen kleiner-gleich 4369 (~4,37 sec) sein.
	     Die Werte fuer A, B und C capture muessen aufsteigend sein und werden nur verwendet,
	     wenn bei der Instanziierung entsprechende CBFs registriert wurden.  Der Wert
         fuer millisecs_CNT dient zum Setzen eines Startwerts fuer den Counter.
		 */
	void start_ms(unsigned int millisecs_A, unsigned int millisecs_B=0, unsigned int millisecs_C=0,
	              unsigned int millisecs_CNT=0, uint8_t rpt=255);

    /*! \brief Starte den Timer in Aufloesung Mikrosekunden fuer rpt Runden bezogen 
	           auf den hoechsten konfigurierten event (A, B oder C); bei rpt=255 endlos, 
			   mindestens aber einmal (auch bei rpt = 0).

 	     Die Werte fuer die microsecs muessen kleiner-gleich 32768 (32768 µs ~32,7ms) sein.
	     Die Werte fuer A, B und C capture muessen aufsteigend sein und werden nur verwendet,
	     wenn bei der Instanziierung entsprechende CBFs registriert wurden.  Der Wert
         fuer microsecs_CNT dient zum Setzen eines Startwerts fuer den Counter.
		 */
	void start_us(unsigned int microsecs_A, unsigned int microsecs_B=0, unsigned int microsecs_C=0,
	              unsigned int microsecs_CNT=0, uint8_t rpt=255);

   /*! \cond */
   // Wird bei Compare-A IRPT gerufen, um dem Timer Objekt den Eintritt des
   // Ereignisses mitzuteilen. Sie ruft ihrerseits die CBF_A der Appliaktion
   inline void notify_A () {
	   if (cbf_A != NULL) cbf_A();
       if (cbf_B == NULL) this->reset();};
   
	// Wird bei Compare-B IRPT gerufen, um dem Timer Objekt den Eintritt des
	// Ereignisses mitzuteilen. Sie ruft ihrerseits die CBF_B der Appliaktion
	inline void notify_B () {
		if (cbf_B != NULL) cbf_B();
	    if (cbf_C == NULL) this->reset();};
	
	// Wird bei Compare-C IRPT gerufen, um dem Timer Objekt den Eintritt des
	// Ereignisses mitzuteilen. Sie ruft ihrerseits die CBF_C der Appliaktion
	// (Compare C ist nur bei 16 Bit Timern vorhanden)
	inline void notify_C () {
	   	if (cbf_C != NULL) cbf_C();
	    if (cbf_O == NULL) this->reset();};

    // Wird bei Overflow IRPT gerufen, um dem Timer Objekt den Eintritt des
    // Ereignisses mitzuteilen. Sie ruft ihrerseits die CBF_O der Appliaktion
    inline void notify_O () {
	    if (cbf_O != NULL) cbf_O();
        this->reset();};

	// Starte den Timer fuer eine neue Runde
	inline void reset () {
		if (repeats > 1){
			if (repeats != 255) repeats--;
			if      (tccra == tc_vect[1]) TCNT1 = tcnt_init;
			else if (tccra == tc_vect[3]) TCNT3 = tcnt_init;
			else if (tccra == tc_vect[4]) TCNT4 = tcnt_init;
			else /* TC5 */                TCNT5 = tcnt_init;
		}
		else
			this->stop();
	};
	
	// gib den momentanen Zaehlerstand zurueck.
	inline uint16_t counter_value() {
		if      (tccra == tc_vect[1]) return TCNT1;
		else if (tccra == tc_vect[3]) return TCNT3;
		else if (tccra == tc_vect[4]) return TCNT4;
		else /* TC5 */                return TCNT5;
	};
	/*! \endcond */
		
};

////====================================================================
//
///*! \brief Unterscheidet die moegliche Zeitaufloesung mit der der Timer des DelayHandlers
           //arbeitet, also ob die Parameter \c ticks der \c delay_wait() und \c delay_nowait()
		   //Methoden als milli- oder mikro-Sekunden Werte interpretiert werden. Dabei
		   //ist noch zu beachten, dass der \c ticks Wert mit \c MIN_MS_TICK_LEN bzw.
		   //mit \c MIN_US_TICK_LEN multipliziert wird, also als Vielfache der gewaehlten
		   //Basis-Tick-Laenge zu verstehen ist.
	//\note Die Aufloesung Mikrosekunden sollte nur mit Bedacht gewaehlt werden, wenn 
	      //tatsaechlich eine hohe zeitliche Aufloesung benoetigt wird.
//*/
//enum delay_time_resolution {
	//use_us, /*!< Die Zeitangaben \c ticks bei \c delay_wait() und \c delay_nowait() sind
	             //mikro-Sekunden. */
	//use_ms  /*!< Die Zeitangaben \c ticks bei \c delay_wait() und \c delay_nowait() sind
	             //milli-Sekunden.*/
//};
//
///*! \brief Dient dazu den DelayHandler fuer single- oder multi-tasking Betrieb zu konfigurieren.
           //Im multi-tasking Betrieb wird \c delay_wait() durch Taskwechsel (\c yield() ) 
		   //implementiert. Im single-tasking Betrieb als busy-waiting Schleife. Fuer die
		   //Methode \c delay_nowait() besteht kein Unterschied. */
//enum op_mode_t {
	//single_tasking,  /*!< Arbeite mit busy-waiting. */
	//multi_tasking    /*!< Arbeite mit Taskwechsel. */
//};
//
///*! \brief Die Basis-Tick-Laenge des Timers im DelayHandler wenn \c use_ms konfiguriert wurde. */
//#define MIN_MS_TICK_LEN ((unsigned int)(1))   // 1 milli-Sekunde
 //
///*! \brief Die Basis-Tick-Laenge des Timers im DelayHandler wenn \c use_us konfiguriert wurde. */
//#define MIN_US_TICK_LEN ((unsigned int)(1))   // 1 micro-Sekunde
//
///*! \cond */
///*! \brief Maximale Anzahl der Delays, die gleichzeitig verwaltet werden koennen.
    //\note  Diese Zahl kann nicht veraendert werden ohne weitreichende Aenderungen in der 
	       //Kodierung von IDs, Indexen und Handles.
//*/
//#define NUM_OF_CONCURRENT_DELAYS  16  
//
///*! \brief 
    //Der delay_entry_handle_t codiert die Position des Auftrags in den niederwertigen 4 Bit der Zahl.
	//Da die Eintraege in der Auftragstabelle wiederverwendet werden, ist ueber die Tabellenposition
	//keine eindeutige Unterscheidung der Auftraege moeglich (wenn Tasks veraltete Eintragsnummern
	//verwenden). Daher werden die verbleibenden 12 Bit als fortlaufend vergebene Id des Auftrags
	//verwendet. ID (12 Bit) plus Index (4 Bit) ergeben zusammen ein 16 Bit Handle.
//*/
//typedef uint16_t delay_handle_t;
//
//#define INVALID_ID            (uint16_t)(0xffff)    // Valide IDs laufen zwischen 1 und 0x0ffe
//#define INVALID_HANDLE        (uint16_t)(0xffff)
//#define GET_INDEX(handle)     ((uint8_t)((uint16_t)(0x000F) & (uint16_t)(handle)))
//#define GET_ID(handle)        ((uint16_t)(((uint16_t)(0xFFF0) & (uint16_t)(handle))>>4))
//#define NEXT_ID               ((++id_count >= (uint16_t)(0x0FFF)) ? (id_count = 1) : id_count)
//#define MAKE_HANDLE(id,index) ((uint16_t)(((uint16_t)(id)<<4) + (uint16_t)(index)))
//
//typedef struct{
	//uint16_t  id;             /* Intern vergebene Nummer zwischen 1 und 0x0ffe, zur eindeutigen 
	                             //Identifikation der Auftraege. Laeuft bei Erreichen von 0x0fff ueber
				        		 //und beginnt wieder bei 1.*/
	//uint16_t  ticks;          /* Die Anzahl ticks, die bei jeder Auftragsrunde ablaufen muessen. */
	//uint16_t  tick_count;     /* Verbleibende Zeit deraktuellen Auftragrunde; je nach konfigurierter
	                             //delay_time_resolution sind das Vielfache von MIN_US_TICK_LEN oder
								 //von  MIN_MS_TICK_LEN. */
	//CBF_t     cbf;            /* Optionale Callback Funktion, um das Ende des Delays aktiv zu melden. */
	//uint8_t   repeats;        /* Die Anzahl der Wiederholungen dieses Zeitauftrags. 255 bedeutet endlos. */
	//uint8_t   rem_repeats;    /* Die Anzahl der noch abzuarbeitenden Wiederholungen. 255 bedeutet endlos.*/
//}delay_entry_t;
//
///*! \endcond */
//
//
///*! \brief Die Klasse DelayHandler implementiert "thread-safe" blockierende und nicht blockierende
           //delay Methoden verwendbar im single- oder multi-tasking Betrieb. Diese Methoden sind
		   //im multi-tasking Betrieb unbedingt anstelle der delay-Makros aus \c util/delay.h zu 
		   //verwenden, da diese Makros im multi-tasking Kontext nicht funktionieren, denn sie sind
		   //nicht reentrant implementiert (ihre Nutzung fuehrt zu "seltsamem" Systemverhalten).
	//\details Die Nutzung des DelayHandlers ist multi-tasking-sicher implementiert. Es 
	         //kann nur *ein* DelayHandler-Objekt instanziiert werden (Singleton-Klasse). 
			 //Dabei muss festgelegt werden, ob der DelayHandler im Mikrosekunden oder 
			 //im Millisekunden-Bereich arbeiten soll. Der Delayhandler ist darauf ausgelegt 
			 //bis maximal 16 Zeitauftraege gleichzeitig zu verwalten. Beachte die maximale Laenge 
			 //eines Zeitauftrags siehe Doku zur \c timer16 Klasse
	//\note Der Mikrosekunden-Bereich sollte nur verwendet werden, wenn fuer die Anwendung 
	      //die Aufloesung im Mikrosekunden-Bereich tatsaechlich noetig ist, da dies sonst
		  //zu einer unnoetig hohen Interrupt-Frequenz (Prozessorlast und Erhoehung der 
		  //Latenz anderer Interrupts) fuehrt.
		  //
	//\verbatim
	//Hier ein typisches Anwendungsschema des Delayhandlers fuer einen Programmabschnitt in dem
	//eine Nutzerinteraktion ablaeuft, der aber einen time-out haben soll, sofern der Nutzer fuer
	//eine gewissen Zeit nichts tut:
	//
	//DelayHandler dh(...)
//
	//delay_handle_t dhandle;
//
	//dhandle = dh.nowait (2500, 4);  // 10 sec time-out setzen: 4 Mal 2.5 sec und kein Callback
//
	//while (1) {
            //// hier passiert die Nutzerinteraktion
			//if (!dh.reset(dhandle)) // versuche 10 sec time-out zurueckzusetzen
    			//break;   // ging nicht: timer war schon abgelaufen, da Nutzer zu langsam oder inaktiv
//
            //// Pruefe, ob diese Schleife regulaer beendet werden soll
			//if (ende) {
				//dh.cancel(dhandle);   // dh aufraeumen
                //break;
			//}
        	//if (dh.is_expired(dhandle))
               //break;  // time-out wegen fehlender Nutzeraktion abgelaufen
	//}
//
	//\endverbatim
//*/
		//
//
//class DelayHandler : private Timer16 {
	//private:
		//static DelayHandler* delay_handler_instance;
        //static volatile delay_entry_t delay_tab[NUM_OF_CONCURRENT_DELAYS];
		//static volatile uint8_t entry_count;
		//static volatile uint16_t id_count;
		//static void tick_call_back(void);
		//delay_time_resolution res;
		//op_mode_t opm;
		//
	//public:
		///*! \brief Erzeuge den DelayHandler fuer die Applikation.
			//\param [in] tc_handle Der zu verwendende 16-Bit Timer.
			//\param [in] r Die zu verwendende zeitliche Aufloesung des Timers. Default sind Millisekunden.
//
			//\param [in] o Die Verwendung im single- oder multi-tasking Modus. Default ist single-tasking.
		//*/
		//DelayHandler(uint8_t tc_handle, delay_time_resolution r=use_ms, op_mode_t o=single_tasking);
		//
		///*! \brief Verzoegere den aktuellen Task um die Anzahl der angegebenen ticks.
		    //\param [in] ticks Die Anzahl der ticks, die vergehen muessen, bis der Task weiterlaeuft.
			                  //Dies ist eine Mindestwartezeit (untere Grenze). Hinzu kommt die Zeit
							  //die vergeht, bis der Task wieder vom Scheduler aktiviert wird (im multi-tasking Fall). Wenn
							  //0 uebergeben wird, passiert nichts. Beachte, dass der DelayHandler auf ein
							  //timer_16 Objekt abgebildet wird, also die maximale ticks Anzahl 4369 (~4,37 ms) ist.
							  //Laengere Zeiten koennen mit dem rpt Faktor gebildet werden.
			//\param [in] rpt Die Anzahl der Wiederholungen dieses Auftrags. Bei rpt == 0 oder rpt == 255
			                //verhaelt sich diese Methode identisch zu rpt == 1 - es wird also genau
							//eine Runde ausgefuehrt. 
							//Bei rpt == 1 bis rpt == 254 werden entsprechend viele Wiederholungen des
							//Auftrags ausgefuehrt und dann der Auftrag geloescht.
			//\returns  Boolescher Wert, der angibt, ob der Warteauftrag erfolgreich war. Im Falle
					  //\c false konnte der Auftrag nicht ausgefuehrt werden, da die Auftragstabelle
					  //voll war. Abgesehen von der Laufzeit der Funktion ist keine Verzoegerung erfolgt.
			//\details Der Task wird fuer die Dauer der Wartezeit in einer busy-waiting Schleife blockiert,
			         //gibt (im multi-tasking Modus) dabei aber die Kontrolle (mittels \c yield()) ab.
					 //Die Wartezeit ist *die untere Grenze* der Zeit, die vergeht bis der Task wieder
					 //weiterlaeuft. Sie ergibt sich bei gewaehlter Aufloesung Millisekunden als
					 //\c ticks*MIN_MS_TICK_LEN und bei Mikrosekunden als \c ticks*MIN_US_TICK_LEN.
		//*/
		//bool wait(uint16_t ticks, uint8_t rpt=0);
		//
		///*! \brief Informiere den aktuellen Task nach Ablauf der Anzahl der angegebenen ticks durch
		           //Aufruf der Callback-Funktion. Alternativ (wenn kein Callback angegeben wird) oder
				   //zusaetzlich zum Callback kann der Task den Status aktiv mittels \c is_expired() abfragen.
		    //\param [in] ticks Die Anzahl der ticks, die vergehen muessen, bis die (optionale) Callback-Funktion
			                  //gerufen wird. Dies ist die Mindestwartezeit (untere Grenze). Wenn
							  //0 uebergeben wird, passiert nichts, aber es wird \c INVALID_HANDLE
							  //gemeldet. Bei fehlendem Callback laeuft die Zeit einfach ab und dies
							  //kann mittels \c is_not_expired() festgestellt werden.
							  //Beachte, dass der DelayHandler auf ein
							  //timer_16 Objekt abgebildet wird, also die maximale ticks Anzahl 4369 ist.
							  //Laengere Zeiten koennen mit dem rpt Faktor gebildet werden.
			//\param [in] rpt Die Anzahl der Wiederholungen dieses Auftrags. Bei rpt == 0 verhaelt sich
							//diese Methode identisch zu rpt == 1 - es wird also mindestens eine Runde
							//ausgefuehrt. Bei rpt == 255 wird der Auftrag endlos wiederholt ausgefuehrt.
							//Bei rpt == 1 bis rpt == 254 werden entsprechend viele Wiederholungen des
							//Auftrags ausgefuehrt und dann der Auftrag geloescht.
			//\param [in] cbf Die zu rufende Callback Funktion. Eine parameterlose globale Funktion oder
			                //\c NULL. Im Fall \c NULL muss der Task den Ablauf der Wartezeit aktiv mittels
							//\c is_not_expired() pruefen.			
            //\returns  Handle mit dem spaeter auf den Zeitauftrag zugegriffen werden kann, z.B. um
			          //zu testen, ob der Auftrag bereits abgearbeitet ist (\c is_not_expired())
					  //oder um des Auftrag vorzeitig zu loeschen (\c cancel()) oder zurueck zu setzen (\c reset()). Wenn der
					  //Wert \c INVALID_HANDLE zurueck kommt, konnte der Auftrag nicht ausgefuehrt
					  //werden, da die Auftragstabelle voll war oder der Wert von \c ticks 0 war.
			//\note Die Callback Funktion wird *nicht* im Kontext des Tasks ausgefuehrt, der sie 
			      //registriert hat, sondern im Rahmen der Interrupt Service Routine des Timers. Sie
				  //darf also nur sehr kurz sein und z.B. ueber eine globale Variable den Ablauf der
				  //Wartezeit an ihren Task melden, der dann bei naechster Aktivierung durch den
				  //Scheduler darauf reagieren kann.
			//\details Der Task wird nicht blockiert. Nach Ablauf der Wartezeit wird die registrierte 
			         //Callback-Funktion gerufen. Die Wartezeit ist *die untere Grenze* der
					 //Zeit, die bis zum Aufruf vergeht. Sie ergibt sich bei gewaehlter Aufloesung 
					 //Millisekunden als \c ticks*MIN_MS_TICK_LEN und bei Mikrosekunden als
					 //\c ticks*MIN_US_TICK_LEN.
		//*/
		//delay_handle_t nowait(uint16_t ticks, uint8_t rpt=0, CBF_t cbf=NULL);
		//
			//
		///*! \brief Teste, ob der durch den Handle \c dh spezifizierte Auftrag noch existiert und wieviele
		           //Wiederholugneg des Auftrags noch ausstehen.
			//\retval 255 Der Auftrag mit diesem Handle laeuft endlos.
			//\retval 1..254  Der Auftrag laeuft noch fuer die gelieferte Anzahl Wiederholungen.
			//\retval 0   Der Auftrag mit diesem Handle ist nicht (mehr) zu finden - er ist abgearbeitet.
		//*/
		//uint8_t remaining_repeats(delay_handle_t dh);
		//
		//
		///*! \brief Teste, ob der durch den Handle \c dh spezifizierte Auftrag noch existiert.
		    //\retval true Der Auftrag laeuft noch.
			//\retval false Der Auftrag ist abgelaufen und existiert nicht mehr.
		//*/
		//bool is_expired(delay_handle_t dh);
		//
		//
		///*! \brief Loesche den durch den Handle \c dh spezifizierten Auftrag.
			//\retval >0 Der Auftrag wurde gefunden und geloescht und die Anzahl der noch verbliebenen Runden 
			           //wird zurueckgeliefert.
			//\retval 0  Der Auftrag mit diesem Handle wurde nicht (mehr) gefunden.
		//*/
		//uint8_t cancel(delay_handle_t dh);
		//
		///*! \brief Setze den durch den Handle \c dh spezifizierten Auftrag auf seinen Startzustand zurueck, 
		           //sofern der Auftrag noch nicht abgelaufen ist.
			//\retval true Der Auftrag wurde gefunden und auf seinen Startzustand zurueckgesetzt.
			//\retval false Der Auftrag mit diesem Handle wurde nicht (mehr) gefunden.
		//*/
		//bool reset(delay_handle_t dh);
//
//};


/*! }@   Ende group 2 Timer */

#endif /* TIMER_H_ */