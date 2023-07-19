
#include "../include/keyboardReader.h"

class ConnectionHandler;

keyboardReader::keyboardReader(ConnectionHandler &hand, StompProtocol &protocol):  handler(&hand),protocol(&protocol){
}


void keyboardReader::run() {
    while(1) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        const string out = protocol->processKeyboard(line);
        if(out!="") {
            handler->sendFrameAscii(out, '\0');
        }
    }
}

keyboardReader::keyboardReader(const keyboardReader &kt): handler(kt.handler),protocol(kt.protocol) {}
keyboardReader& keyboardReader::operator=(const keyboardReader &kt){
    handler=kt.handler;
    protocol=kt.protocol;
    return  *this;
}

keyboardReader::~keyboardReader() {
    delete(handler);
    delete(protocol);
}