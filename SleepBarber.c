/*
 * SleepBarber
 *   Programma che usa i semafori e i pthread
 *   per risolvere il problema del barbiere addormentato.
 *
 *  Compila con:
 *	cc SleepBarber.c -o SleepBarber -lpthread -lm
 */

#define _REENTRANT

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

// Il numero massimo di thread del cliente.
#define MAX_CLIENTI 25

// Prototipi delle funzioni
void *cliente(void *num);
void *barbiere(void *);

void randwait(int secs);

// Definiamo i semafori.

// La sala d'attesa limita il numero di clienti
// consentiti che entrano in una sola volta.
sem_t salaAttesa;   

// sediaBarbiere garantisce l'accesso mutuamente esclusivo.
sem_t sediaBarbiere;

// pillolaBarbiere e' usata per far dormire il barbiere.
sem_t pillolaBarbiere;

// cintura per far aspettare il cliente
sem_t cintura;

// Flag che stoppa il barbiere quando tutti i clienti
// sono stati serviti.
int serviti = 0;

int main(int argc, char *argv[]) {
    pthread_t btid;
    pthread_t tid[MAX_CLIENTI];
    long semeRandom;
    int i, numeroClienti, numeroSedie;
    int Numero[MAX_CLIENTI];
    
        
    // Verifica che ci sia il numero corretto di argomenti
    // nella riga di comando.
    if (argc != 4) {
	   printf("Come si usa: ./SleepBarber <Numero clienti> <Numero sedie> <Numero seme casuale>\n");
	   exit(-1);
    }
    
    // Ottenuti gli argomenti dalla linea di comando
    // li converte in interi.
    numeroClienti = atoi(argv[1]);
    numeroSedie = atoi(argv[2]);
    semeRandom = atol(argv[3]);
    
    // Assicuriamoci che il numero di thread sia inferiore al numero di
    //clienti che possiamo supportare.
    if (numeroClienti > MAX_CLIENTI) {
	   printf("Il numero massimo di clienti è %d.\n", MAX_CLIENTI);
	   exit(-1);
    }
    
    printf("\n------------------------------| SleepBarber |---------------------------------\n\n");
    printf("Una soluzione al problema del barbiere addormentato usando i semafori e i pthread.\n");
    
    // Inizializza il generatore di numeri casuali con un nuovo seme.
    srand48(semeRandom);

    // Inizializza la matrice dei numeri.
    for (i=0; i<MAX_CLIENTI; i++) {
	   Numero[i] = i;
    }
		
    // Inizializza i semafori con i valori iniziali
    sem_init(&salaAttesa, 0, numeroSedie);
    sem_init(&sediaBarbiere, 0, 1);
    sem_init(&pillolaBarbiere, 0, 0);
    sem_init(&cintura, 0, 0);
    
    // Crea il processo barbiere.
    pthread_create(&btid, NULL, barbiere, NULL);

    // Crea il processo consumatore.
    for (i=0; i<numeroClienti; i++) {
	   pthread_create(&tid[i], NULL, cliente, (void *)&Numero[i]);
    }

    // Unisciti a ciascuno dei thread per aspettare che finiscano.
    for (i=0; i<numeroClienti; i++) {
	   pthread_join(tid[i],NULL);
    }

    // Quando tutti i clienti hanno finito, 
    // killa il thread del barbiere.
    serviti = 1;
    sem_post(&pillolaBarbiere);  // Svegliamo il barbiere cosi uscira.
    pthread_join(btid,NULL);	
}

void *cliente(void *numero) {
    int num = *(int *)numero;

    // Per arrivare al negozio impiega un po di tempo 
    // in modo casuale
    printf("Cliente %d sta per arrivare in negozio.\n", num);
    randwait(5);
    printf("Cliente %d e' arrivato dal barbiere.\n", num);

    // Aspetta che si apra uno spazio nella sala d'attesa
    sem_wait(&salaAttesa);
    printf("Cliente %d entra in sala d'attesa.\n", num);

    // Aspetta che la sedia da barbiere sia libera
    sem_wait(&sediaBarbiere);

    // La sedia è libera quindi rinuncia al 
    //posto in sala d'attesa.
    sem_post(&salaAttesa);

    // Sveglia il barbiere
    printf("Cliente %d sveglia il barbiere.\n", num);
    sem_post(&pillolaBarbiere);

    // Aspetta che il barbiere finisca di tagliare i capelli.
    sem_wait(&cintura);
    
    // Si alza dalla sedia e se ne va.
    sem_post(&sediaBarbiere);
    printf("Cliente %d esce dal salone.\n", num);
}

void *barbiere(void *junk) {
    // Mentre ci sono ancora clienti da servire...
    // Il nostro barbiere è onniscente e prevede se
    // ci saranno altri clienti da servire.
    while (!serviti) {

	   // Dorme finche' non arriva un cliente a svegliarlo
	   printf("Il barbiere sta dormendo.\n");
	   sem_wait(&pillolaBarbiere);

	   // Salta alla fine
	   if (!serviti) {

	    // Impiega una quantita' di tempo casuale per 
        // tagliare i capelli del cliente
	    printf("Il barbiere sta tagliando i capelli.\n");
	    randwait(3);
	    printf("Il barbiere ha finito di tagliare i capelli.\n");

	    // Rilascia il cliente al termine del taglio
	    sem_post(&cintura);
	   } else {
	    printf("Il barbiere torna a casa felice per oggi.\n");
	   }
    }
}

void randwait(int secs) {
    int drm;
    // Genera un numero a caso...
    drm = (int) ((drand48() * secs) + 1);
    sleep(drm);
}
