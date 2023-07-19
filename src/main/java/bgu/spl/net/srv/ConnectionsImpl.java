package bgu.spl.net.srv;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class ConnectionsImpl <T> implements Connections <T>{
    static private AtomicInteger connCounter = new AtomicInteger(0);
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> idToHandler;

    private ConcurrentHashMap<String, ConcurrentLinkedQueue<Integer>> topicToConIds;
    private ConcurrentHashMap<Integer, ConcurrentHashMap<String, Integer>> conIdToTopicSubId;
    private ConcurrentHashMap<Integer, ConcurrentHashMap<Integer, String>> conIdToSubIdTopic;

    public ConcurrentHashMap<String,String> userToPassword;

    public ConnectionsImpl(){
        idToHandler = new ConcurrentHashMap<>();
        topicToConIds = new ConcurrentHashMap<>();
        conIdToTopicSubId = new ConcurrentHashMap<>();
        conIdToSubIdTopic = new ConcurrentHashMap<>();
        userToPassword = new ConcurrentHashMap<>();
    }

    public boolean send(int connectionId, T msg){
        if (idToHandler.containsKey(connectionId)){
            ConnectionHandler<T> handler = idToHandler.get(connectionId);
            synchronized(handler){
                idToHandler.get(connectionId).send(msg);
                if(msg.toString().startsWith("ERROR")){ // ERROR frame is sent
                    disconnect(connectionId);
                }
            }
            return true;
        }
        return false;
    }

    public void send(String channel, T msg){
//        unused
    }

    public void disconnect(int connectionId){
        if(idToHandler.containsKey(connectionId)){
            if(conIdToTopicSubId.containsKey(connectionId)){
                ConcurrentHashMap<String, Integer> userTopics = conIdToTopicSubId.get(new Integer(connectionId));
                for(Map.Entry<String, Integer> topicEntry : userTopics.entrySet()){ // removing user from topic perspective
                    String topic = topicEntry.getKey();
                    removeIdFromTopic(topic, connectionId);
                }
                // removing user from user's perspective
                conIdToTopicSubId.remove(connectionId);
                conIdToSubIdTopic.remove(connectionId);
            }
            idToHandler.remove(connectionId);
        }
    }

    public void subscribeUser(String topic, Integer subId, Integer conId){
        if(topicToConIds.containsKey(topic)){ //topic exists
            topicToConIds.get(topic).add(conId);
        }
        else{ //topic doesn't exist
            ConcurrentLinkedQueue<Integer> q = new ConcurrentLinkedQueue<>();
            q.add(conId);
            topicToConIds.put(topic, q);
        }

        if(conIdToSubIdTopic.get(conId) == null){ // the user is not following any topic
            ConcurrentHashMap<Integer, String> map= new ConcurrentHashMap<>();
            map.put(subId, topic);
            conIdToSubIdTopic.put(conId,map);
        }
        else
            conIdToSubIdTopic.get(conId).put(subId, topic);

        if(conIdToTopicSubId.get(conId) == null){ // the user is not following any topic
            ConcurrentHashMap<String, Integer> map= new ConcurrentHashMap<>();
            map.put(topic, subId);
            conIdToTopicSubId.put(conId,map);
        }
        else
            conIdToTopicSubId.get(conId).put(topic, subId);

    }

    public void unSubscribeUser(Integer subId, Integer conId){
        if(conIdToSubIdTopic.containsKey(conId)){
            String topic = conIdToSubIdTopic.get(conId).get(subId);
            topicToConIds.get(topic).remove(conId); //removes the user from the topic
            conIdToSubIdTopic.get(conId).remove(subId); //removes the subId (witch represents the topic) from the user
            if(conIdToTopicSubId.containsKey(conId)) {
                conIdToTopicSubId.get(conId).remove(topic); //removes the topic from the user
            }
        }
    }
    public void newTopic(String topic){
        if(!topicToConIds.containsKey(topic)){
            ConcurrentLinkedQueue<Integer> subscribers = new ConcurrentLinkedQueue<>();
            topicToConIds.put(topic,subscribers);
        }
    }

    public int getNewId(){
        return connCounter.incrementAndGet();
    }

    public void addConnectionB(BlockingConnectionHandler<T> handler){
        this.idToHandler.put(handler.getId(), handler);
    }

    public void addConnectionNB(NonBlockingConnectionHandler<T> handler){
        this.idToHandler.put(handler.getId(), handler);
    }

    public void addToUserPassword(String userName, String password){
        this.userToPassword.put(userName, password);
    }

    public void removeIdFromTopic(String topic , Integer conId){
        topicToConIds.get(topic).remove(conId);
    }

    public boolean isUserSubscribed(Integer conId, String topic){
        if(topicToConIds.containsKey(topic)) //checks if the topic exists
            return topicToConIds.get(topic).contains(conId);
        else
            return false;
    }

    public ConcurrentLinkedQueue<Integer> getSubscribers(String topic){
        if(topicToConIds.containsKey(topic))
            return topicToConIds.get(topic);
        return null;
    }

    public Integer getSubIdForTopic(Integer conId, String topic){
        if(conIdToTopicSubId.containsKey(conId) && conIdToTopicSubId.get(conId).containsKey(topic))
            return conIdToTopicSubId.get(conId).get(topic);
        return null;
    }

    public ConcurrentHashMap<Integer, ConnectionHandler<T>> getIdToHandlerList(){
        return idToHandler;
    }

    public ConnectionHandler<T> getHandler(int id){
        return idToHandler.get(id);
    }
}