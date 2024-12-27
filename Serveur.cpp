#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include "FichierUtilisateur.h"

int idQ,idShm,idSem;
int fdPipe[2];
TAB_CONNEXIONS *tab;
int dsc;

void afficheTab();
int checkLogin(MESSAGE *usr);

int main()
{
  // Armement des signaux
  // TO DO

  // Creation des ressources
  // Creation de la file de message
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }

  // TO BE CONTINUED
  if((idShm = shmget(CLE, SIZESHM, IPC_CREAT | IPC_EXCL | 0600)) == -1)
  {
    perror("(SERVEUR) Erreur de shmget()...\n");
    exit(1);
  }

  // Creation du pipe
  // TO DO

  // Initialisation du tableau de connexions
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = 0;

  afficheTab();

  // Creation du processus Publicite (étape 2)
  // TO DO

  // Creation du processus AccesBD (étape 4)
  // TO DO

  tab->pidServeur = getpid();

  MESSAGE m;
  MESSAGE reponse;
  int nbClient = 0;

  while(1)
  {
  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

    switch(m.requete)
    {
      case CONNECT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);

                      if(nbClient >= 5)
                        perror("Plus de place pour un autre client...\n");
                      else
                      {
                        for(int i = 0; i < 6; i++)
                        {
                          if(tab->connexions[i].pidFenetre == 0)
                          {
                            tab->connexions[i].pidFenetre = m.expediteur;
                            nbClient++;
                            i = 6;
                          }
                        }
                      }

                      break;

      case DECONNECT : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);

                      for(int i = 0; i < 6; i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur)
                        {
                          tab->connexions[i].pidFenetre = 0;
                          strcpy(tab->connexions[i].nom, "");
                          tab->connexions[i].pidCaddie = 0;

                          kill(m.expediteur, SIGUSR2);
                        }
                      }

                      break;

      case LOGIN :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);
                      
                      reponse.data1 = checkLogin(&m);
                      reponse.requete = LOGIN;
                      reponse.type = m.expediteur;
                      strcpy(reponse.data4, m.data4);
                      kill(m.expediteur, SIGUSR1);


                      if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        perror("(SERVEUR) Erreur d'envoi de la reponse...\n");


      case LOGOUT :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case UPDATE_PUB :  // TO DO
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case PAYER : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
    }
    afficheTab();
  }
}

void afficheTab()
{
  fprintf(stderr,"Pid Serveur   : %d\n",tab->pidServeur);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid AccesBD   : %d\n",tab->pidAccesBD);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
  fprintf(stderr,"\n");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//    Fonctions du Switch             ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// CONNECT

// DECONNECT

//LOGIN
int checkLogin(MESSAGE *usr)
{
  int pos;
  printf("--- Entrée dans checkLogin() ---\n");

  pos = estPresent(usr->data2);

  if(usr->data1 == 1) // Is Nouveau checked
  {
    if(pos >= 0)
    {
      strcpy(usr->data4, "Client deja existant");
      perror("Client deja existant\n");
      return 0;
    }
    else
    {
      ajouteUtilisateur(usr->data2, usr->data3);
      return 1;
    }
  }
  else
  {
    if(pos == -1)
    {
      strcpy(usr->data4, "Client Inexistant");
      perror("Client Inexistant...\n");
      return 0;
    }
    else  // IsNouveau not checked & il existe dans le fichier
    {
      if(verifieMotDePasse(pos, usr->data3))
      {
        strcpy(usr->data4, "Bon mpd!");
        perror("Bon mdp !\n");
        return 1;
      }
      else
      {
        strcpy(usr->data4, "Mdp errone...");
        perror("Mdp errone...\n");
        return 0;
      }
    }
  }
}

//LOGOUT

// UPDATEPUB

// CONSULT

// ACHAT

// CADDIE

// CANCEL

// CANCEL_ALL

// PAYER

// NEW_PUB
