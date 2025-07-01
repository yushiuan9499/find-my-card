#ifndef BOX_H
#define BOX_H

#include "Core/JvTime.h"
#include "Core/Labeled_GPS.h"
#include <map>

class Card;
class Server;

class Box : public Core {
private:
protected:
  struct Session {
    JvTime lastActive;
    string username = "";
    string passwd = "";
    void clear();
  };
  // id --> card mapping
  std::map<std::string, Card *> cards;
  // GPS location of the box
  Labeled_GPS gps;
  Server *server; // Pointer to the server for communication
  Session sess;

  /**
   * @brief get current session and check session is still valid
   * @return current session
   */
  const Session &getSession();

public:
  Box(Server *server, const Labeled_GPS &gpsLocation);
  Box(Server *server, Json::Value *arg_json_ptr);
  Box() = default;
  virtual ~Box();

  /**
   * @brief login the session
   * @param username: username to login
   * @param passwd: password, not necessary when addCard
   * @return nickname of user
   */
  virtual string login(const string &username, const string &passwd = "");

  /**
   * @brief put a card into the box
   * @param card: pointer to the card to be added
   * @return nullptr if the card add successfully
   *         otherwise, return the card itself
   */
  virtual Card *addCard(Card *card);

  /**
   * @brief retrieve a card from the box
   * @param cardId: the id of the card to be retrieved
   * @param verificationCode:
   * @param card: pointer to the card for payment
   * @return: pointer to the card if found, nullptr if not found or error occurs
   */
  virtual Card *retrieveCard(const std::string &cardId, int verificationCode,
                             Card *card = nullptr);

  /**
   * @brief get the GPS location of the box
   * @return: Labeled_GPS object representing the GPS location of the box
   */
  Labeled_GPS getGPSLocation() const;

  /**
   * @brief redeem a reward for a user
   * @param amount: the amount of reward to redeem, -1 for all available
   * @param card: pointer to the card for receive reward
   * @return the reward balance after redemption, or -1 if the user is invalid
   */
  virtual int redeemReward(int amount, Card *card);

  virtual Json::Value *dump2JSON(void) const override;
  virtual void JSON2Object(const Json::Value *arg_json_ptr) override;
};

#endif // BOX_H
