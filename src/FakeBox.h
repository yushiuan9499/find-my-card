#ifndef FAKE_BOX_H
#define FAKE_BOX_H

#include "Box.h"

class FakeBox : public Box {
private:
protected:
public:
  /**
   * @brief login the session
   * @param username: username to login
   * @param passwd: password, not necessary when addCard
   * @return empty string
   */
  string login(const string &username, const string &passwd = "") override;

  /**
   * @brief put a card into the box
   * @param card: pointer to the card to be added
   * @return nullptr if the card add successfully
   *         otherwise, return the card itself
   */
  Card *addCard(Card *card) override;

  /**
   * @brief retrieve a card from the box
   * @param cardId: the id of the card to be retrieved
   * @param verificationCode:
   * @param card: pointer to the card for payment
   * @return: always return nullptr
   */
  Card *retrieveCard(const std::string &cardId, int verificationCode,
                     Card *card = nullptr) override;

  /**
   * @brief redeem a reward for a user
   * @param amount: the amount of reward to redeem, -1 for all available
   * @param card: pointer to the card for receive reward
   * @return always return -1
   */
  int redeemReward(int amount, Card *card) override;
};

#endif // FAKE_BOX_H
