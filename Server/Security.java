import com.google.common.base.Charsets;
import com.google.common.base.Preconditions;
import com.google.common.base.Splitter;
import com.google.common.collect.Iterables;
import com.google.common.collect.Multimap;
import com.google.common.collect.MultimapBuilder;
import com.google.common.io.Files;

import java.io.BufferedReader;
import java.util.Collection;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.Scanner;

/*
	Class that manages signatures
*/
public class Security{

	// Define and create directory as required
	private static final File SIGNATURE_DIR = new File("oldTrustyServer/signatures");

	static{
		if(!SIGNATURE_DIR.exists()){
			SIGNATURE_DIR.mkdirs();
		}
	}


	// Map of files to public key - signature pairs
	// Keys are hashed as are a collection of values
	private static final Multimap<String, SecUtility.SigPair> fileToSignatures =
		MultimapBuilder.hashKeys().hashSetValues().build();

	/*
		initialise

		Initialise Signature functions for server
	*/
	public static void initialise(){
		// Fill the Multimap
		// Check each file in signature directory into multimap for access
		for(File f : SIGNATURE_DIR.listFiles()){
			try{
				String name = f.getName();
				if(name.endsWith(".sig")){
					loadMapWithFile(name.substring(0, name.length() - 4));
				}
			}
			catch(Exception e){
				e.printStackTrace();
			}
		}
	}

	/*
		loadMapWithFile

		@param file 	name of signature file
	*/
	private static void loadMapWithFile(String file) throws Exception{
		System.err.println("Loading " + file + " signatures...");
		BufferedReader br = 
			new BufferedReader(
				new InputStreamReader(
					new FileInputStream(signatureFromFile(file)), 
						Charsets.UTF_8));
		Scanner scan = new Scanner(br);
		//  Move through line by line
		while(scan.hasNextLine()){
			String[] variables = Iterables.toArray(
				Splitter.on(' ').omitEmptyStrings().trimResults().split(scan.nextLine()), String.class);
			Preconditions.checkArgument(variables.length == 2);
			SecUtility.SigPair sP = new SecUtility.SigPair(variables[0], variables[1]);
			if(SecUtility.verifyData(sP, FileSpace.readFile(file))){
				//  If signature verified then load into multimap
				fileToSignatures.put(file, sP);
			}
		}
		scan.close();
	}


	/*
		addSignature

		Add signature for file provided

		@param file 			name of file
		@param signaturePair	Signature
		@return boolean check if signatue added correclty or not
	*/
	public static boolean addSignature(String file, SecUtility.SigPair sP){
		try{
			// Check if valid
			if(!SecUtility.verifyData(sP, FileSpace.readFile(file))){
				return false;
			}
			// Save signature to multimap
			fileToSignatures.put(file, sP);
			File f = signatureFromFile(file);
			// Check if file exists, if not creates it or if so, updates it
			Files.touch(f);
			// fil is updated with the public key and signature data
			Files.append(sP.getEncodedPubKey() + " " 
				+ sP.getEncodedSigData() + "\n", 
				f, Charsets.UTF_8);	// Using UTF_8 character set can be changed, not sure what it's signed with
			return true;
		}catch(Exception e){
			e.printStackTrace();
		}
		return false;
	}


	/*
		fileAlreadySigned

		Check if file already signed
		
		@param 	file 	name of file
		@param 	sP 		the signature 
	*/
	public static boolean fileAlreadySigned(String file, SecUtility.SigPair sP){
		// Check if multimap has signature
		return fileToSignatures.containsEntry(file, sP);
	}


	/*
		getSigsOfFile

		Provide list of signatures for the specified file

		@param file 	name of file
		@return 		List of signatures
	*/
	public static Collection<SecUtility.SigPair> getSigsOfFile(String file){
		// List of signatures from multimap
		return fileToSignatures.get(file);
	}

	/*
		clearSigsOfFile

		Clear all signatures from file

		@param file 	name of file
	*/
	public static void clearSigsOfFile(String file){
		File signedFile = signatureFromFile(file);
		if(signedFile.exists()){
			signedFile.delete();
		}
		if(fileToSignatures.containsKey(file)){
			//  If file present in multimap as key, remove all instances
			fileToSignatures.removeAll(file);
		}
	}



	/*
		Find .sig file for file named

		@param file 	name of file
	*/
	private static File signatureFromFile(String file){
		return new File(SIGNATURE_DIR, file + ".sig");
	}
}