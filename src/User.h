#ifndef USER_H
#define USER_H

#include <set>
#include <string>

class Card;

class User {
private:
  std::string username;
  std::string emailPassword;
  std::string password; // Password for the user
  std::string email;
  std::set<Card *> cards; // Cards owned by the user

protected:
public:
  User(const std::string &usrName, const std::string &passwd,
       const std::string &emailAddr, const std::string &emailPasswd);
  virtual ~User();
};

#endif // USER_H
