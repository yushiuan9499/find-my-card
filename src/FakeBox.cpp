#include "FakeBox.h"
#include "Card.h"
using namespace std;

string FakeBox::login(const string &username, const string &passwd) {
  return "";
}

Card *FakeBox::addCard(Card *card) {
  cards[card->getId()] = card; // Add the card to the box
  return nullptr;              // Card added successfully
}

Card *FakeBox::retrieveCard(const std::string &cardId, int verificationCode,
                            Card *paymentCard) {
  return nullptr;
}

int FakeBox::redeemReward(int amount, Card *card) { return -1; }
