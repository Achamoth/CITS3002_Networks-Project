import java.io.*;
import java.net.*;
import java.util.*;

public class Server {

	public static void main(String[] args) {
		
		//Establish new socket that monitors port 8189
		ServerSocket s =  new ServerSocket(8189);
		
        int i = 1;
        
		//Main server loop
		while(true) {
            //Idles until client connects to port
			Socket incoming = s.accept();
            //Print connection message
            System.out.println("Spawning " + i);
            //Establshes new thread for each client connection
            Runnable r = new ThreadedHandler(incoming);
            Thread t = new Thread(r);
            t.start();
            i++;
		}
	}
}

//Class handles client input for one server socket connection (allowing for multiple simultaneous client connections)
public class ThreadedHandler implements Runnable {
	private Socket incoming;
	
	//Constructs handler
	public ThreadedHandler(Socket i) {
		incoming = i;
	}
	
    //Runs whenever client connection is established
	public void run() {
		//Establishes incoming and outgoing streams
        InputStream inStream = incoming.getInputStream();
        OutputStream outStream = incoming.getOutPutStream();
        
        
    }