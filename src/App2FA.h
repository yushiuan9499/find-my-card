#ifndef APP_2FA_H
#define APP_2FA_H

#include <string>

class Server;

class App2FA {
private:
  long long secret; // Secret key for the app 2FA
  long long id;

protected:
public:
  App2FA();
  App2FA(const std::string &username, Server *server);
  virtual ~App2FA();
  /**
   * @brief Generate a verification code based on the app secret and current
   * time
   * @return Generated verification code as a string
   */
  int generateVerificationCode() const;

  /**
   * @brief Set server for the app 2FA
   * @param username The username of the user
   * @param server Pointer to the server instance
   */
  void setServer(const std::string &username, Server *server);
};

#endif // APP_2FA_H
