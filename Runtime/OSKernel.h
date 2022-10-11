#ifndef __OSKERNEL_H__
#define __OSKERNEL_H__

#include <stddef.h>

/*! 
 	\version   2.0
 	\date      7.9.2019
 	\author    Bomarius
 	\copyright Bomarius
 	\file      OSKernel.h
*/

/*! \brief Modul OSKernel - Betriebssystem-Kern fuer praeemptives Multitasking.

	Der Kern des multitasking Betriebssystems enhaelt einen praemptiven Scheduler,
	der reihum alle registrierten und aktuell aktiven Tasks fuer je eine Zeitscheibe
	aktiviert und bei Ende der Zeitscheibe suspendiert, sofern sich der Taskt nicht
	schon vorher selbst suspendiert hat, bspw. mittels \c yield() oder Aufruf einer
	blockierenden Systemfunktion.
	
	Tasks sind parameterlose Prozeduren vom Typ \c task_f_t, die endlos laufen. Die
	Prozedurzeiger aller Tasks werden beim Betriebssystemkern mittels \c task_insert()
	registiert und dann wird der Kern mittels Aufruf \c kernel() gestartet, wodurch 
	das System in den multi-Tasking Modus uebergeht (und nicht mehr zurueckkehrt).
	
	Innerhalb von Tasks koennen Funktionen des Kernels genutzt werden, um sich selbst
	zu suspendieren, um die eigene Task-Nummer zu ermitteln und dann damit Einstellungen
	der Taskparameter vorzunehmen, den Task zu (de)aktivieren oder dessen Laufzeit
	abzufragen.
	
	Aenderungen mit Version 2:
	
	Die Funktion \c stop_kernel() zum gewaltsamen Beenden des multitasking Betriebs
	wurde hinzugefuegt. 
	
	Eine optionale Semaphor-Warteliste, die mittels \c USE_SEMAPHORE_WAITING_LIST aktviert
	wird, wurde hinzugefuegt. Die Semaphor-Warteliste ist eine FiFo-Liste von Tasks die
	aktuell blockierend auf ein Semaphor warten und beim Task-Scheduling bevorzug werden
	sollen, so dass der jeweils am laengsten auf ein Semaphor wartende Task  beim 
	naechsten Taskwechsel zuerst aktiviert wird. Die Semaphor-Warteliste kann ausgeschaltet
	werden. Das fuehrt dann aber ggf. zum Verhungern von Tasks, die sehr lange oder 
	unendlich auf ein Semaphor warten, da andere Tasks ihm permament zuvor kommen. 
	Daher empfielt es sich die Semaphor-Warteliste aktiviert zu lassen.
	
	Durch die Einfuehrung der Klasse \c DelayHandler im Modul Timer sind timergesteuerte
	Delays, blockierend wie auch nichtblockierend, im single- wie auch im multitasking
	Modus moeglich. Dies ersetzt einerseits die ohnehin nicht multi-tasking tauglichen 
	Makros aus \c util/delay.h sowie die nun obsolete Funktion \c sleep_laps(). 

   \defgroup group11 OSKernel
   @{
*/

/*! \brief Nicht erlaubte Task ID. Als Fehleranzeige genutzt. */
#define NOT_A_TASK  255

/*! \brief  Reservierung der Stackgroesse pro Task. Beachte: die 32 Register
            der CPU muessen dort mindestens gerettet werden. Dazu kommen die
			Bedarfe aus den Prozeduraufrufen des Tasks. */
#define STACK_SIZE 800

/*! \brief Maximale Anzahl Tasks im System (inkl. idle task).

     Beachte, dass fuer die maximale Anzahl Tasks im SRAM statisch ungefaehr
	 TASK_LIMIT * (STACK_SIZE + 50) Bytes reserviert werden, unabhaengig davon,
	 wieviele Tasks mittels \a task_insert() tatsaechlich erzeugt werden. */
#define TASK_LIMIT 8

/*! \brief Schaltet das bevorzugte Scheduling von auf Semaphore wartende
           Tasks ein, um ein Verhungern von Tasks zu verhindern. */
#define USE_SEMAPHORE_WAITING_LIST

/*! \brief  Funktions-Zeiger-Typ fuer Task Prozeduren. 

     Task Prozeduren sind parameterlose Prozeduren, die in einer Endlosschleife
	 laufen. Eine Taskende ist aktuell nicht vorgesehen, lediglich ein gewaltsames
	 Beenden des Schedulers mittels \a kernel_stop(). */
typedef void(*task_f_t)(void);

/*! \brief  Funktions-Zeiger-Typ fuer die Exit Prozedur. 

     Eine Exit Prozedur ist eine Prozedur, die fuer Abschlussarbeiten
	 nach Beendigung des Multitaskings genutzt werden kann.
	 \param [in] reason Grund des Abbruchs.
*/

typedef void(*exit_proc_t)(uint8_t reason);

/*! \brief  Die Werte des Typs Priority werden vom Scheduler auf sleep laps abgebildet und
            simulieren dadurch eine Ausfuehrungsprioritaet ueber die relative 
			Ausfuehrungshaeufigkeit des Tasks. 
*/
enum Priority {
	Low = 4,	/*!< Tasks dieser Prioritaet werden nur alle 4 Scheduling Runden aktiviert. */
	Medium = 2, /*!< Tasks dieser Prioritaet werden nur alle 2 Scheduling Runden aktiviert.*/
	High = 0    /*!< Tasks dieser Prioritaet werden jede Scheduling Runde aktiviert.*/
	};

/*! \brief  Laenge der Zeitscheibe in ms die der Scheduler jedem Task gewaehrt.

    Beachte: Da vom Kernel der TC0 verwendet wird, muss der Wert <= 17 sein.
*/
#define TIME_SLICE_LENGTH  15

/*! \brief  Kontrolle der Verwendung von Zeitscheiben. Mittels \a JitterControl wird jedem
            Task fuer eine volle Zeitscheibe Zeit gegeben, selbst wenn dieser zuvor
			mittels \a yield() die Kontrolle zurueckgibt.
*/
enum SchedulingType { 
	Simple,			/*!< Tasks werden ohne jitter-Kontrolle ausgefuehrt. */
	JitterControl   /*!< Unverbrauchte Restzeiten von Tasks werden vom Betriebssystemkern bis
	                    Erreichung des Endes des Zeitslots aufgefuellt (CPU schlaeft), um 
						feste Taktung zu erreichen und so Jitter zu minimieren. */
	};
	
/*! \brief  Start des Betriebssystem Kerns. Muss im Applikationsprogramm (main) gerufen
            werden, nachdem alle Applikations-Tasks registriert sind. Verwendet einen
			praeemptiven round-robin scheduler. Diese Prozedur kehrt nie zurueck, ist also
			letzte Anweisung in main().
			
	\param [in] schedt Hiermit kann die Art der Jitter-Kontrolle eingestellt werden. \a Simple 
	bedeutet, dass jeder Task maximal seinen Zeitslot bekommt. Wenn der Task vor Ablauf
	seines Slots mittels \a yield() die Kontrolle zurueckgibt, wird sofort zum naechsten Task gewechselt. Dies
	nutzt die CPU optimal aus. Bei \a JitteControl wird jedem Task zwangsweise ein
	voller Zeitslot zugewiesen. Dadurch entstehen Wartezeiten, die die CPU-Auslastung
	reduzieren dafuer die Regelmaessigkeit der Taskausfuehrung erhoehen (falls die Anwendung
	dies erfordert).
	\param [in] ep Ein Zeiger auf eine Exit-Prozedur der Applikation, die nach Stop des
	Schedulers (siehe \a Kernel_stop() ) aufgerufen werden soll.  		
*/
void kernel (SchedulingType schedt = Simple, exit_proc_t ep = NULL) __attribute__ ((noreturn));

/*! \brief Stoppe den Betriebssystemkern (den Scheduler). Beendet den multi-tasking Modus,
           schalte Interrupts aus, und rufe danach eine ggf beim Kernel-Start registrierte
		   Exit Prozedur auf. Uebergibt dabei den Wert des Parameters reason an die Exit Prozedur.
*/
void stop_kernel(uint8_t reason = 0) __attribute__ ((noreturn));

/*! \brief  Gibt die Kontrolle vorzeitig ans Betriebssystem ab und loese task scheduling aus.

	Ein Task verwendet yield(), wenn eine Aufgabe (vorlaeufig) beendet werden kann und er
	anderen Tasks moeglichst bald die Ausfuehrung ermoeglichen will. Beachte: nur wenn
	der \a kernel() mit Modus \a Simple gestartet wurde erfolgt sofortiger Wechsel, sonst
	schlaeft die CPU bis zum Ende der Zeitscheibe des rufenden Tasks.
 */
void yield (void);

/*! \brief Registriere neuen Task mit einer definierten Prioritaet. Default ist \a High. 

    \param [in] tf Zeiger auf die parameterlose, endlos laufende Task-Prozdur.
	\param [in] prio Ueber die Prioritaet kann gesteuert werden, wie haeufig der Task 
	            relativ zu anderen Tasks aktiviert werden soll. Die Prioritaet kann 
				jederzeit mit task_set_prio() veraendert werden.
    \retval Ganze-Zahl Die vom Kernel vergebene Task Id des Tasks oder NOT_A_TASK im Fehlerfall.

     Die Registrierung neuer Tasks kann auch noch im Multi-Tasking Modus erfolgen.
 */
uint8_t task_insert (task_f_t tf, Priority prio = High);

/*! \brief Liefere Task ID des aktuellen Tasks. */
uint8_t current_task_id();

/*! \brief  Aendere Prioritaet des aktuellen Tasks. 

	Ein Task kann nur seine eigene Prioritaet veraendern, nicht die anderer Tasks. */
uint8_t task_set_prio(Priority prio);

/*! \brief  Der Task mit der ID tid wird voruebergehend aus dem Scheduling herausgenommen,
            aber nicht geloescht. 
	\param [in] prio Ueber die Prioritaet kann gesteuert werden, wie haeufig der Task
	                 relativ zu anderen Tasks aktiviert werden soll.
	
	Ein deaktivierter Task kann jederzeit mit \a activate() oder mit \a sleep_laps() wieder
    reaktiviert werden. Bei Anwendung auf den eigenen Task wird dieser sofort deaktiviert. */
void deactivate (uint8_t tid);

/*! \brief Der Task mit ID tid wird wieder aktviert.
	\param [in] tid Eine gueltige Task Id, die von task_insert() geliefert wurde.

	Sollten noch Warte-Runden eines \a sleep_laps() Aufrufs anstehen, werden
	diese jetzt geloescht. */
void activate (uint8_t tid);

/*! \brief  Liefert true wenn Task mit ID tid nicht deaktiviert ist. 
	\param [in] tid Eine gueltige Task Id, die von task_insert() geliefert wurde.
	
	Beachte: Der als aktiv erkannte Task kann allerdings sleeping oder suspended sein, 
	wenn es nicht der eigene Task ist. Die Anwendung auf den eigenen Task ist sinnlos. */
bool is_active (uint8_t tid);

/*! \brief Liefert die bislang verbrauchte Zeit des Tasks mit ID tid in Millisekunden.

	\param [in] tid Eine gueltige Task Id, die von task_insert() geliefert wurde.
*/
uint32_t elapsed_ms (uint8_t tid);


/*! \cond   Nur zur internen Verwendung */

// Registriere einen Task der auf ein Semaphor blockierend wartet.
void sem_wait (uint8_t tid);
// Hole den am laengsten auf Semaphore wartenden Tasks aus der Warteliste.
uint8_t sem_waiting ();

/*! \endcond */

/*! @} Ende group11 OSKernel */

#endif // __OSKERNEL_H__