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

int idQ;
MYSQL* connexion;

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(ACCESBD %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération descripteur lecture du pipe
  int fdRpipe = atoi(argv[1]);

  // Connexion à la base de donnée
  // TO DO
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;

  MESSAGE m;
  MESSAGE reponse;

  char requete[200];

  while(1)
  {
    // Lecture d'une requete sur le pipe
    // TO DO
    int ret = read(fdRpipe, &m, sizeof(MESSAGE) - sizeof(long));
    
    if(ret < 0)
    {
      perror("(ACCESBD) Erreur de read");
      mysql_close(connexion);
      exit(1);
    }
    else if(ret == 0)
    {
      printf("(ACCESBD) no one on the pipe...\n");
    }

    switch(m.requete)
    {
      case CONSULT :  // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD
                      sprintf(requete, "select * from UNIX_FINAL where id = %d;", m.data1);
                      if(mysql_query(connexion, requete) != 0)
                      {
                        fprintf(stderr, "Erreur de mysql_query %s\n", mysql_error(connexion));
                      }
                      else
                      {
                        reponse.requete = CONSULT;
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        
                        if((resultat = mysql_store_result(connexion)) == NULL)
                        {
                          fprintf(stderr, "Erreur de mysql_store_result %s\n", mysql_error(connexion));
                          reponse.data1 = -1;
                        }
                        else
                        {
                          // Preparation de la reponse
                          Tuple = mysql_fetch_row(resultat);

                          reponse.data1 = atoi(Tuple[0]);
                          strcpy(reponse.data2, Tuple[1]);
                          strcpy(reponse.data3, Tuple[3]);
                          strcpy(reponse.data4, Tuple[4]);
                          reponse.data5 = atof(Tuple[2]);
                        }
                        // Envoi de la reponse au bon caddie
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                          perror("Erreur de snd Caddie");
                      }

                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Finalisation et envoi de la reponse
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Mise à jour du stock en BD
                      break;

      case LOGOUT :   //En plus pour dire au process qu'il doit mourir
                      fprintf(stderr,"(ACCESBD %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      mysql_close(connexion);
                      exit(0);
                      break;


    }
  }
}
