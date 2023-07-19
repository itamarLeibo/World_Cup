
#include <mutex>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"

class keyboardReader {
private: 
    ConnectionHandler* handler;
    StompProtocol* protocol;
public:
    keyboardReader(ConnectionHandler& hand,StompProtocol& protocol);
    void run();
    keyboardReader(const keyboardReader& kt);
    keyboardReader & operator=(const keyboardReader &kt);
    ~keyboardReader();
};
