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

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#include "p2-aUEBs.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int main(int argc,char *argv[])
{
 /* Declaració de variables, p.e., int n;                                 */
    char TextRes[300];
    int sck;
    FILE *fp;
    char linia[50], opcio[20], valor[20];
    int portTCP;
    int fileLog;
    // char arrelUEB[200] = {0};

 /* Expressions, estructures de control, crides a funcions, etc.          */
    
    // Obrir fitxer serUEB.log
    int fileLog = open("serUEB.log", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fileLog == -1){
        perror("\nError al obrir el fitxer per desar-lo.");
        exit(-1);
    }

    // Llegir fitxer config
    fp = fopen("p2-serUEB.cfg", "r");
    if(fp == NULL){
        perror("Error en obrir el fitxer de configuració.\n");
        exit(-1);
    }
    while(fgets(linia, sizeof(linia), fp) != NULL){
        if (sscanf(linia, "%s %s", opcio, valor) != 2) {
            printf("Format del fitxer cfg incorrecte.\n");
            dprintf(fileLog, "Format del fitxer cfg incorrecte.\n");
            exit(-1);
        }

        // Comprovar l'opció que estem llegint
        if (strcmp(opcio, "#portTCP") == 0) {
            portTCP = atoi(valor);
        } 
        // else if (strcmp(opcio, "#Arrel") == 0) {
        //     strncpy(arrelUEB, valor, sizeof(arrelUEB) - 1);
        // }
    }

    // Inicia Servidor
    if(UEBs_IniciaServ(&sck, portTCP, TextRes) == -1){
        printf("UEBs_IniciaServ(): %s\n", TextRes);
        dprintf(fileLog, "UEBs_IniciaServ(): %s\n", TextRes);
        exit(-1);
    }
    printf("%s\n\n", TextRes);
    
    // Bucle per esperar i rebre peticions
    while(1){
        // Accepta connexio
        int sckCon = UEBs_AcceptaConnexio(sck, TextRes);
        if(sckCon == -1){
            printf("UEBs_AcceptaConnexio(): %s\n", TextRes);
            dprintf(fileLog, "UEBs_AcceptaConnexio(): %s\n", TextRes);
            exit(-1);
        }
        
        // Mostrar @sockets
        char IPloc[16];
        int portTCPloc;
        char IPrem[16];
        int portTCPrem;
        
        if(UEBs_TrobaAdrSckConnexio(sckCon, IPloc, &portTCPloc, IPrem, &portTCPrem, TextRes) == -1){
            printf("UEBc_TrobaAdrSckConnexio(): %s\n", TextRes);
            dprintf(fileLog, "UEBc_TrobaAdrSckConnexio(): %s\n", TextRes);
            exit(-1);
        }
        printf("Connexió TCP @sck ser %s:%d @sck cli %s:%d\n", IPloc, portTCPloc, IPrem, portTCPrem);
        dprintf(fileLog, "Connexió TCP @sck ser %s:%d @sck cli %s:%d\n", IPloc, portTCPloc, IPrem, portTCPrem);
        
        // Servir peticio
        char NomFitx[200];
        char TipusPeticio[4];
        int servPet = UEBs_ServeixPeticio(sckCon, TipusPeticio, NomFitx, TextRes);
        while(servPet != -3){
            // Client tanca connexio
            if(servPet != -3){
                printf("Petició %s del fitxer %s\n", TipusPeticio, NomFitx);
                dprintf(fileLog, "Petició %s del fitxer %s\n", TipusPeticio, NomFitx);
            }

            // Errors
            if(servPet == -1 || servPet == -2 || servPet == -4){
                printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
                dprintf(fileLog, "UEBc_ServeixPeticio(): %s\n\n", TextRes);
                exit(-1);
            }

            // Tot correcte
            if(servPet == 0 || servPet == 1){
                printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
                dprintf(fileLog, "UEBc_ServeixPeticio(): %s\n\n", TextRes);
            }
        
            // Netejar variables i servir peticio
            memset(NomFitx, 0, sizeof(NomFitx));
            memset(TipusPeticio, 0, sizeof(TipusPeticio));
            servPet = UEBs_ServeixPeticio(sckCon, TipusPeticio, NomFitx, TextRes);
        }

        // Client tanca connexio
        printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
        dprintf(fileLog, "UEBc_ServeixPeticio(): %s\n\n", TextRes);
    }
    close(fileLog);
}

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es troben */
/* a l'inici d'aquest fitxer.                                             */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/*int FuncioInterna(arg1, arg2...)
{
	
} */

