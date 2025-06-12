#include "Card.h"

Card::Card(const std::string &cardId) : id(cardId) {}
Card::~Card() {}
std::string Card::getId() const { return id; }
