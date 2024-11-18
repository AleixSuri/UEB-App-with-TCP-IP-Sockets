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

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#include "p2-aUEBc.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */
#define MAX_FITX  10000

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int main(int argc,char *argv[])
{
 /* Declaració de variables, p.e., int n;                                 */
    int op;
    char TextRes[300];
    int sckCon;

    // Variables per mesurar el temps
    struct timespec start, end;


 /* Expressions, estructures de control, crides a funcions, etc.          */
    printf("Vols sortir o fer una petició? (0-Sortir, 1-Obtenir):\n");
    scanf("%d", &op);
    if(op != 0 && op == 1){
        // Obtenir informacio de peticio
        TextRes[0] = '\0';
        char IPser[16];
        int portTCPser;
        char NomFitx[200]; 
        char Fitx[MAX_FITX];
        int LongFitx;

        printf("@IPservidor:\n");
        scanf("%s", IPser);
        printf("#portTCPservidor:\n");
        scanf("%d", &portTCPser);

        if(portTCPser == 0){
            portTCPser = 6000;
        }

        printf("nom_fitxer:\n");
        scanf("%s", NomFitx);

        // Demana connexio
        if((sckCon = UEBc_DemanaConnexio(IPser, portTCPser, TextRes)) == -1){
            printf("UEBc_DemanaConnexio(): %s\n", TextRes);
            exit(-1);
        }

        while(op != 0){
            // Mostrar @sockets
            char IPloc[16];
            int portTCPloc;
            char IPrem[16];
            int portTCPrem;
            if(UEBc_TrobaAdrSckConnexio(sckCon, IPloc, &portTCPloc, IPrem, &portTCPrem, TextRes) == -1){
                printf("UEBc_TrobaAdrSckConnexio(): %s\n", TextRes);
                exit(-1);
            }
            printf("\nPetició OBT, @IP:#portTCP servidor %s:%d, fitxer %s\n", IPser, portTCPser, NomFitx);
            printf("Connexió TCP @sck cli %s:%d @sck ser %s:%d\n", IPloc, portTCPloc, IPrem, portTCPrem);

            // Obtenir fitxer
            gettimeofday(&start, NULL);
            char nomAux[200];
            memset(nomAux, 0, sizeof(nomAux));
            memcpy(nomAux, NomFitx, (int) strlen(NomFitx));
            int obtFit = UEBc_ObteFitxer(sckCon, nomAux, Fitx, &LongFitx, TextRes);
            if(obtFit == -1 || obtFit == -2){
                printf("UEBc_ObteFitxer(): %s\n", TextRes);
                exit(-1);
            }
            else if(obtFit == -3){
                printf("UEBc_ObteFitxer(): %s\n", TextRes);
                exit(0);
            }
            
            printf("UEBc_ObteFitxer(): %s\n\n", TextRes);
            
            // Mostrar fitxer
            if(write(1, Fitx, LongFitx) == -1){
                perror("Error en mostrar el contingut del fitxer.");
                exit(-1);
            }

            gettimeofday(&end, NULL);
            long seconds = end.tv_sec - start.tv_sec;
            long nanoseconds = end.tv_nsec - start.tv_nsec;
            double elapsed = seconds + nanoseconds*1e-9;

            // Guardar fitxer
            if(obtFit == 0){
                char nomFitxer[200];
                strcpy(nomFitxer, &NomFitx[1]); //treure '/' NomFitxer
                int file = open(nomFitxer, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if(file == -1){
                    perror("\nError al obrir el fitxer per desar-lo.");
                    exit(-1);
                }
                
                int n = write(file, Fitx, LongFitx);
                if(n == -1){
                    perror("\nError al escriure el fitxer per desar-lo.");
                    close(file);
                    exit(-1);
                }

                if(close(file) == -1){
                    perror("\nError al tancar el fitxer desat.");
                    exit(-1);
                }
            }

            // Mostrar el temps
            printf("\n\nTemps de resposta: %.9f segons \n", elapsed);

            // Tornar a demanar?
            printf("\n\nVols obtenir un fitxer o acabar? (1-Obtenir, 0-Acabar):\n");
            scanf("%d", &op); 
            if(op != 0 && op == 1){
                // Netejar variables i demanar informacio
                char IPserAux[16];
                strcpy(IPserAux, IPser);
                int portTCPserAux = portTCPser;

                memset(TextRes, 0, sizeof(TextRes));
                memset(IPser, 0, sizeof(IPser));
                memset(NomFitx, 0, sizeof(NomFitx));
                memset(Fitx, 0, sizeof(Fitx));
                 
                printf("@IPservidor:\n");
                scanf("%s", IPser);
                printf("#portTCPservidor:\n");
                scanf("%d", &portTCPser);

                if(portTCPser == 0){
                    portTCPser = 6000;
                }

                printf("nom_fitxer:\n");
                scanf("%s", NomFitx);

                // Si es vol conectar a un altre servidor...
                if((strcmp(IPser, IPserAux) != 0) || (portTCPser != portTCPserAux)){
                    if(UEBc_TancaConnexio(sckCon, TextRes) == -1){
                        printf("UEBc_TancaConnexio(): %s", TextRes);
                        exit(-1);
                    }

                    if((sckCon = UEBc_DemanaConnexio(IPser, portTCPser, TextRes)) == -1){
                        printf("UEBc_DemanaConnexio(): %s\n", TextRes);
                        exit(-1);
                    }
                }
            }
        }
        // Tanca connexio
        if(UEBc_TancaConnexio(sckCon, TextRes) == -1){
            printf("UEBc_TancaConnexio(): %s", TextRes);
            exit(-1);
        }
    }
}

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es troben */
/* a l'inici d'aquest fitxer.                                             */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/*int FuncioInterna(arg1, arg2...)
{
	
} */

