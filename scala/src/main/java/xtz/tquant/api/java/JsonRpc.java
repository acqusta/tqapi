package xtz.tquant.api.java;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.DeserializationFeature;
import org.zeromq.*;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.*;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.module.scala.DefaultScalaModule;
import org.msgpack.jackson.dataformat.MessagePackFactory;

public interface JsonRpc {

    @JsonIgnoreProperties
    class JsonRpcError {
        public int      code;
        public String   message;
        public Object   data;
    }

    @JsonIgnoreProperties
    class JsonRpcMessage {
        public String method;
        public Object params;
        public Object result;
        public JsonRpcError error;
        public int id;

        public JsonRpcMessage() {}

        public JsonRpcMessage(String method, Object params, int callId) {
            this.method = method;
            this.params = params;
            this.id     = callId;
        }
    }

    @JsonIgnoreProperties
    class JsonRpcCallResult {
        public Object result;
        public JsonRpcError error;
    }

    @JsonIgnoreProperties
    class JsonRpcClient {

        public static interface Callback {
            void onConnected ();
            void onDisconnected ();
            void onNotification (String event, Object value);
        }

        private ConcurrentHashMap<Integer, CompletableFuture<JsonRpcCallResult>> wait_result_map =
            new ConcurrentHashMap<Integer, CompletableFuture<JsonRpcCallResult>>();
        
        private ZContext ctx = new  ZContext();

        private ZMQ.Socket push_sock;
        private ZMQ.Socket pull_sock;
        private ZMQ.Socket remote_sock;
        private String addr;

        private volatile boolean should_close = false;
        private long last_heartbeat_rsp_time = 0;
        private boolean connected = false;

        private Thread main_thread = new Thread ( new Runnable() { public void run() { mainRun(); } });

        private ObjectMapper mapper = new ObjectMapper( new MessagePackFactory());

        public JsonRpcClient() {

            push_sock = ctx.createSocket(ZMQ.PUSH);
            push_sock.bind("inproc://send_msg");
            pull_sock = ctx.createSocket(ZMQ.PULL);
            pull_sock.connect("inproc://send_msg");

            main_thread.setDaemon(true);
            main_thread.start();

            mapper.setSerializationInclusion(JsonInclude.Include.NON_NULL);
            mapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
            mapper.registerModule(new DefaultScalaModule());
        }

        private Integer cur_callid = 0;
    
        private Integer nextCallId() {
            synchronized (cur_callid) {
                cur_callid++;
                return cur_callid;
            }
        }

        private Callback callback = null;

        public void setCallback(Callback callback) {
            this.callback = callback;
        }

        private void mainRun() {

            last_heartbeat_rsp_time = System.currentTimeMillis();
            long heartbeat_req_time = 0;

            ZMQ.Poller poller = null;

            while (!should_close) {
                
                try {

                    if (connected) {
                        if (System.currentTimeMillis() - last_heartbeat_rsp_time > 6000 ) {
                            this.connected = false;
                            if (this.callback != null)
                                callback.onDisconnected();
                        }
                    }

                    if (this.remote_sock != null && System.currentTimeMillis() - heartbeat_req_time > 2000) {
                        heartbeat_req_time = System.currentTimeMillis();
                        doSendHeartBeat();
                    }

                    if (poller == null) {
                        if (remote_sock != null) {
                            poller = ctx.createPoller(2);
                            poller.register(pull_sock, ZMQ.Poller.POLLIN);
                            poller.register(remote_sock, ZMQ.Poller.POLLIN);
                        }
                        else {
                            poller = ctx.createPoller(1);
                            poller.register(pull_sock, ZMQ.Poller.POLLIN);
                        }
                    }

                    poller.poll(1000);

                    if (poller.pollin(0)) {
                        byte[] cmd = pull_sock.recv();
                        byte[] data = null;
                        while (pull_sock.hasReceiveMore())
                            data = pull_sock.recv();

                        switch (cmd[0]) {
                            case 'S' :
                                if (data != null)
                                    doSend(data);
                                break;
                            case 'C' :
                                doConnect();
                                poller = null;
                                break;
                            case 'D' :
                                break;
                            default:
                                System.out.println("Unknown command: " + cmd[0]);
                        }
                    }
                    if (poller == null) continue;

                    if (poller.getSize() == 2 && poller.pollin(1)) {
                        doRecv();
                    }
                }catch (Throwable t) {
                    t.printStackTrace();
                }
            }
        }

        private void doRecv() {
            
            try {
                byte[] data = this.remote_sock.recv(ZMQ.DONTWAIT);
                if (data == null) return;

                JsonRpcMessage msg = mapper.readValue(data, JsonRpcMessage.class);

                if (msg.method != null && msg.method.equals(".sys.heartbeat")) {
                    last_heartbeat_rsp_time = System.currentTimeMillis();
                    if ( ! this.connected ) {
                        this.connected = true;
                        if (this.callback != null) this.callback.onConnected();
                    }

                    if (this.callback != null)
                        this.callback.onNotification(msg.method, msg.result);

                } else if (msg.id != 0) {

                    JsonRpcCallResult result = new JsonRpcCallResult();
                    result.result = msg.result;
                    result.error = msg.error;

                    CompletableFuture<JsonRpcCallResult> future = wait_result_map.getOrDefault(msg.id, null);
                    if (future!=null) {
                        wait_result_map.remove(msg.id);
                        future.complete(result);
                    }
                } else {
                    // Notification message                    
                    if (msg.method != null && msg.params != null && this.callback != null )
                        this.callback.onNotification(msg.method, msg.params);
                }
            } catch (Throwable t){
                t.printStackTrace();
            }
            
        }
    
        public boolean connect(String addr) {

            if (addr == null || addr.isEmpty()) return false;
            this.addr = addr;


            synchronized (push_sock) {
                push_sock.send("C");//.getBytes("UTF-8")) ;
            }

            return true;
        }
    
        private void doConnect() {

            //System.out.println("connect to " + addr);
            ZMQ.Socket sock = ctx.createSocket(ZMQ.DEALER);
            sock.setReceiveTimeOut(2000);
            sock.setSendTimeOut(2000);
            sock.connect(addr);

            if (this.remote_sock != null) this.remote_sock.close();
            
            this.remote_sock = sock;
        }

        private void doSend(byte[] data){

            if (this.remote_sock == null) return;

            if (!this.remote_sock.send(data, 0)) {
                //System.out.println("Send failed ");
            }
        }

        private void doSendHeartBeat() {

            Map<String, Object> params = new HashMap<String, Object>();
            params.put( "time", System.currentTimeMillis() );
            JsonRpcMessage msg = new JsonRpcMessage(".sys.heartbeat", params,  this.nextCallId());

            try {
                byte[] data = mapper.writeValueAsBytes(msg);
                doSend(data);
            } catch (JsonProcessingException e) {
                e.printStackTrace();
            }
        }

        public void close() {

            should_close = true;
            synchronized (push_sock) {
                push_sock.send("D");//.getBytes("UTF-8"))
            }
        }

        public JsonRpcCallResult call(String method, Object params, int timeout) {

            String err_msg = null;
            try {
                Integer callId = nextCallId();

                CompletableFuture<JsonRpcCallResult> future = null;
                if (timeout != 0 ) {
                    future = new CompletableFuture<JsonRpcCallResult>();
                    wait_result_map.put(callId, future);
                }

                JsonRpcMessage msg = new JsonRpcMessage(method, params, callId);

                byte[] data = mapper.writeValueAsBytes(msg);

                synchronized(push_sock) {
                    push_sock.sendMore("S");
                    push_sock.send(data, 0);
                }

                if (timeout ==0 ) return null;

                long end = System.currentTimeMillis() + timeout;

                JsonRpcCallResult result = null;
                result = future.get(timeout, TimeUnit.MILLISECONDS);
                if (result == null) {
                    synchronized (wait_result_map) {
                        wait_result_map.remove(callId);
                    }
                }

                return result;

            } catch (JsonProcessingException e) {
                e.printStackTrace();
                err_msg = e.getMessage();
            } catch (InterruptedException e) {
                e.printStackTrace();
                err_msg = e.getMessage();
            } catch (ExecutionException e) {
                e.printStackTrace();
                err_msg = e.getMessage();
            } catch (TimeoutException e) {
                //e.printStackTrace();
                err_msg = e.getMessage();
            }

            JsonRpcCallResult result = new JsonRpcCallResult();

            result.result = null;
            result.error = new JsonRpcError();
            result.error.code = -1;
            result.error.message = err_msg;
            result.error.data = null;

            return result;
        }

    }
    
}


