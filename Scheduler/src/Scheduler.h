/*
 Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either 
 version 2 of the License, or (at your option) any later 
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/


#ifndef Scheduler_h
	#define Scheduler_h

	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "PMutex.h"
	#include "SchedulerErrors.h"
	#include "Times.h"


	/**
		La libreria Scheduler gestisce la schedulazione di attivita'.
		Ogni schedulazione viene incapsulata all'interno di un oggetto
		di tipo Times che e' un componente passivo: infatti esso
		viene attivato da un oggetto di tipo Scheduler che costituisce
		il motore dello scheduler.

		La classe Scheduler eredita dalla classe PosixThread e rappresenta
		uno scheduler. Essa gestisce gli eventuali timeouts
		di oggetti di tipo Times.
		Un oggetto di tipo Times puo' essere pensato come un orologio cui
		possono essere inseriti allarmi (timeouts).
		L'attivita' dello scheduler consiste nel controllare periodicamente
		gli oggetti di tipo Times e, nel momento in cui viene riscontrato
		un timeout, invocare il metodo handleTimeout dell'oggetto Times.
	
		Per la gestione del cambio ora legale da parte degli oggetti
		di tipo Times vedi la descrizione della classe Times.
	
		Il tipo Times e' una classe astratta in quanto possiede il metodo
		virtuale puro handleTimeOut che deve essere ridefinito dall'utente
		in modo che esso possa realizzare le operazioni da eseguire
		quando si verifica un timeout.
		L'utente della libreria deve semplicemente creare una classe
		per ogni Times personalizzato che eredita dalla classe Times e
		ridefinire il metodo handleTimeout.

		E' lo scheduler che automaticamente si preoccupa di gestire gli oggetti
		di tipo Times e chiamare il metodo handleTimeout ogni qual volta
		si verifica un timeout. 

		Gli esempi che si trovano nella directory examples chiariranno
		l'uso di questa libreria.
	*/
	#ifdef WIN32
		typedef class Scheduler: public WinThread
	#else
		typedef class Scheduler: public PosixThread
	#endif

	{
		private:
			typedef enum SchedulerStatus {
				SCHEDULER_BUILDED,
				SCHEDULER_INITIALIZED,
				SCHEDULER_STARTED,
				SCHEDULER_SUSPENDED
			} SchedulerStatus_t, *SchedulerStatus_p;

		private:
			unsigned long			_ulThreadSleepInMilliSecs;
			SchedulerStatus_t		_schedulerStatus;
			unsigned long			_ulTimesPointerToAllocOnOverflow;
			unsigned long			_ulAllocatedTimesPointerNumber;

			Error getTimesPointerIndex (Times_p pTimes,
				long *plTimesPointerIndex);


		protected:
			// mutex for the private and protected variables
			PMutex_t				_mtSchedulerMutex;
			Times_p					*_pTimesList;
			unsigned long			_ulTimesPointerNumber;


			Scheduler (const Scheduler &);

			/**
				In questo metodo viene realizzato il corpo principale
				dello scheduler.
				Esso chiama il metodo handleTimes per la gestione degli eventi.
			*/
			Error run (void);

			/**
				Il metodo gestisce tutti i Times attivati all'interno dello
				scheduler ed in particolare, per ogni Times, l'algoritmo che
				il metodo esegue risulta essere il seguente:
			
					se il Times e' in stato STARTED
						se il timeout del Times e' scaduto
							chiama il metodo handleTimeout
							se il periodo del Times e' 0
								chiama il metodo deactiveTimes per quel Times
							altrimenti
								chiama il metodo updateNextExpirationDateTime
								al Times
			*/
			virtual Error handleTimes (void);


		public:
			/**
				Costruttore
			*/
			Scheduler (void);

			/**
				Distruttore
			*/
			~Scheduler (void);

			/**
				Inizializza lo scheduler.
			
				Il parametro ulThreadSleepInMilliSecs indica il periodo
				espresso in milli-secondi tra un controllo dei Times e l'altro.
			
				L'oggetto Scheduler conterra' i puntatori ad oggetti
				di tipo Times all'interno di un vettore.
				Il parametro lTimesPointerToAllocOnOverflow indica che
				il vettore sara' inizialmente dimensionato per 20
				puntatori ai Times.
				Ogni qual volta si supera questa soglia saranno eseguite
				le seguenti operazioni:
					1. sara' allocato un vettore avente la
						dimensione precedente + 20
					2. saranno copiati i precedenti puntatori ai Times
						nel nuovo vettore
					3. sara' deallocato il precedente vettore
			*/
			Error init (unsigned long ulThreadSleepInMilliSecs = 1000,
				unsigned long ulTimesPointerToAllocOnOverflow = 20);

			/**
				Finish the object freeing his internal list
				from any Times object.
				If bDestroyTimes is true it performs also the following
				actions:
				- call the finish method to each Times object
				- delete each Times object
			*/
			Error finish (Boolean_t bDestroyTimes);

			/**
				Il metodo attiva lo scheduler thread.
				Vedi PosixThread:: start.
			*/
			Error start (void);

			/**
				Sospende il funzionamento dello scheduler.
				Il thread non viene interrotto.
			*/
			Error suspend (void);

			/**
				Viene chiamato per riattivare lo scheduler dopo che sia stato
				interrotto dal metodo suspend.
			*/
			Error resume (void);

			/**
				Interrompe l'esecuzione del thread.
				Vedi PosixThread:: cancel.
			*/
			Error cancel (void);

			/**
				Viene dato allo scheduler un ulteriore Times da gestire.
			*/
			Error activeTimes (Times_p pTimes);

			/**
				Il metodo permette di disattivare un Times attivato dal
				metodo activeTimes.
			*/
			Error deactiveTimes (Times_p pTimes);

			/**
				Il metodo permette di disattivare un Times attivato dal
				metodo activeTimes.
			*/
			Error deactiveTimes (long lTimesPointerIndex);

			/**
				Ritorna il numero di Times correntemente gestiti
				dallo scheduler.
			*/
			Error getTimesNumber (unsigned long *pulTimesNumber);

			/**
				I Times gestiti dallo scheduler sono inseriti in un vettore
				le cui dimensioni cambiano dinamicamente.
				Il metodo ritorna il puntatore al Times che correntemente 
				occupa la posizione indicata dal parametro lTimesIndex.
			*/
			Error getTimes (unsigned long ulTimesIndex, Times_p *pTimes);

			/**
				Ritorna lo stato dello scheduler.
			*/
			Error getSchedulerStatus (SchedulerStatus_p pssSchedulerStatus);

	} Scheduler_t, *Scheduler_p;

#endif

