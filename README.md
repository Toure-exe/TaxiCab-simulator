## Autori: Ismaila Toure & Taralli Giulio

# TaxiCab-simulator
(progetto di sistemi operativi) Simulazione di una città (matrice condivisa) in cui ci sono dei processi taxi che vanno in giro a soddisfare delle richiesta da parte di altri processi, portandoli a destinazione.


# Processi SO_SOURCES:

Al momento della generazione/inizializzazione degli SO_SOURCES (ovvero il momento in cui generiamo le
coordinate di generazione) i processi aspettano che tutti gli SO_SOURCES processi siano stati
correttamente inizializzati. Una volta assicurata la corretta generazione, i processi sono in grado di generare
le proprie destinazioni, inviandole ai processi SO_TAXI grazie a una coda di messaggi.
Il singolo processo SO_SOURCES genererà le sue destinazioni in un intervallo di tempo fisso da noi
prestabilito.

# Processi SO_TAXI:
Al momento della generazione degli SO_TAXI (ovvero il momento in cui generiamo le coordinate di
generazione) i processi aspettano che tutti gli SO_TAXI e gli SO_SOURCES processi siano stati correttamente
inizializzati.
Una volta assicurata la corretta generazione, i processi sono in grado di leggere le destinazioni dalla coda di
messaggi, caricando la destinazione in quattro vettori condivisi contenenti coordinate di destinazione e
coordinate di generazione della richiesta SO_SOURCES.
I quattro vettori condivisi si comportano in modo simile a delle pipe, tutti i processi hanno un indice
condiviso con il quale possono muoversi tra il vettore, per riuscire a scorrere il vettore, è necessario
soddisfare la richiesta corrente.
Una volta caricato ciò, i processi SO_TAXI passano alla fase movimento, se non sono su una cella
SO_SOURCES, allora cercano nelle celle adiacenti se ne hanno una, se sì, si spostano verso quella cella.
Altrimenti si muovono randomicamente (evitando buchi e/o di finire fuori mappa) fino a trovare una cella
SO_SOURCES.
Se un taxi si trova su una cella SO_SOURCES, fa un controllo sui vettori condivisi e vede se la sua posizione
attuale coincide con la posizione della generazione della richiesta corrente, se sì, il taxi provvede
all’associazione della richiesta salvando in locale le destinazioni e incrementando l’indice condiviso,
altrimenti aspetta sulla cella SO_SOURCES che l’indice condiviso scorra.
Una volta che il processo taxi porta la richiesta a destinazione, il processo si mette alla ricerca di una nuova
richiesta SO_SOURCES da soddisfare.
Quando un SO_TAXI si trova in una situazione di stallo, usiamo la system call execv per generare un nuovo
processo.


#Processo Master:

Il processo Master inizialmente legge i dati di configurazione dal file “configurazione.txt” analizza il numero
di buchi e lo confronta con la grandezza della mappa. Se è possibile generare il numero di buchi richiesto il
Master provvede a generare la mappa, altrimenti si arresta inviando un errore.
Il Master crea i processi SO_SOURCES e SO_TAXI e aspetta la loro creazione (ovvero il momento in cui
vengono generate le loro coordinate di generazione) per far partire la simulazione attraverso un segnale di
alarm.
A fine simulazione, l’handler cattura il segnale alarm e uccide tutti i processi figli, successivamente il master
provvede a stampare i vari output statistici della simulazione.
