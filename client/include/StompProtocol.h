
#include "ConnectionHandler.h"
#include "Game.h" 
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


using namespace std;
using std::unordered_map;

class StompProtocol {
private:
    int receiptCounter;
    int disconnectReceipt;
    int subId;
    string user;

    ConnectionHandler* handler;
    unordered_map<string, int> gameToId; 
    unordered_map<int, string> idToGame;
    unordered_map<int, pair<string, string>> receiptToCommand;
    unordered_map<string, Game> games;

    string connected();
    string login(vector<string> msg);
    string join(vector<string> msg);
    string exit(vector<string> msg);
    string report(vector<string> msg);
    string summary(vector<string> msg);
    string logout(vector<string> msg);
    string frameCreator(string command, string headers, string body);
    string sendFrame(string msg, string topic);
    string receipt (vector<string> tokens);
    string error(vector<string> tokens);
    string message(vector<string> msg);
    string findHeader (string head,vector <string> msg);
    map<string, string> generateStats(vector<string> input, pair<int,int> range);
    pair<int, int> findStatsInd(vector<string> lines, string type);
    string statsToString(unordered_map<string, string> stats);
    void disconnect();

public:
    StompProtocol(ConnectionHandler & handler);
    StompProtocol(const StompProtocol &other);
    StompProtocol & operator=(const StompProtocol &other);
    virtual ~StompProtocol();
    StompProtocol(StompProtocol && other);
    StompProtocol & operator=(StompProtocol &&other);

    string processServerFrame(string msg);
    string processKeyboard(string msg);
    

};
