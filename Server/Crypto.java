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
    
    //Encrypt plainText with public key (for public key challenge)
    public static byte[] encrypt(PublicKey key, byte[] plainText) throws Exception {
        //Create RSA Cipher object (specifying algorithm, mode, and padding
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        //Initialize Cipher object
        cipher.init(Cipher.ENCRYPT_MODE, key);
        //Encrypt plaintext using the public key
        byte[] cipherText = cipher.doFinal(plainText);
        return cipherText;
    }
    
    //Decrypt cipher with private key (for public key challenge)
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
        
        //Obtain Server's public key
        PublicKey publicKey = getPublicKey("PEM/public.crt");
        
        //Generate random symmetric key http://stackoverflow.com/questions/18228579/how-to-create-a-secure-random-aes-key-in-java
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(128);
        SecretKey secretKey = keyGen.generateKey();
        
        //Create Cipher object for file encryption
        Cipher fileCipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        //Create Cipher object for encryption of symmetric key
        Cipher keyCipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        
        //Initialize Cipher objects
        fileCipher.init(Cipher.ENCRYPT_MODE, secretKey);
        keyCipher.init(Cipher.ENCRYPT_MODE, publicKey);
        
        //Open CSV file and read contents into byte array
        byte[] inputBytes = Server.fileToBytes("data.csv");
        
        //Get IV from cipher
        byte[] iv = fileCipher.getIV();
        System.out.print(iv.length);
        
        //Write encrypted file to disk
        File out = new File("data");
        FileOutputStream fos = new FileOutputStream(out, false);
        CipherOutputStream cos = new CipherOutputStream(fos, fileCipher);
        cos.write(inputBytes);
        cos.flush();
        cos.close();
        
        //Write encrypted symmetric key to disk
        File outKey = new File("symmetric");
        fos = new FileOutputStream(outKey, false);
        cos = new CipherOutputStream(fos, keyCipher);
        byte[] keyBytes = secretKey.getEncoded();
        cos.write(keyBytes);
        cos.flush();
        cos.close();
        
        //Write iv to disk
        keyCipher.init(Cipher.ENCRYPT_MODE, publicKey);
        File outIv = new File("iv");
        fos = new FileOutputStream(outIv, false);
        byte[] encryptedIV = keyCipher.doFinal(iv);
        fos.write(encryptedIV);
        fos.flush();
    
        //Close file streams
        fos.close();
        cos.close();
        
        //Delete "data.csv" (delete unencrypted data file)
        File in = new File("data.csv");
        in.delete();
    }
    
    //Decrypts "data" into "data.csv", then deletes "data"
    public static void decryptCSV() throws Exception {
        
        //Create Cipher objects
        Cipher fileCipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        
        //retrieve symmetric key with private key
        SecretKey symmetricKey = getSymmetricKey();
        
        //Retrieve iv
        byte[] iv = retrieveIV();
        
        //Initialize Cipher object
        fileCipher.init(Cipher.DECRYPT_MODE, symmetricKey, new IvParameterSpec(iv));
        
        //Open Encrypted file and read into byte array
        File in = new File("data");
        if(!in.isFile()) return ;
        FileInputStream fis = new FileInputStream(in);
        byte[] inputBytes = new byte[(int) in.length()];
        fis.read(inputBytes);
        
        //Decrypt file and store plain text in byte array
        byte[] outputBytes = fileCipher.doFinal(inputBytes);
        
        //Write decrypted file to disk
        File out = new File("data.csv");
        FileOutputStream fos = new FileOutputStream(out);
        fos.write(outputBytes);
        fos.flush();
        
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
    private static SecretKey getSymmetricKey() throws Exception{
        //First, store file's contents in byte array
        byte[] encryptedBytes = Server.fileToBytes("symmetric");
        //Now, retrieve Server's private key
        PrivateKey privateKey = getPrivateKey("PEM/private.key");
        //Now, set up Cipher to decrypt key
        Cipher keyCipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        keyCipher.init(Cipher.DECRYPT_MODE, privateKey);
        //Now, decrypt key
        byte[] keyBytes = keyCipher.doFinal(encryptedBytes);
        //Now, retrieve secret key object from byte array
        SecretKey secretKey = new SecretKeySpec(keyBytes, 0, keyBytes.length, "AES");
        //Return secret key
        return secretKey;
    }
    
    //Retrieved iv from disk
    private static byte[] retrieveIV() throws Exception {
        //Store file contents into byte array
        byte[] encrypted = Server.fileToBytes("iv");
        //Retrieve Server's private key
        PrivateKey privateKey = getPrivateKey("PEM/private.key");
        //Now, set up Cipher to decrypt IV
        Cipher ivCipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        ivCipher.init(Cipher.DECRYPT_MODE, privateKey);
        //Decrypt IV
        byte[] iv = ivCipher.doFinal(encrypted);
        //Return decrypted IV
        return iv;
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