#ifndef SERVER_H
#define SERVER_H

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

  EmailServer *emailServer;
  /**
   * @brief Send a notification to the user
   * @param id: the id of the user
   * @param message: the message to be sent to the user
   */
  void notifyUser(long long id, const std::string &message);

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
   * @return true if the user is added successfully, false if the username
   * already exists
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
  bool checkUser(const std::string &usrName, const std::string &passwd);

  /**
   * @brief notify server a card found
   * @param id: the ID of card
   * @retval true if the process is successful, false if error occurs
   */
  bool notifyCardFound(const std::string &id);
};

#endif // SERVER_H
