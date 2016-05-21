import java.util.ArrayList;
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
    public boolean meetsRequirements(int minCircleSize, String requiredMember) {
        boolean reqMember = (requiredMember != null);
        
        if(!reqMember) {
            //No member required. Simply find the first circle that satisfies minCircleSize
        }
        
        //A specific member is required. Find all circles including that member
    }
}