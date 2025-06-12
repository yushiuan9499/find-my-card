#ifndef SERVER_H
#define SERVER_H

#include "Core/Labeled_GPS.h"
#include <map>
#include <string>

class EmailServer;

class Server {
private:
  // username -> user id mapping
  std::map<std::string, long long> usrId;
  // user id -> password mapping
  std::map<long long, std::string> passwd;
  // user id -> email address mapping
  std::map<long long, std::string> emailAddr;
  // card id -> owner id mapping
  std::map<std::string, long long> cardOwnerId;
  // Server's email address
  std::string address;
  // Server's email password
  std::string emailPasswd;
  // Next available user ID
  long long nextId;

  EmailServer *emailServer;
  /**
   * @brief Send a notification to the user
   * @param id: the id of the user
   * @param subject: the subject of the email
   * @param body: the body to be sent to the user
   */
  void notifyUser(long long id, const std::string &subject,
                  const std::string &body) const;

protected:
public:
  Server(const std::string &serverAddress, const std::string &serverEmailPasswd,
         EmailServer *emailServerPtr);
  virtual ~Server();

  /**
   * @brief Add a user to the server with username and password
   * @param usrName: the username of the user
   * @param passwd: the password of the user
   * @param emailAddr: the email address of the user
   * @retval true: the user is added successfully
   *         false: the username already exists or the email address is invalid
   */
  bool addUser(const std::string &usrName, const std::string &passwd,
               const std::string &emailAddr);
  /**
   * @brief Remove a user from the server
   * @param usrName: the username of the user to be removed
   * @param passwd: the password of the user to be removed
   * @return true if the user is removed successfully, false if the username
   * does not exist or the password does not match
   */
  bool removeUser(const std::string &usrName, const std::string &passwd);
  /**
   * @brief Check if the username and password match
   * @param usrName: the username of the user
   * @param passwd: the password of the user
   * @return true if the username and password match, false otherwise
   */
  bool checkUser(const std::string &usrName, const std::string &passwd) const;

  /**
   * @brief Add a card to the server
   * @param owner: the username of the owner of the card
   * @param passwd: the password of the owner of the card
   * @param id: the ID of the card
   * @return true if the card is added successfully, false if the owner does not
   */
  bool addCard(const std::string &owner, const std::string &passwd,
               const std::string &id);

  /**
   * @brief notify server a card found
   * @param id: the ID of card
   * @retval true if the process is successful, false if error occurs
   */
  bool notifyCardFound(const std::string &id, const Labeled_GPS &gps) const;
};

#endif // SERVER_H
