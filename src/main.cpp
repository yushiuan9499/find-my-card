#include "Box.h"
#include "Card.h"
#include "EmailServer.h"
#include "Env.h"
#include "Server.h"
#include "User.h"
#include <fstream>
#include <iostream>
using namespace std;

struct AppContext {
  std::map<std::string, User *> &users;
  std::map<std::string, Card *> &cards;
  EmailServer &emailServer;
  Server &server;
  Box &box1;
};

void dumpJSON(const AppContext &appContext, const std::string &outputFile,
              const std::string &desc = "") {
  Json::Value json;
  for (const auto &userPair : appContext.users) {
    json["users"].append(*userPair.second->dump2JSON());
  }
  json["box1"] = *appContext.box1.dump2JSON();
  json["server"] = *appContext.server.dump2JSON();
  json["emailServer"] = *appContext.emailServer.dump2JSON();
  for (const auto &cardPair : appContext.cards) {
    json["cardsLost"].append(*cardPair.second->dump2JSON());
  }
  json["now"] = Env::getNowStr();
  json["!description"] = desc;

  ofstream ofs(outputFile);
  if (ofs.is_open()) {
    ofs << json.toStyledString();
    ofs.close();
  } else {
    cerr << "Failed to open output file." << endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <json_file_dir>" << endl;
    return -1;
  }

  // Load scenario from JSON file
  string scenarioFile = argv[1] + string("/scenario0.json");
  Json::Value scenarioJson;
  if (myFile2JSON(scenarioFile.c_str(), &scenarioJson) != EE1520_ERROR_NORMAL) {
    cerr << "Failed to read scenario file: " << scenarioFile << endl;
    return -1;
  }
  try {
    // Initialize environment
    map<string, User *> users;
    map<string, Card *> cards;
    EmailServer emailServer;
    Server server{&emailServer, &scenarioJson["server"]};
    for (int i = 0; i < scenarioJson["users"].size(); i++) {
      string userId = scenarioJson["users"][i]["username"].asString();
      users[userId] =
          new User(&server, &emailServer, &scenarioJson["users"][i]);
    }
    Box box1{&server, &scenarioJson["box1"]};
    Env::setNow(string("2025-06-01T12:00:00+0800"));
    AppContext appContext{users, cards, emailServer, server, box1};

    // Process actions from JSON file
    string actionFile = argv[1] + string("/actions.json");
    Json::Value actionsJson;
    if (myFile2JSON(actionFile.c_str(), &actionsJson) != EE1520_ERROR_NORMAL) {
      cerr << "Failed to read action file: " << actionFile << endl;
      return -1;
    }

    for (int i = 0; i < actionsJson.size(); i++) {
      string action = actionsJson[i]["action"].asString();
      string who = actionsJson[i]["who"].asString();
      User &user = *users[who];
      // Related to card
      if (action == "addCard") {
        string cardId = actionsJson[i]["cardId"].asString();
        user.addCardToServer(cardId);
      } else if (action == "removeCard") {
        string cardId = actionsJson[i]["cardId"].asString();
        cards[cardId] = user.removeCard(cardId);
      } else if (action == "getCard") {
        string cardId = actionsJson[i]["cardId"].asString();
        user.addCard(cards[cardId]);
        cards.erase(cardId);
      } else if (action == "dropCard") {
        string cardId = actionsJson[i]["cardId"].asString();
        user.dropCard(&box1, cardId);
      } else if (action == "retrieveCard") {
        string cardId = actionsJson[i]["cardId"].asString();
        string paymentCardId = actionsJson[i]["paymentCardId"].asString();
        user.retrieveCard(&box1, cardId, paymentCardId);
      }
      // Related to email
      else if (action == "readMail") {
        int mailId = actionsJson[i]["mailId"].asInt();
        user.readMail(mailId);
      }
      // Related to server
      else if (action == "setVerificationType") {
        string verificationType = actionsJson[i]["verificationType"].asString();
        if (verificationType == "EMAIL") {
          user.setVerificationType(UserInfo::EMAIL);
        } else if (verificationType == "APP") {
          user.setVerificationType(UserInfo::APP);
        } else {
          cerr << "Unknown verification type: " << verificationType << endl;
          return -1;
        }
      } else if (action == "redeemReward") {
        string cardId = actionsJson[i]["cardId"].asString();
        int amount = actionsJson[i]["amount"].asInt();
        user.redeemReward(&box1, cardId, amount);
      } else if (action == "rejectRetrieve") {
        string cardId = actionsJson[i]["cardId"].asString();
        user.rejectRetrieve(cardId);
      } else {
        cerr << "Unknown action: " << action << endl;
        return -1;
      }
      Env::moveNow(1, 0, 0);
      dumpJSON(appContext,
               argv[1] + string("/scenario") + to_string(i + 1) +
                   string(".json"),
               "Scenario after action " + to_string(i + 1) + ": " + action);
    }
  } catch (ee1520_Exception &e) {
    cerr << "Exception occurred: " << e.dump2JSON()->toStyledString() << endl;
    return -1;
  }
  return 0;
}
