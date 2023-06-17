import java.util.Comparator;
import java.util.List;

public class ChildPtrComparator implements Comparator<Node> {
    @Override
    public int compare(final Node n1, final Node n2) {
        List<Integer> n1KeyList = n1.getKeyList();
        List<Integer> n2KeyList = n2.getKeyList();

        if(n1KeyList != null && n2KeyList == null){
            return n1KeyList.get(0).compareTo(null);
        } else if(n1KeyList == null && n2KeyList != null){
            return n2KeyList.get(0).compareTo(null);
        }
        
        return n1KeyList.get(0).compareTo(n2KeyList.get(0));
    }
}