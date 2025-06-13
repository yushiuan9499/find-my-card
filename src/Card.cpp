#include "Card.h"
#include "Core/ee1520_Common.h"

Card::Card(const std::string &cardId) : id(cardId) {}
Card::Card(const Json::Value *arg_json_ptr) {
  ee1520_Exception lv_exception{};
  ee1520_Exception *lv_exception_ptr = &lv_exception;

  JSON2Object_precheck(arg_json_ptr, lv_exception_ptr,
                       EE1520_ERROR_JSON2OBJECT_CARD);

  if (!hasException(String, (*arg_json_ptr)["id"], lv_exception_ptr,
                    EE1520_ERROR_JSON2OBJECT_CARD, "id")) {
    id = (*arg_json_ptr)["id"].asString();
  }
  if (lv_exception_ptr->info_vector.size() != 0) {
    throw(*lv_exception_ptr); // Throw exception if there are errors
  }
}
Card::~Card() {}

std::string Card::getId() const { return id; }

Json::Value *Card::dump2JSON(void) const {
  Json::Value *json = new Json::Value();
  (*json)["id"] = id;
  return json; // Return the JSON representation of the card
}
