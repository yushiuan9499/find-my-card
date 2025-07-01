#include "Server.h"
#include "Core/Labeled_GPS.h"
#include "Core/ee1520_Common.h"
#include "Core/ee1520_Exception.h"
#include "Core/utils.h"
#include "EmailServer.h"
#include "Env.h"
#include <random>
#include <string>
#include <time.h>
using namespace std;

Server::Server(const string &serverAddress, const string &serverEmailPasswd,
               EmailServer *emailServerPtr)
    : address(serverAddress), emailPasswd(serverEmailPasswd),
      emailServer(emailServerPtr), nextId(0) {
  emailServer->addAddress(serverAddress, serverEmailPasswd);
}

Server::Server(EmailServer *emailServerPtr, const Json::Value *arg_json_ptr)
    : emailServer(emailServerPtr), nextId(0) {
  JSON2Object(arg_json_ptr);
}

Server::~Server() {}

void Server::notifyUser(long long id, const string &subject, const string &body,
                        const string &cardId, int verificationCode) const {
  if (auto it = userInfo.find(id); it != userInfo.end()) {
    string userAddr = it->second.email;
    Email email;
    email.subject = subject;
    email.body = body;
    email.sender = address;
    email.recipient = userAddr;
    email.cardId = cardId; // Set the card ID if applicable
    email.verificationCode = verificationCode;
    emailServer->sendEmail(email, emailPasswd);
  }
}

bool Server::addUser(const string &username, const string &passwd,
                     const string &emailAddr, const string &nickname) {
  if (userId.find(username) != userId.end() || emailAddr.empty()) {
    return false; // Username already exists or invalid email address
  }
  long long id = nextId++;
  userId[username] = id;
  this->userInfo[id].username = username;
  this->userInfo[id].passwd = passwd;
  this->userInfo[id].email = emailAddr;
  this->userInfo[id].nickname = nickname;
  return true; // User added successfully
}

bool Server::removeUser(const string &username, const string &passwd) {
  if (!checkUser(username, passwd)) {
    return false; // Password does not match
  }
  auto it = userId.find(username);
  long long id = userId[username];
  userId.erase(it);
  this->userInfo.erase(id);
  return true; // User removed successfully
}

bool Server::rejectRetrieve(const string &username, const string &passwd,
                            const string &id) {
  if (!checkUser(username, passwd)) {
    return false; // User does not exist or password does not match
  }
  auto it = cardOwnerId.find(id);
  if (it == cardOwnerId.end()) {
    return false; // Card ID not found
  }
  long long ownerId = it->second;
  if (ownerId != userId[username]) {
    return false; // User is not the owner of the card
  }
  cardRejectInfo[id] = cardFindInfo[id]; // Store the find info for rejection
  cardFindInfo.erase(id);                // Remove the find info for the card
  return true;                           // Card retrieval rejected successfully
}

bool Server::setVerificationType(const string &username, const string &passwd,
                                 UserInfo::VerificationType type) {
  if (!checkUser(username, passwd)) {
    return false; // User does not exist or password does not match
  }
  if (userInfo[userId[username]].cardFoundCount) {
    return false; // Cannot change verification type while cards are found
  }
  long long id = userId[username];
  this->userInfo[id].verificationType = type; // Set the verification type
  return true; // Verification type set successfully
}

bool Server::checkUser(const string &username, const string &passwd) const {
  auto it = userId.find(username);
  if (it == userId.end()) {
    return false; // Username not found
  }
  long long id = it->second;
  auto userIt = this->userInfo.find(id);
  return userIt != this->userInfo.end() && userIt->second.passwd == passwd;
}

string Server::getNickname(const string &username) const {
  auto it = userId.find(username);
  if (it == userId.end()) {
    return ""; // Username not found
  }
  long long id = it->second;
  return userInfo.at(id).nickname;
}

bool Server::addCard(const string &username, const string &passwd,
                     const string &cardId) {
  if (!checkUser(username, passwd)) {
    return false; // User does not exist or password does not match
  }
  long long id = userId[username];
  cardOwnerId[cardId] = id; // Map card ID to user ID
  return true;              // Card added successfully
}

bool Server::notifyCardFound(const string &cardId, const Labeled_GPS &gps,
                             const string &username, int reward) {
  static bool seeded = false;
  if (!seeded) {
    srand(time(nullptr)); // Seed the random number generator
    seeded = true;
  }

  // Check if the card ID exists in the mapping
  auto it = cardOwnerId.find(cardId);
  if (it == cardOwnerId.end()) {
    return false; // Card ID not found
  }

  // Create a FindInfo object
  FindInfo findInfo;

  if (!username.empty()) {
    if (auto userIt = userId.find(username); userIt == userId.end()) {
      return false; // Error: Username not found
    } else {
      findInfo.finderId = userIt->second; // Set finder ID from username
      findInfo.reward = reward;           // Set reward for finding the card
    }
  }
  findInfo.gps = gps;            // Set GPS location where the card was found
  findInfo.time = Env::getNow(); // Set the current time

  // Notify the owner of the card
  long long ownerId = it->second;
  string body = "Your card with ID " + cardId +
                " has been found at location: " + gps.label + "( " +
                to_string(gps.latitude) + ", " + to_string(gps.longitude) +
                " )."; // Create notification body with GPS info"
  // Notify the owner of the card
  if (userInfo[ownerId].verificationType == UserInfo::EMAIL) {
    // Generate a random verification code
    int verificationCode = rand() % 1000000; // Random 6-digit code
    body += "\nVerification Code: " + to_string(verificationCode) +
            ". Please use this code to verify the card retrieval.";
    findInfo.verificationCode = verificationCode; // Set verification code
    notifyUser(ownerId, "Your Card is Found", body, cardId, verificationCode);
  } else {
    notifyUser(ownerId, "Your Card is Found", body, cardId);
  }

  userInfo[ownerId].cardFoundCount++; // Increment card found count
  cardFindInfo[cardId] = findInfo;
  return true; // Notification sent successfully
}

bool Server::notifyCardRetrieved(const string &cardId, int verificationCode) {
  // Check if the card ID exists in the mapping
  auto findIt = cardFindInfo.find(cardId);
  if (findIt == cardFindInfo.end()) {
    return false; // Card ID not found
  }

  // Get the find info for the card
  FindInfo &findInfo = findIt->second;

  long long ownerId = cardOwnerId[cardId];
  if (userInfo[ownerId].verificationType == UserInfo::EMAIL &&
      findInfo.verificationCode != verificationCode) {
    return false; // Verification code does not match
  } else if (userInfo[ownerId].verificationType == UserInfo::APP) {
    long long correctCode = Utils::generateVerificationCode(

        secret2FA[userInfo[ownerId].id], mktime(Env::getNow().getStdTM()));
    if (correctCode != verificationCode) {
      return false; // No finder ID available for app verification
    }
  }

  // Notify the owner of the card
  notifyUser(ownerId, "Your Card is Retrieved",
             "Your card with ID " + cardId + " has been retrieved.");
  if (findInfo.finderId != -1) {
    // Notify the finder of the card if they are registered
    notifyUser(
        findInfo.finderId, "Card Retrieved",
        "The card you found has been retrieved. Thank you for your help!");
    rewardBalance[findInfo.finderId] += findInfo.reward; // Add reward
  }

  // Remove the find info for the card
  cardFindInfo.erase(findIt);
  userInfo[ownerId].cardFoundCount--; // Decrement card found count
  return true;                        // Notification sent successfully
}

const FindInfo *Server::findInfo(const string &cardId) const {
  auto it = cardFindInfo.find(cardId);
  if (it == cardFindInfo.end()) {
    return nullptr; // Card ID not found, return nullptr
  }
  return &it->second; // Return the find info for the card
}
int Server::getBalance(const string &username, const string &password) const {
  if (!checkUser(username, password)) {
    return -1;
  }
  return rewardBalance.at(userId.at(username)); // Return the user's balance
}

int Server::redeemReward(const string &username, const string &password,
                         int amount) {
  if (!checkUser(username, password)) {
    return -1; // User does not exist or password does not match
  }
  long long id = userId[username];
  if (amount < 0) {
    amount = rewardBalance[id]; // Redeem all available rewards
  }
  if (rewardBalance[id] < amount) {
    return -1; // Not enough balance to redeem
  }
  rewardBalance[id] -= amount; // Deduct the redeemed amount
  return amount;               // Return the remaining balance
}

pair<long long, long long> Server::setup2FA(const string &username) {
  // Generate a random verification code
  static bool seeded = false;
  if (!seeded) {
    srand(time(nullptr)); // Seed the random number generator
    seeded = true;
  }
  if (userId.find(username) == userId.end()) {
    return make_pair(-1, -1); // User does not exist
  }
  if (userInfo[userId[username]].verificationType != UserInfo::APP) {
    return make_pair(-1, -1); // 2FA is not set up for this user
  }
  long long id = secret2FA.size();       // Use the index as the ID for 2FA
  userInfo[userId[username]].id = id;    // Set the ID in user info
  long long secret = rand() % 100000000; // Random 8-digit code
  secret2FA.push_back(secret);
  return make_pair(id, secret); // Return the ID and secret key
}

Json::Value *
Server::dumpCard2JSON(const pair<string, long long> cardPair) const {
  Json::Value *json = new Json::Value();
  (*json)["id"] = cardPair.first; // Card ID
  (*json)["ownerUsername"] = userInfo.at(cardPair.second).username;
  if (cardFindInfo.find(cardPair.first) != cardFindInfo.end()) {
    (*json)["findInfo"] = Json::Value(Json::objectValue);
    const FindInfo &findInfo = cardFindInfo.at(cardPair.first);
    (*json)["findInfo"]["reward"] = findInfo.reward;
    (*json)["findInfo"]["gps"] = *findInfo.gps.dump2JSON();
    (*json)["findInfo"]["time"] = *findInfo.time.dump2JSON();
    if (findInfo.verificationCode != -1) {
      (*json)["findInfo"]["verificationCode"] = findInfo.verificationCode;
    }
  }
  return json; // Return the JSON representation of the card
}

void Server::JSON2FindInfo(const Json::Value *arg_json_ptr,
                           FindInfo &findInfo) {
  // Check if the JSON pointer is valid
  ee1520_Exception lv_exception{};
  ee1520_Exception *lv_exception_ptr = &lv_exception;
  JSON2Object_precheck(arg_json_ptr, lv_exception_ptr,
                       EE1520_ERROR_JSON2OBJECT_SERVER);

  const Json::Value &findJson = *arg_json_ptr;
  // reward
  if (!hasException(Integer, findJson["reward"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_SERVER,
                    "cards[].findInfo.reward")) {
    findInfo.reward = findJson["reward"].asInt();
  }
  // GPS location
  if (!hasException(Object, findJson["gps"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_SERVER, "cards[].findInfo.gps")) {
    Labeled_GPS gps;
    gps.JSON2Object(&findJson["gps"]);
    findInfo.gps = gps; // Set GPS location
  }
  // time
  if (!hasException(Object, findJson["time"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_SERVER, "cards[].findInfo.time")) {
    JvTime time;
    time.JSON2Object(&findJson["time"]);
    findInfo.time = time; // Set the time when the card was found
  }
  // verification code
  if (findJson.isMember("verificationCode")) {
    if (!hasException(Integer, findJson["verificationCode"], lv_exception_ptr,
                      EE1520_ERROR_JSON2OBJECT_SERVER,
                      "cards[].findInfo.verificationCode")) {
      findInfo.verificationCode = findJson["verificationCode"].asInt();
    }
  } else {
    findInfo.verificationCode = -1; // No verification code
  }
  if (lv_exception_ptr->info_vector.size() != 0) {
    throw(*lv_exception_ptr); // Throw exception if there are errors
  }
  return; // Successfully parsed the find info
}

Json::Value *Server::dump2JSON() const {
  Json::Value *json = new Json::Value();
  (*json)["address"] = address;
  (*json)["emailPassword"] = emailPasswd;

  // Dump user information
  (*json)["users"] = Json::Value(Json::objectValue);
  for (const auto &user : userId) {
    Json::Value userJson;
    const auto &userInfo = this->userInfo.at(user.second);
    userJson["password"] = userInfo.passwd;
    userJson["email"] = userInfo.email;
    userJson["nickname"] = userInfo.nickname;
    if (userInfo.verificationType == UserInfo::EMAIL) {
      userJson["verificationType"] = "EMAIL";
    } else if (userInfo.verificationType == UserInfo::APP) {
      userJson["verificationType"] = "APP";
    }
    if (rewardBalance.find(user.second) != rewardBalance.end()) {
      userJson["rewardBalance"] =
          (Json::Value::Int64)rewardBalance.at(user.second);
    }
    (*json)["users"][user.first] = userJson;
  }

  // Dump card information
  (*json)["cards"] = Json::Value(Json::arrayValue);
  (*json)["rejectCards"] = Json::Value(Json::arrayValue);
  for (const auto &card : cardOwnerId) {
    // card.first is the card ID, card.second is the owner ID
    if (cardFindInfo.find(card.first) != cardFindInfo.end()) {
      (*json)["cards"].append(*dumpCard2JSON(card));
    }
    if (cardRejectInfo.find(card.first) != cardRejectInfo.end()) {
      (*json)["rejectCards"].append(*dumpCard2JSON(card));
    }
  }

  return json; // Return the JSON representation of the server
}

void Server::JSON2Object(const Json::Value *arg_json_ptr) {
  // Check if the JSON pointer is valid
  ee1520_Exception lv_exception{};
  ee1520_Exception *lv_exception_ptr = &lv_exception;
  JSON2Object_precheck(arg_json_ptr, lv_exception_ptr,
                       EE1520_ERROR_JSON2OBJECT_SERVER);
#define exceptionCheck(type, jv_ptr, which_string)                             \
  hasException(type, jv_ptr, lv_exception_ptr,                                 \
               EE1520_ERROR_JSON2OBJECT_SERVER, which_string)
  string tmpAddress;
  string tmpEmailPasswd;

  // Extract server address and email password
  if (!exceptionCheck(String, (*arg_json_ptr)["address"], "address")) {
    tmpAddress = (*arg_json_ptr)["address"].asString();
  }
  if (!exceptionCheck(String, (*arg_json_ptr)["emailPassword"],
                      "emailPassword")) {
    tmpEmailPasswd = (*arg_json_ptr)["emailPassword"].asString();
  }
  // A temporary map to store data during JSON parsing
  map<string, long long> tmpUserId;
  map<long long, UserInfo> tmpUserInfo;
  map<string, long long> tmpCardOwnerId;
  map<string, FindInfo> tmpCardFindInfo;
  map<long long, long long> tmpRewardBalance;
  long long tmpNextId = 0;
  // Extract user information
  if (!exceptionCheck(Object, (*arg_json_ptr)["users"], "users")) {
    const Json::Value &users = (*arg_json_ptr)["users"];
    for (const auto &user : users.getMemberNames()) {
      long long id = tmpNextId++; // Assign a new ID for the user
      tmpUserId[user] = id;       // Map username to user ID
      tmpUserInfo[id].username = user;
      // password
      if (!exceptionCheck(String, users[user]["password"],
                          "users." + user + ".password")) {
        tmpUserInfo[id].passwd = users[user]["password"].asString();
      }
      // email
      if (!exceptionCheck(String, users[user]["email"],
                          "users." + user + ".email")) {
        tmpUserInfo[id].email = users[user]["email"].asString();
      }
      // nickname
      if (!exceptionCheck(String, users[user]["nickname"],
                          "users." + user + ".nickname")) {
        tmpUserInfo[id].nickname = users[user]["nickname"].asString();
      }
      // reward balance
      if (users[user].isMember("rewardBalance")) {
        if (!exceptionCheck(Integer, users[user]["rewardBalance"],
                            "users." + user + ".rewardBalance")) {
          tmpRewardBalance[id] =
              users[user]["rewardBalance"].asInt64(); // Set reward balance
        }
      }
      // verification type
      if (!exceptionCheck(String, users[user]["verificationType"],
                          "users." + user + ".verificationType")) {
        std::string type = users[user]["verificationType"].asString();
        if (type == "EMAIL") {
          tmpUserInfo[id].verificationType = UserInfo::EMAIL;
        } else if (type == "APP") {
          tmpUserInfo[id].verificationType = UserInfo::APP;
        } else {
          Exception_Info *ei_ptr = new Exception_Info{};
          ei_ptr->where_code = EE1520_ERROR_JSON2OBJECT_SERVER;
          ei_ptr->which_string = "users." + user + ".verificationType";
          ei_ptr->how_code = EE1520_ERROR_NORMAL;
          ei_ptr->what_code = EE1520_ERROR_JSON_KEY_TYPE_MISMATCHED;

          lv_exception_ptr->info_vector.push_back(ei_ptr);
        }
      }
    }
  }

  // Extract card owner information
  if (!exceptionCheck(Array, (*arg_json_ptr)["cards"], "cards")) {
    const Json::Value &cards = (*arg_json_ptr)["cards"];
    for (const auto &card : cards) {
      if (!hasException(String, card["id"], lv_exception_ptr,
                        EE1520_ERROR_JSON2OBJECT_SERVER, "cards[].id")) {
        std::string cardId = card["id"].asString();

        if (!exceptionCheck(String, card["ownerUsername"],
                            "cards[].ownerUsername")) {

          std::string ownerUsername = card["ownerUsername"].asString();

          if (tmpUserId.find(ownerUsername) != tmpUserId.end()) {
            long long ownerId = tmpUserId[ownerUsername];
            tmpCardOwnerId[cardId] = ownerId;
          } else {
            // wrong ownerUsername, throw an exception
            Exception_Info *ei_ptr = new Exception_Info{};
            ei_ptr->where_code = EE1520_ERROR_JSON2OBJECT_SERVER;
            ei_ptr->which_string = "cards[].ownerUsername: " + ownerUsername;
            ei_ptr->how_code = EE1520_ERROR_NORMAL;
            ei_ptr->what_code = EE1520_ERROR_USER_NOT_FOUND;

            lv_exception_ptr->info_vector.push_back(ei_ptr);
          }
        }
        // Extract find info if available
        if (card.isMember("findInfo")) {
          if (!exceptionCheck(Object, card["findInfo"], "cards[].findInfo")) {
            FindInfo findInfo;
            const Json::Value &findJson = card["findInfo"];
            // find name
            if (!exceptionCheck(String, findJson["finderName"],
                                "cards[].findInfo.finderName")) {
              std::string finderName = findJson["finderName"].asString();
              if (tmpUserId.find(finderName) != tmpUserId.end()) {
                findInfo.finderId = tmpUserId[finderName];
              } else {
                // wrong finderName, throw an exception
                Exception_Info *ei_ptr = new Exception_Info{};
                ei_ptr->where_code = EE1520_ERROR_JSON2OBJECT_SERVER;
                ei_ptr->which_string =
                    "cards[].findInfo.finderName: " + finderName;
                ei_ptr->how_code = EE1520_ERROR_NORMAL;
                ei_ptr->what_code = EE1520_ERROR_USER_NOT_FOUND;

                lv_exception_ptr->info_vector.push_back(ei_ptr);
              }
            }
            JSON2FindInfo(&findJson, findInfo);
          }
        }
      }
    }
  }
#undef exceptionCheck
  if ((lv_exception_ptr->info_vector.size() != 0)) {
    throw(*lv_exception_ptr); // Throw exception if there are errors
  }
  if (!address.empty() && !emailPasswd.empty()) {
    emailServer->removeAddress(address, emailPasswd);
  }
  // Assign the parsed data to the server's member variables
  swap(userId, tmpUserId);
  swap(userInfo, tmpUserInfo);
  swap(cardOwnerId, tmpCardOwnerId);
  swap(cardFindInfo, tmpCardFindInfo);
  swap(rewardBalance, tmpRewardBalance);
  nextId = tmpNextId;
  address = tmpAddress;
  emailPasswd = tmpEmailPasswd;
  emailServer->addAddress(address, emailPasswd);
}
