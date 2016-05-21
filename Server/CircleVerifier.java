import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;
import com.google.common.collect.MultimapBuilder;
import com.google.common.collect.Sets;
import java.util.Collection;
import java.util.List;
import java.util.Set;

public class CircleVerifier{
	//	Structure that will hold the entire graph
	private final Multimap<String, String> edges;
	//	Set of individual circles (connected certificates)
	private final Set<String> circleSet;
	//	Set that holds the certificate file names that have been visited
	private final Set<String> visited;


    /*
        Constructor

        @param file     File to be verified
    */
    public CircleVerifier(String file){
        // Make custom multimap
        // Use a hashmap to map keys to values
        // Use a hashset to store value collections
        this.edges = MultimapBuilder.hashKeys().hashSetValues().build();
        this.circleSet = Sets.newHashSet();
        //  Add all the certificates which are connected
        circleSet.addAll(getRelatedCertificates(file));
        //  For each certificate file, add it as a node to the graph
        for(String certificate : circleSet){
            addEdges(certificate);
        }
    }

    /*
        getRelatedCertificates

        @param file     Name of file
        @return         Names of related certificates in a Set
    */
    private Set<String> getRelatedCertificates(String file){
        //  Get all the certificates and signature data of the certificates
        //  which have vouched for this file
        Collection<SecUtility.SigPair> sigs = 
            Security.getSigsOfFile(file);
        Set<String> relatedCerts = Sets.newHashSet();
        for(SecUtility.SigPair sp : sigs){
            relatedCerts.addAll(FileSpace.getCertsForPubKey(sp.getEncodedPubKey));
        }
    }



    /*
        addEdges

        Add a certificate that vouches another certificate (recursive DFS)
        Populate the edges container.
        A certificate is like a node on the graph

        @param certificate  Certificate name 
    */
    private void addEdges(String certificate){
        //  Populate a set with the unique certificates which are
        //  signed by "certificate"
        Set<String> adjacentCerts = getRelatedCertificates(certificate);
        //  Mark certificate (node) as being visited
        visited.add(certificate);
        for(String cert : adjacentCerts){
            //  Populate edges with the connection between an adjacent
            //  certificate and the certificate being queried
            edges.put(cert, certificate);
            //  If this cert (node) has already been visited, visit it now
            if(!visited.contains(cert)){
                addEdges(cert);
            }
        }
    }


    /*
        Largest Circle

        @return     All names of the certificates in Circle
    */
    public List<String> getLargestCircle(){
        List<String> maxCircle = Lists.newArrayList();
        //  Find largest circle by performing DFS on each cert (node)
        for(String cert : circleSet){
            //  Clear visited set
            visited.clear();
            //  Perform dfs looking for self to make a circle
            List<String> circle = depthFirstSearch(cert, cert);
            if(circle.size() > maxCircle.size()){
                maxCircle = circle;
            }
        }
        return Lists.reverse(maxCircle);
    }


    /*
        Find largest circle from given node

        @param start    Node that circle starts and eds
        @param node     Current node
        @return         Largest circle currently known
    */
    private List<String> depthFirstSearch(String start, String node){
        List<String> maxCircle = Lists.newArrayList();

        //  Base Case, start circle
        if(node.equals(start)){
            maxCircle.add(start);
        }
        //  Base Case, already part of circle
        if(visited.contains(node)){
            return maxCircle;
        }
        visited.add(node);
        for(String adjacent : edges.get(node)){
            //  Follow all adjacent edges on DFS
            List<String> circle = depthFirstSearch(start, adjacent);
            if(!circle.isEmpty()){
                circle.add(node);
                if(circle.size() > maxCircle.size()){
                    // Check and update if max Circle
                    maxCircle = circle;
                }
            }
        }
        //  Found largest circle that this node is part of
        //  Can be removed
        visited.remove(node);
        return maxCircle;
    }


    /*
        Find if circle meets minimum secuirty requirements

        @param minLength   Required length
        @return true if greater than ore quual to minLength     
    */
    public boolean minCircleSize(int minLength){
        return getCircleLength(getLargestCircle()) >= minLength;
    }


    /*
        Returns the circle's length
        Corrects for 0 and 1 removing self in count
    */
    public static int getCircleLength(List<String> circle){
        if(circle.size() == 0){
            return 0;
        }
        if(circle.size() == 1){
            return 1;
        }
        return circle.size() - 1;
    }

}