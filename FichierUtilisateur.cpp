#include "FichierUtilisateur.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int estPresent(const char* nom)
{
  printf("--- Entrée dans estPresent() ---\n");

  UTILISATEUR readUser;
  int position = 0, descrFile, found = 0;
  
  if((descrFile = open(FICHIER_CLIENTS, O_RDONLY)) == -1){
    perror("Erreur de open()");
    return (-1);
  }

  while(read(descrFile, &readUser, sizeof(UTILISATEUR)) > 0)
  {

    if(strcmp(nom,readUser.nom) == 0)
    {
      found = 1;
      break;
    }

    position++;
  }

  if(close(descrFile)){
    perror("Erreur de close()");
    exit(1);
  }

  if(found == 0)
  {
    printf("Sup nigger\n");
    return -1;
  }
  else
    return position;
}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
  printf("--- Entrée dans hash() ---\n");

  int retourHash = 0;
  for(size_t i = 0; i < strlen(motDePasse); i++)
    retourHash += (i + 1) * (int)motDePasse[i];

  return retourHash % 97;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteUtilisateur(const char* name, const char* motDePasse)
{
  printf("--- Entrée dans ajouteUtilisateur() ---\n");

  int descrFile;
  UTILISATEUR writeUser;

  strcpy(writeUser.nom, name);
  writeUser.hash = hash(motDePasse);

  if((descrFile = open(FICHIER_CLIENTS, O_WRONLY | O_CREAT)) == -1){
    perror("Erreur de open()");
    exit(1);
  }

  lseek(descrFile, 0, SEEK_END);
  write(descrFile, &writeUser, sizeof(UTILISATEUR));
  printf("Ajout Effectué\n");

  if(close(descrFile)){
    perror("Erreur de close()");
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////////////////////
int verifieMotDePasse(int pos, const char* motDePasse)
{
  printf("--- Entrée dans verifieMotDePasse() ---\n");

  int descrFile;
  UTILISATEUR checkUser;

  if((descrFile = open(FICHIER_CLIENTS, O_RDONLY)) == -1){
    perror("Erreur de open()");
    return -1;
  }

  lseek(descrFile, (pos * sizeof(UTILISATEUR)), SEEK_SET);
  read(descrFile, &checkUser, sizeof(UTILISATEUR));

  if(close(descrFile)){
    perror("Erreur de close()");
    return -1;
  }
  
  if(checkUser.hash == hash(motDePasse))
    return 1;
  else
    return 0;
}