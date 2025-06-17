#ifndef EMAIL_SERVER_H
#define EMAIL_SERVER_H

#include "Core/JvTime.h"
#include <map>
#include <set>
#include <string>

enum EmailError {
  NONE = 0,
  EMAIL_NOT_SENT,
  INVALID_RECIPIENT,
  WRONG_SENDER_OR_PASSWORD,
  EMAIL_NOT_FOUND,
  ADDRESS_ALREADY_EXISTS,
  INVALID_ADDRESS,
  INVALID_PASSWORD
};

struct Email {
  // For human readability
  std::string subject;
  std::string body;
  // For object and human
  std::string sender;
  std::string recipient;
  JvTime time; // Time when the email was sent
  // For object
  int verificationCode = -1;
  std::string cardId = ""; // Card ID if applicable
};

class EmailServer {
private:
  // address -> id
  std::map<std::string, long long> addressId;
  // id -> password mapping
  std::map<long long, std::string> idPasswd;
  // id -> email ID -> Email object
  std::map<long long, std::map<long long, Email *>> emails;
  // Counter for email IDs per user
  std::map<long long, long long> emailIdCounter;
  // Next available ID for new users
  long long nextId;
  /**
   * @brief Check if the email&password match
   * @param address: email address of the user
   * @param passwd: password for the user
   * @return true if the email and password match, false otherwise
   */
  bool checkPasswd(const std::string &address, const std::string &passwd) const;

protected:
public:
  EmailServer();
  virtual ~EmailServer();
  /**
   * @brief Add a addres to the email server
   * @param address: email address of the user
   * @param passwd: password for the user
   * @retval ADDRESS_ALREADY_EXISTS: the address already exists,
   *         INVALID_ADDRESS: the address's format is invalid,
   *         INVALID_PASSWORD: the password is invalid,
   */
  EmailError addAddress(const std::string &address, const std::string &passwd);

  /**
   * @brief Remove an address from the email server
   * @param address: email address of the user
   * @param passwd: password for the user
   * @return true if the user is removed successfully, false if the address does
   * not exist or the password does not match
   */
  bool removeAddress(const std::string &address, const std::string &passwd);

  /**
   * @brief Send an email from one address to another
   * @param email: Email object containing subject, body, sender, and recipient
   * @param passwd: password of the sender
   * @retval NONE if the email is sent successfully,
   *         WRONG_SENDER_OR_PASSWORD if the sender's email or password is
   *                incorrect,
   *         INVALID_RECIPIENT if the recipient is invalid
   *         EMAIL_NOT_SENT if the email could not be sent with other reasons
   */
  EmailError sendEmail(const Email &email, const std::string &passwd);

  /**
   * @brief Get all emails in box
   * @param address: email address of the user
   * @param passwd: password for the user
   * @return set of email IDs to Email objects
   */
  const std::set<long long> getEmails(const std::string &address,
                                      const std::string &passwd) const;
  /**
   * @brief Get an email by ID
   * @param address: email address of the user
   * @param passwd: password for the user
   * @param emailId: ID of the email to retrieve
   * @return pointer to the Email object if found, nullptr if not found or error
   */
  const Email *getEmailById(const std::string &address,
                            const std::string &passwd, long long emailId) const;
  /**
   * @brief Delete an email by ID
   * @param address: email address of the user
   * @param passwd: password for the user
   * @param emailId: ID of the email to delete
   * @retval NONE if the email is deleted successfully,
   *         WRONG_SENDER_OR_PASSWORD if the sender's email or password is
   *         incorrect,
   *         EMAIL_NOT_FOUND if the email with the given ID does not exist
   */
  EmailError deleteEmailById(const std::string &address,
                             const std::string &passwd, long long emailId);
};

#endif // EMAIL_SERVER_H
