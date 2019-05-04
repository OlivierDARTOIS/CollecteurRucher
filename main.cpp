/* 
 * File:   main.cpp
 * Lecture d'un fichier de réception d'un sms de gammu au format 'detail'
 * extraction des données UUEncodée
 * Author: Olivier DARTOIS
 *
 * Created on 31 mars 2019, 15:46
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "ClientTCP.h"


// Fonction tokenize -> découpage de chaines suivant un caractère
// Decompose une chaine (string) en un vecteur de sous-chaines en
// fonction d'un caractère clé
void tokenize(const std::string& str,
    std::vector<std::string>& tokens,
    const std::string& delimiters)
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// Fonction decode base64
// Site: https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;

void build_decoding_table() {

    decoding_table = reinterpret_cast<char *>(malloc(256));

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {

    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = reinterpret_cast<unsigned char *>(malloc(*output_length));
    if (decoded_data == NULL) return NULL;

    for (unsigned int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(data[i++])];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(data[i++])];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(data[i++])];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(data[i++])];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

void base64_cleanup() {
    free(decoding_table);
}
/*****************************************************************************/

// Programme principale
int main(int argc, char** argv) {

    std::cout << "-- Debut programme" << std::endl;
    if (argc == 1) {
        std::cout << "syntaxe: " << argv[0] << " fichier_sms_gammu_format_detail" << std::endl;
        return 0;
    }
    
    std::string nomFich(argv[1]);
    std::ifstream fichSMS(nomFich);
    
    if (!fichSMS.is_open()) {
        std::cout << "Fichier inexistant" << std::endl;
        return 0;
    }
    
    std::string txtSMS;
    std::string ligne;
    std::string data;
    bool message = false;
    bool premiereLigne = true;
    
    while (getline(fichSMS,ligne)) {
        if (message) {
            if (ligne.at(0) == ';') {
                if (ligne.size()>2) {
                    txtSMS += ligne.substr(2, ligne.size()-2);
                    if (premiereLigne) {
                        txtSMS += "\n";
                        premiereLigne = false;
                    }
                }
            }
            else
                message = false;
        }
        if (ligne.substr(0,10) == "[SMSBackup")
            message = true;
    }
    fichSMS.close();
    
    // Decodage si SMS contient une image webp UUencodée
    if (txtSMS.substr(0,12) == "begin-base64") {
        std::cout << "Photo UUEncode" << std::endl;
        txtSMS = txtSMS.substr(0, txtSMS.size()-4); // suppression quatre = à la fin
        size_t posDepart = txtSMS.find("\n"); // fin de la première ligne
        std::string nomFichWebP = txtSMS.substr(17, posDepart-17); // recupération du nom de fichier
        data += txtSMS.substr(posDepart+1,txtSMS.size()); // les données
        size_t longueur;
        
        // decodage des données 
        unsigned char* pData = base64_decode(data.data(), data.size(), &longueur);
        if (pData == NULL)
            std::cout << "Pb décodage des données" << std::endl;
        else
        {
            std::string cheminSauv = "/usr/share/grafana/public/img/";
            std::ofstream photo(cheminSauv+nomFichWebP, std::ios::binary);
            if (photo.is_open()) {
                for (unsigned int i=0; i<longueur; i++)
                    photo << pData[i];
                photo.close();
            }
            else {
                std::cout << "Ecriture du fichier impossible. Pb de droits ?" << std::endl;
            }
        }
    }
    else {
        std::cout << "Message SMS Ruche" << std::endl;
        size_t posDepart = txtSMS.find("\n"); // fin de la première ligne
        std::string nomRucher = txtSMS.substr(0,posDepart);
        std::string donnees = txtSMS.substr(posDepart+1, txtSMS.size());
        
        std::vector<std::string> champsDonnees;
        tokenize(donnees, champsDonnees, ";");
        
        std::string dateHeureMesureRucher = nomRucher + " dateHeure=\"" + champsDonnees.at(0) + "\"";
        
        ClientTCP* clientTCP = new ClientTCP();
        clientTCP->connection("127.0.0.1", 55555);
        clientTCP->envoieDonnees(dateHeureMesureRucher);
        clientTCP->fermeture();
        delete(clientTCP);
        std::cout << dateHeureMesureRucher << std::endl;
        
        const int nbDonneesRuche = 5;
        int nbRuches = ((champsDonnees.size() - 1) / nbDonneesRuche); 
        for (int i=0; i < nbRuches; i++) {
            std::string donneesRucheXRucher = 
                nomRucher + "-" + champsDonnees.at(i*nbDonneesRuche+1) + " ";
            donneesRucheXRucher += "temperature=" + champsDonnees.at(i*nbDonneesRuche+2) + ",";
            donneesRucheXRucher += "humidite=" + champsDonnees.at(i*nbDonneesRuche+3) + ",";
            donneesRucheXRucher += "pression=" + champsDonnees.at(i*nbDonneesRuche+4) + ",";
            donneesRucheXRucher += "masse=" + champsDonnees.at(i*nbDonneesRuche+5);
            std::cout << donneesRucheXRucher << std::endl;
            // Attention pas de vérification des codes retours ci-dessous
            ClientTCP* clientTCP = new ClientTCP();
            clientTCP->connection("127.0.0.1", 55555);
            clientTCP->envoieDonnees(donneesRucheXRucher);
            clientTCP->fermeture();
            delete(clientTCP);
        }
    }
    
    base64_cleanup();
    
    std::cout << "-- Fin programme" << std::endl;
    return 0;
}

