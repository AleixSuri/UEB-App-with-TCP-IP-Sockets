/**************************************************************************/
/*                                                                        */
/* L'aplicació UEB amb sockets TCP/IP                                     */
/* Fitxer aUEB.c que implementa la capa d'aplicació de UEB, sobre la      */
/* cap de transport TCP (fent crides a la "nova" interfície de la         */
/* capa TCP o "nova" interfície de sockets TCP), en la part servidora.    */
/*                                                                        */
/* Autors: Arnau Herrera i Aleix Suriñach                                 */
/* Data: novembre 2024                                                    */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "p2-tTCP.h"
#include "p2-aUEBs.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int ConstiEnvMis(int SckCon, const char *tipus, const char *info1, int long1);
int RepiDesconstMis(int SckCon, char *tipus, char *info1, int *long1);

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int UEBs_FuncioExterna(arg1, arg2...) { }  */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa UEB, en la part servidora.             */



/* Inicia el S UEB: crea un nou socket TCP "servidor" a una @IP local     */
/* qualsevol i al #port TCP “portTCPser”. Escriu l'identificador del      */
/* socket creat a "SckEsc".                                               */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha un error en la interfície de sockets.                      */
int UEBs_IniciaServ(int *SckEsc, int portTCPser, char *TextRes)
{
    int CodiRes;
	*SckEsc = TCP_CreaSockServidor("0.0.0.0", portTCPser);
    if(*SckEsc == -1){
        sprintf(TextRes, "TCP_CreaSockServidor(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }

    sprintf(TextRes, "Socket servidor creat correctament\0");
    return *SckEsc;
}

/* Accepta una connexió d'un C UEB que arriba a través del socket TCP     */
/* "servidor" d'identificador "SckEsc".                                   */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket TCP connectat si tot va bé;                */
/* -1 si hi ha un error a la interfície de sockets.                       */
int UEBs_AcceptaConnexio(int SckEsc, char *TextRes)
{
    int CodiRes;
    char ipRem[16];
    int portRem;
    int res = TCP_AcceptaConnexio(SckEsc,ipRem, &portRem);
    if(res == -1){
        sprintf(TextRes, "TCP_AcceptaConnexio(): %s", T_ObteTextRes(&CodiRes));
        TCP_TancaSock(res);
        return -1;
    }
    
    sprintf(TextRes, "Connexió establerta amb èxit\0");
    return res;
}

/* Serveix una petició UEB d'un C a través de la connexió TCP             */
/* d'identificador "SckCon". A "TipusPeticio" hi escriu el tipus de       */
/* petició (p.e., OBT) i a "NomFitx" el nom del fitxer de la petició.     */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "TipusPeticio" és un "string" de C (vector de chars imprimibles acabat */
/* en '\0') d'una longitud de 4 chars (incloent '\0').                    */
/* "NomFitx" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud <= 10000 chars (incloent '\0').                   */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si el fitxer existeix al servidor;                                  */
/*  1 la petició és ERRònia (p.e., el fitxer no existeix);                */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio, etc.); */
/* -3 si l'altra part tanca la connexió;                                  */
/* -4 si hi ha problemes amb el fitxer de la petició (p.e., nomfitxer no  */
/*  comença per /, fitxer no es pot llegir, fitxer massa gran, etc.).     */
int UEBs_ServeixPeticio(int SckCon, char *TipusPeticio, char *NomFitx, char *TextRes)
{
    int CodiRes;
    char *info1; //nom fitxer
    int *long1; //mida caracters nom fitxer
    
    int res = RepiDesconstMis(SckCon, TipusPeticio, info1, long1);
    if(res == -1){
        sprintf(TextRes, "TCP_Rep(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }
    else if(res == -2){
        sprintf(TextRes, "Protocol incorrecte.");
        return -2;
    }
    else if(res == -3){
        sprintf(TextRes, "El client ha tancat la connexió.");
        return -3;
    }

    //Buscar i treure informació del NomFitxer rebut
    int file, bytesFitxer;
    char nomfitxer[200], contingutFitxer[10000];
    file = open(nomfitxer, O_RDONLY);
    bytesFitxer = read(file, contingutFitxer, 10000);

    if(info1[0] != '/'){
        sprintf(TextRes, "El nom del fitxer ha de començar amb '/'");
        return -4;
    }
    if(bytesFitxer == -1){
        sprintf(TextRes, "Fitxer no es pot llegir correctament.");
        return -4;
    }
    if(bytesFitxer == 10000){
        sprintf(TextRes, "Mida del fitxer més gran de la permesa.");
        return -4;
    }

    char tipusEnv[3]; 
    int longEnv;
    if(file == -1 ){ //fitxer no existeix
        memcpy(tipusEnv, "ERR", 3);
        longEnv = 18;
        char infoEnv[longEnv];
        memcpy(infoEnv, "1 fitxer no trobat", longEnv);

        ConstiEnvMis(SckCon, tipusEnv, infoEnv, longEnv);
        sprintf(TextRes, "Tot bé, fitxer no existeix.");
        return 1;
    }
    else{
        memcpy(tipusEnv, "COR", 3);
        longEnv = bytesFitxer-1; // -1 per treure EOF
        char infoEnv[longEnv];
        memcpy(infoEnv, contingutFitxer, longEnv);

        int res2 = ConstiEnvMis(SckCon, tipusEnv, infoEnv, longEnv);
        if(res2 == -1){
            sprintf(TextRes, "TCP_Envia(): %s", T_ObteTextRes(&CodiRes));
            return -1;
        }
        else if(res2 == -2){
            sprintf(TextRes, "Protocol incorrecte.");
            return -2;
        }
        else{
	        sprintf(TextRes, "Tot bé, el fitxer existeix al servidor.");
            return 0;
        }
    }
}

/* Tanca la connexió TCP d'identificador "SckCon".                        */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*   0 si tot va bé;                                                      */
/*  -1 si hi ha un error a la interfície de sockets.                      */
int UEBs_TancaConnexio(int SckCon, char *TextRes)
{
    int CodiRes;
	int res = TCP_TancaSock(SckCon);
	if(res == -1){
        sprintf(TextRes, "TCP_TancaSock(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }

    strcpy(TextRes, "Socket servidor tancat correctament\0");
    return res;
}

/* Donat el socket TCP “connectat” d’identificador “SckCon”, troba        */
/* l’adreça del socket local, omplint “IPloc” i “portTCPloc” amb          */
/* respectivament, la seva @IP i #port TCP, i troba l'adreça del socket   */
/* remot amb qui està connectat, omplint “IPrem” i  “portTCPrem” amb      */
/* respectivament, la seva @IP i #port TCP.                               */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "IPloc" i "IPrem" són "strings" de C (vector de chars imprimibles      */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*   0 si tot va bé;                                                      */
/*  -1 si hi ha un error a la interfície de sockets.                      */
int UEBs_TrobaAdrSckConnexio(int SckCon, char *IPloc, int *portTCPloc, char *IPrem, int *portTCPrem, char *TextRes)
{
    int CodiRes;
    int res1 = TCP_TrobaAdrSockLoc(SckCon, IPloc, portTCPloc);
    int res2 = TCP_TrobaAdrSockRem(SckCon, IPrem, portTCPrem);

    if((res1 == -1)){
        sprintf(TextRes, "TCP_TrobaAdrSockLoc(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }

    if((res2 == -1)){
        sprintf(TextRes, "TCP_TrobaAdrSockRem(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }

    sprintf(TextRes, "Tot bé\0");
    return res1;
}


/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int UEBs_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */



/* "Construeix" un missatge de PUEB a partir dels seus camps tipus,       */
/* long1 i info1, escrits, respectivament a "tipus", "long1" i "info1"    */
/* (que té una longitud de "long1" bytes), i l'envia a través del         */
/* socket TCP “connectat” d’identificador “SckCon”.                       */
/*                                                                        */
/* "tipus" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud de 4 chars (incloent '\0').                       */
/* "info1" és un vector de chars (bytes) qualsevol (recordeu que en C,    */
/* un char és un enter de 8 bits) d'una longitud <= 9999 bytes.           */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio).       */
int ConstiEnvMis(int SckCon, const char *tipus, const char *info1, int long1)
{
	char SeqBytes[10006];
    int LongSeqBytes = 0;

    int campTipus = strlen(tipus);

    if((campTipus != 3) || (long1 > 9999) || (strcmp(tipus, "OBT") == 0)){
        return -2;
    }

    char longAux[5];
    sprintf(longAux, "%.4d", long1);

    strcat(SeqBytes, tipus);
    strcat(SeqBytes, longAux);
    memcpy(SeqBytes+7, info1, long1);
    LongSeqBytes = 7 + long1;

    if(TCP_Envia(SckCon, SeqBytes, LongSeqBytes) == -1){
        return -1;
    }

    return 0;
}   

/* Rep a través del socket TCP “connectat” d’identificador “SckCon” un    */
/* missatge de PUEB i el "desconstrueix", és a dir, obté els seus camps   */
/* tipus, long1 i info1, que escriu, respectivament a "tipus", "long1"    */
/* i "info1" (que té una longitud de "long1" bytes).                      */
/*                                                                        */
/* "tipus" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud de 4 chars (incloent '\0').                       */
/* "info1" és un vector de chars (bytes) qualsevol (recordeu que en C,    */
/* un char és un enter de 8 bits) d'una longitud <= 9999 bytes.           */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé,                                                       */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio);       */
/* -3 si l'altra part tanca la connexió.                                  */
int RepiDesconstMis(int SckCon, char *tipus, char *info1, int *long1)
{
	char SeqBytes[10006];
    int LongSeqBytes;
    int res = TCP_Rep(SckCon, SeqBytes, LongSeqBytes);
    
    if(res == -1){
        return -1;
    }
    else if(res == 0){
        return -3;  
    }
    else{
        memcpy(tipus, SeqBytes, 3);
        tipus[3] = '\0';

        char longAux[5];
        memcpy(longAux, SeqBytes+3, 4);
        longAux[4] = '\0';
        *long1 = atoi(longAux);

        if((LongSeqBytes < 8) || (strcmp(tipus, "OBT") != 0) || ((LongSeqBytes - 7) != *long1)){
            return -2;
        }
        else{
            memcpy(info1, SeqBytes+7, *long1);
            return 0;
        } 
    }    
}