#ifndef BOX_H
#define BOX_H

#include "Core/Labeled_GPS.h"
#include <map>

class Card;
class Server;

class Box {
private:
  // id --> card mapping
  std::map<std::string, Card *> cards;
  // GPS location of the box
  Labeled_GPS gps;
  Server *server; // Pointer to the server for communication

protected:
public:
  Box(const Labeled_GPS &gpsLocation);
  virtual ~Box();

  /**
   * @brief put a card into the box
   * @param card: pointer to the card to be added
   * @return nullptr if the card add successfully
   *         otherwise, return the card itself
   */
  Card *addCard(Card *card);

  /**
   * @brief retrieve a card from the box
   * @param usrName: the username of the user who wants to retrieve the card
   * @param cardId: the id of the card to be retrieved
   * @param passwd: the password of the user
   * @return: pointer to the card if found, nullptr if not found or error occurs
   */
  Card *retrieveCard(const std::string &usrName, const std::string &cardId,
                     const std::string &passwd);
  /**
   * @brief get the GPS location of the box
   * @return: Labeled_GPS object representing the GPS location of the box
   */
  Labeled_GPS getGPSLocation() const;
};

#endif // BOX_H
