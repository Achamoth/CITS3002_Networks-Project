/*
 CITS3002 Project 2016
 Name:			Ammar Abu Shamleh
 Student number: 21521274
 Date:           May 2016
 */

import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

/**
 *  Class that governs writing to persisent storage using Csv files
 **/

public class CsvWriter {
    //CSV Delimiters
    private static final String COMMA_DELIMITER = ",";
    private static final String NEWLINE_DELIMITER = "\n";
    
    /*
     * Writes the file and vouch data to a CSV file using delimiters
     */
    public static void writeToCSV(String filename, ArrayList<ServerFile> files) {
        FileWriter fw = null;
        
        try {
            fw = new FileWriter(filename);
            
            //Write all file names to the CSV file
            fw.append("***FILE_BEGIN***");
            fw.append(NEWLINE_DELIMITER);
            fw.append("" + files.size());
            fw.append(NEWLINE_DELIMITER);
            for(ServerFile curFile : files) {
                fw.append(curFile.getFilename());
                fw.append(NEWLINE_DELIMITER);
                //Mark start of certificates
                fw.append("***CERT_START***");
                fw.append(NEWLINE_DELIMITER);
                //Write all certificates to CSV file
                ArrayList<String> certificates = curFile.getVouchers();
                for(String curCert : certificates) {
                    fw.append(curCert);
                    fw.append(NEWLINE_DELIMITER);
                }
                fw.append("***CERT_END***");
                fw.append(NEWLINE_DELIMITER);
            }
            fw.append("***FILE_END***");
        }
        catch(Exception e) {
            System.out.println("Error in CsvFileWriter");
            e.printStackTrace();
        }
        finally {
            try {
                fw.flush();
                fw.close();
                //Encrypt file
                try {
                    Crypto.encryptCSV();
                } catch(Exception e) {
                    System.out.println("Error encrypting CSV file");
                    e.printStackTrace();
                }
            }
            catch (IOException e) {
                System.out.println("Error in flushing/closing filewriter");
                e.printStackTrace();
            }
        }
    }
}