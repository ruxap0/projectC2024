#include <csignal>
#include <cstddef>
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
#include <mysql.h>
#include "protocole.h" // contient la cle et la structure d'un message


#include <errno.h>
int idQ;

ARTICLE articles[10];

int nbArticles = 0;

int fdWpipe;
int pidClient;

void handlerSIGALRM(int sig);
void cancelall();

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  /*sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);*/

  // Armement des signaux
  // TO DO
  signal(SIGALRM, handlerSIGALRM);

  // Boucle init id Article -1
  for(int i = 0; i < 10; i++) 
    articles[i].id = -1;

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  MESSAGE m;
  MESSAGE reponse;
  
  char newUser[20];

  // Récupération descripteur écriture du pipe
  fdWpipe = atoi(argv[1]);

  while(1)
  {
    alarm(60);
    
    if(msgrcv(idQ,&m,sizeof(MESSAGE) - sizeof(long),getpid(),0) == -1)
    {
      char* tm = strerror(errno);
      printf("JOLI PETIT MESSAGE : %s\n", tm);
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }
    
    alarm(0);

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                      pidClient = m.expediteur;
                      break;

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      exit(0);
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      
                      m.expediteur = getpid();

                      if(write(fdWpipe, &m, sizeof(MESSAGE) - sizeof(long)) != (sizeof(MESSAGE) - sizeof(long)))
                        perror("(CADDIE) Erreur de Write");
                      else
                      {
                        MESSAGE conf;
                        //cv ici
                        if(msgrcv(idQ, &conf, sizeof(MESSAGE) - sizeof(long), getpid(), 0) == -1)
                          perror("(CADDIE) Erreur de rcv CONF");
                        else if(conf.data1 != -1)
                        {
                          MESSAGE repCons;
                          repCons.type = pidClient;
                          repCons.expediteur = getpid();
                          repCons.requete = CONSULT;
                          repCons.data1 = conf.data1;
                          strcpy(repCons.data2, conf.data2);
                          strcpy(repCons.data3, conf.data3);
                          strcpy(repCons.data4, conf.data4);
                          repCons.data5 = conf.data5;
                          if(msgsnd(idQ, &repCons, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                          {
                            perror("(CADDIE) Erreur de snd CONF");
                          }
                          else
                            kill(pidClient, SIGUSR1);
                        }
                      }

                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // on transfert la requete à AccesBD
                      m.expediteur = getpid();
                      if(write(fdWpipe, &m, sizeof(MESSAGE) - sizeof(long)) != (sizeof(MESSAGE) - sizeof(long)))
                        perror("(CADDIE) Erreur de Write");
                      else
                      {
                        MESSAGE conf;
                        if(msgrcv(idQ, &conf, sizeof(MESSAGE) - sizeof(long), getpid(), 0) == -1)
                          perror("(CADDIE) Erreur de rcv ACHAT");
                        else
                        {
                          if(strcmp(conf.data3, "0") != 0)
                          {
                            for(int i = 0; i < 10; i++)
                            {
                              if(articles[i].id == -1)
                              {
                                articles[i].id = m.data1;
                                strcpy(articles[i].intitule, conf.data2);
                                articles[i].prix = conf.data5;
                                articles[i].stock = atoi(conf.data3);
                                nbArticles++;
                                break;
                              }
                            }                            
                          }

                          printf("BLYYYYYYYAAAAAAAAAAAAAAT\n");
                          
                          conf.type = pidClient;
                          conf.expediteur = getpid();
                          if(msgsnd(idQ, &conf, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                            perror("(CADDIE) Erreur de snd");
                          else
                           kill(pidClient, SIGUSR1);
                        }
                      }
                      
                      // on attend la réponse venant de AccesBD
                        
                      // Envoi de la reponse au client

                      break;

      case CADDIE :
      {
                      // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      
                      for(int i = 0; i < 10; i++)
                      {
                        if(articles[i].id != -1)
                        {
                          MESSAGE msgArt;
                          msgArt.requete = CADDIE;
                          msgArt.expediteur = getpid();
                          msgArt.type = pidClient;

                          msgArt.data1 = articles[i].id;
                          strcpy(msgArt.data2, articles[i].intitule);
                          sprintf(msgArt.data3, "%d", articles[i].stock);
                          msgArt.data5 = articles[i].prix;
                          
                          if(msgsnd(idQ, &msgArt, sizeof(MESSAGE) - sizeof(long), IPC_NOWAIT) == -1)
                            perror("(CADDIE) Erreur de snd CADDIE");
                          else
                            kill(pidClient, SIGUSR1);
                        }
                      }
                      
                      break;
      }   

      case CANCEL :   // TO DO
      {
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                      // on transmet la requete à AccesBD
                      m.expediteur = getpid();
                      int i = m.data1;
                      m.data1 = articles[i].id;
                      sprintf(m.data2, "%d", articles[i].stock);

                      if(write(fdWpipe, &m, sizeof(MESSAGE) - sizeof(long)) != (sizeof(MESSAGE) - sizeof(long)))
                        perror("(CADDIE) Erreur de write");

                      // Suppression de l'aricle du panier
                      articles[i].id = -1;
                      nbArticles--;

                      break;
      }

      case CANCEL_ALL : // TO DO
      {
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                      cancelall();

                      break;
      }
      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier
                      for(int i = 0; i < 10; i++)
                        articles[i].id = -1;

                      nbArticles = 0;
                      
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  // Annulation du caddie et mise à jour de la BD
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier
  cancelall();
  // Envoi d'un Time Out au client (s'il existe toujours)

  MESSAGE tmout;
  tmout.type = pidClient;
  tmout.requete = TIME_OUT;
  tmout.expediteur = getpid();

  if(msgsnd(idQ, &tmout, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    perror("(CADDIE) Erreur de snd TIMEOUT");
  else
    kill(pidClient, SIGUSR1);
         
  exit(0);
}

///////////////////////////////////////////////////////////////
////// Fonctions supp     /////////////////////////////////////
///////////////////////////////////////////////////////////////

void cancelall()
{
  MESSAGE m;
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier
  int i = 0;

  while(nbArticles > 0 && i < 10) // i pas trop utile mais au cas où on sait jamais
  {
    if(articles[i].id != -1)
    {
      m.expediteur = getpid();
      m.requete = CANCEL;
      m.data1 = articles[i].id,
      sprintf(m.data2, "%d", articles[i].stock);
                          
      if(write(fdWpipe, &m, sizeof(MESSAGE) - sizeof(long)) != (sizeof(MESSAGE) - sizeof(long)))
        perror("(CADDIE) Erreur de write");
                          
      articles[i].id = -1;
      nbArticles--;
    }
    i++;
  }
}