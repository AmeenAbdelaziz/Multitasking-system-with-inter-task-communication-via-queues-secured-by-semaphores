#ifndef BOUNDEDQUEUE_H_
#define BOUNDEDQUEUE_H_

/*! \brief Modul fuer FIFO-Queue Ringpuffer.

	\version   1.0
	\date      9.2.2018
	\author    Bomarius
	\copyright Bomarius
	\file      BoundedQueue.h
  */ 


/*! \brief Modul fuer FIFO-Queue Ringpuffer.

	\defgroup group8 BoundedQueue
	
    Die Klasse BoundedQueue implementiert einen FIFO Queue Ringpuffer, der Eintraege
	vom Typ unsigned char aufnehmen kann.  Die Bedeutung eines Zeichens (Buchstabe oder Zahl)
	ist von der verwendenden Applikation bestimmt. Sofern Tasks zusammenhaengende
	Zeichenketten ohne Unterbrechung durch andere Tasks in eine Queue ablegen wollen,
	muss zusaetzlich ein Semaphor genutzt werden, da die Atomaritaet der Queue-Methoden
	immer nur das Lesen/Schreiben eines einzelnen Zeichens garantiert.

	@{
*/

/*! \brief Definiert die Laenge des Ring-Puffers aller BoundedQueue Objekte. 

	Die Laenge der BoundenQueue Objekte wird mit diesem Makro statisch festgelegt, 
	da eine dynamische Allokation mit variierenden Groessen mangels heap nicht 
	moeglich ist.
*/
#define BOUNDEDQUEUE_SIZE ((uint8_t)(10))

/*! \brief Sonderzeichen, darf nicht als Nachrichtenzeichen verwendet werden.

	Wird als spezielles Zeichen von der Methode \a read() verwendet, um die leere
	Queue anzuzeigen. 
*/
#define NAC ((unsigned char)(0))

/*! \brief Steuert das Verhalten der Queue bei Volllaufen: die jeweils aeltesten noch nicht
           gelesenen Werte werden ueberschrieben. */
#define ALLOW_OVERWRITE ((uint8_t)(0))
/*! \brief Steuert das Verhalten der Queue bei Volllaufen: write() scheitert und 
           noch nicht gelesene Werte werden nicht ueberschrieben. Default.*/
#define NO_OVERWRITE    ((uint8_t)(1))
       

/*! \brief  Implementiert einen FIFO Queue Ringpuffer. 

	Der Konstruktor ist nicht multi-tasking sicher, da von statischer Allokation vor dem 
	Wechsel in den Multitasking Modus ausgegangen wird. Die anderen Methoden sind atomar,
    soweit noetig - sie operieren auf Einzelzeichen-Basis. Sofern Tasks Zeichenketten ohne
	Unterbrechung durch andere Tasks in eine Queue ablegen wollen, muss zusaetzlich ein
	Semaphor genutzt werden, da die Atomaritaet der Queue-Methoden immer nur das 
	Lesen/Schreiben eines einzelnen Zeichens garantiert. 
*/

class BoundedQueue {
	private:
		uint8_t       mode;      // Ueberschreiben bei vollem Puffer oder nicht
	    uint8_t       count;     // Anzahl aktuell belegter Plaetze der Queue
		uint8_t       write_pos; // naechste beschreibbare Position in content[]
		uint8_t       read_pos;  // naechste zu lesende Position in content[]
		unsigned char content [BOUNDEDQUEUE_SIZE];  // Inhalt der Queue
		
	public:

		/*!  \brief Erzeuge eine bounded queue. 
		     \param [in]  m Steuert das Verhalten bei Vollaufen der Queue. 
			 
			 Im Modus NO_OVERWRITE lehnt die Queue weitere Schreibevorgaenge ab (der Aufruf 
			 scheitert), wenn die Queue voll ist. Im Modus ALLOW_OVERWRITE wird in diesem
			 Fall der aelteste Eintrag der Queue ueberschrieben. */
	    BoundedQueue (uint8_t m = NO_OVERWRITE) {read_pos = 0; write_pos = 0; count = 0; mode = m;};
			
		/*! \brief Setze Queue auf Initialzustand (leer). */
		void clear ();
		
		/*! \brief Liefere Anzahl aktuell noch freier Stellen der Queue. */
		uint8_t get_free_size ();

		/*! \brief Liefere Anzahl aktuell besetzter Stellen der Queue. */
		uint8_t get_used_size () {return count;};
			
		/*! \brief Liefere das naechste Zeichen (nicht blockierend). 
		
			Falls die Queue leer ist, wird das Zeichen \a NAC geliefert. Das gelesene Zeichen 
			wird aus der Queue entfernt. */
		unsigned char read ();
		
		/*! \brief Liefere das naechste Zeichen. Warte ggf. blockierend, bis eines aus
		          der Queue gelesen werden kann. */
		unsigned char wait_read ();

		/*! \brief Versuche ein Zeichen in die Queue zu schreiben. 
		    \param [in] ch Ein beliebiger binaerer 8-Bit Wert. Der Wert \a NAC dient als 
			               Fehleranzeige bei read() und sollte nicht geschrieben werden.
		
			Wenn die Queue im Modus NO_OVERWRITE arbeitet und die Queue bereits voll ist,
			wird false geliefert. Der Aufrufer muss dann den Schreibeversuch spaeter wiederholen.
			In diesem Modus werden noch nicht gelesene Zeichen niemals ueberschrieben.
			Im Modus ALLOW_OVERWRITE wird bei voller Queue niemals false geliefert, sondern es 
			wird das aelteste noch nicht gelesene Zeichen ueberschrieben. Dieses Zeichen ist
			dann fuer den lesenden Task unwiederbringlich verloren.	*/	
        bool write (unsigned char ch);
};

/*! @} */   // end group8 BoundedQueue

#endif /* BOUNDEDQUEUE_H_ */

