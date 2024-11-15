/**************************************************************************/
/*                                                                        */
/* L'aplicació UEB amb sockets TCP/IP                                     */
/* Fitxer cliUEB.c que implementa la interfície aplicació-usuari          */
/* d'un client de l'aplicació UEB, sobre la capa d'aplicació de           */
/* (la part client de) UEB (fent crides a la interfície de la part        */
/* client de la capa UEB).                                                */
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

#include "p2-aUEBc.h"

#define MAX_FITX  10000


void demanaInformacioServidor(char *IPser, int *portTCPser, char *NomFitx);
int connectaServidor(const char *IPser, int portTCPser, char *TextRes);
int obtFitxerDelServidor(int sckCon, const char *NomFitx, char *TextRes);
void desaFitxer(const char *NomFitx, const char *Fitx, int LongFitx);


int main(int argc, char *argv[]) {
    int op;
    char TextRes[300];
    int sckCon;

    printf("Vols sortir o fer una petició? (0-Sortir, 1-Obtenir):\n");
    scanf("%d", &op);

    if (op != 0 && op == 1) {
        char IPser[16];
        int portTCPser;
        char NomFitx[200];
        
        demanaInformacioServidor(IPser, &portTCPser, NomFitx);
        
        sckCon = connectaServidor(IPser, portTCPser, TextRes);
        if (sckCon == -1) exit(-1);

        while (op != 0) {
            printf("\nPetició OBT, @IP:#portTCP servidor %s:%d, fitxer %s\n", IPser, portTCPser, NomFitx);
            
            if (obtFitxerDelServidor(sckCon, NomFitx, TextRes) == -1) exit(-1);
            
            desaFitxer(NomFitx, TextRes, strlen(TextRes));
            
            printf("\n\nVols obtenir un fitxer o acabar? (1-Obtenir, 0-Acabar):\n");
            scanf("%d", &op);

            if (op == 1) {
                demanaInformacioServidor(IPser, &portTCPser, NomFitx);

                if (UEBc_TancaConnexio(sckCon, TextRes) == -1) {
                    printf("UEBc_TancaConnexio(): %s", TextRes);
                    exit(-1);
                }
                
                sckCon = connectaServidor(IPser, portTCPser, TextRes);
                if (sckCon == -1) exit(-1);
            }
        }

        if (UEBc_TancaConnexio(sckCon, TextRes) == -1) {
            printf("UEBc_TancaConnexio(): %s", TextRes);
            exit(-1);
        }
    }
}



/* Funció que sol·licita la informació del servidor (IP i port) i del fitxer.
   Arguments: char *IPser, int *portTCPser, char *NomFitx - on es desarà la informació.
   Retorna: res. */
void demanaInformacioServidor(char *IPser, int *portTCPser, char *NomFitx) {
    printf("@IPservidor:\n");
    scanf("%s", IPser);
    printf("#portTCPservidor:\n");
    scanf("%d", portTCPser);

    if (*portTCPser == 0) {
        *portTCPser = 6000;
    }

    printf("nom_fitxer:\n");
    scanf("%s", NomFitx);
}

/* Funció que es connecta al servidor UEB.
   Arguments: char *IPser, int portTCPser, char *TextRes.
   Retorna: l'identificador del socket de connexió o -1 en cas d'error. */
int connectaServidor(const char *IPser, int portTCPser, char *TextRes) {
    int sckCon = UEBc_DemanaConnexio(IPser, portTCPser, TextRes);
    if (sckCon == -1) {
        printf("UEBc_DemanaConnexio(): %s\n", TextRes);
    }
    return sckCon;
}

/* Funció que obté i mostra el fitxer del servidor.
   Arguments: int sckCon, const char *NomFitx, char *TextRes.
   Retorna: 0 si tot va bé, -1 si hi ha error en la connexió o es tanca la connexió. */
int obtFitxerDelServidor(int sckCon, const char *NomFitx, char *TextRes) {
    char Fitx[MAX_FITX];
    int LongFitx;
    struct timeval start, end;

    gettimeofday(&start, NULL);

    int obtFit = UEBc_ObteFitxer(sckCon, NomFitx, Fitx, &LongFitx, TextRes);
    if (obtFit == -1 || obtFit == -2) {
        printf("UEBc_ObteFitxer(): %s\n", TextRes);
        return -1;
    } else if (obtFit == -3) {
        printf("UEBc_ObteFitxer(): %s\n", TextRes);
        return -1;
    }

    printf("UEBc_ObteFitxer(): %s\n\n", TextRes);

    if (write(1, Fitx, LongFitx) == -1) {
        perror("Error en mostrar el contingut del fitxer.");
        return -1;
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6;
    printf("\n\nTemps de resposta: %.9f segons \n", elapsed);

    return 0;
}

/* Funció que desa el fitxer rebut en disc.
   Arguments: const char *NomFitx, const char *Fitx, int LongFitx.
   Retorna: res. */
void desaFitxer(const char *NomFitx, const char *Fitx, int LongFitx) {
    char nomFitxer[200];
    strcpy(nomFitxer, &NomFitx[1]);  // treure '/' de NomFitx

    int file = open(nomFitxer, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file == -1) {
        perror("\nError al obrir el fitxer per desar-lo.");
        exit(-1);
    }

    if (write(file, Fitx, LongFitx) == -1) {
        perror("\nError al escriure el fitxer per desar-lo.");
        close(file);
        exit(-1);
    }

    if (close(file) == -1) {
        perror("\nError al tancar el fitxer desat.");
        exit(-1);
    }
}