#include "Server.h"
#include "Core/Labeled_GPS.h"
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
                     const string &emailAddr) {
  if (userId.find(username) != userId.end() || emailAddr.empty()) {
    return false; // Username already exists or invalid email address
  }
  long long id = nextId++;
  userId[username] = id;
  this->userInfo[id].passwd = passwd;
  this->userInfo[id].email = emailAddr;
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
