#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <string.h> //enlever le .h s'il y a un pb
using namespace std;

#include "protocole.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

extern WindowClient *w;

int idQ, idShm;
bool logged = false;
char* pShm;
ARTICLE articleEnCours;
float totalCaddie = 0.0;

void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);

#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de l'identifiant de la file de messages
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    // TO DO
    if((idQ = msgget(CLE, 0)) == -1)
      perror("Erreur de récupération de la file de message...\n");

    // Recuperation de l'identifiant de la mémoire partagée
    fprintf(stderr,"%d Recuperation de l'id de la mémoire partagée\n",getpid());
    // TO DO
    if((idShm = shmget(CLE, 0, 0)) != 0)
      perror("Erreur de récupération de la mémoire partagée...\n");

    // Attachement à la mémoire partagée
    // TO DO
    if((pShm = (char*)shmat(idShm, NULL, 0)) == (char*)-1)
      perror("Erreur de shmat()...\n");

    // Armement des signaux
    // TO DO
    struct sigaction s_action1;
    s_action1.sa_handler = handlerSIGUSR1;
    sigaction(SIGUSR1, &s_action1, nullptr);

    struct sigaction s_action2;
    s_action2.sa_handler = handlerSIGUSR2;
    sigaction(SIGUSR2, &s_action2, nullptr);

    // Envoi d'une requete de connexion au serveur
    // TO DO
    MESSAGE connectRequest;
    connectRequest.expediteur = getpid();
    connectRequest.type = 1;
    connectRequest.requete = CONNECT;
    if(msgsnd(idQ, &connectRequest, sizeof(MESSAGE) - sizeof(long), 0))
      perror("Erreur de l'envoi de la demande de connection...\n");
    else
      printf("Requete de demande de connexion envoyée !\n");

    // Exemples à supprimer
    setArticle("pommes",5.53,18,"pommes.jpg");
    ajouteArticleTablePanier("cerises",8.96,2);
}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  totalCaddie = 0.0;
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  // TO DO (étape 1)
  // Envoi d'une requete DECONNECT au serveur
  MESSAGE deconnect;
  
  // envoi d'un logout si logged
  if(logged)
    w->on_pushButtonLogout_clicked();

  // Envoi d'une requete de deconnexion au serveur
  deconnect.type = 1;
  deconnect.expediteur = getpid();
  deconnect.requete = DECONNECT;

  if(msgsnd(idQ, &deconnect, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    perror("Erreur d'envoi de message de deconnexion...\n");

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
  MESSAGE loginReq;

  if(strlen(getMotDePasse()) < 4 || strlen(getNom()) < 4)
    perror("Nom ou Mot de passe trop court... (min 3 caractères)\n");
  else
  {
    loginReq.expediteur = getpid();
    loginReq.type = 1;
    loginReq.requete = LOGIN;
    if(isNouveauClientChecked())
      loginReq.data1 = 1;
    else
      loginReq.data1 = 0;
    strcpy(loginReq.data2, getNom());
    strcpy(loginReq.data3, getMotDePasse());

    if(msgsnd(idQ, &loginReq, sizeof(MESSAGE) - sizeof(long), 0) == -1)
      perror("Erreur d'envoi de la requête de login...\n");

  }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
  /*MESSAGE req;
    // Envoi d'une requete CANCEL_ALL au serveur (au cas où le panier n'est pas vide)
    // TO DO
  req.expediteur = getpid();
  req.type = 1;
  req.requete = CANCEL_ALL;

  if(msgsnd(idQ, &req, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    perror("Erreur d'envoi de cancel_all...\n");
  else*/

  // Envoi d'une requete de logout au serveur
  // TO DO
  MESSAGE logoutReq;

  logoutReq.expediteur = getpid();
  logoutReq.type = 1;
  logoutReq.requete = LOGOUT;

  if(msgsnd(idQ, &logoutReq, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    perror("Erreur de l'envoi de logout...\n");
  else
    logoutOK();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
    // TO DO (étape 5)
    // Envoi d'une requete ACHAT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL au serveur

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL_ALL au serveur

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
    // TO DO (étape 7)
    // Envoi d'une requete PAYER au serveur

    char tmp[100];
    sprintf(tmp,"Merci pour votre paiement de %.2f ! Votre commande sera livrée tout prochainement.",totalCaddie);
    dialogueMessage("Payer...",tmp);

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR1(int sig)
{
    MESSAGE m;
    printf("WAZZZZZZZZZZZZUP\n");
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) != -1)  // !!! a modifier en temps voulu !!!
    {
      
      switch(m.requete)
      {
        case LOGIN :
                    if(m.data1 == 1)
                    {
                      w->loginOK();
                      w->dialogueMessage("LOGIN", m.data4);
                    }
                    else
                    {
                      w->dialogueMessage("LOGIN ERROR", m.data4);
                    }
      
                    break;

        case CONSULT : // TO DO (étape 3)
                    break;

        case ACHAT : // TO DO (étape 5)
                    break;

         case CADDIE : // TO DO (étape 5)
                    break;

         case TIME_OUT : // TO DO (étape 6)
                    break;

         case BUSY : // TO DO (étape 7)
                    break;

         default :
                    break;
      }
    }
}

void handlerSIGUSR2(int sig)
{
  w->setPublicite(pShm);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
