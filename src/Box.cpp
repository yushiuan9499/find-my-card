#include "Box.h"
#include "Card.h"
#include "Core/ee1520_Common.h"
#include "Server.h"
#include <cassert>
using namespace std;
Box::Box(const Labeled_GPS &gpsLocation) : gps(gpsLocation) {}
Box::Box(Json::Value *arg_json_ptr) { JSON2Object(arg_json_ptr); }
Box::~Box() {
  // Clean up the cards in the box
  for (auto &pair : cards) {
    delete pair.second; // Assuming ownership of Card objects
  }
  cards.clear();
}
Card *Box::addCard(Card *card) {
  if (card == nullptr) {
    return card; // Return the card itself if it's null
  }
  assert(cards.find(card->getId()) == cards.end() &&
         "Card with the same ID already exists in the box");
  if (!server->notifyCardFound(card->getId(), gps)) {
    return card; // Card with the same ID already exists, return the card itself
  }
  cards[card->getId()] = card; // Add the card to the box
  return nullptr;              // Card added successfully
}
Card *Box::retrieveCard(const std::string &usrName, const std::string &cardId,
                        const std::string &passwd) {
  // Check if the user is authenticated
  if (!server->checkUser(usrName, passwd)) {
    return nullptr; // Authentication failed
  }

  // Find the card by its name
  if (auto it = cards.find(cardId); it != cards.end()) {
    Card *card = it->second; // Get the card pointer
    cards.erase(it);         // Remove the card from the box
    return card;             // Return the card pointer
  }

  return nullptr; // Card not found
}
Labeled_GPS Box::getGPSLocation() const {
  return gps; // Return the GPS location of the box
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

void Box::JSON2Object(Json::Value *arg_json_ptr) {
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
