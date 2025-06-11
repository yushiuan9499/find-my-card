#ifndef EMAIL_SERVER_H
#define EMAIL_SERVER_H

#include <map>
#include <set>
#include <string>

enum EmailError {
  NONE = 0,
  EMAIL_NOT_SENT,
  INVALID_RECIPIENT,
  WRONG_SENDER_OR_PASSWORD,
  EMAIL_NOT_FOUND,
};

struct Email {
  std::string subject;
  std::string body;
  std::string sender;
  std::string recipient; // Set of recipient email addresses
};

class EmailServer {
private:
  // address -> id
  std::map<std::string, long long> addressId;
  // id -> password mapping
  std::map<long long, std::string> idPasswd;
  std::map<long long, std::map<long long, Email *>>
      emails;       // id -> email ID -> Email object
  long long nextId; // Next available ID for new users
  /**
   * @brief Check if the email&password match
   * @param address: email address of the user
   * @param passwd: password for the user
   * @return true if the email and password match, false otherwise
   */
  bool checkPasswd(const std::string &address, const std::string &passwd);

protected:
public:
  EmailServer();
  virtual ~EmailServer();
  /**
   * @brief Add a addres to the email server
   * @param address: email address of the user
   * @param passwd: password for the user
   * @return true if the user is added successfully, false if the address
   * already exists
   */
  bool addAddress(const std::string &address, const std::string &passwd);

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
   * @return map of email IDs to Email objects
   */
  const std::map<long long, Email *> &getEmails(const std::string &address,
                                                const std::string &passwd);
  /**
   * @brief Get an email by ID
   * @param address: email address of the user
   * @param passwd: password for the user
   * @param emailId: ID of the email to retrieve
   * @return pointer to the Email object if found, nullptr if not found or error
   */
  Email const *getEmailById(const std::string &address,
                            const std::string &passwd, long long emailId);
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
