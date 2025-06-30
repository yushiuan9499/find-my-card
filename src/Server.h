#ifndef SERVER_H
#define SERVER_H

#include "Core/JvTime.h"
#include "Core/Labeled_GPS.h"
#include <map>
#include <string>

class EmailServer;

struct FindInfo {
  JvTime time;               // Time when the card was found
  Labeled_GPS gps;           // GPS location where the card was found
  long long finderId = -1;   // ID of the user who found the card
  int reward = 0;            // Reward for finding the card
  int verificationCode = -1; // Verification code for the finder
};

struct UserInfo {
  enum VerificationType : uint8_t {
    EMAIL, // Email verification
    APP,   // App 2FA verification
  };
  std::string username;                      // Username of the user
  std::string passwd;                        // Password of the user
  std::string email;                         // Email address of the user
  VerificationType verificationType = EMAIL; // Type of verification used
  long long id = -1;                         // User ID, -1 if not set
  int cardFoundCount = 0; // Count of user's cards found, for locking the
                          // verification type change
};

class Server : public Core {
private:
  // username -> user id mapping
  std::map<std::string, long long> userId;
  // user id -> user info mapping
  std::map<long long, UserInfo> userInfo;
  // user id -> reward balance mapping
  std::map<long long, long long> rewardBalance;
  // card id -> owner id mapping
  std::map<std::string, long long> cardOwnerId;
  // card id -> find info mapping
  std::map<std::string, FindInfo> cardFindInfo;
  // card id -> reject card info mapping
  std::map<std::string, FindInfo> cardRejectInfo;
  std::vector<long long> secret2FA; // Verification codes for cards
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
   * @param cardId: the card ID if applicable, "" if not
   * @param verificationCode: the verification code to be sent to the user
   */
  // TODO: make arguments shorter
  void notifyUser(long long id, const std::string &subject,
                  const std::string &body, const std::string &cardId = "",
                  int verificationCode = -1) const;

protected:
public:
  Server(const std::string &serverAddress, const std::string &serverEmailPasswd,
         EmailServer *emailServerPtr);
  Server(EmailServer *emailServerPtr, const Json::Value *arg_json_ptr);
  virtual ~Server();

  /**
   * @brief Add a user to the server with username and password
   * @param username: the username of the user
   * @param passwd: the password of the user
   * @param emailAddr: the email address of the user
   * @retval true: the user is added successfully
   *         false: the username already exists or the email address is invalid
   */
  bool addUser(const std::string &username, const std::string &passwd,
               const std::string &emailAddr);
  /**
   * @brief Remove a user from the server
   * @param username: the username of the user to be removed
   * @param passwd: the password of the user to be removed
   * @return true if the user is removed successfully, false if the username
   * does not exist or the password does not match
   */
  bool removeUser(const std::string &username, const std::string &passwd);
  /**
   * @brief Check if the username and password match
   * @param username: the username of the user
   * @param passwd: the password of the user
   * @return true if the username and password match, false otherwise
   */
  bool checkUser(const std::string &username, const std::string &passwd) const;
  /**
   * @brief Set the verification type for a user
   * @param username: the username of the user
   * @param passwd: the password of the user
   * @param type: the verification type to be set
   * @return true if the verification type is set successfully, false if the
   * user does not exist or the password does not match
   */
  bool setVerificationType(const std::string &username,
                           const std::string &passwd,
                           UserInfo::VerificationType type);

  /**
   * @brief reject to retrieve a card
   * @param username: the username of the user who wants to retrieve the card
   * @param passwd: the password of the user who wants to retrieve the card
   * @param id: the ID of the card should be retrieved
   * @return true if the retrieval is rejected successfully, false if the user
   */
  bool rejectRetrieve(const std::string &username, const std::string &passwd,
                      const std::string &id);

  /**
   * @brief Add a card to the server
   * @param owner: the username of the owner of the card
   * @param passwd: the password of the owner of the card
   * @param id: the ID of the card
   * @return true if the card is added successfully, false if the owner does
   * not
   */
  bool addCard(const std::string &owner, const std::string &passwd,
               const std::string &id);

  /**
   * @brief notify server a card found
   * @param id: the ID of card
   * @param gps: the GPS location where the card is found
   * @param username: the username of the user who found the card
   * @retval true if the process is successful, false if error occurs
   */
  bool notifyCardFound(const std::string &id, const Labeled_GPS &gps,
                       const string &username = "", int reward = 0);
  /**
   * @brief notify server a card is retrieved
   * @param id: the ID of card
   * @param verificationCode: the verification code for the card retrieval
   * @return true if the process is successful, false if error occurs
   */
  bool notifyCardRetrieved(const std::string &id, int verificationCode);

  /**
   * @brief Get the find info of a card
   * @param id: the ID of the card
   * @return FindInfo object containing the find information
   */
  const FindInfo *findInfo(const std::string &id) const;
  /**
   * @brief Get the balance of a user's reward
   * @param username: the username of the user
   * @param password: the pass word of the user
   * @return the balance of the user if valid, otherwise -1
   */
  int getBalance(const string &username, const string &password) const;
  /**
   * @brief redeem a reward for a user
   * @param username: the username of the user
   * @param password: the password of the user
   * @param amount: the amount of reward to redeem, -1 for all available
   * @return the reward balance after redemption, or -1 if the user is invalid
   */
  int redeemReward(const std::string &username, const std::string &password,
                   int amount);
  /**
   * @brief Setup 2FA
   * @param username: the username of the user
   * @return pair<id, secret> where id is the id for the 2FA and secret is the
   * secret key, otherwise pair(-1, -1) if error occurs
   */
  std::pair<long long, long long> setup2FA(const std::string &username);

  Json::Value *dumpCard2JSON(const pair<std::string, long long> cardPair) const;
  void JSON2FindInfo(const Json::Value *arg_json_ptr, FindInfo &findInfo);

  virtual Json::Value *dump2JSON(void) const override;
  virtual void JSON2Object(const Json::Value *arg_json_ptr) override;
};

#endif // SERVER_H
