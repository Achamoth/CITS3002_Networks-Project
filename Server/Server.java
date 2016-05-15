import java.io.*;
import java.net.*;
import java.util.*;
import java.nio.file.Path;
import java.nio.file.Paths;

//Written by Ammar Abu Shamleh, with reference to resources (all listed and named in relevant sections of code)

//Code ideas for server taken from "Core Java: Volume 2. Chapter 3: Networking"
public class Server {

    private static final int PORT = 8889;
    private static final int BUFFER_SIZE = 100;
    
    private static final int ACKNOWLEDGMENT = 10;
    private static final int FILE_TRUSTWORTHY = 8;
    private static final int FILE_UNTRUSTWORTHY = 9;
    private static final int FILE_NOT_FOUND = 15;
    private static final int FILE_FOUND = 16;
    
    private static ArrayList<ServerFile> files;
    
	public static void main(String[] args) {
        //Initialize file list
        files = new ArrayList<ServerFile>();
        
        //TODO: Read csv file containing 'files' data
        
		//Establish new socket that monitors specified port
        ServerSocket s = null;
        try {
            s =  new ServerSocket(PORT);
        } catch(Exception e) {
            e.printStackTrace();
        }
		
        int i = 1;
        
        //Print ip address and hostname of current server http://crunchify.com/how-to-get-server-ip-address-and-hostname-in-java/
        InetAddress ip;
        String hostname;
        try {
            ip = InetAddress.getLocalHost();
            hostname = ip.getHostName();
            System.out.println("Current IP address : " + ip);
            System.out.println("Current Hostname : " +hostname);
            System.out.print("\n");
        } catch(Exception e) {
            e.printStackTrace();
        }
        
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
        
        //TODO: Write 'files' data to csv file
	}
    
    //Takes socket as parameter and saves file to disk
    public static void saveFile(Socket s, boolean isCert) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        FileOutputStream fos = null;
        
        //Read file name off socket
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        String filename = in.readLine().trim();
        
        /* NOT SURE WHAT THIS DOES, HONESTLY, BUT IT SEEMS LIKE IT MIGHT BE USEFUL FOR SOMETHING, SO I'LL LEAVE IT FOR NOW
        Path currentRelativePath = Paths.get("");
        String curDir = currentRelativePath.toAbsolutePath().toString();
        System.out.println("Current relative path is: " + s);
        */
        
        //Create file in correct directory
        String curPath = System.getProperty("user.dir");
        String filePath;
        if(!isCert) filePath = curPath + "/Files/";
        else filePath = curPath + "/Certificates/";
        fos = new FileOutputStream(filePath + filename);
        
        //Send ack back to client
        DataOutputStream dos = new DataOutputStream(outStream);
        dos.writeInt(ACKNOWLEDGMENT);
        
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
        
        //Record file in server's file list if it's a regular file
        if(!isCert) {
            ServerFile file = new ServerFile(filename);
            files.add(file);
        }
        //TODO: If file is a certificate.......lots of stuff
        
        //Close relevant resources
        fos.close();
        in.close();
        outStream.close();
        inStream.close();
    }
    
    //Takes socket as parameter and sends requested file through it
    public static void sendFile(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        DataOutputStream dos = new DataOutputStream(outStream);
        FileInputStream fis = null;
        
        //Read filename
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        String filename = in.readLine().trim();
        
        //Find file
        String curPath = System.getProperty("user.dir");
        String filePath = curPath + "/Files/";
        File f = new File(filePath + filename);
        
        //Make sure file exists
        boolean fileExists = f.exists() && !f.isDirectory();
        if(!fileExists) {
            //If it doesn't, tell client file doesn't exist
            dos.writeInt(FILE_NOT_FOUND);
            return ;
        }
        dos.writeInt(FILE_FOUND);
        
        //Check that file satisfies client's trust requirements
        boolean trustworthy = checkTrust(filename);
        if(!trustworthy) {
            dos.writeInt(FILE_UNTRUSTWORTHY);
            return ;
        }
        dos.writeInt(FILE_TRUSTWORTHY);
        
        //Otherwise, send file byte by byte
        fis = new FileInputStream(f);
        try {
            while(true) {
                int b = fis.read();
                if(b == -1) break;
                outStream.write(b);
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
        
        //Close relevant resources
        dos.close();
        fis.close();
        in.close();
        inStream.close();
        outStream.close();
    }
    
    public static void throwException(String message) throws Exception {
        throw new Exception(message);
    }
    
    private static boolean checkTrust(String filename) {
        //Activate below code when I develop ServerFile further
        /*ServerFile f = findFile(filename);
        if(f == null) {
            //Oh oh. This should never really happen
        }*/
        
        //TODO: Check circle size on file against client specifications
        //TODO: Check if any circle contain required member (if client has specified one)
        return true; //For now
    }
    
    private static ServerFile findFile(String filename) {
        for(ServerFile f : files) {
            if(f.getFilename().trim().equals(filename)) {
                return f;
            }
        }
        return null;
    }
}

//Class handles client input for one server socket connection (allowing for multiple simultaneous client connections)
class ThreadedHandler implements Runnable {
    private static final int UPLOAD = 1;
    private static final int DOWNLOAD = 2;
    private static final int UPLOAD_CERT = 3;
    
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
                //Client wants to send file (not a certificate)
                try {
                    Server.saveFile(incoming, false);
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
            case UPLOAD_CERT:
                //Client wants to sned certificate
                try {
                    Server.saveFile(incoming, true);
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