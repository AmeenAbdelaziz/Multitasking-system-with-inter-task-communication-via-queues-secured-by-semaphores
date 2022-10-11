#ifndef BINARYSEMAPHOR_H_
#define BINARYSEMAPHOR_H_

/*! \brief Modul fuer binaere Semaphore
	\version   2.0
	\date      7.9.2019
	\author    Bomarius
	\copyright Bomarius
	\file      BinarySemaphor.h
*/


/*! \brief Modul fuer binaere Semaphore

	\defgroup group7 Semaphore

   @{
	   
   Version 2 mit optionaler Semaphor-Warteliste zur Beeinflussung des Schedulings.
	   
   Einfaches binaeres Semaphor - es kann nur belegt oder frei sein, kann darueber
   hinaus aber nicht zaehlen.
    
   \note Nur derjenige Task, der das Semaphor aktuell 
   belegt hat kann es auch wieder freigeben. Die Methoden \c is_free() und \c wait() koennen
   auch zur Synchronisierung zwischen Tasks verwendet werden. Die Methoden von BinarySemaphor
   sind atomar (multi-tasking sicher) definiert.
   
   \note Die seit Version 2 verfuegbare Semaphor-Warteliste ist eine Liste von Tasks die blockierend
   auf ein Semaphor warten und beim Task-Scheduling bevorzug werden. Der am laengsten auf
   ein Semaphor wartende Task wird zuerst aktiviert. Die Semaphor-Warteliste kann im Modul
   OSKernel ausgeschaltet werden. Das fuehrt dann aber ggf. zum Verhungern von Tasks,
   die sehr lange oder unendlich auf ein Semaphor warten, da andere Tasks permament zuvor 
   kommen. Daher empfielt es sich die Semaphor-Warteliste aktiviert zu lassen.
      
   \verbatim
   
   Verwendungsbeispiel 1 (zwei verschraenkt koordinierte Tasks):
   Zwei Tasks, die konkurierend auf eine gemeinsame Ressource zugreifen.
   Zur Koordination wurde das Semaphor \c shared_resource_semaphor angelegt.
   Durch die gezeigte Verwendung von \c wait_aquire(), \c release() und \c yield()
   werden die beiden Tasks strikt verschraenkt ausgefuehrt.
   
   BinarySemaphor shared_resource_semaphor;
   
   void task1 (void) {
	   while (1) {
		   shared_resource_semaphor.wait_aquire();
		   // do something with shared resource
		   shared_resource_semaphor.release();
		   yield();
	   }
   }
   
	void task2 (void) {
		while (1) {
			shared_resource_semaphor.wait_aquire();
			// do something else with shared resource
			shared_resource_semaphor.release();
			yield();
		}
	}
   
   
   Verwendungsbeispiel 2 (zwei entkoppelt koordinierte Tasks):
   Zwei Tasks, die konkurierend auf eine gemeinsame Ressource zugreifen.
   Zur Koordination wurde das Semaphor \c shared_resource_semaphor angelegt.
   Durch die gezeigte Verwendung von \c wait_aquire(), \c release() und \c yield()
   wird der Zugriff koordiniert, ohne eine bestimmte Abfolge der Tasks
   zu erzwingen. Exklusiver Zugriff pro Task ueber laengere Zeit, solange
   die Ressource fuer die jeweilige Operation(sfolge) bereit ist.
   
   BinarySemaphor shared_resource_semaphor;
   
   void task1 (void) {
	   while (1) {
		   shared_resource_semaphor.wait_aquire();
		   while (shared_resource_ready()) {
			   write_shared_resource();
			   <optional: yield();>
		   }
		   shared_resource_semaphor.release();
		   yield();
	   }
   }
   
   void task2 (void) {
	   while (1) {
		   shared_resource_semaphor.wait_aquire();
		   while (shared_resource_ready()) {
			   read_shared_resource();
			   <optional: yield();>
		   }
		   shared_resource_semaphor.release();
		   yield();
	   }
   }
   
    \endverbatim
*/

class BinarySemaphor {
	private:
		uint8_t owner;
		
	public:
		BinarySemaphor () : owner(0) {};
			
		/*! \brief Versuche das Semaphor zu belegen (nicht blockierend).
		    \retval true  Semaphor wurde erfolgreich vom rufenden Task belegt.
			\retval false Semaphor ist bereits von einem anderen Task belegt.
		*/
		bool aquire ();
		
		/*! \brief Versuche das Semaphor zu belegen (blockierend).
		    \details Ein belegtes Semaphor (ausser wenn es vom rufenden Task selbst belegt ist) fuehrt
			zum blockierenden Warten des Aufrufs (der rufende Task wird automatisch suspendiert
			bis das Semaphor frei wird). Bei Rueckkehr aus dem Aufruf ist das Semaphor fuer den
			aufrufenden Task belegt.
		*/
		void wait_aquire ();
		
		/*! \brief Versuche das Semaphor freizugeben (nicht blockierend).
		    \retval true  Semaphor wurde erfolgreich frei gegeben.
		    \retval false Semaphor ist von einem anderen Task belegt und konnte nicht
	                      freigegeben werden.
		    \details Wenn der aufrufende Task nicht Besitzer des Semaphors ist, scheitert der Aufruf 
			         und das Semaphor bleibt belegt. Wenn das Semaphor freigegeben wird erfolgt
					 (bei aktivierter Semaphor-Warteliste) ein Taskwechsel.
		*/		
		bool release ();
		
		/*! \brief Teste nicht blockierend, ob das Semaphor gerade frei ist.
			\retval true  Semaphor ist nicht belegt, auch nicht vom rufenden Task. Beachte:
			              ein unmittelbar folgender Aufruf von \c aquire() kann trotzdem scheitern
						  und \c wait_aquire() kann blockieren, falls ein anderer Task schneller
						  das Semaphor belegt.
			\retval false Semaphor ist von einem Task belegt, ggf. vom rufenden Task selbst.

		*/
		bool is_free () {return owner == 0;};
			
		/*! \brief Teste, ob das Semaphor vom rufenden Task	belegt ist.
			\retval true  Das Semaphor wurde vom rufenden Task zuvor bereits erfolgreich belegt.
			\retval false Das Semaphor ist entweder frei oder ist aktuell von einem anderen Task belegt.
		*/
		bool is_mine ();
		
		/*! \brief Warte blockierend auf das Freiwerden des Semaphors.
		 
		    \note Das Semaphor wird \a nicht automatisch belegt. Es muss mittels \c aquire() 
			oder \c wait_aquire() belegt werden. Im Falle von \c aquire() kann dies scheitern, falls
			ein anderer Task zuvor kam; \c wait_aquire() kann zum Blockieren des Tasks fuehren.
		*/
		void wait ();
};

/*! @} */  // end group7 Semaphore

#endif /* BINARYSEMAPHOR_H_ */