#include "Box.h"
#include "Card.h"
#include "Server.h"
#include <cassert>
using namespace std;
Box::Box(const Labeled_GPS &gpsLocation) : gps(gpsLocation) {}
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
