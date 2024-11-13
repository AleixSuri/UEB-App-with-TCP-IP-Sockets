/**************************************************************************/
/*                                                                        */
/* L'aplicació ECO amb sockets TCP/IP                                     */
/* Fitxer capçalera de tTCP.c                                             */
/*                                                                        */
/* Autors: Arnau Herrera i Aleix Suriñach                                 */
/* Data: octubre 2024                                                     */
/*                                                                        */
/**************************************************************************/

int TCP_CreaSockClient(const char *IPloc, int portTCPloc);
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc);
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem);
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem);
int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes);
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes);
int TCP_TancaSock(int Sck);
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc);
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem);
char *T_ObteTextRes(int *CodiRes);
