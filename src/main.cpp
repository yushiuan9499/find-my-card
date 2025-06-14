#include "Box.h"
#include "Card.h"
#include "EmailServer.h"
#include "Env.h"
#include "Server.h"
#include "User.h"
#include <fstream>
#include <iostream>
using namespace std;

int main() {
  // Load scenario from JSON file
  char scenarioFile[] = "./json/scenario1.json";
  EmailServer emailServer;
  Server server{"server@gmail.com", "serverPassword123", &emailServer};
  Json::Value scenarioJson;
  if (myFile2JSON(scenarioFile, &scenarioJson) != EE1520_ERROR_NORMAL) {
    cerr << "Failed to read scenario file: " << scenarioFile << endl;
    return -1;
  }
  User yushiuan9499{&server, &emailServer, &scenarioJson["yushiuan9499"]};
  User tobiichi3227{&server, &emailServer, &scenarioJson["tobiichi3227"]};
  Box box1{&server, &scenarioJson["box1"]};

  // Add the card to the server
  yushiuan9499.addCardToServer(string("31415926535"));

  Env::setNow(string("2025-06-01T12:00:00+0800"));
  // yushiuan9499 lost his card
  Card *tmpCard = yushiuan9499.removeCard(string("31415926535"));
  cout << "yushiuan9499: " << yushiuan9499.dump2JSON()->toStyledString()
       << endl;
  // tobiichi3227 found the card
  tobiichi3227.addCard(tmpCard);
  cout << "tobiichi3227: " << tobiichi3227.dump2JSON()->toStyledString()
       << endl;
  // tobiichi3227 put the card into box1
  tobiichi3227.dropCard(&box1, tmpCard);
  cout << "box1: " << box1.dump2JSON()->toStyledString() << endl;
  // yushiuan9499 receive the email
  yushiuan9499.readMail(*yushiuan9499.getEmailIds().begin());
  // yushiuan9499 retrieve the card from box1
  yushiuan9499.retrieveCard(&box1, string("31415926535"), string("998244353"));
  // print the state of all users and the box
  cout << "yushiuan9499: " << yushiuan9499.dump2JSON()->toStyledString()
       << endl;
  cout << "tobiichi3227: " << tobiichi3227.dump2JSON()->toStyledString()
       << endl;
  cout << "box1: " << box1.dump2JSON()->toStyledString() << endl;
  // tobiichi3227 check its reward
  cout << "tobiichi3227's reward: " << tobiichi3227.readReward() << endl;
  // tobiichi3227 redeem its reward
  cout << tobiichi3227.redeemReward(&box1, "27182818284") << endl;
  cout << "tobiichi3227 after redeem: "
       << tobiichi3227.dump2JSON()->toStyledString() << endl;
  cout << "tobiichi3227's reward after redeem: " << tobiichi3227.readReward()
       << endl;
  return 0;
}
