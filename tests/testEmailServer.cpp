#include "EmailServer.h"
#include <iostream>
#include <string>
using namespace std;

struct Person {
  string emailAddress;
  string passwd;
};

int main() {
  // Example usage of the Server class
  EmailServer emailServer; // Assuming EmailServer is defined elsewhere
  Person people[4] = {{"nobody@gmail.com", "passwordOfNobody"},
                      {"hero@yahoo.com", "passwordOfHero"},
                      {"admin@gmail.com", "passwordOfAdmin"},
                      {"stuednt@gmail.com", "passwordOfStudent"}};
  /* Test add user */
  for (int i = 0; i < 4; i++) {
    if (emailServer.addAddress(people[i].emailAddress, people[i].passwd)) {
      cerr << "Failed to add user " << people[i].emailAddress << "." << endl;
      return -1;
    }
  }
  if (emailServer.addAddress("wrongEmail", "passwordOfWrongEmail") !=
      INVALID_ADDRESS) {
    cerr << "Added user with invalid email, which should not happen." << endl;
    return -1;
  }
  if (emailServer.addAddress("short@gmail.com", "short") != INVALID_PASSWORD) {
    cerr << "Added user with short password, which should not happen." << endl;
    return -1;
  }
  cout << "\033[32mAll add user tests passed.\033[0m" << endl;
  return 0;
}
