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

 /* Expressions, estructures de control, crides a funcions, etc.          */
    
    printf("Vols sortir o fer una petició? (0-Sortir, 1-Obtenir):\n");
    scanf("%d", &op);
    if(op != 0){
        while(op != 0){
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

            if((sckCon = UEBc_DemanaConnexio(IPser, portTCPser, TextRes)) == -1){
                printf("UEBc_DemanaConnexio(): %s\n", TextRes);
                exit(-1);
            }

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

            
            int obtFit = UEBc_ObteFitxer(sckCon, NomFitx, Fitx, &LongFitx, TextRes);
            if(obtFit == -1 || obtFit == -2){
                printf("UEBc_ObteFitxer(): %s\n", TextRes);
                exit(-1);
            }
            else if(obtFit == -3){
                printf("UEBc_ObteFitxer(): %s\n", TextRes);
                exit(0);
            }
            else{
                printf("UEBc_ObteFitxer(): %s\n\n", TextRes);
                
                if(write(0, Fitx, LongFitx) == -1){
                    perror("Error en mostrar el contingut del fitxer.");
                    exit(-1);
                }

                if(obtFit == 0){

                    int file = open(NomFitx, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if(file == -1){
                        perror("Error al obrir el fitxer per desar-lo.");
                        exit(-1);
                    }
                    
                    int n = write(file, Fitx, LongFitx);
                    if(n == -1){
                        perror("Error al escriure el fitxer per desar-lo.");
                        close(file);
                        exit(-1);
                    }

                    if(close(file) == -1){
                        perror("Error al tancar el fitxer desat.");
                        exit(-1);
                    }
                }
            } 

            printf("\n\nVols obtenir un fitxer o acabar? (1-Obtenir, 0-Acabar):\n");
            scanf("%d", &op);
        }

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

