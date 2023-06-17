import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import java.util.StringTokenizer;
import javax.swing.tree.DefaultMutableTreeNode;

public class BPT extends Tree {
    private int pageSize = 8; // in practice at least 4KB

    private int maxSizePerPage = -1;
    public int getMaxSizePerPage() {
        return maxSizePerPage;
    }
    public void setMaxSizePerPage(int maxSizePerPage) {
        this.maxSizePerPage = maxSizePerPage;
    }

    private int halfFull = -1;
    public int getHalfFull() {
        return halfFull;
    }
    public void setHalfFull(int halfFull) {
        this.halfFull = halfFull;
    }

    public BPT(){

    }

    private <E> E getLast(Collection<E> c) {
        E last = null;
        for(E e : c) last = e;
        return last;
    }

    private int order;
    public int getOrder() {
        return order;
    }
    public void setOrder(int order) {
        this.order = order;
    }

    @Override
    /**
     * @param fanout fanout is order in BPT, real fanout = order x 2 + 1, also is half full capacity
     * @return void
     */
    public void init(int order) { 
        setFanout((order << 1) + 1);
        setOrder(order);
        // setOrder((order << 1) + 1);
        setMaxSizePerPage(order << 1);
        setHalfFull(order);
        setRoot(null);
    }

    @Override
    public void insert(int key){
        Node root = getRoot();

        if(root == null){
            Node newPage = new Node(getFanout(), true, false, maxSizePerPage);
            newPage.getKeyList().add(key);
            setRoot(newPage);
            return;
        } else {
            StringBuffer keyExper = new StringBuffer("= " + Integer.toString(key));
            // List<Integer> res = lookup(keyExper.toString());
            LookupPair ret = lookup(keyExper.toString());
            List<Integer> res = ret.getResult();

            boolean keyExists = false;
            if(res.size() != 0){ // key exists
                // System.out.println("Duplicated key");
                keyExists = true;
            }

            // invariant: key not exists
            List<Node> _visited = new ArrayList<Node>();
            Node nodeIter = root;
            List<Node> childPtrList = null;
            List<Integer> keyList = null;
            Set<Node> visited = null;

            _visited.add(nodeIter);
            childPtrList = nodeIter.getChildPtrList();
            keyList = nodeIter.getKeyList();
            while(!nodeIter.getIsLeaf()){
                childPtrList = nodeIter.getChildPtrList();
                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(i == (keyList.size() - 1)){
                        if(key < keyList.get(i)){
                            nodeIter = childPtrList.get(i);
                        } else if(key >= keyList.get(i)){
                            nodeIter = childPtrList.get(i + 1);
                        }
                    } else {
                        if(key < keyList.get(i)){
                            nodeIter = childPtrList.get(i);
                            _visited.add(nodeIter);
                            break;
                        } 
                    }
                }

                _visited.add(nodeIter);
            }
            visited = new LinkedHashSet<Node>(_visited);
            keyList = nodeIter.getKeyList();

            if(keyExists){ // overflow page chaining
                if(nodeIter.getOverflowPtr() == null){
                    Node overflowPage = new Node(getFanout(), true, true, maxSizePerPage);
                    nodeIter.setOverflowPtr(overflowPage);
                }

                Node overflowPagePtr = nodeIter.getOverflowPtr();
                while(overflowPagePtr.getKeyList().size() == maxSizePerPage){ // find not full overflow page
                    if(overflowPagePtr.getOverflowPtr() == null)
                        break;
                    overflowPagePtr = overflowPagePtr.getOverflowPtr();
                }

                if(overflowPagePtr.getKeyList().size() == maxSizePerPage){ // last overflow page overflow => create new overflow page
                    Node overflowPageNew = new Node(getFanout(), true, true, maxSizePerPage);
                    overflowPagePtr.setOverflowPtr(overflowPageNew);
                }
                overflowPagePtr.getKeyList().add(key);
            } else {
                if(getMaxSizePerPage() == nodeIter.getKeyList().size()){ // full => overflow
                    Node newRoot = split(visited, key, null, null);
                    if(newRoot != null){
                        setRoot(newRoot);
                    }
                    return;
                }

                // invariant: not full
                keyList.add(key);
                Collections.sort(keyList);
            }
        }
    }

    private Node split(Set<Node> visited, int key, Node n1, Node n2){
        Node nodeNew = null;
        if(visited.size() == 0){
            nodeNew = new Node(getFanout(), false, false, maxSizePerPage);
            nodeNew.getKeyList().add(key);
            nodeNew.getChildPtrList().set(0, n1);
            nodeNew.getChildPtrList().set(1, n2);
            return nodeNew;
        }

        // if(visited.get(visited.size() - 1).getIsLeaf()){           // leaf node split
        if(getLast(visited).getIsLeaf()){           // leaf node split
            // Node nodeToSplit = visited.remove(visited.size() - 1);
            Node nodeToSplit = getLast(visited);
            visited.remove(nodeToSplit);
            nodeToSplit.getKeyList().add(key);
            Collections.sort(nodeToSplit.getKeyList());

            nodeNew = new Node(getFanout(), true, false, maxSizePerPage);
            for(int i = halfFull; i < nodeToSplit.getKeyList().size(); i++){
                nodeNew.getKeyList().add(nodeToSplit.getKeyList().get(i));
            }
            nodeNew.setlNode(nodeToSplit);
            nodeNew.setrNode(nodeToSplit.getrNode());

            if(nodeToSplit.getrNode() != null)
                nodeToSplit.getrNode().setlNode(nodeNew);

            int toDeleteKeyCnt = nodeToSplit.getKeyList().size() - halfFull;
            for(int i = 0; i < toDeleteKeyCnt; i++){
                nodeToSplit.getKeyList().remove(nodeToSplit.getKeyList().size() - 1);
            }
            nodeToSplit.setrNode(nodeNew);

            key = nodeNew.getKeyList().get(0); // key to copy up

            n1 = nodeToSplit;
            n2 = nodeNew;

            Collections.sort(nodeNew.getKeyList());
            Collections.sort(nodeToSplit.getKeyList());
             //// System.out.println("n1 key0: " + n1.getKeyList().get(0));
            // System.out.println("new Node key0: " + n2.getKeyList().get(0));
        } else {
            // Node par = visited.remove(visited.size() - 1);
            Node par = getLast(visited);
            visited.remove(par);
            int idx;
            int childPtrOnLeft;
            int childPtrIdx;

            List<Node> parBackupChildPtrList = new ArrayList<Node>();
            for(int i = 0; i < par.getChildPtrList().size(); i++){
                if(par.getChildPtrList().get(i) != null){
                    parBackupChildPtrList.add(par.getChildPtrList().get(i));
                }
            }

            if(getMaxSizePerPage() == par.getKeyList().size()){          //internal node overflow and split
                int oldKey = key; // used to borrowKey child pointer

                par.getKeyList().add(key);
                Collections.sort(par.getKeyList());

                nodeNew = new Node(getFanout(), false, false, maxSizePerPage);

                // update key
                for(int i = halfFull; i < par.getKeyList().size(); i++){
                    nodeNew.getKeyList().add(par.getKeyList().get(i));
                }

                key = nodeNew.getKeyList().remove(0); // key to push up

                int toDeleteKeyCnt = maxSizePerPage - halfFull + 1;
                for(int i = 0; i < toDeleteKeyCnt; i++){
                    par.getKeyList().remove(par.getKeyList().size() - 1);
                }

                // update ptr
                par.childPtrListReset(par.getChildPtrList()); // reset parent child pointer list because later will be borrowKeyd

                childPtrOnLeft = par.getKeyList().size() + 1;
                childPtrIdx = 0;

                idx = nodeNew.getKeyList().indexOf(oldKey);
                if(idx != -1){ // oldKey at newNode
                    int parBackupToRemoveIdx = parBackupChildPtrList.indexOf(n1);
                    if(idx == 0){ // oldKey at first
                        parBackupChildPtrList.remove(parBackupToRemoveIdx); // do not need to backup because it has splited to n1 and n2

                        for(int i = 0; i < childPtrOnLeft; i++){
                            par.getChildPtrList().set(i, parBackupChildPtrList.get(i));
                        }
                        Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));

                        nodeNew.getChildPtrList().set(childPtrIdx, n1);
                        childPtrIdx++;
                        nodeNew.getChildPtrList().set(childPtrIdx, n2);
                        childPtrIdx++;

                        for(int i = childPtrOnLeft; i < parBackupChildPtrList.size(); i++){
                            nodeNew.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }
                        Collections.sort(nodeNew.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                    } else if(idx == (nodeNew.getKeyList().size() - 1)){ // oldKey at last
                        parBackupChildPtrList.remove(parBackupChildPtrList.size() - 1); // last one do not need to backup because it has splited to n1 and n2

                        for(int i = 0; i < childPtrOnLeft; i++){
                            par.getChildPtrList().set(i, parBackupChildPtrList.get(i));
                        }
                        Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));

                        for(int i = childPtrOnLeft; i < parBackupChildPtrList.size(); i++){
                            nodeNew.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }

                        nodeNew.getChildPtrList().set(childPtrIdx, n1);
                        childPtrIdx++;
                        nodeNew.getChildPtrList().set(childPtrIdx, n2);
                        childPtrIdx++;
                        Collections.sort(nodeNew.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                    } else { // oldKey at middle
                        // 60 70 7 72 73 74 75 76 77 78 79 80 81 82 83 84 120 122 123 200 500 501 93 94 95  => 121 case middle order = 3
                        int toRemoveMiddleIdx = -1;
                        for(int i = 0; i < parBackupChildPtrList.size(); i++){
                            if(parBackupChildPtrList.get(i).getKeyList().contains(n1.getKeyList().get(0))){ // key is unique
                                toRemoveMiddleIdx = i;
                                break;
                            }
                        }
                        parBackupChildPtrList.remove(toRemoveMiddleIdx); // middle one do not need to backup because it has splited to n1 and n2

                        for(int i = 0; i < childPtrOnLeft; i++){
                            par.getChildPtrList().set(i, parBackupChildPtrList.get(i));
                        }
                        Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));

                        for(int i = childPtrOnLeft; i < childPtrOnLeft + idx; i++){
                            nodeNew.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }

                        nodeNew.getChildPtrList().set(childPtrIdx, n1);
                        childPtrIdx++;
                        nodeNew.getChildPtrList().set(childPtrIdx, n2);
                        childPtrIdx++;

                        for(int i = childPtrOnLeft + idx; i < parBackupChildPtrList.size(); i++){
                            nodeNew.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }
                        Collections.sort(nodeNew.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                    }
                } else { // oldKey at par
                    idx = par.getKeyList().indexOf(oldKey);
                    if(idx == 0){ // oldKey at first
                        parBackupChildPtrList.remove(0); // first one do not need to backup because it has splited to n1 and n2

                        par.getChildPtrList().set(childPtrIdx, n1);
                        childPtrIdx++;
                        par.getChildPtrList().set(childPtrIdx, n2);
                        childPtrIdx++;

                        for(int i = 0; i < childPtrOnLeft - 2; i++){
                            par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }
                        Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));

                        for(int i = 0, j = childPtrOnLeft - 2; j < parBackupChildPtrList.size(); i++, j++){
                            nodeNew.getChildPtrList().set(i, parBackupChildPtrList.get(j));
                        }
                        Collections.sort(nodeNew.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                    } else if(idx == (nodeNew.getKeyList().size() - 1)){ // oldKey at last
                        parBackupChildPtrList.remove(parBackupChildPtrList.size() - 1); // last one do not need to backup because it has splited to n1 and n2

                        for(int i = 0; i < childPtrOnLeft - 2; i++){
                            par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }

                        par.getChildPtrList().set(childPtrIdx, n1);
                        childPtrIdx++;
                        par.getChildPtrList().set(childPtrIdx, n2);
                        childPtrIdx++;
                        Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));

                        for(int i = 0, j = childPtrOnLeft - 2; j < parBackupChildPtrList.size(); i++, j++){
                            nodeNew.getChildPtrList().set(i, parBackupChildPtrList.get(j));
                        }
                        Collections.sort(nodeNew.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                    } else { // oldKey at middle
                        // 10 20 30 50 80 90 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 52 53 58  => 60 case middle order = 3
                        int toRemoveMiddleIdx = -1;
                        for(int i = 0; i < parBackupChildPtrList.size(); i++){
                            if(parBackupChildPtrList.get(i).getKeyList().contains(n1.getKeyList().get(0))){ // key is unique
                                toRemoveMiddleIdx = i;
                                break;
                            }
                        }
                        parBackupChildPtrList.remove(toRemoveMiddleIdx); // middle one do not need to backup because it has splited to n1 and n2

                        for(int i = 0; i < idx; i++){
                            par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }

                        par.getChildPtrList().set(childPtrIdx, n1);
                        childPtrIdx++;
                        par.getChildPtrList().set(childPtrIdx, n2);
                        childPtrIdx++;

                        for(int i = idx; i < childPtrOnLeft - 2; i++){
                            par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                            childPtrIdx++;
                        }
                        Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));

                        for(int i = 0, j = childPtrOnLeft - 2; j < parBackupChildPtrList.size(); i++, j++){
                            nodeNew.getChildPtrList().set(i, parBackupChildPtrList.get(j));
                        }
                        Collections.sort(nodeNew.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                    }
                    // 51 29 73 105 15 31, order = 1 hw pdf case 
                    // 1 2 3 4 5 6 7 8 9 10 11 12 => 13, at nodeNew case
                    // 16 17 18 19 20 21 22 23 24 25 26 15 14 => 13, at par case
                    // 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36
                    // 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49
                }
                n1 = par;
                n2 = nodeNew;
            } else {                              // internal node has space
                par.getKeyList().add(key);
                Collections.sort(par.getKeyList());
                par.childPtrListReset(par.getChildPtrList());

                // update ptr
                childPtrIdx = 0;
                idx = par.getKeyList().indexOf(key);
                if(idx == 0){ // key at first
                    parBackupChildPtrList.remove(0); // first one do not need to backup because it has splited to n1 and n2

                    par.getChildPtrList().set(childPtrIdx, n1);
                    childPtrIdx++;
                    par.getChildPtrList().set(childPtrIdx, n2);
                    childPtrIdx++;

                    for(int i = 0; i < parBackupChildPtrList.size(); i++){
                        par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                        childPtrIdx++;
                    }
                    Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                } else if(idx == (par.getKeyList().size() - 1)){ // key at last
                    parBackupChildPtrList.remove(parBackupChildPtrList.size() - 1); // last one do not need to backup because it has splited to n1 and n2

                    for(int i = 0; i < parBackupChildPtrList.size(); i++){
                        par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                        childPtrIdx++;
                    }

                    par.getChildPtrList().set(childPtrIdx, n1);
                    childPtrIdx++;
                    par.getChildPtrList().set(childPtrIdx, n2);
                    childPtrIdx++;
                    Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                } else { // key at middle
                    int toRemoveMiddleIdx = -1;
                    for(int i = 0; i < parBackupChildPtrList.size(); i++){
                        if(parBackupChildPtrList.get(i).getKeyList().contains(n1.getKeyList().get(0))){ // key is unique
                            toRemoveMiddleIdx = i;
                            break;
                        }
                    }
                    parBackupChildPtrList.remove(toRemoveMiddleIdx); // middle one do not need to backup because it has splited to n1 and n2

                    for(int i = 0; i < idx; i++){
                        par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                        childPtrIdx++;
                    }

                    par.getChildPtrList().set(childPtrIdx, n1);
                    childPtrIdx++;
                    par.getChildPtrList().set(childPtrIdx, n2);
                    childPtrIdx++;

                    for(int i = idx; i < parBackupChildPtrList.size(); i++){
                        par.getChildPtrList().set(childPtrIdx, parBackupChildPtrList.get(i));
                        childPtrIdx++;
                    }
                    Collections.sort(par.getChildPtrList(), Comparator.nullsLast(new ChildPtrComparator()));
                }

                return null;   // no need to update root
            }
        }

        return split(visited, key, n1, n2);
    }

    @Override
    public LookupPair lookup(String expr) {
        if(this.root == null){
            return null;
        }

        // List<Integer> result = new ArrayList<Integer>();
        StringTokenizer st = new StringTokenizer(expr, " ");
        LookupPair ret = new LookupPair();
        List<Integer> result = ret.getResult();
        List<Integer> overflow = ret.getOverflow();

        String oper;
        int key;

        String oper1, oper2;
        int key1, key2;

        Node nodeIter = null;
        List<Node> childPtrList = null;
        List<Integer> keyList = null;
        Node right = null, left = null;
        Node overflowPtr = null;

        if(st.countTokens() == 2){
            oper = st.nextToken();
            key = Integer.parseInt(st.nextToken());
            // System.out.println("oper: " + oper);
            // System.out.println("key: " + key);

            nodeIter = this.root;
            childPtrList = null;
            keyList = null;

            while(!nodeIter.getIsLeaf()){
                childPtrList = nodeIter.getChildPtrList();
                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(i == (keyList.size() - 1)){
                        if(key < keyList.get(i)){
                            nodeIter = childPtrList.get(i);
                        } else if(key >= keyList.get(i)){
                            nodeIter = childPtrList.get(i + 1);
                        }
                    } else {
                        if(key < keyList.get(i)){
                            nodeIter = childPtrList.get(i);
                            break;
                        } 
                    }
                }
            }

            // reach data file
            overflowPtr = nodeIter.getOverflowPtr();
            if(oper.equals("=")){
                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) == key){
                        result.add(keyList.get(i));
                    }
                }

                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) == key){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

            } else if(oper.equals(">")){
                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) > key){
                        result.add(keyList.get(i));
                    }
                }

                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) > key){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                nodeIter = nodeIter.getrNode();
                while(nodeIter != null){
                    keyList = nodeIter.getKeyList();

                    for(int i = 0; i < keyList.size(); i++){
                        result.add(keyList.get(i));
                    }

                    overflowPtr = nodeIter.getOverflowPtr();

                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) > key){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    nodeIter = nodeIter.getrNode();
                }
            } else if(oper.equals(">=")){
                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) >= key){
                        result.add(keyList.get(i));
                    }
                }

                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) >= key){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                nodeIter = nodeIter.getrNode();
                while(nodeIter != null){
                    keyList = nodeIter.getKeyList();

                    for(int i = 0; i < keyList.size(); i++){
                        result.add(keyList.get(i));
                    }

                    overflowPtr = nodeIter.getOverflowPtr();
                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) >= key){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    nodeIter = nodeIter.getrNode();
                }
            } else if(oper.equals("<")){

                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) < key){
                        result.add(keyList.get(i));
                    }
                }

                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) < key){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                nodeIter = nodeIter.getlNode();
                while(nodeIter != null){
                    keyList = nodeIter.getKeyList();

                    for(int i = 0; i < keyList.size(); i++){
                        result.add(keyList.get(i));
                    }

                    overflowPtr = nodeIter.getOverflowPtr();
                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) < key){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    nodeIter = nodeIter.getlNode();
                }
            } else if(oper.equals("<=")){
                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) <= key){
                        result.add(keyList.get(i));
                    }
                }

                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) <= key){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                nodeIter = nodeIter.getlNode();
                while(nodeIter != null){
                    keyList = nodeIter.getKeyList();

                    for(int i = 0; i < keyList.size(); i++){
                        result.add(keyList.get(i));
                    }

                    overflowPtr = nodeIter.getOverflowPtr();
                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) <= key){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    nodeIter = nodeIter.getlNode();
                }
            }
        } else {
            key1 = Integer.parseInt(st.nextToken());
            oper1 = st.nextToken();
            st.nextToken(); // the 'key' token, skip it
            oper2 = st.nextToken();
            key2 = Integer.parseInt(st.nextToken());

            nodeIter = this.root;
            childPtrList = null;
            keyList = null;

            while(!nodeIter.getIsLeaf()){
                childPtrList = nodeIter.getChildPtrList();
                keyList = nodeIter.getKeyList();

                for(int i = 0; i < keyList.size(); i++){
                    if(i == (keyList.size() - 1)){
                        if(key1 < keyList.get(i)){
                            nodeIter = childPtrList.get(i);
                        } else if(key1 >= keyList.get(i)){
                            nodeIter = childPtrList.get(i + 1);
                        }
                    } else {
                        if(key1 < keyList.get(i)){
                            nodeIter = childPtrList.get(i);
                            break;
                        } 
                    }
                }
            }

            // reach data file
            boolean done = false;
            if(oper1.equals("<") && oper2.equals("<")){
                right = nodeIter.getrNode();

                keyList = nodeIter.getKeyList();
                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) >= key2){
                        break;
                    }

                    if(keyList.get(i) > key1){
                        result.add(keyList.get(i));
                    }
                }

                overflowPtr = nodeIter.getOverflowPtr();
                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) > key1 && keyList.get(i) < key2){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                // 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36

                while(right != null){
                    nodeIter = right;

                    keyList = nodeIter.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) >= key2){
                            done = true;
                            break;
                        }

                        if(keyList.get(i) < key2){
                            result.add(keyList.get(i));
                        }
                    }

                    overflowPtr = nodeIter.getOverflowPtr();
                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) > key1 && keyList.get(i) < key2){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    if(done)
                        break;

                    right = right.getrNode();
                }
            } else if(oper1.equals("<") && oper2.equals("<=")){
                right = nodeIter.getrNode();

                keyList = nodeIter.getKeyList();
                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) > key2){
                        break;
                    }

                    if(keyList.get(i) > key1){
                        result.add(keyList.get(i));
                    }
                }

                overflowPtr = nodeIter.getOverflowPtr();
                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) > key1 && keyList.get(i) <= key2){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                while(right != null){
                    nodeIter = right;

                    keyList = nodeIter.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) > key2){
                            done = true;
                            break;
                        }

                        if(keyList.get(i) <= key2){
                            result.add(keyList.get(i));
                        }
                    }

                    overflowPtr = nodeIter.getOverflowPtr();
                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) > key1 && keyList.get(i) <= key2){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    if(done)
                        break;

                    right = right.getrNode();
                }
            } else if(oper1.equals("<=") && oper2.equals("<")){
                right = nodeIter.getrNode();

                keyList = nodeIter.getKeyList();
                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) >= key2){
                        break;
                    }

                    if(keyList.get(i) >= key1){
                        result.add(keyList.get(i));
                    }
                }

                overflowPtr = nodeIter.getOverflowPtr();
                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) >= key1 && keyList.get(i) < key2){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                while(right != null){
                    nodeIter = right;

                    keyList = nodeIter.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) >= key2){
                            done = true;
                            break;
                        }

                        if(keyList.get(i) < key2){
                            result.add(keyList.get(i));
                        }
                    }

                    overflowPtr = nodeIter.getOverflowPtr();
                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) >= key1 && keyList.get(i) < key2){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    if(done)
                        break;

                    right = right.getrNode();
                }
            } else if(oper1.equals("<=") && oper2.equals("<=")){
                right = nodeIter.getrNode();

                keyList = nodeIter.getKeyList();
                for(int i = 0; i < keyList.size(); i++){
                    if(keyList.get(i) > key2){
                        break;
                    }

                    if(keyList.get(i) >= key1){
                        result.add(keyList.get(i));
                    }
                }

                overflowPtr = nodeIter.getOverflowPtr();
                while(overflowPtr != null){
                    keyList = overflowPtr.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) >= key1 && keyList.get(i) <= key2){
                            overflow.add(keyList.get(i));
                        }
                    }
                    overflowPtr = overflowPtr.getOverflowPtr();
                }

                while(right != null){
                    nodeIter = right;

                    keyList = nodeIter.getKeyList();
                    for(int i = 0; i < keyList.size(); i++){
                        if(keyList.get(i) > key2){
                            done = true;
                            break;
                        }

                        if(keyList.get(i) <= key2){
                            result.add(keyList.get(i));
                        }
                    }

                    overflowPtr = nodeIter.getOverflowPtr();
                    while(overflowPtr != null){
                        keyList = overflowPtr.getKeyList();
                        for(int i = 0; i < keyList.size(); i++){
                            if(keyList.get(i) >= key1 && keyList.get(i) <= key2){
                                overflow.add(keyList.get(i));
                            }
                        }
                        overflowPtr = overflowPtr.getOverflowPtr();
                    }

                    if(done)
                        break;

                    right = right.getrNode();
                }
            }
        }

        return ret;
    }

    public DefaultMutableTreeNode displayGUI(Node root, DefaultMutableTreeNode par) {
        // pre-order

        if(root == null){
            return null;
        }

        Node nodeIter = root;
        List<Integer> keyList = nodeIter.getKeyList();
        List<Node> childPtrList = nodeIter.getChildPtrList();
        StringBuffer strbuf = new StringBuffer();

        int emptySize = getFanout() - keyList.size() - 1;

        if(keyList.size() == 0){
            strbuf.append("[");
        } else {
            for(int i = 0; i < keyList.size(); i++){
                if(i == 0) {
                    if(nodeIter.getIsLeaf()){
                        strbuf.append("[");
                    } else {
                        strbuf.append("(");
                    }
                } else if(i == keyList.size() - 1){
                    strbuf.append(Integer.toString(keyList.get(i)));
                    if(emptySize >= 1){
                        if(nodeIter.getIsLeaf()){
                            strbuf.append(",");
                        } else {
                            strbuf.append(":");
                        }
                    }
                    break;
                }
                strbuf.append(Integer.toString(keyList.get(i)));
                if(nodeIter.getIsLeaf()){
                    strbuf.append(",");
                } else {
                    strbuf.append(":");
                }
            }
        }

        if(!nodeIter.getIsLeaf()){
            if(emptySize == 0){
                strbuf.append(")");
            } else {
                if(emptySize == 1){
                    // strbuf.append(":");
                    strbuf.append("_");
                    strbuf.append(")");
                } else {
                    for(int i = 0; i < emptySize; i++){
                        if(i == emptySize - 1){
                            strbuf.append("_");
                            strbuf.append(")");
                            break;
                        }
                        strbuf.append("_");
                        strbuf.append(":");
                    }
                }
            }
        } else {
            emptySize = getMaxSizePerPage() - keyList.size();
            if(emptySize == 0){
                strbuf.append("]");
            } else {
                if(emptySize == 1){
                    // strbuf.append(",");
                    strbuf.append("_");
                    strbuf.append("]");
                } else {
                    for(int i = 0; i < emptySize; i++){
                        if(i == emptySize - 1){
                            strbuf.append("_");
                            strbuf.append("]");
                            break;
                        }
                        strbuf.append("_");
                        strbuf.append(",");
                    }
                }
            }
        }
        DefaultMutableTreeNode n, tmp;
        n = new DefaultMutableTreeNode(strbuf.toString());

        if(childPtrList != null){
            for(int i = 0; i < childPtrList.size(); i++){
                tmp = displayGUI(childPtrList.get(i), n);
                if(tmp != null)
                    n.add(tmp);
            }
        }

        return n;
    }

    @Override
    public void display(Node root, int identation) {
        // pre-order

        if(root == null){
            return;
        }

        for(int i = 0; i < identation; i++){
            System.out.print(" ");
        }

        Node nodeIter = root;
        List<Integer> keyList = nodeIter.getKeyList();
        List<Node> childPtrList = nodeIter.getChildPtrList();

        int emptySize = getFanout() - keyList.size() - 1;

        if(keyList.size() == 0){
            System.out.print("[");
        } else {
            for(int i = 0; i < keyList.size(); i++){
                if(i == 0) {
                    if(nodeIter.getIsLeaf()){
                        System.out.print("[");
                    } else {
                        System.out.print("(");
                    }
                } else if(i == keyList.size() - 1){
                    System.out.print(keyList.get(i));
                    if(emptySize >= 1){
                        if(nodeIter.getIsLeaf()){
                            System.out.print(",");
                        } else {
                            System.out.print(":");
                        }
                    }
                    break;
                }
                System.out.print(keyList.get(i));
                if(nodeIter.getIsLeaf()){
                    System.out.print(",");
                } else {
                    System.out.print(":");
                }
            }
        }

        if(!nodeIter.getIsLeaf()){
            if(emptySize == 0){
                System.out.print(")");
            } else {
                if(emptySize == 1){
                    // System.out.print(":");
                    System.out.print("_");
                    System.out.print(")");
                } else {
                    for(int i = 0; i < emptySize; i++){
                        if(i == emptySize - 1){
                            System.out.print("_");
                            System.out.print(")");
                            break;
                        }
                        System.out.print("_");
                        System.out.print(":");
                    }
                }
            }
        } else {
            emptySize = getMaxSizePerPage() - keyList.size();
            if(emptySize == 0){
                System.out.print("]");
            } else {
                if(emptySize == 1){
                    // System.out.print(",");
                    System.out.print("_");
                    System.out.print("]");
                } else {
                    for(int i = 0; i < emptySize; i++){
                        if(i == emptySize - 1){
                            System.out.print("_");
                            System.out.print("]");
                            break;
                        }
                        System.out.print("_");
                        System.out.print(",");
                    }
                }
            }
        }
        System.out.println();

        if(childPtrList != null){
            for(int i = 0; i < childPtrList.size(); i++){
                display(childPtrList.get(i), identation + 2);
            }
        }
    }

    @Override
    public void quit() {
        System.out.println(ConsoleColors.CYAN_BOLD + "Goodbye!" + ConsoleColors.RESET);
        System.exit(0);
    }
}
