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
    : server(server), username(usrName), password(passwd), email(emailAddr),
      emailPassword(emailPasswd), emailServer(emailServer) {
  this->server->addUser(usrName, passwd, emailAddr);
  this->emailServer->addAddress(emailAddr, emailPasswd);
}
User::User(Json::Value *arg_json_ptr) { JSON2Object(arg_json_ptr); }
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

void User::removeCard(Card *card) {
  if (card) {
    cards.erase(card->getId());
  }
}

Card *User::removeCard(const std::string &id) {
  auto it = cards.find(id);
  if (it != cards.end()) {
    cards.erase(it);   // Remove the card from the collection
    return it->second; // Return the removed card
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
  Card *result = box->addCard(it->second);
  if (result == nullptr) {
    removeCard(cardId);
    return true;
  }
  return false; // Failed to add to the box
}

Card *User::retrieveCard(Box *box, const std::string &cardId) {
  if (!box) {
    return nullptr; // Invalid box
  }
  Card *card = box->retrieveCard(username, cardId, password);
  assert(cards.find(card->getId()) == cards.end() &&
         "Card should not be in user's collection after retrieval");
  if (card) {
    addCard(card); // Add the card back to the user's collection
  }
  return card; // Return the retrieved card or nullptr if not found
}

void User::readMail(int index) const {
  if (emailServer) {
    const Email *email =
        emailServer->getEmailById(username, emailPassword, index);
    cout << "Reading email #" << index << ":" << "\n";
    cout << "Subject: " << email->subject << "\n";
    cout << "Body: " << email->body << "\n";
    cout << "From: " << email->sender << endl;
  } else {
    std::cerr << "Email server not set for user: " << username << std::endl;
  }
}

set<long long> User::getEmailIds() const {
  if (emailServer) {
    return emailServer->getEmails(username, emailPassword);
  }
  return {}; // Return an empty set if email server is not set
}

Json::Value *User::dump2JSON() const {
  Json::Value *json = new Json::Value();
  (*json)["username"] = username;
  (*json)["email"] = email;
  (*json)["emailPassword"] = emailPassword;
  (*json)["password"] = password;
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
    this->emailPassword = (*arg_json_ptr)["emailPassword"].asString();
  }
  if (!hasException(String, (*arg_json_ptr)["password"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_USER, "password")) {
    this->password = (*arg_json_ptr)["password"].asString();
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
