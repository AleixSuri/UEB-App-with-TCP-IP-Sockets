/**************************************************************************/
/*                                                                        */
/* L'aplicació ECO amb sockets TCP/IP                                     */
/* Fitxer tTCP.c que "implementa" la capa de transport TCP, o més         */
/* ben dit, que encapsula les funcions de la interfície de sockets        */
/* TCP, en unes altres funcions més simples i entenedores: la "nova"      */
/* interfície de sockets TCP.                                             */
/*                                                                        */
/* Autors: Arnau Herrera i Aleix Suriñach                                 */
/* Data: octubre 2024                                                     */
/*                                                                        */
/**************************************************************************/

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "p2-tTCP.h"


/* Crea un socket TCP “client” a l’@IP “IPloc” i #port TCP “portTCPloc”   */
/* (si “IPloc” és “0.0.0.0” i/o “portTCPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port TCP, respectivament).      */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket creat si tot va bé;                        */
/* -1 si hi ha error.                                                     */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc)
{
	struct sockaddr_in adrloc;
	int i;
	int scon = socket(AF_INET,SOCK_STREAM,0);
	
	adrloc.sin_family=AF_INET;
	adrloc.sin_port=htons(portTCPloc);
	adrloc.sin_addr.s_addr=inet_addr(IPloc);
	for(i=0;i<8;i++){adrloc.sin_zero[i]='\0';}
	if(bind(scon,(struct sockaddr*)&adrloc,sizeof(adrloc))==-1)
	{
		scon = -1;
	}
	
	return scon;
}

/* Crea un socket TCP “servidor” (o en estat d’escolta – listen –) a      */
/* l’@IP “IPloc” i #port TCP “portTCPloc” (si “IPloc” és “0.0.0.0” i/o    */
/* “portTCPloc” és 0 es fa una assignació implícita de l’@IP i/o del      */
/* #port TCP, respectivament).                                            */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket creat si tot va bé;                        */
/* -1 si hi ha error.                                                     */
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc)
{
	int i;
	struct sockaddr_in adrloc;
	int sesc = socket(AF_INET,SOCK_STREAM,0);
		
	adrloc.sin_family=AF_INET;
	adrloc.sin_port=htons(portTCPloc);
	adrloc.sin_addr.s_addr=inet_addr(IPloc);
	for(i=0;i<8;i++){adrloc.sin_zero[i]='\0';}

	if(bind(sesc,(struct sockaddr*)&adrloc,sizeof(adrloc)))
	{
		sesc = -1;
	}
	
	if(listen(sesc,3)==-1)
	{
		sesc = -1;
	}
 
	return sesc;
}

/* El socket TCP “client” d’identificador “Sck” es connecta al socket     */
/* TCP “servidor” d’@IP “IPrem” i #port TCP “portTCPrem” (si tot va bé    */
/* es diu que el socket “Sck” passa a l’estat “connectat” o establert     */
/* – established –). Recordeu que això vol dir que s’estableix una        */
/* connexió TCP (les dues entitats TCP s’intercanvien missatges           */
/* d’establiment de la connexió).                                         */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem)
{
	int i;
	struct sockaddr_in adrrem;
	adrrem.sin_family=AF_INET;
	adrrem.sin_port=htons(portTCPrem);
	adrrem.sin_addr.s_addr=inet_addr(IPrem);
	for(i=0;i<8;i++){adrrem.sin_zero[i]='\0';}

	int connexio = connect(Sck,(struct sockaddr*)&adrrem,sizeof(adrrem));
	
	return connexio;
}

/* El socket TCP “servidor” d’identificador “Sck” accepta fer una         */
/* connexió amb un socket TCP “client” remot, i crea un “nou” socket,     */
/* que és el que es farà servir per enviar i rebre dades a través de la   */
/* connexió (es diu que aquest nou socket es troba en l’estat “connectat” */
/* o establert – established –; el nou socket té la mateixa adreça que    */
/* “Sck”).                                                                */
/*                                                                        */
/* Omple “IPrem” i “portTCPrem” amb respectivament, l’@IP i el #port      */
/* TCP del socket remot amb qui s’ha establert la connexió.               */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket connectat creat si tot va bé;              */
/* -1 si hi ha error.                                                     */
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem)
{
	socklen_t long_adrrem = sizeof(struct sockaddr_in);
	struct sockaddr_in adrrem;
	int scon = accept(Sck,(struct sockaddr*)&adrrem, &long_adrrem);
 
	IPrem = inet_ntoa(adrrem.sin_addr);
	*portTCPrem = ntohs(adrrem.sin_port);
	
	return scon;
}

/* Envia a través del socket TCP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket TCP remot amb qui està connectat.                 */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna:                                                               */
/*  el nombre de bytes enviats si tot va bé;                              */
/* -1 si hi ha error.                                                     */
int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes)
{
	return write(Sck, SeqBytes, LongSeqBytes);
}

/* Rep a través del socket TCP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes” (que té una longitud de “LongSeqBytes” bytes), */
/* o bé detecta que la connexió amb el socket remot ha estat tancada.     */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna:                                                               */
/*  el nombre de bytes rebuts si tot va bé;                               */
/*  0 si la connexió està tancada;                                        */
/* -1 si hi ha error.                                                     */
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes)
{
	return read(Sck, SeqBytes, LongSeqBytes);
}

/* S’allibera (s’esborra) el socket TCP d’identificador “Sck”; si “Sck”   */
/* està connectat es tanca la connexió TCP que té establerta.             */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_TancaSock(int Sck)
{
	return close(Sck);
}

/* Donat el socket TCP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc” i “portTCPloc” amb respectivament, la seva     */
/* @IP i #port TCP.                                                       */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc)
{
	socklen_t long_addrloc = sizeof(struct sockaddr_in);
	struct sockaddr_in addrloc;
	
	int res = getsockname(Sck, (struct sockaddr *)&addrloc, &long_addrloc);
	
	strcpy(IPloc, inet_ntoa(addrloc.sin_addr));
	*portTCPloc = ntohs(addrloc.sin_port);
	
	return res;
}

/* Donat el socket TCP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem” i             */
/* “portTCPrem*” amb respectivament, la seva @IP i #port TCP.             */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem)
{
	socklen_t long_addrem = sizeof(struct sockaddr_in);
	struct sockaddr_in addrem;
	
	int res = getpeername(Sck, (struct sockaddr *)&addrem, &long_addrem);
	
	strcpy(IPrem, inet_ntoa(addrem.sin_addr));
	*portTCPrem = ntohs(addrem.sin_port);
	
	return res;
}

/* Obté un missatge de text de l'S.O. que descriu l'error produït en      */
/* la darrera crida de sockets TCP, i omple "CodiRes" amb la variable     */
/* errno de l'S.O., un codi identificador d'aquest missatge de text       */
/*                                                                        */
/* Retorna:                                                               */
/*  aquest missatge de text en un "string" de C (vector de chars          */
/*  imprimibles acabat en '\0').                                          */
char* T_ObteTextRes(int *CodiRes)
{
	*CodiRes= errno;
	return strerror(errno);
} 