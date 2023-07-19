package bgu.spl.net.impl.stomp;

public class Frame{
    private String command;
    private String header;
    private String body;
    
    public Frame(String command){
        this.command = command;
        this.header = "";
        this.body = "";
    }

    public static Frame connectFrame(){
        Frame outFrame = new Frame ("CONNECTED");
        outFrame.header = "version:1.2";
        return outFrame;
    }

    public static Frame receiptFrame(String receiptId){
        Frame outFrame = new Frame ("RECEIPT");
        outFrame.header = "receipt-id:" + receiptId;
        return outFrame;
    }

    public static Frame messageFrame(String subscription, String destination, String body, int msgId){
        Frame outFrame = new Frame ("MESSAGE");
        outFrame.header = "subscription:" + subscription + "\n" + "message-id:" + msgId + "\n" + "destination:" + destination;
        outFrame.body = body;
        return outFrame;
    }
    
    public static Frame errorFrame(String receiptId, String messageHeader, String originMessage, String errorDetails){
        Frame outFrame = new Frame ("ERROR");
        if (receiptId != "")
            outFrame.header = "receipt-id: message-" + receiptId + "\n" + "message:" + messageHeader;
        else
            outFrame.header = "message:" + messageHeader;
        outFrame.body = "The message:" + "\n" + "-----" + "\n" + originMessage + "\n" + "-----" + "\n" +errorDetails;
        return outFrame;
    }

    public String getCommand(){
        return this.command;
    }

    public String frameToString(){
        return command + '\n' + header + '\n' + '\n' + body + '\n' ;
    }


}
