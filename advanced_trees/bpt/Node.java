import java.util.ArrayList;
import java.util.List;

class Node {
    private List<Integer> keyList = new ArrayList<Integer>();
    public List<Integer> getKeyList() {
        return keyList;
    }

    private List<Node> childPtrList = null;
    public List<Node> getChildPtrList() {
        return childPtrList;
    }

    // for implementing doubly linked list at primary page
    private Node rNode = null;
    public Node getrNode() {
        return rNode;
    }
    public void setrNode(Node rNode) {
        this.rNode = rNode;
    }

    // for implementing doubly linked list at primary page
    private Node lNode = null;
    public Node getlNode() {
        return lNode;
    }
    public void setlNode(Node lNode) {
        this.lNode = lNode;
    }

    // for implementing isam
    private Node overflowPtr = null;
    public Node getOverflowPtr() {
        return overflowPtr;
    }
    public void setOverflowPtr(Node overflowPtr) {
        this.overflowPtr = overflowPtr;
    }

    private boolean isLeaf = false;
    public boolean getIsLeaf() {
        return isLeaf;
    }
    public void setIsLeaf(boolean isLeaf) {
        this.isLeaf = isLeaf;
    }

    private boolean isOverflow = false;
    public boolean getIsOverflow() {
        return isOverflow;
    }
    public void setIsOverflow(boolean isOverflow) {
        this.isOverflow = isOverflow;
    }

    public void childPtrListReset(List<Node> childPtrList){
        for(int i = 0; i < childPtrList.size(); i++){
            childPtrList.set(i, null);
        }
    }

    Node(int fanout, boolean isLeaf){
        setIsLeaf(isLeaf);
        if(!isLeaf){
            childPtrList = new ArrayList<Node>();

            for(int i = 0; i < fanout; i++){
                if(i == fanout - 1){
                    childPtrList.add(null);
                    break;
                }

                childPtrList.add(null);
                keyList.add(-1);
            }
        } else {
            for(int i = 0; i < fanout - 1; i++){
                keyList.add(-1);
            }
        }
    }

    /**
     * @param fanout
     * @param isLeaf
     * @param entryPerPage, max capacity of entry in data file
     */
    Node(int fanout, boolean isLeaf, boolean isOverflow, int entryPerPage){
        setIsLeaf(isLeaf);
        setIsOverflow(isOverflow);
        if(!isLeaf){
            childPtrList = new ArrayList<Node>();

            for(int i = 0; i < fanout; i++){
                if(i == fanout - 1){
                    childPtrList.add(null);
                    break;
                }

                childPtrList.add(null);
            }
        }
    }
}
