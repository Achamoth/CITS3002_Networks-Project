import java.io.*;
import java.net.*;
import java.util.*;

//Written by Ammar Abu Shamleh, with reference to resources (all listed and named in relevant sections of code)

//Code ideas for server taken from "Core Java: Volume 2. Chapter 3: Networking"
public class Server {

    private static final int PORT = 8189;
    private static final int BUFFER_SIZE = 100;
    
    private static final int ACKNOWLEDGMENT = 10;
    
	public static void main(String[] args) {
		
		//Establish new socket that monitors specified port
        ServerSocket s = null;
        try {
            s =  new ServerSocket(PORT);
        } catch(Exception e) {
            e.printStackTrace();
        }
		
        int i = 1;
        
		//Main server loop
		while(true) {
            //Idles until client connects to port
			Socket incoming = null;
            try {
                System.out.println("Waiting on port: " + PORT);
                incoming = s.accept();
            } catch (Exception e) {
                e.printStackTrace();
            }
            //Print connection message
            System.out.println("Spawning " + i);
            //Establshes new thread for each client connection
            Runnable r = new ThreadedHandler(incoming);
            Thread t = new Thread(r);
            t.start();
            i++;
		}
	}
    
    //IGNORE BELOW CODE FOR NOW
    /*Class saves file to disk, reading from Socket 's'
     Code taken from http://www.coderanch.com/t/591112/sockets/java/File-transfer-Client-Server-side
     public static void saveFile(Socket s) throws Exception {
     ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
     ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
     FileOutputStream fos = null;
     byte[] buffer = new byte[BUFFER_SIZE];
     
     //Read file name
     Object o = ois.readObject();
     
     if(o instanceof String) {
     fos = new FileOutputStream(o.toString());
     }
     else {
     throwException("Something is wrong");
     }
     
     //Read file till all bytes are finished
     Integer bytesRead = 0;
     
     do {
     o = ois.readObject();
     
     if(!(o instanceof Integer)) {
     throwException("Something is wrong");
     }
     
     bytesRead = (Integer) o;
     
     o = ois.readObject();
     
     if(!(o instanceof byte[])) {
     throwException("Something is wrong");
     }
     
     buffer = (byte[]) o;
     
     //Write data to output file
     fos.write(buffer, 0, bytesRead);
     
     } while (bytesRead == BUFFER_SIZE);
     
     System.out.println("File transfer success");
     
     fos.close();
     ois.close();
     oos.close();
     }*/
    
    //Takes socket as parameter and saves file to disk
    public static void saveFile(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        FileOutputStream fos = null;
        
        //Read file name off socket
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        String filename = in.readLine().trim();
        System.out.println(filename);
        fos = new FileOutputStream(filename);
        
        //Send ack back to client (doesn't work properly; client receives completely different number). This step is necessary to ensure filename and file are sent separately
        outStream.write(ACKNOWLEDGMENT);
        
        //Read file till all bytes are finished
        while (true) {
            try{
                int b = inStream.read();
                if(b == -1) break;
                fos.write(b);
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        
        //Close relevant resources
        fos.close();
        in.close();
        inStream.close();
    }
    
    //Takes socket as parameter and sends requested file through it
    public static void sendFile(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        //FileOutputStream fos = null;
        
        //Read filename
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        System.out.println(in.readLine());
        
        //TODO: Look for file, and send it to client
    }
    
    public static void throwException(String message) throws Exception {
        throw new Exception(message);
    }
}

//Class handles client input for one server socket connection (allowing for multiple simultaneous client connections)
class ThreadedHandler implements Runnable {
    private static final int UPLOAD = 1;
    private static final int DOWNLOAD = 2;
    
	private Socket incoming;
	
	//Constructs handler
	public ThreadedHandler(Socket i) {
		incoming = i;
	}
	
    //Runs whenever client connection is established
	public void run() {
		//Establishes incoming and outgoing streams
        InputStream inStream = null;
        OutputStream outStream = null;
        try {
            inStream = incoming.getInputStream();
            outStream = incoming.getOutputStream();
        } catch(Exception e) {
            e.printStackTrace();
        }
        
        //Read byte of data off inStream (from client) and decode it to determine what client wants
        int b = 0;
        try {
            b = inStream.read();
        } catch(Exception e) {
            e.printStackTrace();
        }
        
        switch(b) {
            case UPLOAD :
                //Client wants to send file
                try {
                    Server.saveFile(incoming);
                } catch(Exception e) {
                    e.printStackTrace();
                }
                break;
            case DOWNLOAD:
                //Client wants to download file
                try {
                    Server.sendFile(incoming);
                } catch(Exception e) {
                    e.printStackTrace();
                }
                break;
        }
        
        //Close connection
        try {
            incoming.close();
        } catch(IOException e) {
            e.printStackTrace();
        }
        System.out.println("Closed connection on server side");
    }
}