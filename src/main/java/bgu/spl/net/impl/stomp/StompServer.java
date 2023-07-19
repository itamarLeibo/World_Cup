package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.*;
public class StompServer{

public static void main(String[] args) {
        String type = args[1];
        // String type = "tpc";
        int port = Integer.parseInt(args[0]);
        // int port = 7777;
        if(type.equals("tpc")){
                Server.threadPerClient(port, //port
                () -> new StompMessagingProtocolIMPL(), //protocol factory
                StompMessageEncoderDecoder::new //message encoder decoder factory
                 ).serve();
        }
        if(type.equals("reactor")){
                Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                port, //port
                () -> new StompMessagingProtocolIMPL(), //protocol factory
                StompMessageEncoderDecoder::new //message encoder decoder factory
                ).serve();
        }
    }
}
