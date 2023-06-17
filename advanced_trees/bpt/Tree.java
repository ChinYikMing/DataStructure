import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

public abstract class Tree implements TreeInterface {
    protected Node root;
    public Node getRoot() {
        return root;
    }
    public void setRoot(Node root) {
        this.root = root;
    }

    protected int fanout;
    public int getFanout() {
        return fanout;
    }
    public void setFanout(int fanout) {
        this.fanout = fanout;
    }

    protected int maxLevel;
    public int getMaxLevel() {
        return maxLevel;
    }
    public void setMaxLevel(int maxLevel) {
        this.maxLevel = maxLevel;
    }

    Tree(){
        setRoot(null);
    }

    @Override
    public void init(int order) {

    }

    @Override
    public void insert(int key) {
    }

    @Override
    public void delete(int key) {
        
    }

    @Override
    public LookupPair lookup(String expr) {
        return null;
    }

    @Override
    /**
     * level order
     * @param root
     * @return void
     */
    public void display(Node root, int identation) {
        // level-order
        Queue<Node> queue = new LinkedList<Node>();
        if(root == null){
            return;
        }

        queue.add(root);
        queue.add(null);

        Node nodeIter;
        List<Node> childPtrList;
        List<Integer> keyList = null;
        int level = 0;

        while(!queue.isEmpty()){
            nodeIter = queue.poll();
            if(nodeIter == null){
                level++;
                System.out.println();
                queue.add(null);
                if(queue.peek() == null)
                    break;
                else
                    continue;
            }

            keyList = nodeIter.getKeyList();
            childPtrList = nodeIter.getChildPtrList();

            if(keyList != null){
                for(int i = 0; i < keyList.size(); i++){
                    if(i == keyList.size() - 1){
                        System.out.print("(" + keyList.get(i) + ")");
                    } else {
                        System.out.print("(" + keyList.get(i) + "),");
                    }
                }
            }

            if(childPtrList != null){
                for(int i = 0; i < childPtrList.size(); i++){
                    if(childPtrList.get(i) != null){
                        queue.add(childPtrList.get(i));
                    }
                }
            }
        }

        // in-order
        // if(root == null)
        //     return;

        // List<Integer> keyList = root.getKeyList();
        // List<Node> childPtrList = root.getChildPtrList();

        // display(childPtrList.get(0));
        // System.out.println(keyList.get(0));
        // display(childPtrList.get(1));
    }

    @Override
    public void quit() {
    }
}