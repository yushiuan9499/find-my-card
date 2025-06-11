#ifndef CARD_H
#define CARD_H

#include <string>

class Card {
private:
  // Unique identifier for the card
  const std::string id;

protected:
public:
  Card(const std::string &cardId);
  virtual ~Card();
  /**
   * @brief Get the ID of the card
   * @return The ID of the card
   */
  std::string getId() const;
};

#endif // CARD_H
