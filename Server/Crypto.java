/*
 CITS3002 Project 2016
 Name:			Ammar Abu Shamleh, Pradyumn Vij
 Student number: 21521274, 21469477
 Date:           May 2016
 */

import java.io.ByteArrayInputStream;
import java.io.InputStreamReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.security.*;
import java.security.Security;
import java.security.cert.X509Certificate;
import javax.crypto.*;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.spec.IvParameterSpec;
import org.bouncycastle.util.io.pem.PemObject;
import org.bouncycastle.util.io.pem.PemReader;
import org.bouncycastle.openssl.PEMParser;
import org.bouncycastle.openssl.jcajce.JcaPEMKeyConverter;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.asn1.pkcs.PrivateKeyInfo;

/*
 Public Key cryptography using the RSA algorithm.
 */

//This class deals with encryption and decryption for the server-client challenge, and the CSV file
//http://www.mobilefish.com/developer/bouncycastle/bouncycastle.html
//http://www.codejava.net/coding/file-encryption-and-decryption-simple-example
public class Crypto {
    
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
    public static byte[] decrypt(PublicKey key, byte[] cipherText) throws Exception {
        //Create RSA Cipher object (specifying algorithm, block chaining mode, and padding
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        //Initialize Cipher object
        cipher.init(Cipher.DECRYPT_MODE, key);
        //Encrypt plaintext using the public key
        byte[] plainText= cipher.doFinal(cipherText);
        return plainText;
    }
    
    //Encrypts "data.csv" file into "data", then deletes "data.csv"
    public static void encryptCSV() throws Exception {
        
        //Retrieve public key from Server's certificate
        PublicKey key = getPublicKey("PEM/public.crt");
        
        //Generate random symmetric key http://stackoverflow.com/questions/18228579/how-to-create-a-secure-random-aes-key-in-java
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(128);
        SecretKey secretKey = keyGen.generateKey();
        
        //Create Cipher objects (one for encrytping file with symmetric key, and one for encrypting symmetric key with public key)
        Cipher fileCipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        Cipher keyCipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        
        //Initialize Cipher objects
        fileCipher.init(Cipher.ENCRYPT_MODE, secretKey);
        keyCipher.init(Cipher.ENCRYPT_MODE, key);
        
        //Record and save IV for symmetric AES key
        /*byte[] IV = fileCipher.getIV();
        File outIV = new File("iv");
        FileOutputStream fos = new FileOutputStream(outIV, false);
        fos.write(IV);
        fos.close();*/
        
        //Open CSV file and read file into byte array
        File in = new File("data.csv");
        FileInputStream fis = new FileInputStream(in);
        byte[] inputBytes = new byte[(int) in.length()];
        fis.read(inputBytes);
        
        //Encrypt file with symmetric key and store cipher in byte array
        byte[] outputBytes = fileCipher.doFinal(inputBytes);
        
        //Write encrypted file to disk
        File out = new File("data");
        FileOutputStream fos = new FileOutputStream(out);
        fos.write(outputBytes);
        fos.close();
        
        //Encrypt symmetric key with public key, and write encrypted key to disk
        File outKey = new File("symmetric.key");
        fos = new FileOutputStream(outKey, false);
        byte[] keyBytes = secretKey.getEncoded();
        byte[] encryptedKey = keyCipher.doFinal(keyBytes);
        fos.write(encryptedKey);
    
        //Close file streams
        fis.close();
        fos.close();
        
        //Delete "data.csv" (delete unencrypted data file)
        in.delete();
    }
    
    //Decrypts "data" into "data.csv", then deletes "data"
    public static void decryptCSV() throws Exception {
        
        //Retrieve private key from Server's private key file
        PrivateKey privateKey = getPrivateKey("PEM/private.key");
        
        //Create Cipher objects (one for decrypting symmetric key using private key, and one for decrypting file using symmetric key)
        Cipher fileCipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        Cipher keyCipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        
        //Decrypt and retrieve symmetric key with private key
        SecretKey symmetricKey = getSymmetricKey(privateKey, keyCipher);
        
        //Read IV
        byte[] IV = Server.fileToBytes("iv");
        
        //Initialize Cipher object
        fileCipher.init(Cipher.DECRYPT_MODE, symmetricKey, new IvParameterSpec(IV));
        
        //Open Encrypted file and read into byte array
        File in = new File("data");
        if(!in.isFile()) return ;
        FileInputStream fis = new FileInputStream(in);
        byte[] inputBytes = new byte[(int) in.length()];
        fis.read(inputBytes);
        
        //Decrypt file and store plain text in byte array
        byte[] outputBytes = fileCipher.doFinal(inputBytes);
        
        //Write encrypted file to disk
        File out = new File("data.csv");
        FileOutputStream fos = new FileOutputStream(out);
        fos.write(outputBytes);
        
        //Close file streams
        fis.close();
        fos.close();
        
        //Delete "data", "iv" and "symmetric.key"
        in.delete();
        File temp = new File("symmetric.key");
        temp.delete();
        temp = new File("iv");
        temp.delete();
    }
    
    //Given the private key, decrypt the symmetric key file, and retrieve the symmetric key object
    private static SecretKey getSymmetricKey(PrivateKey privateKey, Cipher keyCipher) throws Exception{
        
        //First, store file's contents in byte array
        byte[] encryptedKey = Server.fileToBytes("symmetric.key");
        
        //Now, decrypt byte array using private key
        keyCipher.init(Cipher.DECRYPT_MODE, privateKey);
        byte[] keyBytes = keyCipher.doFinal(encryptedKey);
        
        //Now, retrieve secret key object from byte array
        SecretKey secretKey = new SecretKeySpec(keyBytes, 0, keyBytes.length, "AES");
        
        //Return secret key
        return secretKey;
        
    }
    
    //Retrieve public key object from specified certificate
    private static PublicKey getPublicKey(String certName) throws Exception {
        //Add Bouncy Castle as security provider
        Security.addProvider(new BouncyCastleProvider());
        //Read PEM file into byte array
        byte[] certBytes = Server.fileToBytes(certName);
        //Set up reader and parser
        PemReader reader;
        PEMParser parser;
        //Construct reader and parser
        reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(certBytes)));
        parser = new PEMParser(reader);
        //Reader X509CertificateHolder object from file using parser
        X509CertificateHolder certHolder = (X509CertificateHolder) parser.readObject();
        //Convert X509CertificateHolder object to X509Certificate
        JcaX509CertificateConverter certConverter = new JcaX509CertificateConverter();
        X509Certificate cert = certConverter.setProvider("BC").getCertificate(certHolder);
        //Retrieve PublicKey from X509Certificate
        PublicKey key = cert.getPublicKey();
        //Return PublicKey
        return key;
    }
    
    //Retrieve private key object from specified key file
    private static PrivateKey getPrivateKey(String certName) throws Exception {
        //Add Bouncy Castle as security provider
        Security.addProvider(new BouncyCastleProvider());
        //Read PEM file into byte array
        byte[] keyBytes = Server.fileToBytes(certName);
        //Set up reader and parser
        PemReader reader;
        PEMParser parser;
        //Construct reader and parser
        reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(keyBytes)));
        parser = new PEMParser(reader);
        //Read PrivateKeyInfo object from file using parser
        PrivateKeyInfo keyInfo = (PrivateKeyInfo) parser.readObject();
        //Convert keyInfo into PrivateKey
        JcaPEMKeyConverter keyConverter = new JcaPEMKeyConverter();
        PrivateKey key = keyConverter.setProvider("BC").getPrivateKey(keyInfo);
        //Return private key
        return key;
    }
}