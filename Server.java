import java.io.*;
import java.net.*;
import java.util.*;

//Written by Ammar Abu Shamleh, with reference to resources (all listed and named in relevant sections of code)

//Code ideas for server taken from "Core Java: Volume 2. Chapter 3: Networking"
public class Server {

    private static final int PORT = 8189;
    private static final int BUFFER_SIZE = 100;
    
	public static void main(String[] args) {
		
		//Establish new socket that monitors port 8189
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
                s.accept();
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
    
    /*Class saves file to disk, reading from Socket 's'
    Code taken from http://www.coderanch.com/t/591112/sockets/java/File-transfer-Client-Server-side */
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
    }
    
    public static void throwException(String message) throws Exception {
        throw new Exception(message);
    }
}

//Class handles client input for one server socket connection (allowing for multiple simultaneous client connections)
class ThreadedHandler implements Runnable {
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
        //Set up inStream scanner and read client's instructions
        Scanner in = new Scanner(inStream);
        String line = in.nextLine().trim();
        
        //Decode instruction
        switch(line) {
            case "Upload file":
                try {
                    Server.saveFile(incoming);
                } catch(Exception e) {
                    e.printStackTrace();
                }
                break;
            //Continue writing cases
        }
    }
}