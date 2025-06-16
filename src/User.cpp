#include "User.h"
#include "Box.h"
#include "Card.h"
#include "EmailServer.h"
#include "Server.h"
#include <cassert>
#include <iostream>
using namespace std;

User::User(Server *server, const string &usrName, const string &passwd,
           EmailServer *emailServer, const string &emailAddr,
           const string &emailPasswd)
    : server(server), username(usrName), passwd(passwd), email(emailAddr),
      emailPasswd(emailPasswd), emailServer(emailServer) {
  this->server->addUser(usrName, passwd, emailAddr);
  this->emailServer->addAddress(emailAddr, emailPasswd);
}

User::User(Server *server, EmailServer *emailServer, Json::Value *arg_json_ptr)
    : server(server), emailServer(emailServer) {
  JSON2Object(arg_json_ptr);
  this->server->addUser(username, passwd, email);
  this->emailServer->addAddress(email, emailPasswd);
}

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
  Card *result = box->addCard(card, username);
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
  }
  Card *card = box->retrieveCard(username, cardId, passwd, verificationCode,
                                 cards[paymentCardId]);
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

int User::redeemReward(Box *box, const std::string &cardId, int amount) {
  if (!box || cardId.empty()) {
    return -1; // Invalid box or card ID
  }
  Card *paymentCard = cards[cardId];
  if (!paymentCard) {
    return -1; // Payment card not found
  }
  int reward = box->redeemReward(username, passwd, amount, paymentCard);
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
    cout << "Time: " << email->time.getTimeString() << endl;
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

Json::Value *User::dump2JSON() const {
  Json::Value *json = new Json::Value();
  (*json)["username"] = username;
  (*json)["email"] = email;
  (*json)["emailPassword"] = emailPasswd;
  (*json)["password"] = passwd;
  (*json)["cards"] = Json::Value(Json::arrayValue);

  for (auto card : cards) {
    (*json)["cards"].append(*card.second->dump2JSON());
  }

  return json; // Return the JSON representation of the user
}

void User::JSON2Object(Json::Value *arg_json_ptr) {
  ee1520_Exception lv_exception{};
  ee1520_Exception *lv_exception_ptr = &lv_exception;

  JSON2Object_precheck(arg_json_ptr, lv_exception_ptr,
                       EE1520_ERROR_JSON2OBJECT_USER);

  if (!hasException(String, (*arg_json_ptr)["username"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_USER, "username")) {
    this->username = (*arg_json_ptr)["username"].asString();
  }
  if (!hasException(String, (*arg_json_ptr)["email"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_USER, "email")) {
    this->email = (*arg_json_ptr)["email"].asString();
  }
  if (!hasException(String, (*arg_json_ptr)["emailPassword"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_USER, "emailPassword")) {
    this->emailPasswd = (*arg_json_ptr)["emailPassword"].asString();
  }
  if (!hasException(String, (*arg_json_ptr)["password"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_USER, "password")) {
    this->passwd = (*arg_json_ptr)["password"].asString();
  }

  if (!hasException(Array, (*arg_json_ptr)["cards"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_USER, "cards")) {
    for (unsigned int i = 0; i < (*arg_json_ptr)["cards"].size(); i++) {
      if (!hasException(Object, (*arg_json_ptr)["cards"][i], lv_exception_ptr,
                        EE1520_ERROR_JSON2OBJECT_USER, "cards")) {
        Card *card = new Card(&(*arg_json_ptr)["cards"][i]);
        this->addCard(card);
      }
    }
  }

  if (lv_exception_ptr->info_vector.size() != 0) {
    throw(*lv_exception_ptr); // Throw exception if there are errors
  }
}
