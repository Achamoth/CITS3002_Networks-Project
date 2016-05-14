import java.util.ArrayList;

public class ServerFile {
    
    private String filename;
    private ArrayList<String> vouchers;
    //TODO: A graph of vouchers (for finding a circle of trust)
    
    //Creates new ServerFile object, given the file and its name
    public ServerFile(String fname) {
        this.filename = fname;
        vouchers = new ArrayList<String>();
    }
    
    //Vouches for file with a given certificate
    public void vouch() {
        //Add client to 'vouchers'
        //Add client and their certificate signer to graph
    }
    
    //TODO: A method that searches the graph and returns an array list of all circles of trust on the file
}