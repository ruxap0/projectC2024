#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ, idShm;
char *pShm;
void handlerSIGUSR1(int sig);
void handlerSIGINT(int sig);
int fd;

int main()
{
  // Armement des signaux
  // TO DO
  struct sigaction s_action1;
  s_action1.sa_handler = handlerSIGINT;
  sigaction(SIGINT, &s_action1, nullptr);

  // Masquage des signaux
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask,SIGUSR1);
  sigdelset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(PUBLICITE) Erreur de msgget");
    exit(1);
  }

  // Recuperation de l'identifiant de la mémoire partagée
  if((idShm = shmget(CLE, 0, 0)) == -1)
    perror("(PUBLICITE) Erreur de recup de la mem partagee");

  // Attachement à la mémoire partagée
  if((pShm = (char*)shmat(idShm, NULL, 0)) == (char*)-1)
    perror("(PUBLICITE) Erreur de shmat...");

  // Mise en place de la publicité en mémoire partagée
  char pub[51];
  strcpy(pub,"Bienvenue sur le site du Maraicher en ligne !");

  for (int i=0 ; i <= 50 ; i++) pShm[i] = ' ';
  pShm[51] = '\0';
  int indDebut = 25 - strlen(pub)/2;
  for (int i=0 ; i<strlen(pub) ; i++) pShm[indDebut + i] = pub[i];

  while(1)
  {
    MESSAGE rup; // Request Update Pub
    rup.type = 1;
    rup.requete = UPDATE_PUB;
    rup.expediteur = getpid();

    if(msgsnd(idQ, &rup, sizeof(MESSAGE) - sizeof(long), IPC_NOWAIT) == -1)
      perror("(PUBLICITE) Erreur de snd de update...");

    sleep(1);
    
    // Decallage vers la gauche

    char tmp;
    tmp = pShm[0];
    for(int i = 1; i <= 50; i++)
    {
      pShm[i - 1] = pShm[i];
    }
    pShm[50] = tmp;
  }
}

void handlerSIGUSR1(int sig)
{
  fprintf(stderr,"(PUBLICITE %d) Nouvelle publicite !\n",getpid());

  // Lecture message NEW_PUB

  // Mise en place de la publicité en mémoire partagée
}

void handlerSIGINT(int sig)
{
  fprintf(stderr,"(PUBLICITE) HANDLER DE SIGINT\n");
  exit(0);
}