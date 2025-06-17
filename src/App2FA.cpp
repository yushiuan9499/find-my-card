#include "App2FA.h"
#include "Core/utils.h"
#include "Env.h"
#include "Server.h"
#include <ctime>
#include <utility>
using namespace std;

App2FA::App2FA() : secret(0), id(-1) {}

App2FA::App2FA(const string &username, Server *server) {
  setServer(username, server);
}

App2FA::~App2FA() {}

void App2FA::setServer(const string &username, Server *server) {
  if (server) {
    auto ret = server->setup2FA(username);
    if (ret.first != -1) {
      id = ret.first;      // Set the ID for the 2FA
      secret = ret.second; // Set the secret key for the 2FA
    } else {
      cerr << "Failed to set up App2FA for user: " << username << endl;
    }
  }
}

int App2FA::generateVerificationCode() const {
  if (secret == 0) {
    cerr << "App2FA secret is not set." << endl;
    return -1; // Return -1 if the secret is not set
  }
  // Generate a verification code based on the secret and current time
  long long currentTime = mktime(Env::getNow().getStdTM());
  return Utils::generateVerificationCode(secret, currentTime); // 6-digit code
}
