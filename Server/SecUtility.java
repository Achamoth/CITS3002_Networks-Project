import com.google.common.base.Charsets;
import com.google.common.base.Objects;
import com.google.common.base.Preconditions;
import com.google.common.io.BaseEncoding;

import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.openssl.PEMEncryptedKeyPair;
import org.bouncycastle.openssl.PEMKeyPair;
import org.bouncycastle.openssl.PEMParser;
import org.bouncycastle.openssl.jcajce.JcePEMDecryptorProviderBuilder;

import java.io.IOException;
import java.io.StringReader;
import java.security.*;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Arrays;
/*
	SecUtility class

	Influenced by code written by Brain Tree in handling signatures, keys
	and certificates
	https://github.com/braintree/braintree_java/tree/master/src/main/java/com/braintreegateway

	Deals with Certificates, Keys and Signatures
*/
public class SecUtility{


	public static class SigPair{
		public final byte[] pubKey;
		public final byte[] sigData;

		/*
			Create a new Signature pair
		*/
		public SigPair(String key, String data){
			Preconditions.checkNotNull(key);
			Preconditions.checkNotNull(data);
			this.pubKey = BaseEncoding.base64().decode(key);
			this.sigData = BaseEncoding.base64().decode(data);
		}

		/*
			getPubKey
			getter

			@return 	Public key (encoded)
		*/
		public String getEncodedPubKey(){
			return BaseEncoding.base64().encode(pubKey);
		}

		/*
			getSignature
			getter

			@return 	Signature data (encoded)
		*/
		public String getEncodedSigData(){
			return BaseEncoding.base64().encode(sigData);
		}


		/*
			Provides public key as an object

			@return 	public key as object
		*/
		public PublicKey getPubKey() throws InvalidKeySpecException, 
			NoSuchAlgorithmException{
				KeyFactory keyFactory = KeyFactory.getInstance("RSA");
				X509EncodedKeySpec keySpecification = 
					new X509EncodedKeySpec(pubKey);
			//	Return public key  as an object
			return keyFactory.generatePublic(keySpecification);

		}

		@Override
		public boolean equals(Object object){
			// If is self referential
			if(this == object){
				return true;
			}
			// If NULL or not of this recognised class
			if(object == null || getClass() != object.getClass()){
				return false;
			}
			SigPair temp = (SigPair) object;
			
			// Compare every byte
			return Arrays.equals(pubKey, temp.pubKey) && 
				Arrays.equals(sigData, temp.sigData); 
		}
	}
}