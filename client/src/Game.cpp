#include "../include/Game.h"

Game::Game(std::string gameName):gameName(gameName),
                        userToEvents(unordered_map<string, string>()),
                        userToGeneralStats(unordered_map<string, unordered_map<string, string>>()),
                        userToTeamAstats(unordered_map<string, unordered_map<string, string>>()),
                        userToTeamBstats(unordered_map<string, unordered_map<string, string>>()){

}

Game::~Game(){}

Game::Game(const Game &other):gameName(other.gameName),
                                userToEvents(other.userToEvents),
                                userToGeneralStats(other.userToGeneralStats),
                                userToTeamAstats(other.userToTeamAstats),
                                userToTeamBstats(other.userToTeamBstats){
}

Game& Game::operator=(const Game &other){
    if(&other != this){
        gameName = other.gameName;
        userToEvents = other.userToEvents;
        userToGeneralStats = other.userToGeneralStats;
        userToTeamAstats = other.userToTeamAstats;
        userToTeamBstats = other.userToTeamBstats;
    }
    return *this;
}


Game& Game::operator=(Game &&other){
    if(&other != this){
        gameName = other.gameName;
        userToEvents = other.userToEvents;
        userToGeneralStats = other.userToGeneralStats;
        userToTeamAstats = other.userToTeamAstats;
        userToTeamBstats = other.userToTeamBstats;
    }
    return *this;
}


pair<string,string> Game::getName(){
    int ind = gameName.find("_");
    string teamA = gameName.substr(0,ind);
    string teamB = gameName.substr(ind + 1,gameName.size()-1); 
    return {teamA,teamB};
}

void Game::addEvent(string user, string event){
    if(userToEvents.count(user) > 0){
        string oldEvent = userToEvents.at(user);
        string newEvent = oldEvent + "\n" + event ;
        userToEvents[user] = newEvent;
    }
    else{
        userToEvents[user] = event;
    }
}

unordered_map<string, string> Game::getUserToEvents(){
    return userToEvents;
}

unordered_map<string, unordered_map<string, string>> Game::getuserToGeneralStats(){
    return userToGeneralStats;
}

unordered_map<string, unordered_map<string, string>> Game::getuserToTeamAstats(){
    return userToTeamAstats;
}

unordered_map<string, unordered_map<string, string>> Game::getuserToTeamBstats(){
    return userToTeamBstats;
}




void Game::updateStats(string statsType,map<string, string> newStats, string user){
    if(statsType == "general"){
        for (auto& stat : newStats){
            userToGeneralStats[user][stat.first] = stat.second;
        }
    }
    if(statsType == "teamA"){
        for (auto& stat : newStats){
            userToTeamAstats[user][stat.first] = stat.second;
        }
    }
    if(statsType == "teamB"){
        for (auto& stat : newStats){
            userToTeamBstats[user][stat.first] = stat.second;
        }
    }
}