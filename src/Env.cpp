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
