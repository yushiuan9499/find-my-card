#ifndef CARD_H
#define CARD_H

#include "Core/Core.h"
#include <string>

class Card : public Core {
private:
  // Unique identifier for the card
  std::string id;
  long long balance;

protected:
public:
  // 不應該有卡片沒ID，所以禁止使用無參數的建構子
  Card(const std::string &cardId, int balance = 0);
  Card(const Json::Value *arg_json_ptr);
  virtual ~Card();
  /**
   * @brief Get the ID of the card
   * @return The ID of the card
   */
  std::string getId() const;
  /**
   * @brief Get the balance of the card
   * @return The balance of the card
   */
  long long getBalance() const;
  /**
   * @brief increase or decrease the balance of the card
   * @param amount: the amount to be added or subtracted from the balance
   */
  void adjustBalance(long long amount);

  virtual Json::Value *dump2JSON(void) const override;
};

#endif // CARD_H
