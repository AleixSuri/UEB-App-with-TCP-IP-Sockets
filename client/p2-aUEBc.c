/**************************************************************************/
/*                                                                        */
/* L'aplicació UEB amb sockets TCP/IP                                     */
/* Fitxer aUEBc.c que implementa la capa d'aplicació de UEB, sobre la     */
/* cap de transport TCP (fent crides a la "nova" interfície de la         */
/* capa TCP o "nova" interfície de sockets TCP), en la part client.       */
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
#include "p2-aUEBc.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int ConstiEnvMis(int SckCon, const char *tipus, const char *info1, int long1);
int RepiDesconstMis(int SckCon, char *tipus, char *info1, int *long1);

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int UEBc_FuncioExterna(arg1, arg2...) { }  */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa UEB, en la part client.                */



/* Crea un socket TCP "client" en una @IP i #port TCP local qualsevol, a  */
/* través del qual demana una connexió a un S UEB que escolta peticions   */
/* al socket TCP "servidor" d'@IP "IPser" i #portTCP "portTCPser".        */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "IPser" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket TCP connectat si tot va bé;                */
/* -1 si hi ha un error a la interfície de sockets.                       */
int UEBc_DemanaConnexio(const char *IPser, int portTCPser, char *TextRes)
{
    int CodiRes;
	int sck = TCP_CreaSockClient("0.0.0.0", 0);
    
    if(sck == -1){
        sprintf(TextRes, "TCP_CreaSockClient(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    } 
    
    int res = TCP_DemanaConnexio(sck, IPser, portTCPser);
    if(res == -1){
        sprintf(TextRes, "TCP_DemanaConnexio(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }

    sprintf(TextRes, "Socket creat amb èxit\0");
    return sck;
}

/* Obté el fitxer de nom "NomFitx" del S UEB a través de la connexió TCP  */
/* d'identificador "SckCon". Escriu els bytes del fitxer a "Fitx" i la    */
/* longitud del fitxer en bytes a "LongFitx".                             */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "NomFitx" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud <= 10000 chars (incloent '\0').                   */
/* "Fitx" és un vector de chars (bytes) qualsevol (recordeu que en C,     */
/* un char és un enter de 8 bits) d'una longitud <= 9999 bytes.           */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si el fitxer existeix al servidor;                                  */
/*  1 la petició és ERRònia (p.e., el fitxer no existeix);                */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio, etc.); */
/* -3 si l'altra part tanca la connexió.                                  */
int UEBc_ObteFitxer(int SckCon, const char *NomFitx, char *Fitx, int *LongFitx, char *TextRes)
{
	int CodiRes;
    
    // Envia la petició amb el tipus "OBT" per demanar el fitxer
    int res = ConstiEnvMis(SckCon, "OBT", NomFitx, strlen(NomFitx));
    if(res == -1){
        sprintf(TextRes, "TCP_Envia(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }
    else if(res == -2){
        sprintf(TextRes, "Protocol incorrecte.");
        return -2;
    }

    char *tipus;
    char *info1;
    int *long1;
    int res2 = RepiDesconstMis(SckCon, tipus, info1, long1);
    if(res2 == -1){
        sprintf(TextRes, "TCP_Envia(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }
    else if(res2 == -2){
        sprintf(TextRes, "Protocol incorrecte.");
        return -2;
    }
    else if(res2 == -3){
        sprintf(TextRes, "El servidor ha tancat la connexió.");
        return -3;
    }
    
    memcpy(Fitx, info1, *long1);
    *LongFitx = *long1;
    if (strcmp(tipus, "COR") == 0) {
        // El fitxer existeix
        sprintf(TextRes, "Tot bé, el fitxer existeix al servidor\0");
        return 0;
    }
    else{
        // El fitxer no existeix al servidor
        sprintf(TextRes, "Tot bé, el fitxer no existeix al servidor\0");
        return 1;
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
int UEBc_TancaConnexio(int SckCon, char *TextRes)
{
    int CodiRes;
    int res = TCP_TancaSock(SckCon);
	if(res == -1){
        sprintf(TextRes, "TCP_TancaSock(): %s", T_ObteTextRes(&CodiRes));
        return -1;
    }

    sprintf(TextRes, "Socket client tancat correctament\0");
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
int UEBc_TrobaAdrSckConnexio(int SckCon, char *IPloc, int *portTCPloc, char *IPrem, int *portTCPrem, char *TextRes)
{
    int CodiRes;
    char ipRem[16];
    char ipLoc[16];
    int res1 = TCP_TrobaAdrSockLoc(SckCon, ipLoc, portTCPloc);
    int res2 = TCP_TrobaAdrSockRem(SckCon, ipRem, portTCPrem);

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
/* int UEBc_FuncioExterna(arg1, arg2...)
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

    if((campTipus != 3) || (long1 > 9999) || (strcmp(tipus, "OBT") != 0)){
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
/*  0 si tot va bé;                                                       */
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

        if((LongSeqBytes < 8) || (strcmp(tipus, "OBT") == 0) || ((LongSeqBytes - 7) != *long1)){
            return -2;
        }
        else{
            memcpy(info1, SeqBytes+7, *long1);
            return 0;
        } 
    }
}
