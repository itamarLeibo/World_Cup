#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>

using namespace std;
class Game
{
private:
    string gameName;
    unordered_map<string, string> userToEvents;
    unordered_map<string, unordered_map<string, string>> userToGeneralStats;
    unordered_map<string, unordered_map<string, string>> userToTeamAstats;
    unordered_map<string, unordered_map<string, string>> userToTeamBstats;

public:
    Game(std::string gameName);
    virtual ~Game();
    Game(const Game &other);
    Game & operator=(const Game &other);
    Game & operator=(Game &&other);

    pair<string,string> getName();
    void addEvent(string user, string event);
    void updateStats(string statsType, map<string, string> newStats, string user);
    unordered_map<string, string> getUserToEvents();
    unordered_map<string, unordered_map<string, string>> getuserToGeneralStats();
    unordered_map<string, unordered_map<string, string>> getuserToTeamAstats();
    unordered_map<string, unordered_map<string, string>> getuserToTeamBstats();
};


