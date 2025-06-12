#include "User.h"
#include "Box.h"
#include "Card.h"
#include "EmailServer.h"
#include <cassert>
#include <iostream>
using namespace std;

User::User(const string &usrName, const string &passwd,
           EmailServer *emailServer, const string &emailAddr,
           const string &emailPasswd)
    : username(usrName), password(passwd), email(emailAddr),
      emailPassword(emailPasswd), emailServer(emailServer) {}
User::~User() {
  for (Card *card : cards) {
    delete card; // Clean up dynamically allocated cards
  }
}

void User::addCard(Card *card) {
  if (card) {
    cards.insert(card);
  }
}

void User::removeCard(Card *card) {
  if (card) {
    cards.erase(card);
  }
}

bool User::dropCard(Box *box, Card *card) {
  if (!box || !card) {
    return false; // Invalid box or card
  }
  if (cards.find(card) == cards.end()) {
    return false; // Card not owned by the user
  }
  Card *result = box->addCard(card);
  if (result == nullptr) {
    removeCard(card);
    return true;
  }
  return false; // Failed to add to the box
}

Card *User::retrieveCard(Box *box, const std::string &cardId) {
  if (!box) {
    return nullptr; // Invalid box
  }
  Card *card = box->retrieveCard(username, cardId, password);
  assert(cards.find(card) == cards.end() &&
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
