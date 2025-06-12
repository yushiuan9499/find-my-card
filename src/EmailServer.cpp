#include "EmailServer.h"
using namespace std;

EmailServer::EmailServer() : nextId(0) {}
EmailServer::~EmailServer() {
  for (auto &userEmails : emails) {
    for (auto &emailPair : userEmails.second) {
      delete emailPair.second; // Delete each Email object
    }
  }
}

bool EmailServer::checkPasswd(const std::string &address,
                              const std::string &passwd) const {
  auto it = addressId.find(address);
  if (it == addressId.end())
    return false; // Address not found
  long long id = it->second;
  auto passwdIt = idPasswd.find(id);
  return passwdIt != idPasswd.end() && passwdIt->second == passwd;
}

EmailError EmailServer::addAddress(const string &address,
                                   const string &passwd) {
  if (addressId.find(address) != addressId.end()) {
    return ADDRESS_ALREADY_EXISTS; // Address already exists
  }
  if (passwd.size() < 6 || passwd.size() > 30) {
    return INVALID_PASSWORD; // Password is too short or too long
  }
  // Check if the address format is valid (simple check)
  if (address.find('@') == string::npos || address.find('.') == string::npos) {
    return INVALID_ADDRESS; // Invalid email address format
  }
  long long id = nextId++;
  addressId[address] = id;
  idPasswd[id] = passwd;
  emailIdCounter[id] = 0;
  return NONE; // Address added successfully
}

bool EmailServer::removeAddress(const string &address, const string &passwd) {
  if (!checkPasswd(address, passwd)) {
    return false; // Password does not match
  }

  auto it = addressId.find(address);
  long long id = it->second;
  // Remove the address and associated data
  addressId.erase(it);
  idPasswd.erase(id);
  emails.erase(id);         // Remove all emails for this user
  emailIdCounter.erase(id); // Remove email ID counter for this user

  return true; // Address removed successfully
}

EmailError EmailServer::sendEmail(const Email &email, const string &passwd) {
  if (!checkPasswd(email.sender, passwd)) {
    return WRONG_SENDER_OR_PASSWORD; // Password does not match
  }

  auto participantIt = addressId.find(email.recipient);
  // Check recipient addresses
  if (participantIt == addressId.end()) {
    return INVALID_RECIPIENT; // Recipient address does not exist
  }

  long long participantId = participantIt->second;
  // Create a new email object
  long long emailId = emailIdCounter[participantId]++;
  Email *newEmail = new Email(email);

  // Store the email in the sender's email map
  emails[participantId][emailId] = newEmail;

  // Optionally, you can also store the email in recipients' maps if needed

  return NONE; // Email sent successfully
}

// TODO: find a faster way to get emails' ids
const set<long long> EmailServer::getEmails(const string &address,
                                            const string &passwd) const {
  if (!checkPasswd(address, passwd)) {
    return {}; // Password does not match, return empty set
  }

  auto it = addressId.find(address);

  long long id = it->second;
  auto emailIt = emails.find(id);
  if (emailIt == emails.end()) {
    return {}; // No emails found for this user, return empty set
  }
  set<long long> emailIds;
  for (const auto &emailPair : emailIt->second) {
    emailIds.insert(emailPair.first); // Collect email IDs
  }
  return emailIds; // Return set of email IDs
}

Email const *EmailServer::getEmailById(const string &address,
                                       const string &passwd,
                                       long long emailId) const {
  if (!checkPasswd(address, passwd)) {
    return nullptr; // Password does not match, return nullptr
  }

  long long id = addressId.find(address)->second;
  auto emailIt = emails.find(id);
  if (emailIt == emails.end()) {
    return nullptr; // No emails found for this user, return nullptr
  }

  auto emailPair = emailIt->second.find(emailId);
  if (emailPair == emailIt->second.end()) {
    return nullptr; // Email ID not found, return nullptr
  }

  return emailPair->second; // Return the Email object
}

EmailError EmailServer::deleteEmailById(const string &address,
                                        const string &passwd,
                                        long long emailId) {
  if (!checkPasswd(address, passwd)) {
    return WRONG_SENDER_OR_PASSWORD; // Password does not match
  }

  long long id = addressId.find(address)->second;
  auto emailIt = emails.find(id);
  if (emailIt == emails.end()) {
    return EMAIL_NOT_FOUND; // No emails found for this user
  }

  auto emailPair = emailIt->second.find(emailId);
  if (emailPair == emailIt->second.end()) {
    return EMAIL_NOT_FOUND; // Email ID not found
  }

  delete emailPair->second;         // Delete the Email object
  emailIt->second.erase(emailPair); // Remove from the map

  return NONE; // Email deleted successfully
}
