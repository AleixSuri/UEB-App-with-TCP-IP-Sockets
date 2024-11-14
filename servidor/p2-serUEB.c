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
    
    int portTCP;
    int sck;
    FILE *fp;
    char linia[50], opcio[20], valor[20];

 /* Expressions, estructures de control, crides a funcions, etc.          */
    fp = fopen("p2-serUEB.cfg", "r");
    if (fp == NULL) {
        perror("Error en obrir el fitxer de configuració.\n");
        exit(-1);
    }

    if(fgets(linia, sizeof(linia), fp) == NULL){
        perror("Error en llegir linia a cfg.\n");
        exit(-1);
    }
    if(sscanf(linia, "%s %s", opcio, valor) != 2){
        perror("Format del fitxer cfg incorrecte.\n");
        exit(-1);
    }

    //verificar que cap socket esta connectat en aquest port si no deixar portTCP a 0
    portTCP = atoi(valor);
    if(UEBs_IniciaServ(&sck, portTCP, TextRes) == -1){
        printf("UEBs_IniciaServ(): %s\n", TextRes);
        exit(-1);
    }

    printf("Servidor UEB iniciat al #portTCP: %d\n\n", portTCP);
    
    while(1){
        int sckCon = UEBs_AcceptaConnexio(sck, TextRes);
        if(sckCon == -1){
            printf("UEBs_AcceptaConnexio(): %s\n", TextRes);
            exit(-1);
        }
        
        char IPloc[16];
        int portTCPloc;
        char IPrem[16];
        int portTCPrem;
        
        if(UEBs_TrobaAdrSckConnexio(sckCon, IPloc, &portTCPloc, IPrem, &portTCPrem, TextRes) == -1){
            printf("UEBc_TrobaAdrSckConnexio(): %s\n", TextRes);
            exit(-1);
        }
        printf("Connexió TCP @sck ser %s:%d @sck cli %s:%d\n", IPloc, portTCPloc, IPrem, portTCPrem);
        
        char NomFitx[200];
        char TipusPeticio[4];
        int servPet = UEBs_ServeixPeticio(sckCon, TipusPeticio, NomFitx, TextRes);
        
        printf("Petició %s del fitxer %s ", TipusPeticio, NomFitx);
        while(servPet != -3){
            if(servPet == -1 || servPet == -2 || servPet == -4){
                printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
                exit(-1);
            }

            if(servPet == 0 || servPet == 1){
                printf("UEBc_ServeixPeticio(): %s\n\n", TextRes);
            }
            
            servPet = UEBs_ServeixPeticio(sckCon, TipusPeticio, NomFitx, TextRes);
            printf("Petició OBT del fitxer %s\n", NomFitx);        
        }

        printf("UEBc_ServeixPeticio(): %s\n", TextRes);
    }
}

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es troben */
/* a l'inici d'aquest fitxer.                                             */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/*int FuncioInterna(arg1, arg2...)
{
	
} */

