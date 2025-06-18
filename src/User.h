#ifndef USER_H
#define USER_H

#include "Core/Core.h"
#include "Server.h"
#include <map>
#include <set>
#include <string>

class Card;
class Box;
class EmailServer;
class App2FA;

class User : public Core {
private:
  std::string username;
  std::string emailPasswd;
  std::string passwd; // Password for the user
  std::string email;
  std::map<std::string, Card *> cards; // id -> card owned by the user
  std::map<std::string, int>
      verificationCodes;    // id -> verification code for the card
  EmailServer *emailServer; // Pointer to the email server for communication
  Server *server;           // Pointer to the server for user management
  UserInfo::VerificationType verificationType =
      UserInfo::EMAIL;      // Type of verification used
  App2FA *app2FA = nullptr; // Pointer to the App 2FA instance for verification
protected:
public:
  User(Server *server, const std::string &username, const std::string &passwd,
       EmailServer *emailServer, const std::string &emailAddr,
       const std::string &emailPasswd);
  User(Server *server, EmailServer *emailServer, Json::Value *arg_json_ptr);
  virtual ~User();

  /**
   * @brief add a card to the user's collection
   * @param card: pointer to the card to be added
   */
  void addCard(Card *card);
  /**
   * @brief add the card to server
   * @param id: the id of the card to be added
   * @return true if the card is added successfully,
   *        false if the card already exists or the server is not set
   */
  bool addCardToServer(const std::string &id);
  /**
   * @brief remove a card from the user's collection
   * @param card: pointer to the card to be removed
   */
  void removeCard(Card *card);
  /**
   * @brief remove card by id
   * @param id: the id of the card to be removed
   * @return pointer to the removed card if successful,
   */
  Card *removeCard(const std::string &id);
  /**
   * @brief drop a card into a box
   * @param box: pointer to the box where the card will be dropped
   * @param card: pointer to the card to be dropped
   * @retval true: the card is dropped successfully
   *         false: otherwise
   */
  bool dropCard(Box *box, Card *card);
  /**
   * @brief drop a card into a box by its ID
   * @param box: pointer to the box where the card will be dropped
   * @param cardId: the id of the card to be dropped
   * @retval true: the card is dropped successfully
   *        false: otherwise
   */
  bool dropCard(Box *box, const std::string &cardId);
  /**
   * @brief retrieve a card from a box
   * @param box: pointer to the box where the card will be retrieved from
   * @param cardId: the id of the card to be retrieved
   * @param paymentCardId: the id of the card used for payment (optional)
   * @return pointer to the retrieved card if successful,
   *         otherwise nullptr
   */
  Card *retrieveCard(Box *box, const std::string &cardId,
                     const std::string &paymentCardId = "");

  /**
   * @brief reject the retrieval of a card
   * @param cardId: the id of the card to be rejected
   * @return true if the rejection is successful,
   */
  bool rejectRetrieve(const std::string &cardId);

  /**
   * @brief redeem a reward for the user
   * @param box: pointer to the box where the reward will be redeemed
   * @param cardId: the id of the card used for payment
   * @param amount: the amount of reward to redeem, -1 for all available
   * @return the reward balance after redemption,
   *        or -1 if the user is invalid or the card is not found
   */
  int redeemReward(Box *box, const std::string &cardId, int amount = -1);
  /**
   * @brief read the user's reward
   * @return the reward amount
   */
  int readReward() const;

  /**
   * @brief get the card by its ID
   * @param cardId: the ID of the card to be retrieved
   * @return pointer to the card if found, otherwise nullptr
   */
  const Card *getCard(const std::string &cardId) const;
  /**
   * @brief read the mail by index
   * @param index: the index of the mail to be read
   */
  void readMail(int index);
  /**
   * @brief get all email IDs associated with the user
   */
  std::set<long long> getEmailIds() const;

  /**
   * @brief Set the verification type for the user
   * @param type: the verification type to be set
   */
  void setVerificationType(UserInfo::VerificationType type);

  virtual Json::Value *dump2JSON(void) const override;
  virtual void JSON2Object(Json::Value *arg_json_ptr) override;
};

#endif // USER_H
