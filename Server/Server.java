import java.io.*;
import java.net.*;
import java.util.*;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.KeyStore;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.PrivateKey;
import java.security.Security;
import javax.net.ssl.SSLContext;
import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLServerSocketFactory;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.SSLServerSocket;
import org.bouncycastle.util.io.pem.PemObject;
import org.bouncycastle.util.io.pem.PemReader;
import org.bouncycastle.openssl.PEMParser;
import org.bouncycastle.openssl.jcajce.JcaPEMKeyConverter;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.asn1.pkcs.PrivateKeyInfo;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;

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
        
        //Initialize string pemPath
        String pemPath = System.getProperty("user.dir") + "/PEM/";
        
        //Establish new SSL socket
        SSLServerSocket s = null;
        try {
            //Initialize SSL context using PEM files
            SSLServerSocketFactory sslserversocketfactory = getServerSocketFactoryPEM(pemPath);
            //s =  new ServerSocket(PORT);
            s = (SSLServerSocket) sslserversocketfactory.createServerSocket(PORT);
        } catch(Exception e) {
            e.printStackTrace();
            System.exit(0);
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
	
	//Creates SSLSocketFactory and initialises context using 2 PEM files (public-private key pair)
	//Reference: http://stackoverflow.com/questions/12501117/programmatically-obtain-keystore-from-pem
	//Reference: http://www.bouncycastle.org/wiki/display/JA1/Provider+Installation
	private static SSLServerSocketFactory getServerSocketFactoryPEM(String pemPath) throws Exception {
        SSLContext context = null;
		try {
            //Add bouncy castle provider for Java Security API
            Security.addProvider(new BouncyCastleProvider());
			
            //Set context to SSL
            context = SSLContext.getInstance("SSL");
            
            //Read certificate into byte array
            byte[] certBytes = fileToBytes(pemPath + "public.crt");
            //Read private key into byte array
            byte[] keyBytes = fileToBytes(pemPath + "private.key");
            
            PemReader reader;
            PEMParser parser;
            PemObject temp;
            
            //Use reader to create X509CertificateHolder object from corresponding byte array
            reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(certBytes)));
            parser = new PEMParser(reader);
            X509CertificateHolder certHolder = (X509CertificateHolder)parser.readObject();
            
            //Now convert X509CertificateHolder to X509Certificate http://stackoverflow.com/questions/6370368/bouncycastle-x509certificateholder-to-x509certificate
            JcaX509CertificateConverter certConverter = new JcaX509CertificateConverter();
            X509Certificate cert = certConverter.setProvider("BC").getCertificate(certHolder);
            
            //Use reader to create PrivateKeyInfo object from corresponding byte array
            reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(keyBytes)));
            parser = new PEMParser(reader);
            PrivateKeyInfo keyInfo = (PrivateKeyInfo)parser.readObject();
            
            //Now convert PrivateKeyInfo to java.security.PrivateKey
            JcaPEMKeyConverter keyConverter = new JcaPEMKeyConverter();
            PrivateKey key = keyConverter.setProvider("BC").getPrivateKey(keyInfo);
            
            //Create dynamic keystore to use for SSL context
            KeyStore keystore = KeyStore.getInstance("JKS");
            keystore.load(null);
            keystore.setCertificateEntry("server-cert", cert);
            Certificate[] certChain = {cert}; //Certificate chain for private key (verifying corresponding public key)
            keystore.setKeyEntry("server-key", key, "password".toCharArray(), certChain);
            
            //Create and initialise key manager factory using "SunX509" algorithm
            KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
            kmf.init(keystore, "password".toCharArray());
            
            KeyManager[] km = kmf.getKeyManagers();
            context.init(km, null, null);
            
        } catch (Exception e) {
            e.printStackTrace();
        }
        return context.getServerSocketFactory();
	}
	
	//Read file (pointed to by filename) into array of bytes, and return it
	public static byte[] fileToBytes(String filename) throws FileDoesntExist {
		File f = new File(filename);
		int nbytes = (int) f.length();
		byte[] result = new byte[nbytes];
		
		//Make sure file exists
		if(!f.exists() || f.isDirectory()) {
			//File doesn't exist; print error message and exit
			throw new FileDoesntExist(filename);
		}
		
		//Read file's contents into byte array
		try {
            FileInputStream fis = new FileInputStream(f);
            for(int i=0; i<nbytes; i++) {
                byte b = (byte) fis.read();
                if(b != -1) result[i] = b;
                else {
                    //An error has probably occurred
                }
            }
		} catch(Exception e) {
            e.printStackTrace();
        }
		
		//Now return byte array
		return result;
	}
    
    //Takes socket as parameter and saves file to disk
    public static void saveFile(Socket s, boolean isCert) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        FileOutputStream fos = null;
        
        //Read file name off socket
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        String filename = in.readLine().trim();
        
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
        
        //Read required circle size
        int minCircleSize = inStream.read();
        
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
        boolean trustworthy = checkTrust(filename, minCircleSize);
        if(!trustworthy) {
            dos.writeInt(FILE_UNTRUSTWORTHY);
            return ;
        }
        dos.writeInt(FILE_TRUSTWORTHY);
        
        //If it does, send file byte by byte
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
    
    private static boolean checkTrust(String filename, int minCircleSize) {
        //Activate below code when I develop ServerFile further
        /*ServerFile f = findFile(filename);
         if(f == null) {
         //Oh oh. This should never really happen
         }*/
        
        //TODO: Check circle size on file against client specifications
        //TODO: Check if any circle contain required member (if client has specified one)
        ServerFile f = findFile(filename);
        f.meetsRequirements(0, null);
        return true; //For now
    }
    
    //Service client request to vouch for file
    public static void vouchForFile(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        DataOutputStream dos = new DataOutputStream(outStream);
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        
        //First, client will send filename
        String filename = in.readLine().trim();
        
        //Check if file exists on server, and report results to client
        ServerFile f = findFile(filename);
        boolean fileExists = f != null;
        
        if(!fileExists) {
            //If file doesn't exist, report this to client
            dos.writeInt(FILE_NOT_FOUND);
            return ;
        }
        dos.writeInt(FILE_FOUND);
        
        //Next, client will send certificate name
        String certName = in.readLine().trim();
        
        //Check that certificate exists on server, and report results to client
        String curPath = System.getProperty("user.dir");
        String certPath = curPath + "/Certificates/" + certName;
        boolean certExists = (new File(certPath).isFile());
        
        if(!certExists) {
            //If certificate doesn't exist, report this to client
            dos.writeInt(FILE_NOT_FOUND);
            return ;
        }
        dos.writeInt(FILE_FOUND);
        
        //We've found file and certificate, so now vouch for file with certificate
        f.vouch(certName);
        
        //TODO: Might want to send success code back to client. I'll leave it for now.
    }
    
    //Finds specified file inside 'files' arraylist
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
    private static final int VOUCH = 6;
    
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
                //Client wants to send certificate
                try {
                    Server.saveFile(incoming, true);
                } catch(Exception e) {
                    e.printStackTrace();
                }
                break;
            case VOUCH:
                //Client wants to vouch for specified file with specified certificate
                try {
                    Server.vouchForFile(incoming);
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