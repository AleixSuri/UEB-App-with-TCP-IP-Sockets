/**************************************************************************/
/*                                                                        */
/* L'aplicació UEB amb sockets TCP/IP                                     */
/* Fitxer serUEB.c que implementa la interfície aplicació-administrador   */
/* d'un servidor de l'aplicació UEB, sobre la capa d'aplicació de         */
/* (la part servidora de) UEB (fent crides a la interfície de la part     */
/* servidora de la capa UEB).                                             */
/*                                                                        */
/* Autors: Arnau Herrera i Aleix Suriñach                                 */
/* Data: novembre 2024                                                    */
/*                                                                        */
/**************************************************************************/

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include "p2-aUEBs.h"


int obreLog();
int llegeixConfiguracio(int *portTCP);
int iniciaServidor(int *sck, int portTCP, int fileLog);
int acceptaConnexio(int sck, int fileLog);
void serveixPeticions(int sckCon, int fileLog);


int main(int argc, char *argv[])
{
    int fileLog = obreLog();
    int portTCP;
    if (llegeixConfiguracio(&portTCP) == -1) {
        dprintf(fileLog, "Error en el fitxer de configuració.\n");
        close(fileLog);
        exit(-1);
    }

    int sck;
    if (iniciaServidor(&sck, portTCP, fileLog) == -1) {
        close(fileLog);
        exit(-1);
    }

    while (1) {
        int sckCon = acceptaConnexio(sck, fileLog);
        if (sckCon != -1) {
            serveixPeticions(sckCon, fileLog);
        }
    }
    
    close(fileLog);
}



/* 
    Funció: obreLog
    Propòsit: Obre el fitxer de log per escriure-hi. 
    Arguments: Cap
    Retorn: Retorna el descriptor del fitxer o -1 en cas d'error.
 */
int obreLog() {
    int fileLog = open("serUEB.log", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fileLog == -1) {
        perror("\nError al obrir el fitxer per desar-lo.");
    }
    return fileLog;
}

/* 
    Funció: llegeixConfiguracio
    Propòsit: Llegeix el fitxer de configuració per obtenir el port TCP.
    Arguments: Punter a la variable portTCP.
    Retorn: Retorna 0 si té èxit i -1 si hi ha error.
 */
int llegeixConfiguracio(int *portTCP) {
    FILE *fp = fopen("p2-serUEB.cfg", "r");
    if (fp == NULL) {
        perror("Error en obrir el fitxer de configuració.\n");
        return -1;
    }
    char linia[50], opcio[20], valor[20];
    while (fgets(linia, sizeof(linia), fp) != NULL) {
        if (sscanf(linia, "%s %s", opcio, valor) == 2 && strcmp(opcio, "#portTCP") == 0) {
            *portTCP = atoi(valor);
        }
    }
    fclose(fp);
    return 0;
}

/* 
    Funció: iniciaServidor
    Propòsit: Inicia el servidor amb el port TCP llegit.
    Arguments: Punter al socket, port TCP, i descriptor del fitxer de log.
    Retorn: Retorna 0 si té èxit i -1 si hi ha error.
 */
int iniciaServidor(int *sck, int portTCP, int fileLog) {
    char TextRes[300];
    if (UEBs_IniciaServ(sck, portTCP, TextRes) == -1) {
        printf("UEBs_IniciaServ(): %s\n", TextRes);
        dprintf(fileLog, "UEBs_IniciaServ(): %s\n", TextRes);
        return -1;
    }
    printf("%s\n\n", TextRes);
    return 0;
}

/* 
    Funció: acceptaConnexio
    Propòsit: Accepta una connexió entrant.
    Arguments: Socket de servidor i descriptor de log.
    Retorn: Retorna el socket de connexió o -1 en cas d'error.
 */
int acceptaConnexio(int sck, int fileLog) {
    char TextRes[300];
    int sckCon = UEBs_AcceptaConnexio(sck, TextRes);
    if (sckCon == -1) {
        printf("UEBs_AcceptaConnexio(): %s\n", TextRes);
        dprintf(fileLog, "UEBs_AcceptaConnexio(): %s\n", TextRes);
    }
    return sckCon;
}

/* 
    Funció: serveixPeticions
    Propòsit: Serveix peticions del client connectat.
    Arguments: Socket de connexió i descriptor de log.
    Retorn: Cap
 */
void serveixPeticions(int sckCon, int fileLog) {
    char TextRes[300], TextTemps[300], NomFitx[200], TipusPeticio[4];
    while (1) {
        int servPet = UEBs_ServeixPeticio(sckCon, TipusPeticio, NomFitx, TextRes, TextTemps);
        if (servPet == -3) break;  // El client tanca connexió

        if (servPet == -1 || servPet == -2) {
            printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
            dprintf(fileLog, "UEBc_ServeixPeticio(): %s\n\n", TextRes);
            break;
        }

        printf("Petició %s del fitxer %s\n", TipusPeticio, NomFitx);
        dprintf(fileLog, "Petició %s del fitxer %s\n", TipusPeticio, NomFitx);

        if (servPet == 0 || servPet == 1 || servPet == -4) {
            printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
            dprintf(fileLog, "UEBc_ServeixPeticio(): %s\n", TextRes);
            printf("%s\n", TextTemps);
            dprintf(fileLog, "%s\n", TextTemps);
        }

        memset(NomFitx, 0, sizeof(NomFitx));
        memset(TipusPeticio, 0, sizeof(TipusPeticio));
    }

    printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
    dprintf(fileLog, "UEBc_ServeixPeticio(): %s\n\n", TextRes);
}
