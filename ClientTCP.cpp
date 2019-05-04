// Code C++ pour ClientTCP
// O. DARTOIS - le 21/10/2018

#include "ClientTCP.h"

ClientTCP::ClientTCP()
{
    sock = -1;
    portServer = 0;
    ipServerAddress = "";
}

bool ClientTCP::connection(std::string ipServeur, int port)
{
    this->ipServerAddress = ipServeur;
    this->portServer = port;
    //creation de la socket si elle n'a pas d�j� �t� cr��e
	if (sock == -1)
	{
		//Creation de la socket
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1) {
                    msgErreur = "Erreur creation socket";
                    return false;
                }
	}
	
	if (inet_pton(AF_INET, ipServerAddress.c_str() ,&server.sin_addr.s_addr) != 1) {
            msgErreur = "Erreur verification adresse ip";
            return false;
        }
	server.sin_family = AF_INET;
	server.sin_port = htons(portServer);

	//Connection au serveur distant
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            msgErreur = "Erreur de connection";
            return false;
        }
	return true;
}

bool ClientTCP::envoieDonnees(std::string data)
{
    if( send(sock , data.c_str() , data.size() , 0) < 0)
        return false;
    return true;
}

 bool ClientTCP::receptionDonnees(int sizeMaxData, std::string& rep)
{
    char buffer[sizeMaxData];
    int nbCharReceive = 0;
    
    //Reception de la réponse d'un serveur
    if( (nbCharReceive = recv(sock , buffer , sizeof(buffer) , 0)) < 0)
        return false;
    buffer[nbCharReceive] = '\0';
    
    rep = std::string(buffer);
   
    return true;
}

bool ClientTCP::fermeture()
{
    if (close(sock) < 0)
        return false;
    return true;
}

std::string ClientTCP::recupMessageErreur() {
    std::string temp = msgErreur;
    msgErreur.clear();
    return temp;
}