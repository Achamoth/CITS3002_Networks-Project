import java.io.*;
import java.util.ArrayList;
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
    
    //Calculates a cricle of trust, and checks that the circle satisfies given requirements. Returns true or false
    //TODO: Note, if minCircleSize is 1, we'll just have to check for self-signed certificates manually without using the graph, since the graph doesn't allow for loops
    public boolean meetsRequirements(int minCircleSize, String requiredMember) {
        //Need to start by initializing graph with vouchers
        try {
            DirectedMultigraph<String, DefaultEdge> circle = initGraph();
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        //Identify whether or not the circle requires a specific member
        boolean reqMember = (requiredMember != null);
        
        if(!reqMember) {
            /* No member required. Simply find the first circle that satisfies minCircleSize */
            //Find all directed cycles contained within 'circle'
        }
        
        else {
            /* A specific member is required. Find all circles including that member */
            //TODO: Find all directed cycles contained within 'circle', starting from requiredMember
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
        
        //Second loop to add edges and validate signatures (that's the hard part :( )
        for(String curCert : certificates) {
            //Examine current certificate, and add a directed edge from signer (if they're in the graph) to owner
        }
        return result;
    }
}