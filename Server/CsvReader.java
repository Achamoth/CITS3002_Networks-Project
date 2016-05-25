/*
 CITS3002 Project 2016
 Name:			Ammar Abu Shamleh
 Student number: 21521274
 Date:           May 2016
 */

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.File;
import java.util.ArrayList;
import java.lang.NullPointerException;

/**
 *  Class that governs reading from persistent storage using Csv files
 **/

public class CsvReader {
    //CSV Delimiters
    private static final String COMMA_DELIMITER = ",";
    private static final String NEWLINE_DELIMITER = "\n";
    
    /**
     * Reads CSV file and processes data
     **/
    public static void readCSV(String filename, ArrayList<ServerFile> files) {
        BufferedReader fr = null;
        boolean fileExists = true;
        
    readattempt: try {
        String line = null;
        
        //First, decrypt file, if it exists
        Crypto.decryptCSV();
        
        //Load file and create reader
        File f = new File(filename);
        if(!f.isFile()) {
            fileExists = false;
            break readattempt;
        }
        fr = new BufferedReader(new FileReader(filename));
        
        //Read header
        fr.readLine();
        
        //Read number of files
        int nFiles = Integer.parseInt(fr.readLine().trim());
        for(int i=0; i<nFiles; i++) {
            String curFileName = fr.readLine().trim();
            ServerFile curFile = new ServerFile(curFileName);
            fr.readLine();
            //Read all certificates that have been used to vouch for the file
            while(!(line = fr.readLine()).contains("***CERT_END***")) {
                String curCert = line.trim();
                //Add current certificate to file
                curFile.vouch(curCert);
            }
            files.add(curFile);
        }
        //Read last line
        fr.readLine();
    }
        catch (Exception e) {
            System.out.println("Error in CsvReader");
            e.printStackTrace();
        } finally {
            try {
                if(fileExists) fr.close();
            } catch(IOException e) {
                System.out.println("Error closing filereader");
                e.printStackTrace();
            }
        }
    }
}
