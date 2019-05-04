#ifndef _ClientTCP_H_
#define _ClientTCP_H_

// Classe ClientTCP
// Modification d'un code C++ disponible sur:
// https://www.binarytides.com/code-a-simple-socket-client-class-in-c/ 
// O. DARTOIS - le 21/10/18

#include <sys/socket.h> // socket, recv, send
#include <netinet/in.h> // sockadd_in, htonl
#include <arpa/inet.h>  // inet_addr
#include <unistd.h>     // close
#include <string>

class ClientTCP
{
private:
    int sock;
    std::string ipServerAddress;
    int portServer;
    struct sockaddr_in server;
    std::string msgErreur;
     
public:
    ClientTCP();
    bool connection(std::string ipServer, int port);
    bool envoieDonnees(std::string data);
    bool receptionDonnees(int sizeMaxData, std::string& rep);
    bool fermeture();
    std::string recupMessageErreur();
};

#endif