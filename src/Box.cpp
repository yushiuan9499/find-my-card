#include "Box.h"
#include "Card.h"
#include "Core/ee1520_Common.h"
#include "Env.h"
#include "Server.h"
#include <algorithm>
#include <cassert>
using namespace std;

void Box::Session::clear() {
  username = "";
  passwd = "";
}

Box::Box(Server *server, const Labeled_GPS &gpsLocation)
    : server(server), gps(gpsLocation) {}

Box::Box(Server *server, Json::Value *arg_json_ptr) : server(server) {
  JSON2Object(arg_json_ptr);
}

Box::~Box() {
  // Clean up the cards in the box
  for (auto &pair : cards) {
    delete pair.second; // Assuming ownership of Card objects
  }
  cards.clear();
}

const Box::Session &Box::getSession() {
  constexpr int VALID_SEC = 60;
  if (JvTime currentTime = Env::getNow();
      currentTime - sess.lastActive > VALID_SEC) {
    // session outdated.
    sess.clear();
  } else
    sess.lastActive = currentTime;
  return sess;
}

string Box::login(const string &username, const string &passwd) {
  // NOTE: 為了防暴搜username，實際上要在錯誤時拖延時間
  //       或是讓用戶能用QRcode登入，讓用戶必須有密碼
  string ret = server->getNickname(username);
  if (!passwd.empty() && !server->checkUser(username, passwd))
    return "";
  sess.username = username;
  sess.passwd = passwd;
  sess.lastActive = Env::getNow();
  return ret;
}

Card *Box::addCard(Card *card) {
  if (card == nullptr) {
    return card; // Return the card itself if it's null
  }
  const string &username = getSession().username;
  if (username.empty())
    return card;
  assert(cards.find(card->getId()) == cards.end() &&
         "Card with the same ID already exists in the box");
  int reward =
      (username.empty() ? 0 : min((long long)30, card->getBalance() / 10));
  if (!server->notifyCardFound(card->getId(), gps, username, reward)) {
    return card; // Card with the same ID already exists, return the card itself
  }
  cards[card->getId()] = card; // Add the card to the box
  return nullptr;              // Card added successfully
}

Card *Box::retrieveCard(const std::string &cardId, int verificationCode,
                        Card *paymentCard) {
  const Session &sess = getSession();
  const string &username = sess.username;
  const string &passwd = sess.passwd;
  // Check if the user is authenticated
  if (!server->checkUser(username, passwd)) {
    return nullptr; // Authentication failed
  }

  // Find the card by its name
  if (auto it = cards.find(cardId); it != cards.end()) {
    Card *card = it->second; // Get the card pointer
    int reward = min((long long)30, card->getBalance() / 10);
    if (reward > paymentCard->getBalance()) {
      return nullptr; // Not enough balance on the payment card
    }
    // Notify the server that the card is retrieved
    if (!server->notifyCardRetrieved(cardId, verificationCode)) {
      return nullptr; // Failed to notify the server
    }
    paymentCard->adjustBalance(-reward); // Deduct the reward from payment card
    cards.erase(it);                     // Remove the card from the box
    return card;                         // Return the card pointer
  }

  return nullptr; // Card not found
}

Labeled_GPS Box::getGPSLocation() const {
  return gps; // Return the GPS location of the box
}

int Box::redeemReward(int amount, Card *card) {
  const Session &sess = getSession();
  const string &username = sess.username;
  const string &passwd = sess.passwd;
  if (card == nullptr) {
    return -1; // No card provided for payment
  }
  int ret = server->redeemReward(username, passwd, amount);
  if (ret < 0) {
    return -1; // Redemption failed
  }
  if (ret > 0) {
    card->adjustBalance(ret); // Deduct the redeemed amount from the card
  }
  return ret; // Return the remaining balance after redemption
}

Json::Value *Box::dump2JSON() const {
  Json::Value *json = new Json::Value();
  (*json)["gps"] = *gps.dump2JSON();
  (*json)["cards"] = Json::Value(Json::arrayValue);
  for (const auto &pair : cards) {
    (*json)["cards"].append(*pair.second->dump2JSON());
  }
  return json; // Return the JSON representation of the box
}

void Box::JSON2Object(const Json::Value *arg_json_ptr) {
  ee1520_Exception lv_exception{};
  ee1520_Exception *lv_exception_ptr = &lv_exception;

  JSON2Object_precheck(arg_json_ptr, lv_exception_ptr,
                       EE1520_ERROR_JSON2OBJECT_BOX);
  if (!hasException(Object, (*arg_json_ptr)["GPS"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_BOX, "GPS")) {
    this->gps.JSON2Object(&(*arg_json_ptr)["GPS"]);
  }
  if (!hasException(Array, (*arg_json_ptr)["cards"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_BOX, "cards")) {
    for (unsigned int i = 0; i < (*arg_json_ptr)["cards"].size(); i++) {
      if (!hasException(Object, (*arg_json_ptr)["card"][i], lv_exception_ptr,
                        EE1520_ERROR_JSON2OBJECT_BOX, "cards")) {
        Card *card = new Card(&(*arg_json_ptr)["cards"][i]);
        this->addCard(card);
      }
    }
  }
  if ((lv_exception_ptr->info_vector.size() != 0)) {
    throw(*lv_exception_ptr); // Throw exception if there are errors
  }
}
