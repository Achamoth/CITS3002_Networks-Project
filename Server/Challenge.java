/*
 Public Key cryptography using the RSA algorithm.
 */

import java.security.*;
import javax.crypto.*;

//This class deals with encryption and decryption for the server-client challenge
//http://www.mobilefish.com/developer/bouncycastle/bouncycastle.html
public class Challenge {
    
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
        //Create RSA Cipher object (specifying algorithm, mode, and padding
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        //Initialize Cipher object
        cipher.init(Cipher.DECRYPT_MODE, key);
        //Encrypt plaintext using the public key
        byte[] plainText= cipher.doFinal(cipherText);
        return plainText;
    }
}