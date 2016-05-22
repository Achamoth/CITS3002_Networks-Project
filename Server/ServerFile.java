/*
 CITS3002 Project 2016
 Name:			Ammar Abu Shamleh
 Student number: 21521274
 Date:           May 2016
 */

import java.io.*;
import java.util.ArrayList;
import java.util.Set;
import java.security.cert.X509Certificate;
import javax.security.auth.x500.X500Principal;
import org.bouncycastle.util.io.pem.PemReader;
import org.bouncycastle.openssl.PEMParser;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.jgrapht.graph.DirectedMultigraph;
import org.jgrapht.graph.DefaultEdge;

public class ServerFile {
    
    private String filename; //The file's name
    private ArrayList<String> certificates; //A list of certificates (or their filenames) that have been used to vouch for this file
    
    //Creates new ServerFile object, given the file and its name
    public ServerFile(String fname) {
        this.filename = fname;
        certificates = new ArrayList<String>();
    }
    
    //Vouches for this file with a given certificate
    public void vouch(String certName) {
        //Add voucher to 'vouchers' (authentication of the certificate's existence on the server has already been done before this method is called)
        certificates.add(certName);
    }
    
    //Returns a string representation of this file's name (as it is on the hard disk)
    public String getFilename() {
        return this.filename;
    }
    
    //Calculates a circle of trust, and checks that the circle satisfies given requirements. Returns true or false
    //TODO: Note, if minCircleSize is 1, we'll just have to check for self-signed certificates manually without using the graph, since the graph doesn't allow for loops
    public boolean meetsRequirements(int minCircleSize, String requiredMember) {
        //Need to start by initializing graph with vouchers
        DirectedMultigraph<String, DefaultEdge> circle = null;
        try {
            circle = initGraph();
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(0); //I'm not sure exiting here is a great idea
        }
        
        //Identify whether or not the circle requires a specific member
        boolean reqMember = (requiredMember != null);
        
        if(!reqMember) {
            /* No member required. Simply find the first circle that satisfies minCircleSize */
            //Find all directed cycles contained within 'circle'
        }
        
        else {
            /* A specific member is required. Find all circles including that member */
            
            //First, find corresponding 'certOwner' string for 'requiredMember'
            String certOwner = findVertice(requiredMember, circle);
            
            //If certOwner is still null, then they haven't vouched for the file. Immediately return false
            if(certOwner == null) return false;
            
            //Now, we've found our required start node. Find all directed cycles from this node
            //TODO: Find all directed cycles from certOwner
        }
        return true; //For now
    }
    
    //Uses 'certificates' array list to initialize and return graph structure that contains vouchers and their trust amongst each other
    private DirectedMultigraph<String, DefaultEdge> initGraph() throws Exception{
        //Initialize empty graph
        DirectedMultigraph<String, DefaultEdge> result = new DirectedMultigraph<String, DefaultEdge> (DefaultEdge.class);
        
        //Set up path to certificates
        String certPath = System.getProperty("user.dir") + "/Certificates/";
        
        /*  Need to go through 'certificates' twice. Once to add all nodes, and a second time to add all edges */
        
        //First loop to add nodes
        for(String curCert : certificates) {
            /* Examine current certificate, and add owner to graph as a node */
            
            //First, construct X509Certificate object
            byte[] certBytes = Server.fileToBytes(certPath + curCert);
            PemReader reader;
            PEMParser parser;
            
            reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(certBytes)));
            parser = new PEMParser(reader);
            X509CertificateHolder certHolder = (X509CertificateHolder) parser.readObject();
            
            JcaX509CertificateConverter certCoverter = new JcaX509CertificateConverter();
            X509Certificate cert = certCoverter.setProvider("BC").getCertificate(certHolder);
            
            //Next, get X500Principal object from certificate
            X500Principal subject = cert.getSubjectX500Principal();
            
            //Now, get name of subject (certificate owner)
            String certOwner = subject.getName();
            
            //Now, add owner to graph
            result.addVertex(certOwner);
        }
        
        //Second loop to add edges and validate signatures
        for(String curCert : certificates) {
            /* Examine current certificate, and add a directed edge from signer (if they're in the graph) to owner */
            
            //First, construct X509Certificate object
            byte[] certBytes = Server.fileToBytes(certPath + curCert);
            PemReader reader;
            PEMParser parser;
            
            reader = new PemReader(new InputStreamReader(new ByteArrayInputStream(certBytes)));
            parser = new PEMParser(reader);
            X509CertificateHolder certHolder = (X509CertificateHolder) parser.readObject();
            
            JcaX509CertificateConverter certCoverter = new JcaX509CertificateConverter();
            X509Certificate cert = certCoverter.setProvider("BC").getCertificate(certHolder);
            
            //Next, get X500Principal object from certificate
            X500Principal subject = cert.getSubjectX500Principal();
            
            //Now, get name of subject (certificate owner)
            String certOwner = subject.getName();
            
            //Now, get X500Principal object from certificate (for signer)
            X500Principal signer = cert.getIssuerX500Principal();
            
            //Now, get name of signer
            String certSigner = signer.getname();
            
            //Now, check if signer is also in graph (i.e. they've also vouched for file)
            boolean signerAlsoVouched = result.containsVertex(certSigner);
            
            //If signer isn't in graph, move onto next certificate
            if(!signerAlsoVouched) continue;
            
            //If signer is in graph, verify signature
            //TODO: This
            
            //And now add directed edge from certificate signer to certificate owner
            result.addEdge(certSigner, certOwner);

        }
        return result;
    }
    
    //Given a graph of certificate owners, and a requiredMember (for a circle of trust), find and return the node that represents the requiredMember
    private String findVertice(String requiredMember, DirectedMultigraph<String, DefaultEdge> circle) {
        String certOwner = null;
        Set<String> vertices = circle.vertexSet();
        
        //Check all nodes for a match
        for(String curOwner : vertices) {
            //Split current node's string representation into fields
            String temp = new String(curOwner); //Since split() might modify argument
            String[] tokens = temp.split(",");
            //Obtain "common name" field
            String curName = tokens[0].substring(3);
            //Compare "common name" to requiredMember
            if(curName.equals(requiredMember)) {
                //If they're equal, then this node is the requiredMember
                certOwner = curOwner;
                break;
            }
        }
        return certOwner;
    }
}