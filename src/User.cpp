#include "User.h"
#include "App2FA.h"
#include "Box.h"
#include "Card.h"
#include "EmailServer.h"
#include "Server.h"
#include <cassert>
#include <complex>
#include <iostream>
using namespace std;

User::User()
    : server(nullptr), emailServer(nullptr), verificationType(UserInfo::EMAIL),
      app2FA(nullptr) {
  // Default constructor initializes pointers to nullptr
  // It shouldn't be used
}

User::User(Server *server, const string &usrName, const string &passwd,
           const string &nickname, EmailServer *emailServer,
           const string &emailAddr, const string &emailPasswd)
    : server(server), username(usrName), passwd(passwd), nickname(nickname),
      email(emailAddr), emailPasswd(emailPasswd), emailServer(emailServer) {
  this->server->addUser(usrName, passwd, emailAddr, nickname);
  this->emailServer->addAddress(emailAddr, emailPasswd);
}

User::User(Server *server, EmailServer *emailServer,
           const Json::Value *arg_json_ptr)
    : server(server), emailServer(emailServer) {
  JSON2Object(arg_json_ptr);
  this->emailServer->addAddress(email, emailPasswd);
}
User::User(Server *server, EmailServer *emailServer)
    : server(server), emailServer(emailServer) {}

User::~User() {
  for (auto card : cards) {
    delete card.second; // Clean up dynamically allocated cards
  }
}

void User::addCard(Card *card) {
  if (card) {
    cards[card->getId()] = card;
  }
}

bool User::addCardToServer(const std::string &id) {
  if (server && !id.empty()) {
    if (cards.find(id) == cards.end()) {
      return false; // Card not exists in the user's collection
    }
    return server->addCard(username, passwd, id);
  }
  return false; // Failed to add card to the server or server not set
}

void User::removeCard(Card *card) {
  if (card) {
    cards.erase(card->getId());
  }
}

Card *User::removeCard(const std::string &id) {
  auto it = cards.find(id);
  if (it != cards.end()) {
    auto card = it->second; // Get the card pointer
    cards.erase(it);        // Remove the card from the collection
    return card;
  }
  return nullptr; // Card not found
}

bool User::dropCard(Box *box, Card *card) {
  if (!box || !card) {
    return false; // Invalid box or card
  }
  if (cards.find(card->getId()) == cards.end()) {
    return false; // Card not owned by the user
  }
  string nickname = box->login(username);
  if (nickname != this->nickname)
    return false;
  Card *result = box->addCard(card);
  if (result == nullptr) {
    removeCard(card);
    return true;
  }
  return false; // Failed to add to the box
}

bool User::dropCard(Box *box, const std::string &cardId) {
  if (!box || cardId.empty()) {
    return false; // Invalid box or card ID
  }
  auto it = cards.find(cardId);
  if (it == cards.end()) {
    return false; // Card not owned by the user
  }
  string nickname = box->login(username);
  if (nickname != this->nickname)
    return false;
  Card *result = box->addCard(it->second);
  if (result == nullptr) {
    removeCard(cardId);
    return true;
  }
  return false; // Failed to add to the box
}

Card *User::retrieveCard(Box *box, const std::string &cardId,
                         const std::string &paymentCardId) {
  if (!box) {
    return nullptr; // Invalid box
  }
  if (cardId.empty()) {
    return nullptr; // Invalid card ID or payment card ID
  }
  int verificationCode = -1;
  if (verificationType == UserInfo::EMAIL) {
    // Check if the card ID has a verification code
    if (verificationCodes.find(cardId) != verificationCodes.end()) {
      verificationCode = verificationCodes[cardId];
    } else {
      cout << "No verification code found for card ID: " << cardId << endl;
      return nullptr; // No verification code available
    }
  } else if (verificationType == UserInfo::APP) {
    // Generate verification code using App2FA
    if (!app2FA) {
      cerr << "App2FA not set for user: " << username << endl;
      return nullptr; // App2FA not set
    }
    verificationCode = app2FA->generateVerificationCode();
    if (verificationCode == -1) {
      cerr << "Failed to generate verification code for user: " << username
           << endl;
      return nullptr; // Failed to generate verification code
    }
  }
  string nickname = box->login(username, passwd);
  if (nickname != this->nickname)
    return nullptr;
  Card *card =
      box->retrieveCard(cardId, verificationCode, cards[paymentCardId]);
  assert(cards.find(card->getId()) == cards.end() &&
         "Card should not be in user's collection after retrieval");
  if (card) {
    addCard(card); // Add the card back to the user's collection
    if (verificationType == UserInfo::EMAIL) {
      // Delete the verification code after retrieval
      verificationCodes.erase(cardId);
    }
  }
  return card; // Return the retrieved card or nullptr if not found
}

bool User::rejectRetrieve(const std::string &cardId) {
  if (server && !cardId.empty()) {
    return server->rejectRetrieve(username, passwd, cardId);
  }
  return false; // Failed to reject retrieval or server not set
}

int User::redeemReward(Box *box, const std::string &cardId, int amount) {
  if (!box || cardId.empty()) {
    return -1; // Invalid box or card ID
  }
  Card *paymentCard = cards[cardId];
  if (!paymentCard) {
    return -1; // Payment card not found
  }
  string nickname = box->login(username, passwd);
  if (nickname != this->nickname)
    return -1;
  int reward = box->redeemReward(amount, paymentCard);
  if (reward < 0) {
    return -1; // Redemption failed
  }
  return reward; // Return the remaining balance after redemption
}

int User::readReward() const {
  if (server) {
    return server->getBalance(username, passwd);
  }
  return 0; // Return 0 if server is not set
}

void User::readMail(int index) {
  if (emailServer) {
    const Email *email =
        emailServer->getEmailById(this->email, emailPasswd, index);
    cout << "Reading email #" << index << ":" << "\n";
    cout << "Subject: " << email->subject << "\n";
    cout << "Body: " << email->body << "\n";
    cout << "From: " << email->sender << endl;
    cout << "Time: " << *email->time.getTimeString() << endl;
    if (!email->cardId.empty() && email->verificationCode != -1) {
      verificationCodes[email->cardId] = email->verificationCode;
    }
  } else {
    std::cerr << "Email server not set for user: " << username << std::endl;
  }
}

set<long long> User::getEmailIds() const {
  if (emailServer) {
    return emailServer->getEmails(username, emailPasswd);
  }
  return {}; // Return an empty set if email server is not set
}

void User::setVerificationType(UserInfo::VerificationType type) {
  if (type == UserInfo::EMAIL) {
    server->setVerificationType(username, passwd, type);
    if (app2FA) {
      delete app2FA; // Clean up the app 2FA object if it exists
      app2FA = nullptr;
    }
  } else if (type == UserInfo::APP) {
    server->setVerificationType(username, passwd, type);
    if (app2FA == nullptr) {
      app2FA = new App2FA(username, server); // Create a new App2FA object
    }
  } else {
    cerr << "Invalid verification type" << endl;
    return;
  }
  verificationType = type; // Set the verification type
}

int User::leakVerificationCode(const std::string &cardId) const {
  if (verificationType == UserInfo::EMAIL) {
    auto it = verificationCodes.find(cardId);
    if (it != verificationCodes.end()) {
      return it->second; // Return the verification code if found
    }
  } else if (verificationType == UserInfo::APP) {
    if (app2FA) {
      return app2FA
          ->generateVerificationCode(); // Get the verification code from App2FA
    }
  }
  return -1; // Return -1 if no verification code is available
}

Card *User::stealCard(Box *box, const std::string &cardId,
                      const std::string &username, const std::string &passwd,
                      int verificationCode, const std::string &paymentCardId) {
  if (!box || cardId.empty() || username.empty() || passwd.empty()) {
    return nullptr; // Invalid box, card ID, username, or password
  }
  Card *paymentCard = cards[paymentCardId];
  if (!paymentCard) {
    return nullptr; // Payment card not found
  }
  box->login(username, passwd);
  Card *card = box->retrieveCard(cardId, verificationCode,
                                 paymentCard); // Attempt to retrieve the card
  if (card) {
    addCard(card);
  }

  return card; // Failed to steal card or server not set
}

Json::Value *User::dump2JSON() const {
  Json::Value *json = new Json::Value();
  (*json)["username"] = username;
  (*json)["email"] = email;
  (*json)["emailPassword"] = emailPasswd;
  (*json)["password"] = passwd;
  (*json)["cards"] = Json::Value(Json::arrayValue);
  if (nickname.size())
    (*json)["nickname"] = nickname;
  if (verificationType == UserInfo::EMAIL) {
    (*json)["verificationType"] = "EMAIL";
  } else if (verificationType == UserInfo::APP) {
    (*json)["verificationType"] = "APP";
  }

  for (auto card : cards) {
    (*json)["cards"].append(*card.second->dump2JSON());
  }

  return json; // Return the JSON representation of the user
}

void User::JSON2Object(const Json::Value *arg_json_ptr) {
  ee1520_Exception lv_exception{};
  ee1520_Exception *lv_exception_ptr = &lv_exception;

  JSON2Object_precheck(arg_json_ptr, lv_exception_ptr,
                       EE1520_ERROR_JSON2OBJECT_USER);
#define exceptionCheck(type, jv_ptr, which_string)                             \
  hasException(type, jv_ptr, lv_exception_ptr, EE1520_ERROR_JSON2OBJECT_USER,  \
               which_string)
  if (!exceptionCheck(String, (*arg_json_ptr)["username"], "username")) {
    this->username = (*arg_json_ptr)["username"].asString();
  }
  if (!exceptionCheck(String, (*arg_json_ptr)["email"], "email")) {
    this->email = (*arg_json_ptr)["email"].asString();
  }
  if (!exceptionCheck(String, (*arg_json_ptr)["emailPassword"],
                      "emailPassword")) {
    this->emailPasswd = (*arg_json_ptr)["emailPassword"].asString();
  }
  if (!exceptionCheck(String, (*arg_json_ptr)["password"], "password")) {
    this->passwd = (*arg_json_ptr)["password"].asString();
  }

  if (!exceptionCheck(Array, (*arg_json_ptr)["cards"], "cards")) {
    for (unsigned int i = 0; i < (*arg_json_ptr)["cards"].size(); i++) {
      if (!exceptionCheck(Object, (*arg_json_ptr)["cards"][i],
                          "cards[" + std::to_string(i) + "]")) {
        Card *card = new Card(&(*arg_json_ptr)["cards"][i]);
        this->addCard(card);
      }
    }
  }
  if (arg_json_ptr->isMember("nickname")) {
    if (!exceptionCheck(String, (*arg_json_ptr)["nickname"], "nickname")) {
      this->nickname = (*arg_json_ptr)["nickname"].asString();
    }
  } else
    this->nickname = ""; // Default nickname to username if not set

  server->addUser(username, passwd, email, nickname);
  if (!arg_json_ptr->isMember("verificationType")) {
    // If verificationType is not present, default to EMAIL
    this->setVerificationType(UserInfo::EMAIL);
  } else if (!exceptionCheck(String, (*arg_json_ptr)["verificationType"],
                             "verificationType")) {
    string verificationTypeStr = (*arg_json_ptr)["verificationType"].asString();
    if (verificationTypeStr == "EMAIL") {
      this->setVerificationType(UserInfo::EMAIL);
    } else if (verificationTypeStr == "APP") {
      this->setVerificationType(UserInfo::APP);
    } else {
      // If the verification type is invalid, default to EMAIL
      cerr << "Invalid verification type, defaulting to EMAIL" << endl;
      this->setVerificationType(UserInfo::EMAIL);
    }
  }
#undef exceptionCheck
  if (lv_exception_ptr->info_vector.size() != 0) {
    throw(*lv_exception_ptr); // Throw exception if there are errors
  }
}
