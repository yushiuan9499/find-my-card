#ifndef USER_H
#define USER_H

#include "Core/Core.h"
#include <map>
#include <set>
#include <string>

class Card;
class Box;
class EmailServer;
class Server;

class User : public Core {
private:
  std::string username;
  std::string emailPassword;
  std::string password; // Password for the user
  std::string email;
  std::map<std::string, Card *> cards; // id -> card owned by the user
  EmailServer *emailServer; // Pointer to the email server for communication
  Server *server;           // Pointer to the server for user management

protected:
public:
  User(Server *server, const std::string &usrName, const std::string &passwd,
       EmailServer *emailServer, const std::string &emailAddr,
       const std::string &emailPasswd);
  User(Json::Value *arg_json_ptr);
  virtual ~User();

  /**
   * @brief add a card to the user's collection
   * @param card: pointer to the card to be added
   */
  void addCard(Card *card);
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
  void readMail(int index) const;
  /**
   * @brief get all email IDs associated with the user
   */
  std::set<long long> getEmailIds() const;

  virtual Json::Value *dump2JSON(void) const override;
  virtual void JSON2Object(Json::Value *arg_json_ptr) override;
};

#endif // USER_H
