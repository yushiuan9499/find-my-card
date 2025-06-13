#include "Box.h"
#include "Card.h"
#include "EmailServer.h"
#include "Server.h"
#include "User.h"
#include <fstream>
#include <iostream>
using namespace std;

int main() {
  char scenarioFile[] = "./json/scenario1.json";
  EmailServer emailServer;
  Server server{"server@gmail.com", "serverPassword123", &emailServer};
  Json::Value scenarioJson;
  if (myFile2JSON(scenarioFile, &scenarioJson) != EE1520_ERROR_NORMAL) {
    cerr << "Failed to read scenario file: " << scenarioFile << endl;
    return -1;
  }
  User yushiuan9499{&scenarioJson["yushiuan9499"]};
  User tobiichi3227{&scenarioJson["tobiichi3227"]};
  Box box1{&scenarioJson["box1"]};
  cout << "User yushiuan9499: " << yushiuan9499.dump2JSON()->toStyledString()
       << endl;
  cout << "User tobiichi3227: " << tobiichi3227.dump2JSON()->toStyledString()
       << endl;
  cout << "Box box1: " << box1.dump2JSON()->toStyledString() << endl;
  return 0;
}
