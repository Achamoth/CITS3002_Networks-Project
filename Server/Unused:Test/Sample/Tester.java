/*
 Public Key cryptography using the RSA algorithm.
 */

import java.security.*;
import javax.crypto.*;

import java.io.*;
import java.net.*;
import java.util.*;
import java.math.BigInteger;
import java.nio.file.Path;
import java.nio.file.Paths;
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

//This class deals with encryption and decryption for the server-client challenge
//http://www.mobilefish.com/developer/bouncycastle/bouncycastle.html
public class Tester {
    
    //Encrypt plainText with public key
    public static byte[] encrypt(PublicKey key, byte[] plainText) throws Exception {
        //Create RSA Cipher object (specifying algorithm, mode, and padding
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        //Initialize Cipher object
        cipher.init(Cipher.ENCRYPT_MODE, key);
        //Encrypt plaintext using the public key
        byte[] cipherText = cipher.doFinal(plainText);
        return cipherText;
    }
    
    //Decrypt cipher with private key
    public static byte[] decrypt(PrivateKey key, byte[] cipherText) throws Exception {
        //Create RSA Cipher object (specifying algorithm, mode, and padding
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        //Initialize Cipher object
        cipher.init(Cipher.DECRYPT_MODE, key);
        //Encrypt plaintext using the public key
        byte[] plainText= cipher.doFinal(cipherText);
        return plainText;
    }
    
    //Given a filename, open a file and return its contents as a byte array
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
    
    //Test class to test encryption and decryption using sample public-private key pair
    public static void main(String[] args) {
        try {
            Security.addProvider(new BouncyCastleProvider());
            
            //Produce plaintext
            String test = "Hello";
            byte[] plainText = test.getBytes();
            System.out.println("Plain : " + Arrays.toString(plainText));
            
            //Get public key
            byte[] certBytes = fileToBytes("test.crt");
            PemReader reader;
            PEMParser parser;
            
            reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(certBytes)));
            parser = new PEMParser(reader);
            X509CertificateHolder certHolder = (X509CertificateHolder) parser.readObject();
            
            JcaX509CertificateConverter certCoverter = new JcaX509CertificateConverter();
            X509Certificate certTest = certCoverter.setProvider("BC").getCertificate(certHolder);
            
            PublicKey key = certTest.getPublicKey();
            
            //Encrypt plaintext using public key and produce cipher
            byte[] cipherText = encrypt(key, plainText);
            System.out.println("Cipher : " + Arrays.toString(cipherText));
            
            //Get private key
            byte[] keyBytes = fileToBytes("test.key");
            
            reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(keyBytes)));
            parser = new PEMParser(reader);
            PrivateKeyInfo keyInfo = (PrivateKeyInfo)parser.readObject();
            
            JcaPEMKeyConverter keyConverter = new JcaPEMKeyConverter();
            PrivateKey privateKey = keyConverter.setProvider("BC").getPrivateKey(keyInfo);
            
            //Decrypt cipher using private key and produce plaintext
            byte[] plainTest = decrypt(privateKey, cipherText);
            System.out.println("Decrpyted plain : " + Arrays.toString(plainTest));
            
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
}