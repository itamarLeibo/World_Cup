

#include <sstream>
#include <fstream>
#include <vector>
#include "../include/StompProtocol.h"
#include "../include/event.h"

using namespace std;

StompProtocol::StompProtocol(ConnectionHandler& handler): receiptCounter(1), disconnectReceipt(-1),subId(1),user(""), handler(&handler),
                                                            gameToId(unordered_map<string, int>()),
                                                            idToGame(unordered_map<int, string>()),
                                                            receiptToCommand(unordered_map<int, pair<string, string>>()),
                                                            games(unordered_map<string, Game>()){
}


StompProtocol::StompProtocol(const StompProtocol &other):receiptCounter(1), disconnectReceipt(-1),subId(0),user(""), handler(other.handler),
                                                            gameToId(other.gameToId),
                                                            idToGame(other.idToGame),
                                                            receiptToCommand(other.receiptToCommand),
                                                            games(other.games){

}

StompProtocol& StompProtocol::operator=(const StompProtocol &other){
    if(&other != this){
        receiptCounter = other.receiptCounter;
        disconnectReceipt = other.disconnectReceipt;
        subId = other.subId;
        user = other.user;
        handler = other.handler;
        gameToId =other.gameToId;
        idToGame = other.idToGame;
        receiptToCommand = other.receiptToCommand;
        games = other.games;
    }
    return *this;
}


StompProtocol::~StompProtocol()
{
}

StompProtocol::StompProtocol(StompProtocol && other):receiptCounter(1), disconnectReceipt(-1),subId(0),user(""), handler(other.handler),
                                                            gameToId(other.gameToId),
                                                            idToGame(other.idToGame),
                                                            receiptToCommand(other.receiptToCommand),
                                                            games(other.games){
    other.handler = nullptr;
    
}

string StompProtocol::processKeyboard(string msg) {
    vector<string> input;
    string s;
    stringstream ss(msg);
    while (getline(ss, s, ' ')) {
        input.push_back(s);
    }
    string outputFrame="";
    if(input[0]=="login")
        outputFrame=login(input);
    if(handler->getIsConnected()) {
        if (input[0] == "join")
            outputFrame = join(input);
        if (input[0] == "exit")
            outputFrame = exit(input);
        if (input[0] == "report")
            outputFrame = report(input);
        if (input[0] == "summary")
            outputFrame = summary(input);
        if (input[0] == "logout")
            outputFrame = logout(input);
    }
    return outputFrame;
}

string StompProtocol::login(vector<string> msg) {
    string out = "";
    if(!handler->getIsConnected()) {
        vector<string> hostPort;
        string s;
        stringstream ss(msg[1]);
        while(getline(ss, s, ':')){
            hostPort.push_back(s);
        }
        string host = hostPort[0];
        handler->setHost(host);
        short port = std::stoi(hostPort[1]);
        handler->setPort(port);
        if(!handler->connect()){
            cout << "Could not connect to " + host + " and port " + to_string(port) << endl;
            return "";
        }
        this->user = msg[2];
        string headers = "accept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:" + msg[2] + "\n" +  "passcode:" + msg[3] + "\n" + "receipt-id:" + to_string(receiptCounter);
        out = frameCreator("CONNECT", headers, "");
        receiptToCommand.insert(pair<int, pair<string,string>>(receiptCounter,pair<string,string>(msg[0], "")));
        receiptCounter++;
    }
    else{
        cout<<"The client is already logged in, log out before trying again"<<endl;
    }
    return out;
}

string StompProtocol::join(vector<string> msg) {
    string game = msg[1];
    string out = "";
    if(gameToId.count(game)==0){
        string headers = "destination:" + game + "\n" + "id:" + to_string(subId) +
            "\n" + "receipt:"+to_string(receiptCounter);
        out = frameCreator("SUBSCRIBE",headers,"");
        gameToId.insert(std::make_pair(game, subId));
        idToGame.insert(std::make_pair(subId, game));
        games.insert(std::make_pair(game, Game(game)));
        receiptToCommand.insert(pair<int, pair<string,string>>(receiptCounter,pair<string,string>(msg[0], game)));
        receiptCounter++;
        subId++;
    }
    return out; 
}


string StompProtocol::exit(vector<string> msg) {
    string game = msg[1];
    string out = "";
    if(gameToId.count(game)!=0){
        int subId = gameToId.at(game);
        string headers = "id:" + to_string(subId) + "\n" + "receipt:" + to_string(receiptCounter) ;
        out = frameCreator("UNSUBSCRIBE", headers, "");
        gameToId.erase(game);
        idToGame.erase(subId);
        games.erase(game);
        receiptToCommand.insert(pair<int, pair<string,string>>(receiptCounter,pair<string,string>(msg[0], game)));
        receiptCounter++;
    }
    return out;
}

string StompProtocol::report(vector<string> msg) {
    string json = msg.at(1);
    names_and_events n = parseEventsFile(json);
    for(Event e: n.events){
        string team_a = e.get_team_a_name();
        string team_b = e.get_team_b_name();
        string game_name = team_a + "_" + team_b;
        if(games.count(game_name) == 0)
        {
            cout << "you are unsubscribed to game: " + game_name + "!"<< endl;
        }
        else{
            string eventTime = to_string(e.get_time());
            string eventName = e.get_name();
            map<string, string> newGeneral = e.get_game_updates();
            string generalStr = "";
            for(auto i = newGeneral.begin(); i!=newGeneral.end(); i++){
                generalStr += i->first + ":" + i->second + "\n";
            }

            map<string, string> newAStats = e.get_team_a_updates();
            string teamAStats = "";
            for(auto i = newAStats.begin(); i!=newAStats.end(); i++){
                teamAStats += i->first + ":" + i->second + "\n";
            }

            map<string, string> newBStats = e.get_team_b_updates();
            string teamBStats = "";
            for(auto i = newBStats.begin(); i!=newBStats.end(); i++){
                teamBStats += i->first + ":" + i->second + "\n";
            }

            
            string description = e.get_discription();

            string headers = "destination:" + game_name + "\n" + "receipt:" +to_string(receiptCounter);
            receiptToCommand.insert(pair<int, pair<string,string>>(receiptCounter,pair<string,string>(msg[0], game_name)));
            receiptCounter++;
            string body = "user:" + user + "\n" + "team a:" + team_a + "\n" + "team b:" + team_b 
                            + "\n" + "event name:" + eventName + "\n" + "time:" + eventTime + "\n" +
                            "general game updates:" + "\n" + generalStr + "team a updates:" + "\n"
                            + teamAStats + "team b updates:" + "\n" + teamBStats + 
                            "description:" + "\n" + description;
            string outputFrame = frameCreator("SEND",headers, body);

            handler->sendFrameAscii(outputFrame, '\0');
        }
    }
    return "";
    
}

string StompProtocol::summary(vector<string> msg) {
    string gameName = msg[1];
    string sentUser = msg[2];
    string file = msg[3];
    if(games.count(gameName) > 0){
        Game & currGame = games.at(gameName);
        pair<string,string> gamePair = currGame.getName();
        string generalStats = statsToString(currGame.getuserToGeneralStats()[sentUser]);
        string teamAStats = statsToString(currGame.getuserToTeamAstats()[sentUser]);
        string teamBStats = statsToString(currGame.getuserToGeneralStats()[sentUser]);
        string events = currGame.getUserToEvents()[sentUser];
        string out = gamePair.first + " vs " + gamePair.second + 
                    "\n" + "Game stats:"+ "\n" + "General stats:" + "\n" +
                    generalStats + gamePair.first + " stats:" + "\n" + teamAStats
                     + gamePair.second  + " stats:" + "\n" + teamBStats + "\n" 
                    + "Game event reports:" + "\n" + events;
        ofstream stream;
        stream.open(file, ios::trunc);
        if(stream){
            stream<<out<<endl;
            cout << "Summarized and written in the file" << endl;
        }
        else{
            cout << "couldn't create the file" << endl;
        }
        stream.close();
        
    }

    else{
        cout<<"you are unsubscribed to game " + gameName + "!"<<endl;
    }
    return "";   
}

string StompProtocol::logout(vector<string> msg) {
    string out = frameCreator("DISCONNECT", "receipt-id:" + to_string(receiptCounter), "");
    receiptToCommand.insert(pair<int, pair<string,string>>(receiptCounter,pair<string,string>(msg[0], "")));
    disconnectReceipt = receiptCounter;
    receiptCounter++;
    return out;
}

string StompProtocol::frameCreator(string command, string headers, string body){
    string out = command + "\n";
    out += headers + "\n\n";
    out += body ;
    cout << out << endl;
    return out;

}

string StompProtocol::processServerFrame(string msg) {
    vector<string> input;
    string s;
    stringstream ss(msg);
    while (getline(ss, s, '\n')) {
        // store token string in the vector
        input.push_back(s);
    }

    string out="";
    if(input[0] == "CONNECTED")
        out = "login successfully";
    if(input[0] == "MESSAGE")
        out = message(input);
    if(input[0] == "RECEIPT") 
        out = receipt(input);
    if(input[0] == "ERROR")
        out = error(input); 
    return out;        
    
}

string StompProtocol::message(vector<string> input){
    string game = findHeader("destination", input);
    if(games.count(game) > 0){
        string eventName =findHeader("event name", input); 
        string time = findHeader("time", input);
        string description = input[input.size()-1];
        string fromUser = findHeader("user", input);

        pair<int, int> gUpdatesRange = findStatsInd(input, "G");
        pair<int, int> aUpdatesRange = findStatsInd(input, "A");
        pair<int, int> bUpdatesRange = findStatsInd(input, "B");

        map<string, string> newGeneral = generateStats(input, gUpdatesRange);
        map<string, string> newAStats = generateStats(input, aUpdatesRange);
        map<string, string> newBStats = generateStats(input, bUpdatesRange);

        string event = time + " - " + eventName + ":" + "\n\n" + description + "\n\n";
        Game & currGame = games.at(game);
        currGame.addEvent(fromUser, event);
        currGame.updateStats("general",newGeneral, fromUser);
        currGame.updateStats("teamA",newAStats, fromUser);
        currGame.updateStats("teamB",newBStats, fromUser);

        int firstEmptyInd=0;
        string out ="\nmessage receieved from user " + fromUser + "\n";
        while(!input[firstEmptyInd].empty()){
            firstEmptyInd++;
        }
        int linesNum = input.size();
        for(int i = firstEmptyInd+1 ; i <= linesNum-1 ; i++){
            out += input[i] + "\n";
        }
        return out;
    }
    return "";
}

string StompProtocol::receipt(vector<string> input){
    string out = "";
    string receiptId = input[1].substr(input[1].find(":")+1, input[1].length()); //maybe change to work with find header
    int receiptGot=std::stoi(receiptId);
    if (receiptGot==disconnectReceipt){
        disconnect();
        handler->close();
        return "disconnected";

    }
    if(receiptToCommand.count(receiptGot) != 0){
        string command = receiptToCommand.at(receiptGot).first;
        string game = receiptToCommand.at(receiptGot).second;
        if(command == "join")
            out = "Joined channel " + game;
        if(command == "exit")
            out = "Exited channel " + game;
        if(command == "report"){
            out = "Reported to channel " + game + "! receipt: " + receiptId;
        }
    }
    return out; 
}

string StompProtocol::error(vector<string> input){
    string out = "";
    for(string line: input){
        out += line + "\n";
    }
    disconnect();
    handler->close();
    return out;
}
    

void StompProtocol::disconnect(){
    gameToId.clear();
    idToGame.clear();
    receiptToCommand.clear();

}


string StompProtocol::findHeader(string header,vector<string> msg) {
    for (string line : msg){
        int ind = line.find(":");
        if (line.substr(0,ind)==header)
            return line.substr(ind+1, line.size()-1);
    }
    return "";
}

map<string, string> StompProtocol::generateStats(vector<string> input, pair<int,int> range){
    map<string, string> out;
    if(range.first != -1){
        for(int i = range.first; i<=range.second ; i++){
            int ind = input[i].find(":");
            string headerKey = input[i].substr(0, ind);
            string headerVal = input[i].substr(ind + 1, input[i].length());
            out[headerKey] = headerVal;
        }
    }
    return out;
}

pair<int, int> StompProtocol::findStatsInd(vector<string> lines, string type){
    string startKey = "";
    string endKey = "";
    int startInd = 0;
    if(type == "G"){
        startKey = "general game updates:";
        endKey = "team a updates:";
    }
    if(type == "A"){
        startKey = "team a updates:";
        endKey = "team b updates:";
    }
    if(type == "B"){
        startKey = "team b updates:";
        endKey = "description:";
    }
    while(lines[startInd] != startKey)
        startInd++;
    int endInd = startInd;
    startInd++;
    while(lines[endInd] != endKey)
        endInd++;
    endInd--;
    if(endInd < startInd) // no stats 
        return {-1,-1};
    return {startInd, endInd};
}

string StompProtocol::statsToString(unordered_map<string, string> stats){
    string out = "";
    for(auto stat : stats){
        out+= stat.first + ":" + stat.second + "\n";
    }
    return out;
}




