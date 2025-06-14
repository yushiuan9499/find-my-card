#ifndef BOX_H
#define BOX_H

#include "Core/Labeled_GPS.h"
#include <map>

class Card;
class Server;

class Box : public Core {
private:
  // id --> card mapping
  std::map<std::string, Card *> cards;
  // GPS location of the box
  Labeled_GPS gps;
  Server *server; // Pointer to the server for communication

protected:
public:
  Box(Server *server, const Labeled_GPS &gpsLocation);
  Box(Server *server, Json::Value *arg_json_ptr);
  virtual ~Box();

  /**
   * @brief put a card into the box
   * @param card: pointer to the card to be added
   * @return nullptr if the card add successfully
   *         otherwise, return the card itself
   */
  Card *addCard(Card *card, const string &username = "");

  /**
   * @brief retrieve a card from the box
   * @param usrName: the username of the user who wants to retrieve the card
   * @param cardId: the id of the card to be retrieved
   * @param passwd: the password of the user
   * @param card: pointer to the card for payment
   * @return: pointer to the card if found, nullptr if not found or error occurs
   */
  Card *retrieveCard(const std::string &usrName, const std::string &cardId,
                     const std::string &passwd, Card *card = nullptr);
  /**
   * @brief get the GPS location of the box
   * @return: Labeled_GPS object representing the GPS location of the box
   */
  Labeled_GPS getGPSLocation() const;

  /**
   * @brief redeem a reward for a user
   * @param username: the username of the user
   * @param password: the password of the user
   * @param amount: the amount of reward to redeem, -1 for all available
   * @param card: pointer to the card for payment, if any
   * @return the reward balance after redemption, or -1 if the user is invalid
   */
  int redeemReward(const std::string &username, const std::string &password,
                   int amount, Card *card);

  virtual Json::Value *dump2JSON(void) const override;
  virtual void JSON2Object(Json::Value *arg_json_ptr) override;
};

#endif // BOX_H
