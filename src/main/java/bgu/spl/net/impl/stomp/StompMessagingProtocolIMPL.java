package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

public class StompMessagingProtocolIMPL implements StompMessagingProtocol<String>{
    private ConnectionsImpl<String> connections;
    private int connectionId;
    private boolean shouldTerminate;
    private static AtomicInteger msgId = new AtomicInteger(0);
    private boolean isLogged;

    public void start(int connectionId, Connections<String> connections){
        this.connectionId = connectionId;
        this.connections = (ConnectionsImpl<String>)connections;
        shouldTerminate = false;
        isLogged = false;
    }

    public void process(String message){
        String [] lines = message.split("\n");
        String command = lines[0];
        if(command.equals("CONNECT")){
            processConnect(message, lines);
        }
        else{
            if(!isLogged){
                sendFrame(Frame.errorFrame(validateAndGetHeader("receipt-id", lines), "malformed frame received",
                        message, "command is not found"));
            }
            if(command.equals("SEND"))
                processSend(message, lines);
            if(command.equals("SUBSCRIBE"))
                processSubscribe(message, lines);
            if(command.equals("UNSUBSCRIBE"))
                processUnsubscribe(message, lines);
            if(command.equals("DISCONNECT"))
                processDisconnect(message, lines); 
            }
        }

    public void processConnect(String originMessage, String [] lines){
        String [] expectedHeaders = {"accept-version", "host", "login", "passcode"};
        Frame outputFrame = null;
        String receipt = validateAndGetHeader("receipt-id", lines);
        if(processHeadersValidation(expectedHeaders, lines)){
            String version = validateAndGetHeader("accept-version", lines);
            String host = validateAndGetHeader("host", lines);
            if(version.equals("1.2") && host.equals("stomp.cs.bgu.ac.il")){
                String userName = validateAndGetHeader("login", lines);
                String password = validateAndGetHeader("passcode", lines);

                String loginMessage = checkLogin(userName, password);
                if(loginMessage.equals("connected successfully")) {
                    isLogged = true;
                    outputFrame = Frame.connectFrame();
                }
                else
                    outputFrame = Frame.errorFrame(receipt, "malformed frame received", originMessage, loginMessage);
            }
            else
                outputFrame = Frame.errorFrame(receipt, "malformed frame received", originMessage, "version or host invalid");
        }
        sendFrame(outputFrame);
    }

    public void processDisconnect(String originMessage, String [] lines){
        String [] expectedHeaders = {"receipt-id"};
        if(processHeadersValidation(expectedHeaders, lines)){
            String receipt = validateAndGetHeader("receipt-id", lines);
            connections.send(connectionId,Frame.receiptFrame(receipt).frameToString());
            connections.disconnect(connectionId);//check if user matches client
            terminate();
        }
    }


    private void processSend(String originMessage, String [] lines){
        String [] expectedHeaders = {"destination"};
        Frame outputFrame = null;
        if(processHeadersValidation(expectedHeaders, lines)){
            String destination = validateAndGetHeader("destination", lines);
            if(connections.isUserSubscribed(connectionId, destination)){
                int firstEmptyInd = 0;
                while(!lines[firstEmptyInd].isEmpty()){
                    firstEmptyInd++;
                }
                String body = String.join("\n", Arrays.copyOfRange(lines, firstEmptyInd + 1, lines.length));
                for(Integer subscriber : connections.getSubscribers(destination)){
                    Integer subId = connections.getSubIdForTopic(subscriber, destination);
                    if(subId != null) {
                        Frame messageFrame = Frame.messageFrame(subId.toString(), destination, body, getMsgId());
                        connections.send(subscriber, messageFrame.frameToString());
                    }
                }
                sendReceipt(lines);
            }
            else{
                outputFrame = Frame.errorFrame(validateAndGetHeader("receipt-id", lines),
                        "malformed frame received", originMessage, "user is not subscribed to this topic" );
            }
        }
        sendFrame(outputFrame);
    }

    private void processSubscribe(String originMessage, String [] lines){
        String [] expectedHeaders = {"destination", "id", "receipt"};
        Frame outputFrame = null;
        if(processHeadersValidation(expectedHeaders, lines)){
            String destination = validateAndGetHeader("destination", lines);
            String subIdString = validateAndGetHeader("id", lines);
            try {
                Integer subId = Integer.parseInt(subIdString);
                connections.newTopic(destination); // not creating new one if topic already exists
                connections.subscribeUser(destination, subId, connectionId);
                sendReceipt(lines);
            } catch (NumberFormatException nfe) {
                outputFrame = Frame.errorFrame(validateAndGetHeader("receipt-id", lines),
                        "malformed frame received", originMessage, "provided subscription id is not a number" );
                sendFrame(outputFrame);        
            }
        }
        
    }

    private void processUnsubscribe(String originMessage, String [] lines){
        String [] expectedHeaders = {"id"};
        Frame outputFrame = null;
        if(processHeadersValidation(expectedHeaders, lines)){
            String subIdString = validateAndGetHeader("id", lines);
            try {
                Integer subId = Integer.parseInt(subIdString);
                connections.unSubscribeUser(subId, connectionId);
                sendReceipt(lines);
            } catch (NumberFormatException nfe) {
                outputFrame = Frame.errorFrame(validateAndGetHeader("receipt-id", lines),
                        "malformed frame received", originMessage, "provided subscription id is not a number" );
            }
        }
        sendFrame(outputFrame);
    }

    private boolean processHeadersValidation(String [] expectedHeaders, String [] lines){
        for(String h : expectedHeaders){
            if(validateAndGetHeader(h,lines).isEmpty()) {
                Frame eFrame = Frame.errorFrame(validateAndGetHeader("receipt-id", lines), "headers are not as required",
                        String.join("\n", lines), "header \"" + h + "\" is invalid");
                connections.send(connectionId, eFrame.frameToString());
                return false;
            }
        }
        return true;
    }

    public String checkLogin(String userName, String password) {
        if (isLogged)
            return "user already logged in";
        if (connections.userToPassword.containsKey(userName)) { // user exists
            if (connections.userToPassword.get(userName).equals(password)) { //user's password is correct
                return "connected successfully";
            } else {
                return "wrong password";
            }
        } else {
            connections.addToUserPassword(userName, password); // adding new username
            return "connected successfully";
        }
    }

    private String validateAndGetHeader(String header, String [] lines) {
        for (String l : lines) {
            if(l.isEmpty())
                return "";
            String[] headers = l.split(":");
            if (headers[0].equals(header))
                return headers[1];
        }
        return "";
    }

    public boolean shouldTerminate(){
        return shouldTerminate;
    }

    public void terminate(){
        isLogged = false;
        shouldTerminate = true;
    }

    private void sendFrame(Frame frame){
        if(frame != null){
            System.out.println(frame.frameToString());
            connections.send(connectionId, frame.frameToString());
            if(frame.getCommand().equals("ERROR")){
                terminate();
            }
        }
    }

    /**
     * sending receipt frame if needed
     */
    private void sendReceipt(String [] message){
        String receipt = validateAndGetHeader("receipt", message);
        if(receipt != ""){
            connections.send(connectionId, Frame.receiptFrame(receipt).frameToString());
        }
    }

    public int getMsgId(){
        return msgId.incrementAndGet();
    }
}
