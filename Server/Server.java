/*
 CITS3002 Project 2016
 Name:			Ammar Abu Shamleh
 Student number: 21521274
 Date:           May 2016
 */

import java.io.*;
import java.net.*;
import java.util.*;
import java.nio.ByteBuffer;
import java.security.KeyStore;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Security;
import java.security.cert.CertPath;
import java.security.cert.CertificateFactory;
import javax.security.auth.x500.X500Principal;
import javax.net.ssl.SSLContext;
import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLServerSocketFactory;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.SSLServerSocket;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSession;
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
    private static final int MEMBER_REQUIRED = 17;
    private static final int MEMBER_NOT_REQUIRED = 18;
    private static final int CLIENT_VALID_FILE = 19;
    private static final int CLIENT_INVALID_FILE = 20;
    private static final int PASS_CHALLENGE = 21;
    private static final int FAIL_CHALLENGE = 22;
    private static final int CRYPTO_FAIL = 23;
    
    private static ArrayList<ServerFile> files;
    
	public static void main(String[] args) {
        //Initialize file list
        files = new ArrayList<ServerFile>();
        
        //Read csv file containing 'files' data
        CsvReader.readCSV("data.csv", files);
        
        //Ensure that data is written to CSV file on shutdown
        //http://stackoverflow.com/questions/2361510/how-to-save-application-options-before-exit
        Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
            
            public void run() {
                CsvWriter.writeToCSV("data.csv", files);
            }
        }));
        
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
    public static void saveFile(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        DataInputStream dis = new DataInputStream(inStream);
        FileOutputStream fos = null;
        
        //First, client will report whether or not it has the file
        int clientHasFile = dis.readInt();
        if(clientHasFile == CLIENT_INVALID_FILE) {
            return ;
        }
        
        //Read file name off socket
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        String filename = in.readLine().trim();
        
        //Create file in correct directory
        String curPath = System.getProperty("user.dir");
        String filePath = curPath + "/Files/";
        fos = new FileOutputStream(filePath + filename, false);
        
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
        
        //Record file in server's file list; replace old file if it already exists
        ServerFile sf = findFile(filename);
        files.remove(sf);
        ServerFile file = new ServerFile(filename);
        files.add(file);
        
        //Close relevant resources
        fos.close();
        in.close();
        outStream.close();
        inStream.close();
    }
    
    //Takes socket as parameter and saves certificate to disk
    public static void saveCertificate(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        FileOutputStream fos = null;
        
        //Read filename
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        in.readLine().trim();
        
        //Create temp certificate in correct directory
        String curPath = System.getProperty("user.dir");
        String filePath = curPath + "/Certificates/";
        fos = new FileOutputStream(filePath + "temp.crt");
        
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
        
        //Close file output stream
        fos.close();
        
        //Now, determine common name on certificate
        String commonName = getCommonName();
        
        //Create new file with commonName as filename, and copy contents on "temp.crt" into it
        File dest = new File("Certificates/" + commonName + ".crt");
        File source = new File(filePath + "temp.crt");
        copyFileUsingFileStreams(source, dest);
        removeCertFromCircles(commonName+".crt");
        
        //Now delete "temp.crt"
        source.delete();
        
        //Close relevant resources
        in.close();
        outStream.close();
        inStream.close();
    }
    
    /*
     * Given a  certificate name, remove that certificate from every file's list of vouchers
     * To be used when a new certificate is uploaded to the server; ensures no security breach when a certificate is replaced
     * Method ensures that a certificate replacement removes new cert from all circles of trust
     */
    private static void removeCertFromCircles(String certName) {
        for(ServerFile f : files) {
            f.removeCert(certName);
        }
    }
    
    //Copy one file into another https://examples.javacodegeeks.com/core-java/io/file/4-ways-to-copy-file-in-java/
    private static void copyFileUsingFileStreams(File source, File dest)
    throws IOException {
        InputStream input = null;
        OutputStream output = null;
        try {
            input = new FileInputStream(source);
            output = new FileOutputStream(dest, false);
            byte[] buf = new byte[1024];
            int bytesRead;
            while ((bytesRead = input.read(buf)) > 0) {
                output.write(buf, 0, bytesRead);
            }
        } finally {
            input.close();
            output.close();
        }
    }
    
    /*
     * Looks at "temp.crt" in certificates folder and determines the common name of the certificate's owner
     * For use with saveCertificate() method
     */
    private static String getCommonName() throws Exception {
        //Determine filePath
        String filePath = System.getProperty("user.dir") + "/Certificates/temp.crt";
        
        //Open certificate
        byte[] certBytes = fileToBytes(filePath);
        
        PemReader reader;
        PEMParser parser;
        
        //Use reader to create X509CertificateHolder object from corresponding byte array
        reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(certBytes)));
        parser = new PEMParser(reader);
        X509CertificateHolder certHolder = (X509CertificateHolder)parser.readObject();
        
        //Now convert X509CertificateHolder to X509Certificate
        JcaX509CertificateConverter certConverter = new JcaX509CertificateConverter();
        X509Certificate cert = certConverter.setProvider("BC").getCertificate(certHolder);
        
        //Now, get name on certificate
        X500Principal owner = cert.getSubjectX500Principal();
        String name = owner.getName();
        
        //Now, retrieve common name
        String[] tokens = name.split(",");
        String result = tokens[0].substring(3);
        
        return result;
    }
    
    //Takes socket as parameter and sends requested file through it
    public static void sendFile(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
        DataInputStream din = new DataInputStream(inStream);
        DataOutputStream dos = new DataOutputStream(outStream);
        FileInputStream fis = null;
        
        //Read required circle size
        int minCircleSize = din.readInt();
        
        //Check if there is a required member (for the circle of trust)
        int memberRequired = din.readInt();
        String requiredMember = null;
        
        //If there is, client will send name of required member
        if(memberRequired == MEMBER_REQUIRED) {
            //Read name of required member
            requiredMember = in.readLine().trim();
        }
        if(memberRequired != MEMBER_NOT_REQUIRED) {
            //An error must have occurred....do something
        }
        
        //Read filename
        String filename = in.readLine().trim();
        
        //Find file
        String curPath = System.getProperty("user.dir");
        String filePath = curPath + "/Files/";
        File f = new File(filePath + filename);
        
        //Make sure file exists
        boolean fileExists = f.exists() && !f.isDirectory() && (findFile(filename) != null);
        if(!fileExists) {
            //If it doesn't, tell client file doesn't exist
            dos.writeInt(FILE_NOT_FOUND);
            return ;
        }
        dos.writeInt(FILE_FOUND);
        
        //Check that file satisfies client's trust requirements
        boolean trustworthy = checkTrust(filename, minCircleSize, requiredMember);
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
        din.close();
        inStream.close();
        outStream.close();
    }
    
    public static void throwException(String message) throws Exception {
        throw new Exception(message);
    }
    
    //Determine if file is trustworthy with respect to given parameters
    private static boolean checkTrust(String filename, int minCircleSize, String requiredMember) {
        ServerFile f = findFile(filename);
        return f.meetsRequirements(minCircleSize, requiredMember);
    }
    
    //Service client request to vouch for file
    public static void vouchForFile(Socket s) throws Exception {
        InputStream inStream = s.getInputStream();
        OutputStream outStream = s.getOutputStream();
        DataOutputStream dos = new DataOutputStream(outStream);
        DataInputStream dis = new DataInputStream(inStream);
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
        
        try {
            /* Now, verify that client owns certificate by sending them a challenge */
            
            //Generate a random challenge number
            Random rng = new Random(System.nanoTime());
            int challenge = rng.nextInt(10000)+1;
            
            /* Encrypt challenge number with public key on specified certificate */
            byte[] certBytes = fileToBytes(certPath);
            PemReader reader;
            PEMParser parser;
            //Use reader to create X509CertificateHolder object from corresponding byte array
            reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(certBytes)));
            parser = new PEMParser(reader);
            X509CertificateHolder certHolder = (X509CertificateHolder)parser.readObject();
            //Now convert X509CertificateHolder to X509Certificate
            JcaX509CertificateConverter certConverter = new JcaX509CertificateConverter();
            X509Certificate cert = certConverter.setProvider("BC").getCertificate(certHolder);
            //Extract public ket from X509Certificate
            PublicKey key = cert.getPublicKey();
            //Encrypt challenge number using public key
            byte[] plainText = intToByteArray(challenge);
            byte[] cipherText = Crypto.encrypt(key, plainText);
            
            //Send client length of cipher
            dos.writeInt(cipherText.length);
            
            //Send encrypted challenge number to client byte-by-byte
            for(int i=0; i<cipherText.length; i++) {
                outStream.write(cipherText[i]);
            }
            
            //Receive keyLength
            int keyLength = dis.readInt();
            
            //Receive encrypted, incremented challenge number from client
            byte[] modifiedCipher = new byte[128];
            for(int i=0; i<keyLength; i++) {
                byte b = (byte) inStream.read();
                modifiedCipher[i] = b;
            }
            
            //Decrypt received data with client's public key, and compare to challenge number
            byte[] modifiedPlain = Crypto.decrypt(key, modifiedCipher);
            int modChallenge = byteArrayToInt(modifiedPlain);
            boolean pass = (modChallenge == challenge + 1);
            
            //Tell client whether they passed or failed
            if(pass) {
                //Tell client they passed
                dos.writeInt(PASS_CHALLENGE);
            }
            else {
                //Tell client they failed. Return
                dos.writeInt(FAIL_CHALLENGE);
                return;
            }
        } catch(Exception e) {
            //Encryption/decryption probably failed
            e.printStackTrace();
            return ;
        }
        
        //We've found file and certificate, so now vouch for file with certificate
        f.vouch(certName);
        
        //TODO: Might want to send success code back to client. I'll leave it for now.
        
        //Close relevant resources
        inStream.close();
        outStream.close();
        dos.close();
        dis.close();
        in.close();
    }
    
    //Convert byte array to int
    //http://stackoverflow.com/questions/5399798/byte-array-and-int-conversion-in-java
    public static int byteArrayToInt(byte[] b)
    {
        ByteBuffer wrapped = ByteBuffer.wrap(b);
        return wrapped.getInt();
    }
    
    //Convert an int to a byte array
    public static byte[] intToByteArray(int x) {
        return ByteBuffer.allocate(4).putInt(x).array();
    }
    
    //Service client request for file list
    //http://www.java-forums.org/new-java/34049-simple-socket-program-java-client-c-server.html
    public static void listFiles(Socket s) throws Exception {
        OutputStream outStream = s.getOutputStream();
        InputStream inStream = s.getInputStream();
        DataOutputStream dos = new DataOutputStream(outStream);
        DataInputStream dis = new DataInputStream(inStream);
        
        
        //First, server needs to send number of files
        int nFiles = files.size();
        dos.writeInt(nFiles);
        
        //Next, server will send data for each file
        for(ServerFile f : files) {
            //Start with filename
            char[] strArray = f.getFilename().toCharArray();
            //First send length
            dos.writeInt(strArray.length);
            //Then send name
            for(int j=0; j<strArray.length; j++) {
                outStream.write(strArray[j]);
            }
            
            //Next, send circle size and circle details of file
            List<String> largestCircle = f.getCircleDetails();
            int circleSize = 0;
            if(largestCircle != null) circleSize = largestCircle.size();
            
            //Start with circle size
            dos.writeInt(circleSize);
            //Check if largestCircle is still null (i.e. circle of 0)
            if(largestCircle == null) {
                continue;
            }
            //Now send name of each circle member
            for(String member : largestCircle) {
                //First, grab common name from cleartext of certificate
                String commonName = retrieveCertName(member);
                strArray = commonName.toCharArray();
                //First send the length
                dos.writeInt(strArray.length);
                //Now send the name
                for(int i=0; i<strArray.length; i++) {
                    outStream.write(strArray[i]);
                }
            }
        }
        
        //Close resources
        dis.close();
        dos.close();
        outStream.close();
        inStream.close();
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
    
    //Given the cleartext of a certificate, get the common name and return it
    private static String retrieveCertName(String certText) {
        String copy = new String(certText);
        String[] tokens = copy.split(",");
        return tokens[0].substring(3);
    }
}

//Class handles client input for one server socket connection (allowing for multiple simultaneous client connections)
class ThreadedHandler implements Runnable {
    private static final int UPLOAD = 1;
    private static final int DOWNLOAD = 2;
    private static final int UPLOAD_CERT = 3;
    private static final int LIST = 4;
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
        DataInputStream din = null;;
        try {
            din = new DataInputStream(inStream);
            b = din.readInt();
        } catch(Exception e) {
            e.printStackTrace();
        }
        
        switch(b) {
            case UPLOAD :
                //Client wants to send file (not a certificate)
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
            case UPLOAD_CERT:
                //Client wants to send certificate
                try {
                    Server.saveCertificate(incoming);
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
            case LIST:
                //Client wants Server to send list of files, and protection on each file
                try {
                    Server.listFiles(incoming);
                } catch(Exception e) {
                    e.printStackTrace();
                }
                break;
        }
        
        //Close connection
        try {
            incoming.close();
            din.close();
        } catch(IOException e) {
            e.printStackTrace();
        }
        System.out.println("Closed connection on server side");
    }
}