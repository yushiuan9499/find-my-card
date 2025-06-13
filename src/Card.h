#ifndef CARD_H
#define CARD_H

#include "Core/Core.h"
#include <string>

class Card : public Core {
private:
  // Unique identifier for the card
  std::string id;

protected:
public:
  // 不應該有卡片沒ID，所以禁止使用無參數的建構子
  Card(const std::string &cardId);
  Card(const Json::Value *arg_json_ptr);
  virtual ~Card();
  /**
   * @brief Get the ID of the card
   * @return The ID of the card
   */
  std::string getId() const;

  virtual Json::Value *dump2JSON(void) const override;
};

#endif // CARD_H
