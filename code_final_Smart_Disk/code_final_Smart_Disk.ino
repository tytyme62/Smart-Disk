#include <Wire.h> //librairie qui permet d'utiliser le ports de l'arduino en entrée et sortie
#include <LiquidCrystal_I2C.h> //librairie qui permet de grandement faciliter l'utilisation de l'écran lcd en comprenant des fonction d'écriture, d'effacage ect..
#include <MKRGSM.h> //cette librairie nous sert à nous servir du module GSM présent sur le mkr gsm 1400 à l'aide de fonctions qui facilité l'initialisation de la sim, l'envoie et la réception de messages
//stdlib.h>
#include <DS3231.h> // cette librairie nous permet d'utiliser simplement l'horloge rtc qui communique en I2c avec des fonctions qui nous permettent notament de récupérer l'heure s'implement

RTClib myRTC; //définition d'une structure de type RTClib qui nous permet d'enregistrer l'heure lorsqu'elle est récupérer par l'horloge
LiquidCrystal_I2C lcd(0x27, 20, 4); //définition du type d'écran lcd utiliser et de son adresse d'esclave pour la communication en I2C avec les fonctions.

//constantes
const int D0 = 0; //définition de constantes symboliques qui permettent d'améliorer la lisibilité du code
const int D1 = 1;
const int D2 = 2;
const short TEMPMAX = 52; //préciser le nombre de secondes max avant qu'on zap le mess pour l'inscription

//variables globale
char carac;
String listUtilisateurs[]={"","",""}; //tableau de chaine de caractère qui sert à enregistrer le nom des 3 utilisateurs potentiels
String numeroUtilisateur[]={"","",""}; //tableau de chaine de caractères qui permet d'enregistrer le numéro de téléphone des 3 utilisateurs potentiels
int temperature;
short heures;
short minutes;
short utilisateur=-1; //nombre qui va nous permettre de renseigner l'utilisateur qui est connecté (-1 lorsqu'il n'y en à pas sinon 1, 2 ou 3)


//prototypes
char aquerir(void); //fonction qui nous permet de récupérer un caractère sur le clavier
void menuDeroulant(void); //il s'agit d'une fonctione qui se répète indéfiniment et que l'on peut considérer comme le menu de base (c'est le premier menu que l'on voit lorsque l'utilisateur met en marche le Smart Disk)
void menuConnexion(void); //il s'agit d'une fonction qui affiche un menu intéractif qui permet à l'utilisateur de changer d'utilisateur connecté
void menuUtilistaeurs(void); //il s'agit d'une fonction qui affiche un menu intéractif qui permet à l'utilisateur de gérer les utilisateurs présents sur le smart disk, que ce soit d'en ajouter ou d'en supprimer
void supprimerUtilistaeurs(void);  //il s'agit d'une fonction qui affiche un menu intéractif qui permet à l'utilisateur de supprimer un utilisateur en supprimant ses infirmations des tableaux listUtilisateurs et numeroUtilisateur
void stationnement(void); //il s'agit d'une fonction qui affiche un menu intéractif qui permet à l'utilisateur de renseigner en temps de stationnement et ensuite de lancer un timer qui se chargera de l'envoie des messages de rappel concernant le temps de stationnement
void enregistrement(void); //il s'agit d'une fonction qui permet à l'utilisateur de renseigner un numéro de téléphone d'un utilisateur pas encore enregistré dans le smart disk afin de l'enregistrer en envoyer un message demandant son nom d'utilisateur et en gérant la réception (ou non) du message
void initSMS(void); //fonction qui permet d'entrer le code pin (1234 ici) de la sim et d'en vérifier le fonctionnement

GSM gsmAccess;
GSM_SMS sms;

//ici on initialise tous les ports nécessaires, les variables et le materielle comme la sim ou l'écran
void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  //while(!Serial){} //ces deux lignes serait à avoir un affichage à l'écran de l'ordinateur lorsque l'arduino était branché afin de débuger l'application.
  Wire.begin(); //ici on initialise l'utilisation des ports, nottament de l'I2C
  pinMode(A2,INPUT); //on met le port analogique A2 du capteur de température en entrée.
  
  pinMode(A3, INPUT); //ligne 1 k sur les 7 lignes suivantes, on met en entrée ou en sortie les port utlisées par le clavier numérique
  pinMode(A4, INPUT); //ligne 2 j les ports numérique D0,D1 et D2 sont mis en sortie tandis que les ports analogiques A3, A4, A5 et A6 sont mis en entrée.
  pinMode(A5, INPUT); //ligne 3 h
  pinMode(A6, INPUT); //ligne 4 g
  pinMode(D0, OUTPUT); //colonne 1 f
  pinMode(D1, OUTPUT); //colonne 2 e
  pinMode(D2, OUTPUT); //colonne 3 d
  lcd.init(); //ici on initialise l'écran lcd
  lcd.backlight(); //ici on allume la lumière arrière de l'écran lcd qui permet de le rendre plus lisible
  initSMS();
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("entrée loop");
  menuDeroulant(); //on ne sort jamais de cette fonction

}

char aquerir(void) {
  //tant que l'utilisateur n'appuie sur aucune touche du clavier numérique, on ne sort pas de cette fonction
  //cette fonction fonctionne en métant à niveau haut les sortie des colonnes 1,2 et 3 succésivement
  //et en vérifiant sur chaque lignes si on retrouve une entrée à 1 
    while (1){
    digitalWrite(D0, HIGH); //ici on met à niveau haut la sortie numérique du port 0 (ligne 1)
    delay(60); //delais qui laisse les temps au courant de circuler et qui permet une meilleur précision sur la saisie
    if (digitalRead(A3)==1) { //ici on lit l'entrée analogique A3 avec une valeur numérique (soit haut 1 ou bas 0)
      while (digitalRead(A3)==1) {} //si on trouve un niveau haut, cela veut dire que l'utilisateur à appuyé sur la touche de la ligne 1 colonne 1, soit le 1
      return'1'; //donc on retourne le caractère lié à la touche
      }
    if (digitalRead(A4)==1) {
      while (digitalRead(A4)==1) {}
      return '4';
      }
    if (digitalRead(A5)==1) {
      while (digitalRead(A5)==1) {}
      return '7';
      }
    if (digitalRead(A6)==1) {
      while (digitalRead(A6)==1) {}
      return '*';
      }
    digitalWrite(D0, LOW);
    digitalWrite(D1, HIGH);
    delay(60);
    if (digitalRead(A3)==1) {
      while (digitalRead(A3)==1) {}
      return '2';
      }
    if (digitalRead(A4)==1) {
      while (digitalRead(A4)==1) {}
      return '5';
      }
    if (digitalRead(A5)==1) {
      while (digitalRead(A5)==1) {}
      return '8';
      }
    if (digitalRead(A6)==1) {
      while (digitalRead(A6)==1) {}
      return '0';
      }
    digitalWrite(D1, LOW);
     digitalWrite(D2, HIGH);
    delay(60);
    if (digitalRead(A3)==1) {
      while (digitalRead(A3)==1) {}
      return '3';
      }
    if (digitalRead(A4)==1) {
      while (digitalRead(A4)==1) {}
      return '6';
      }
    if (digitalRead(A5)==1) {
      while (digitalRead(A5)==1) {}
      return '9';
      }
    if (digitalRead(A6)==1) {
      while (digitalRead(A6)==1) {}
      return '#';
      }
    digitalWrite(D2, LOW);
    }
}

void menuDeroulant(void){
  String minutes, heures, temp_str;
  int mesure;
  String menu[]={"XXhXX XXC", "Bonjour", "1. Se connecter", ""}; //stockage du menu qui sera affiché sur l'écran lcd dans un tableau de chaine de caractères
  //Serial.println("entrée menu déroulant");
  DateTime now = myRTC.now(); //récupération de l'heure dans l'horloge
  minutes=now.minute(); //récupération des minutes dans l'heure qui à été récupérée
  if (minutes.length()<2){ //si les minutes ne sont composées que d'un seul caractère, alors on lui ajour un 0 au début
    minutes="0"+minutes;
  }
  heures=now.hour(); //récupération de l'heure
  mesure=analogRead(A2); //lecture de la valeur analogique du capteur de température
  mesure = mesure * (5.0 / 1023.0 * 100.0); //convertion en degrès C
  temp_str=mesure; //ajout de cette valeur dans une chaine de caractères
  menu[0]=" "+heures+"h"+minutes+" "+temp_str+((char)223)+"C"; //réécriture de la première ligne du menu avec les informations récoltées
  if (utilisateur!=-1){ //vérification si un utilisateur est connecté
    menu[1]="Bonjour "+listUtilisateurs[utilisateur]; //si un utilisateur est connecté, on modifie le menu pour qu'il soit plus personnel
    menu[2]="1. Se stationner"; //si un utilisateur est connecté, on affiche la possibilité de se stationner en l'ajoutant au menu
  }
  //affichage du menu de base
  lcd.clear(); //on efface l'affichage de l'écran 
  lcd.print(menu[0]); //on affiche la première ligne du menu à l'écran
  lcd.setCursor(0,1); //on déplace le curseur pour qu'il se trouve en position 0 sur la deuxième ligne
  lcd.print(menu[1]); //on affiche la deuxième ligne du menu
  lcd.setCursor(0,2); //ect
  lcd.print(menu[2]);
  lcd.setCursor(0,3);
  lcd.print(menu[3]);
  //Serial.println("menu affiché");
  while(1){ // on répète indéfiniment
    DateTime now = myRTC.now(); 
    minutes=now.minute();
    if (minutes.length()<2){ //si les minutes ne sont composées que d'un seul caractère, alors on lui ajour un 0 au début
    minutes="0"+minutes;
    }
    heures=now.hour();
    mesure=analogRead(A2);
    mesure = mesure * (5.0 / 1023.0 * 100.0);
    temp_str=mesure;
    menu[0]=heures+"h"+minutes+" "+temp_str+((char)223)+"C"; // char 223 est le caractère '°'
    if (utilisateur!=-1){
      menu[1]="Bonjour "+listUtilisateurs[utilisateur];
      menu[2]="1. Se stationner";
    }
    if (listUtilisateurs[0]=="" && listUtilisateurs[1]=="" && listUtilisateurs[2]=="") menu[3]=""; //on vérifie si un utilisateur est enregistré dans le Smart Disk
    else menu[3]="2. Utilisateurs"; //si un utilisateur est enregistré, on ajoute dans le menu l'onglet de gestion des utilisateurs
    lcd.clear();
    lcd.print(menu[0]);
    lcd.setCursor(0,1);
    lcd.print(menu[1]);
    lcd.setCursor(0,2);
    lcd.print(menu[2]);
    lcd.setCursor(0,3);
    lcd.print(menu[3]);
    // attention, la fonction ne sort que lorsqu'une touche est préssé par l'utilisateur, ce qui fige le Smart Disk
    carac=aquerir(); //on récupère un caractère sur le clavier par l'utilisateur
    switch (carac){
      case '1': //lorsque l'utilisateur à présse la touche 1
                if(utilisateur==-1){ //si aucun utilisateur n'est connecté
                  ////Serial.println("utilisateur non connecté");
                  if (listUtilisateurs[0]=="" && listUtilisateurs[1]=="" && listUtilisateurs[2]==""){ //on regarde si au moins un utilisateur est enregistré dans l'appareil
                    //Serial.println("enregistrement");
                    //si aucun utilisateur n'est enregistré dans le Smart Disk, l'utilisateur est redirigé dans la fonction d'enregistrement
                    enregistrement();
                    //à la sortie de cette fonction, si tout c'est bien passé, l'utilisateur est connecté, on ajoute donc son prénom après le bonjour et l'onglet de stationnement
                    if (utilisateur!=-1){
                      menu[1]="Bonjour "+listUtilisateurs[utilisateur];
                      menu[2]="1. Se stationner";
                    }
                  }
                  //si au moins un utilisateur est enregistré dans le Smart Disk, alors l'utilisateur est redirigé vers le menu de connexion. 
                  else{
                    //Serial.println("connexion");
                    menuConnexion();
                    //Serial.println("sortie connexion");
                    //Serial.print("caractere :");
                    //Serial.println(carac);
                    //après le menu connexion, l'utilisateur à surement fait le choix de se conencter, on vérifier donc si tel est le cas
                    //si oui, on ajoute son prénom au menu et la posibilté de se stationner
                    if (utilisateur!=-1){
                      menu[1]="Bonjour "+listUtilisateurs[utilisateur];
                      menu[2]="1. Se stationner";
                    }
                  }
                }
                else{ //ici un utilisateur est connecté sur la machine, on le redirige donc vers l'onglet de stationnement
                  Serial.println("stationement");
                  stationnement();
                }
              break;

      case '2': //l'utilisateur appuie sur 2
                if (menu[3]!=""){ //si l'indice 3 du menu n'est pas vide, cela signifie qu'il y à au moins un utilisateur d'enregistré dans le Smart Disk
                //on le redirige donc vers le menu de connexion
                  menuUtilistaeurs();
                if (utilisateur==-1){
                      menu[1]="Bonjour ";
                      menu[2]="1. Se connecter";
                    }
                }
              break;
      default : //Serial.println("carractère non reconnu");
              break;
    }
  }
}

void menuConnexion(void){
  if (listUtilisateurs[0]=="" && listUtilisateurs[1]=="" && listUtilisateurs[2]=="") return; //si aucun utilisateur n'est présent dans le, Smart Disk, on quitte la fonction
  String minutes, heures, temp_str;
  String menu[]={"XXhXX XX°C", "Bonjour", "", "", ""}; //tableau de chaine de caractères qui permet d'enregistrer l'affichage du menu
  int mesure;
  DateTime now = myRTC.now();
  minutes=now.minute();
  heures=now.hour();
  mesure=analogRead(A2);
  mesure = mesure * (5.0 / 1023.0 * 100.0);
  temp_str=mesure;
  menu[0]=heures+"h"+minutes+" "+temp_str+((char)223)+"C";
  for (short i=1; i<4; i++){ //on ajoute le numéro d'utilisateur de chaques utilisateurs présent dans le Smart Disk au menu
    if (numeroUtilisateur[i-1]!=""){
      menu[1+i]=String(i)+". "+numeroUtilisateur[i-1];
    }
  }
  char caractere='\0';
  short indice=0;
  while (caractere!='*') { //l'étoile est le caractère de retour
  //on affiche le menu (déroulant ou l'utilisateur peut faire varier indice en pprésant 0 ou 8 afin de modifier l'affichage)
    lcd.clear();
    lcd.print(menu[indice]);
    lcd.setCursor(0,1);
    lcd.print(menu[indice+1]);
    lcd.setCursor(0,2);
    lcd.print(menu[indice+2]);
    lcd.setCursor(0,3);
    lcd.print(menu[indice+3]);
    caractere=aquerir();
    Serial.print("caractere :");
    Serial.println(caractere);
    if (caractere=='0'){ //si l'utilisateur presse 0, on fait déscendre le menu
      indice=1; //ici puisque le menu n'est constitué que de 5 champs et l'écran faisant une taille de 4 lignes, on peut se contenter de faire varier i entre 0 et 1
    }
    else if (caractere=='8'){
      indice=0;
    }
    else if (caractere=='1'){ //l'utilisateur presse 1 pour se connecter au numéro d'utilisateur 1
      if (numeroUtilisateur[0]!=""){ //on vérifier que l'utilisateur d'indice 1 existe
        utilisateur=0;
        break;
      }
    }
    else if (caractere=='2'){ //l'utilisateur presse 1 pour se connecter au numéro d'utilisateur 2
      if (numeroUtilisateur[1]!=""){ //on vérifier que l'utilisateur d'indice 2 existe
        utilisateur=1;
        break;
      }
    }
    else if (caractere=='3'){
      if (numeroUtilisateur[2]!=""){
        utilisateur=2;
        break;
      }
    }
  }
}

void enregistrement(void){
  short i=0;
  char caractere='\0';
  String remoteNum=""; // chaine de caractères qui va permettre d'enregistrer le numéro de téléphone de l'utilisateur
  String prenom=""; // chaine de caractères qui va permettre d'enregistrer le prénom de l'utilisateur après réception du message
  String minutes, heures, temp_str;
  int mesure;
  DateTime now = myRTC.now();
  minutes=now.minute();
  if (minutes.length()<2){ //si les minutes ne sont composées que d'un seul caractère, alors on lui ajour un 0 au début
    minutes="0"+minutes;
  }
  heures=now.hour();
  mesure=analogRead(A2);
  mesure = mesure * (5.0 / 1023.0 * 100.0);
  temp_str=mesure;
  String temp=heures+"h"+minutes+" "+temp_str+((char)223)+"C";
  //Serial.println("enregistremeny fct");
  while(listUtilisateurs[i]!="" && i<3){ //on cherche un emplacement vide dans le tableau d'enregistrement des utilisateurs afin de trouver une place ou sera enregistré ce nouvel utilisateur
    i++;
  }
  if (i>=3){ // si aucune place n'a été trouvé, on affiche une erreur et on sort de la fonction
    lcd.clear();
    lcd.print("Erreur");
    lcd.setCursor(0,1);
    lcd.print("trop util");
    delay(2000);
  }
  else{ // ici une place est disponible dans le tableau d'enregistrement des utilisateurs
    i=0;
    //Serial.println("place");
    lcd.clear();
    lcd.print(temp); //affichage de l'heure et de la température
    lcd.setCursor(0,1);
    lcd.print("num: ");
    while ((caractere!='#')){ //# est le caractère de valdation pour l'utilisateur, donc tant qu'il ne le rpesse pas, on récupère le numéro de téléphone
      caractere = aquerir(); //on récupére les caractères du numéro de téléphone un à un
      if (caractere!='#'){
        if (caractere=='*'){ //* étant à la fois le caractère de retour et de suppression d'erreur, on vérifie la taille du numéro de téléphone qui à été présse, si sa taille est nul alors on quitte la fonction
          if (remoteNum.length()==0) break;
          i--; // si la taille du numéro de téléphone n'est pas nul, alors on enlève le dernier caractère qui à été pressé qui se trouve à l'indice i
          remoteNum.remove(i);
          //on réaffiche le nouveau numéro de téléphone sans oublier l'heure et la température (qui d'ailleur n'est pas actualisé pendant ce procéssus de récupération du numéro de téléphone)
          lcd.clear();
          lcd.print(temp);
          lcd.setCursor(0,1);
          lcd.print("num: ");
          lcd.print(remoteNum);
        }
        else{ //si le caractère n'est ni une étoile ni un dièze, alors il s'agit d'un chiffre qu'on ajoute au reste du numéro de téléphone
          lcd.print(caractere);
          remoteNum+=caractere;
          i++;
        }
      }
      else{
          if (remoteNum.length() != 10){
            caractere='\0';
            lcd.clear();
            lcd.print("numero invalide");
            delay(2000);
            lcd.clear();
            lcd.print(temp);
            lcd.setCursor(0,1);
            lcd.print("num: ");
            lcd.print(remoteNum);
          }

      }
    }
    if (remoteNum.length()==0){ //si l'utilisateur à décidé de quitter, alors la taille du numéro de téléphone est nul, on entre annule la fonction
      lcd.clear();
      lcd.print("annulation"); //affichage de l'annulation
      delay(2000);
    }
    else{ //si la taille du numéro de téléphone n'est pas nul
    //on affiche la bonne validation du numéro de téléphone
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(temp);
      lcd.setCursor(0,1);
      lcd.print("Valide");
      while (sms.available()) sms.flush();
      // on met en place la communication vers le numéro de téléphone de l'utilisateur
      sms.beginSMS(remoteNum.c_str());
      //on écrit le message qui sera envoyé à l'utilisateur
      sms.print("Bonjour, votre numero de telephone a bien ete enregistre dans votre SMART DISK. Veuillez repondre à ce message par votre prenom. Bonne journee");
      // on envoie le message
      sms.endSMS();
      //on affiche à l'écran le bonne envoie du message que l'utilisateur dervait réceptionner
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SMS envoye");
      lcd.setCursor(0,1);
      lcd.print("Attente de rep");
      bool pass=false; //flag qui va nous permettre de savoir si l'utilisateur souhaite que l'on utilise son numéro de téléphone pour l'identifier
      short tempo=0; //compteur qui va nous permettre d'attendre TEMPMAX temps avant de stoppper l'attente de reception du sms de l'utilisateur et qu'on utilise son numéro de téléphone pour l'identifier
      digitalWrite(D0, HIGH); //on met à niveau haut toute la dernière ligne du pavé numérique afin de pouvoir lire si l'utilisateur appuie sur * pour stopper l'attente de réception 
      while (!sms.available() && tempo!=TEMPMAX){ //tant que l'on à pas reçu de message et que on à pas attendu TEMPMAX, on lit le pavé numérique pour savoir si l'utilisateur souhaite passer et on incrément tempo
        if (digitalRead(A6)==1){
          pass=true;
          break;
        }
        delay(1000); 
        tempo++;
      }//Serial.println("attente message"); delay(2000);}
      //Serial.println("message reçu");
      digitalWrite(D0, LOW); //on remet à niveau bas la dernière ligne du pavé numérique
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ok");

      if (tempo==TEMPMAX || pass==true){ //si on à atteint le temps d'attente max ou si l'utilisateur à décider de passer, on utilise le numéro de téléphone entrée par l'utilisateur pour l'identifier
        //on indique à l'utilisateur que l'on va utiliser son numéro de téléphone comme prénom
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("utilisation du");
        lcd.setCursor(0,1);
        lcd.print("num");
        delay(3000);
        prenom=remoteNum; 
      }
      //ici on à donc forcément reçu un message avec le prénom de l'utilisateur
      else{
        int c;
        while((c = sms.read()) != -1 && prenom.length()<=12){ //tant que l'on peut lire des caractères dans le message et que le prénom ne dépasse pas 12 caractères, on continue
          prenom+=((char)c); //on ajoute à prénom les caractères lues dans le message un à un
          //Serial.print((char)c);
        }
        sms.flush(); //on vide le reste du sms afin d'éviter qu'il ne soit lue une prochaine fois par mégarde
      }
      i=0;
      while(listUtilisateurs[i]!="" && i<3){ //on cherche à nouveau l'emplacement où sera enregistré l'utilisateur
        i++;
      }
      //on ajoute au tableaux respectif d'enregistrement le prénom (ou le numéro de téléphone) et le numéro de téléphone de l'utilisateur enregistré
      listUtilisateurs[i]=prenom;
      numeroUtilisateur[i]=remoteNum;
      utilisateur=i; //on connecte l'utilisateur au nouvel utilisateur enregistré
    }
  }
}

void menuUtilistaeurs(void){

  String menu[]={"XXhXX XX°C", "", "", "", "1. Supprimer util", "2. Changer util", ""}; //stockage du menu qui sera affiché sur l'écran lcd dans un tableau de chaine de caractères
  String minutes, heures, temp_str;
  short indice=0, i=0;
  char caractere='\0';
  int mesure;
  DateTime now = myRTC.now();
  minutes=now.minute();
  if (minutes.length()<2){ //si les minutes ne sont composées que d'un seul caractère, alors on lui ajour un 0 au début
    minutes="0"+minutes;
  }
  heures=now.hour();
  mesure=analogRead(A2);
  mesure = mesure * (5.0 / 1023.0 * 100.0);
  temp_str=mesure;
  menu[0]=heures+"h"+minutes+" "+temp_str+((char)223)+"C";
  for (short i=0; i<3; i++){
    //si un emplacement d'enregistrement d'utilisateur est libre, on ajoute au menu l'onglet qui permet d'en ajouter un
    if (listUtilisateurs[i]=="") menu[6]="3. Ajouter un util";
    //on ajoute au menu le numéro de tout les utilisateurs enregistrés dans le Smart Disk
    menu[1+i]=numeroUtilisateur[i];
  }
  if (utilisateur!=-1) menu[1+utilisateur]+='*'; //si un utilisateur est enregistré, on ajoute une étoile à coté de son numéro dans le menu
  while (caractere!='*'){ //tant que l'utilisateur ne quitte pas en appyant sur *
    lcd.clear();
    lcd.print(menu[indice]);
    lcd.setCursor(0,1);
    lcd.print(menu[indice+1]);
    lcd.setCursor(0,2);
    lcd.print(menu[indice+2]);
    lcd.setCursor(0,3);
    lcd.print(menu[indice+3]);
    caractere = aquerir(); //on récupère une saisie au pavé numérique
    switch (caractere){
    case '1': //si l'utilisateur presse 1
            supprimerUtilistaeurs(); //affichage de l'onglet de suppréssion d'un utilisateur
            for (short i=0; i<3; i++){ //on vérifie si un emplacement est libre dans le tableau d'enregistrement des utilisateurs afin d'ajouter l'onglet qui permet d'en enregistrer un nouveau
            if (listUtilisateurs[i]=="") menu[6]="3. Ajouter un util";
            //on actualise la liste des utilisateurs présents sur le Smart Disk
            menu[1+i]=numeroUtilisateur[i]; // si l'utilisateur connecté n'as pas été supprimer, on ajoute une étoile à coté de son numéro de téléphone dans le menu
            }
            if (utilisateur!=-1) menu[1+utilisateur]+='*';
             break;
    case '2': //si l'utilisateur presse 2
            menuConnexion(); //on affiche le menu de connexion qui permet à l'utilisateur de se connecter
            for (short i=0; i<3; i++) menu[1+i]=numeroUtilisateur[i]; //on actualise les numéros des utilisateurs présent dans le Smart Disk
            if (utilisateur!=-1) menu[1+utilisateur]+='*'; //on ajoute l'étoile à coté du numéro d'utilisateur auquel l'utilisateur s'estb connecté (ou pas si il ne s'est connecté à aucun utilisateur)
             break;
    case '3': //si l'utilisateur presse 3
            //on vérifie si un emplécement est libre sur le Smart Disk pour acceuillir un nouvel utilisateur en vérifiant si le menu "3. Ajouter un util" est présent
            if(menu[6]!="") enregistrement(); //si un emplacement est libre, l'utilisateur est donc renvoyer vers la fonction d'enregistrement d'un nouvel utilisateur
            for (short i=0; i<3; i++) menu[1+i]=numeroUtilisateur[i]; //on actualise les numéros des utilisateurs présent dans le Smart Disk
            if (utilisateur!=-1) menu[1+utilisateur]+='*'; //on ajoute l'étoile à coté du numéro d'utilisateur auquel l'utilisateur s'estb connecté (ou pas si il ne s'est connecté à aucun utilisateur)
             break;
    case '0': //si l'utilisateur presse 0
             indice++; //on fait défiler le menu vers le bas en augmentant l'indice 
             if (indice>=4) indice=0; //si on sort du menu (taille de 7 lignes), on retourne à la première ligne
             break;
    case '8': //si l'utilisateur presse 8
             indice--; //on fait défiler le menu vers le haut en diminuant l'indice 
             if (indice <0) indice=3; //si on sort du menu, on retourne à la dernière ligne
             break; 
    default ://Serial.println("carractère non reconnu");
             break;
    }
  }
}

void supprimerUtilistaeurs(void){
  String menu[]={"Suppression", "", "", ""}; //stockage du menu qui sera affiché sur l'écran lcd dans un tableau de chaine de caractères
  char caractere='\0';
  int temp=0;
  for (short i=1; i<4; i++){ //on ajoute la liste des utilisateur au menu
    menu[i]=String(i)+". "+numeroUtilisateur[i-1];
  }
  while (caractere!='*'){ //tant que l'utilisateur ne presse pas * pour quitter
    for (short i=1; i<4; i++){ //on actualise la liste des utilisateurs au cas ou l'utilisateur en supprime un
      menu[i]=String(i)+". "+numeroUtilisateur[i-1];
    }
    //on affiche le menu
    lcd.clear(); 
    lcd.print(menu[0]);
    lcd.setCursor(0,1);
    lcd.print(menu[1]);
    lcd.setCursor(0,2);
    lcd.print(menu[2]);
    lcd.setCursor(0,3);
    lcd.print(menu[3]);
    caractere = aquerir(); //on récupère la saisie de l'utilisateur
    switch (caractere){
    case '1': 
    case '2':
    case '3': //si l'utilisateur appuie sur un nombre
              temp=atoi(&caractere);
              temp-=1; //on ajuste la saisie pour qu'elle puisse être utiliser comme indice du tableau d'enregistrement des utilisateurs
              listUtilisateurs[temp]=""; //on met la chaine vide à l'indice correspond à l'utilisateur que l'utilisateur souhaite supprimer
              numeroUtilisateur[temp]=""; //on met la chaine vide à l'indice correspond au numéro de l'utilisateur que l'utilisateur souhaite supprimer
             if (utilisateur==temp) utilisateur= -1; //si l'utilisateur était connecté à l'utilisateur supprimé on le déconnecte
             caractere='\0';
             break;

    default ://Serial.println("carractère non reconnu");
             break;
    }
  }
}

//TODO: commenter le dessous

void stationnement(void){
  String duree="", heure, minutes, secondes, temp_str;
  short sec=0, i=0;
  int temps, mesure;
  int min;
  char caractere='1';
  lcd.clear();
  lcd.print("Duree autorisee:");
  lcd.setCursor(0,1);
  lcd.print(" minutes");
  while (caractere!='\0'){ //tant que l'utilisateur ne quitte pas (si l'utilisateur presse * on change le caractère par le caractère nul si la longueur du temps de stationnement est null pour ne pas confondre avec un retour de saisie)
    caractere = aquerir(); // on récupère la saisie de l'utilisateur
    if (caractere!='#'){ //si l'utilisateur n'as pas saisie valider
      if (caractere=='*'){ //si la saisie est *
        i--; 
        if (duree.length()==0) caractere='\0'; //si la longueur du temps de stationnement est nulle on remplace le caractère par '\0' pour quitter
        else{
          duree.remove(i); //si la chaine n'est pas vide on retire le dernier caractère saisie par l'utilisateur
          lcd.clear(); //on réecrit ce qui à été saisie par l'utilisateur
          lcd.print("Duree autorisee:");
          lcd.setCursor(0,1);
          lcd.print(duree);
          lcd.print(" minutes");
        } 
      }
      else{ //si l'utilisateur à appuyé sur un chiffre
        duree+=caractere; //on ajoute le chiffre à la saisie de l'utilisateur
        if (duree.toInt()>90){ //si la saisie dépasse 90 on la remplace par 90 (temps max de stationnement 1h30)
          duree="90";
        }
        if (duree.length()==2 && duree.toInt()<15){ //si la durée est composé de deux chiffre mais est plus petite que 15, on la remplace par 15 (temps min de stationnement 15 min)
          duree="15";
        }
        lcd.setCursor(0,1); //on réecrit la saisie de l'utilisateur
        lcd.print(duree);
        lcd.print(" minutes");
        i++;
      }
    }
    else if (duree.length()==2 ) caractere='\0'; //si l'utilisateur à saisie la touche de validation on vérifie que la saisie est composé de deux caractères, si ce n'est pas le cas on ne valide pas
  }
  if (caractere!='*'){ //si la dernière saisie de l'utilisateur n'as pas été un retour
    caractere!='\0';

    DateTime now = myRTC.now(); //on actualise l'heure
    minutes=now.minute();
    secondes=now.second();
    heure=now.hour();
    temps=duree.toInt();
    min=duree.toInt();
    if (minutes.length()==1) minutes="0"+minutes; 
    lcd.clear();
    lcd.print(heure); //on affiche l'heure
    lcd.print("h");
    lcd.print(minutes);
    mesure=analogRead(A2);
    mesure = mesure * (5.0 / 1023.0 * 100.0);
    temp_str=mesure;
    temp_str=" "+temp_str+((char)223)+"C";
    lcd.print(temp_str); //on affiche la température
    lcd.setCursor(0,1); //on affiche le temps de stationnement restant
    lcd.print("il reste ");
    lcd.print(min);
    lcd.print(" minutes");
    digitalWrite(D0, HIGH); //on met à niveau haut la dernière ligne du pavé numérique afin de tester si l'utilisateur appuie sur * pour quitter le mode de stationnement
    while (min != 0 && digitalRead(A6)!=1){ //temps que le temps de stationnement ne touche pas à sa fin ou que l'utilisateur n'appuie pas sur * pour quitter

      delay(960); //on attend 0.96s
      sec++;
      if (sec==60){ //chaque 60s on diminue le temps de stationnement restant
        min--;
        sec=0; //on remet les secondes à 0
        now = myRTC.now(); //on actualise l'heure
        heure=now.hour();
        minutes=now.minute();
        secondes=now.second();
        if (minutes.length()==1) minutes="0"+minutes;
        lcd.clear();
        lcd.print(heure);
        lcd.print("h");
        lcd.print(minutes);
        mesure=analogRead(A2); //on actualise la température
        mesure = mesure * (5.0 / 1023.0 * 100.0);
        temp_str=mesure;
        temp_str=" "+temp_str+((char)223)+"C";
        lcd.print(temp_str); //on actualise le temps de stationnement restant
        lcd.setCursor(0,1);
        lcd.print("il reste ");
        lcd.print(min);
        lcd.print(" minutes");
      }
      

      if (((min==20) || (min==5)) && (sec == 10)){ //si il reste 5 ou 20 min de temps de stationnement, on enevoie un message pour en avertir l'utilisateur
        sms.beginSMS(numeroUtilisateur[utilisateur].c_str()); //on récupère le numéro de l'utilisateur connecté grace à numeroUtilisateur[utilisateur].c_str()
        sms.print("Bonjour "); 
        sms.print(listUtilisateurs[utilisateur].c_str()); //on récupère le prénom de l'utilisateur connecté grace à listUtilisateurs[utilisateur].c_str()
        sms.print(", il ne vous reste plus que ");
        sms.print(min); // on indique le temps de stationnement restant(donc 5 ou 20 min)
        sms.print(" minutes de temps de stationnement autorise.");
        sms.endSMS();
      }
    }

    if (min == 0){ //si le temps de stationnement à touché à sa fin on envoie un message à l'utilisateur pour l'en informer
      sms.beginSMS(numeroUtilisateur[utilisateur].c_str());
      sms.print("Bonjour ");
      sms.print(listUtilisateurs[utilisateur].c_str());
      sms.print(" votre temps de stationnement est fini.");
      sms.endSMS();
    }
    else{ //si l'utilisateur à décidé de quitter le mode de stationnement
      lcd.clear();
      lcd.print("annulation");
      lcd.setCursor(0,1);
      lcd.print("stationnement");
      delay(2000);
    }
    digitalWrite(D0, LOW); //on remet à niveau bas la dernière ligne du clavier 
  }
}


void initSMS(void){

  bool connected = false;
  lcd.clear();
  lcd.print("Initialisation");
  lcd.setCursor(0,1);
  lcd.print("de la SIM");
  //Serial.println("menu affiché");
  
  while (!connected) {
    if (gsmAccess.begin("1234") == GSM_READY) {
      connected = true;
    } else {
      delay(1000);
    }
  }
  
}