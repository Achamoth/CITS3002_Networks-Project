import java.util.ArrayList;
import org.jgrapht.graph.DirectedMultigraph;
import org.jgrapht.graph.DefaultEdge;

public class ServerFile {
    
    private String filename;
    private ArrayList<String> vouchers;
    private DirectedMultigraph<Person, DefaultEdge> circle;
    
    //Creates new ServerFile object, given the file and its name
    public ServerFile(String fname) {
        this.filename = fname;
        vouchers = new ArrayList<String>();
        circle = new DirectedMultigraph<Person, DefaultEdge>(DefaultEdge.class);
    }
    
    //Vouches for this file with a given certificate
    public void vouch(Person p) {
        //Add voucher to 'vouchers'
        vouchers.add(p.getName());
        
        //Add client and their certificate signer to graph
    }
    
    public String getFilename() {
        return this.filename;
    }
    
    //TODO: A method that searches the graph and returns an array list of all circles of trust on the file
}