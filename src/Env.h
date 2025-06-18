#ifndef ENV_H
#define ENV_H

#include "Core/JvTime.h"

class Env {
private:
  static JvTime now; // Current time in the environment
public:
  Env() = delete;          // Prevent instantiation of Env class
  virtual ~Env() = delete; // Prevent deletion of Env class

  /**
   * @brief Get the current time in the environment
   * @return JvTime object representing the current time
   */
  static JvTime getNow();
  /**
   * @brief Get the current time in the environment as a string
   * @return String representation of the current time
   */
  static std::string getNowStr();

  /**
   * @brief Set the current time in the environment
   * @param newTime: JvTime object representing the new time to set
   */
  static void setNow(const JvTime &newTime);
  /**
   * @brief Set the current time in the environment from a string
   * @param newTimeStr: String representation of the new time to set
   */
  static void setNow(const std::string &newTimeStr);

  /**
   * @brief Move the current time forward by a specified amount
   * @param hours: Number of hours to move forward
   * @param minutes: Number of minutes to move forward
   * @param seconds: Number of seconds to move forward
   */
  static void moveNow(int hours, int minutes, int seconds);
};
#endif // ENV_H
