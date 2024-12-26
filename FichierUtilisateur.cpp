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
  UTILISATEUR readUser;
  int position = 0, descrFile, foundIt = 0;
  
  if((descrFile = open(FICHIER_UTILISATEURS, O_RDONLY)) == -1){
    perror("Erreur de open()");
    return (-1);
  }

  while(read(descrFile, &readUser, sizeof(UTILISATEUR)) != -1 && foundIt != 1){
    position += 1;
    if(nom == readUser.nom)
      foundIt = 1;
  }

  if(close(descrFile)){
    perror("Erreur de close()");
    exit(1);
  }
  if(foundIt == 0)
    return -1;
  else
    return position;
}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
  int retourHash = 0;
  for(int i = 0; i < strlen(motDePasse); i++)
    retourHash += (i + 1) * (int)motDePasse[i];

  return retourHash % 97;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteUtilisateur(const char* name, const char* motDePasse)
{
  int descrFile;
  UTILISATEUR writeUser;

  strcpy(writeUser.nom, name);
  writeUser.hash = hash(motDePasse);

  if((descrFile = open(FICHIER_UTILISATEURS, O_WRONLY | O_CREAT)) == -1){
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
  int descrFile;
  UTILISATEUR checkUser;

  if((descrFile = open(FICHIER_UTILISATEURS, O_RDONLY)) == -1){
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

////////////////////////////////////////////////////////////////////////////////////
int listeUtilisateurs(UTILISATEUR *vecteur) // le vecteur doit etre suffisamment grand
{
  int descrFile, countuser = 0;

  if ((descrFile = open(FICHIER_UTILISATEURS, O_RDONLY)) == -1){
    perror("Erreur de open()");
    return -1;
  }

  for(int i = 0; read(descrFile, &vecteur[i], sizeof(UTILISATEUR)) > 0; i++, countuser++){}
  
  if(close(descrFile)){
    perror("Erreur de close()");
    return -1;
  }

  return countuser;
}