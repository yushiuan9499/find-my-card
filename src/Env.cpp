#include "Env.h"
using namespace std;

JvTime Env::now;

JvTime Env::getNow() {
  return now; // Return the current time in the environment
}

std::string Env::getNowStr() { return *now.getTimeString(); }

void Env::setNow(const JvTime &newTime) {
  now = newTime; // Set the current time in the environment
}

void Env::setNow(const std::string &newTimeStr) {
  now = JvTime(newTimeStr.c_str());
}

void Env::moveNow(int hours, int minutes, int seconds) {
  struct std::tm *tm_ptr = now.getStdTM();
  tm_ptr->tm_hour += hours;
  tm_ptr->tm_min += minutes;
  tm_ptr->tm_sec += seconds;

  // Normalize the time structure
  mktime(tm_ptr);
  now.setStdTM(tm_ptr);
  free(tm_ptr);
}
