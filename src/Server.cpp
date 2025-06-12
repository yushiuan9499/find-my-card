#include "Server.h"
#include "Core/Labeled_GPS.h"
#include "EmailServer.h"
#include <string>
using namespace std;

Server::Server(const string &serverAddress, const string &serverEmailPasswd,
               EmailServer *emailServerPtr)
    : address(serverAddress), emailPasswd(serverEmailPasswd),
      emailServer(emailServerPtr), nextId(0) {}
Server::~Server() {}

void Server::notifyUser(long long id, const string &subject,
                        const string &body) const {
  if (auto it = emailAddr.find(id); it != emailAddr.end()) {
    string userAddr = it->second;
    Email email;
    email.subject = subject;
    email.body = body;
    email.sender = address;
    email.recipient = userAddr;
    emailServer->sendEmail(email, emailPasswd);
  }
}

bool Server::addUser(const string &usrName, const string &passwd,
                     const string &emailAddr) {
  if (usrId.find(usrName) != usrId.end() || emailAddr.empty()) {
    return false; // Username already exists or invalid email address
  }
  long long id = nextId++;
  usrId[usrName] = id;
  this->passwd[id] = passwd;
  this->emailAddr[id] = emailAddr;
  return true; // User added successfully
}
bool Server::removeUser(const string &usrName, const string &passwd) {
  if (!checkUser(usrName, passwd)) {
    return false; // Password does not match
  }
  auto it = usrId.find(usrName);
  long long id = usrId[usrName];
  usrId.erase(it);
  this->passwd.erase(id);
  emailAddr.erase(id); // Remove email address mapping
  return true;         // User removed successfully
}
bool Server::checkUser(const string &usrName, const string &passwd) const {
  auto it = usrId.find(usrName);
  if (it == usrId.end()) {
    return false; // Username not found
  }
  long long id = it->second;
  auto passwdIt = this->passwd.find(id);
  return passwdIt != this->passwd.end() && passwdIt->second == passwd;
}
bool Server::addCard(const string &usrName, const string &passwd,
                     const string &cardId) {
  if (!checkUser(usrName, passwd)) {
    return false; // User does not exist or password does not match
  }
  long long id = usrId[usrName];
  cardOwnerId[cardId] = id; // Map card ID to user ID
  return true;              // Card added successfully
}
bool Server::notifyCardFound(const string &cardId,
                             const Labeled_GPS &gps) const {
  auto it = cardOwnerId.find(cardId);
  if (it == cardOwnerId.end()) {
    return false; // Card ID not found
  }
  long long ownerId = it->second;
  string body = "Your card with ID " + cardId +
                " has been found at location: " + gps.label + "( " +
                to_string(gps.latitude) + ", " + to_string(gps.longitude) +
                " )."; // Create notification body with GPS info"
  notifyUser(ownerId, "Your Card is Found",
             body); // Notify the owner of the card
  return true;      // Notification sent successfully
}
