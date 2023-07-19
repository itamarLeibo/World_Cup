
#include <thread>
#include "../include/keyboardReader.h"



int main(int argc, char *argv[]) {
    ConnectionHandler *handler = new ConnectionHandler();
    StompProtocol *protocol  = new StompProtocol(*handler);
    keyboardReader keyReader(*handler, *protocol);
    thread keyThread(&keyboardReader::run, &keyReader);
    while(1){
        if(handler->getIsConnected()) {
            string ans;
            if (!handler->getFrameAscii(ans, '\0')) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            string out = protocol->processServerFrame(ans);
            if (out != "")
                cout << out << endl;
        }
    }
    keyThread.join();
    delete(handler);
    delete(protocol);
}
