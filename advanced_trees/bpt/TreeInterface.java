public interface TreeInterface {
    abstract void init(int order);
    abstract void insert(int key);
    abstract void delete(int key);
    abstract LookupPair lookup(String expr);
    abstract void display(Node root, int identation);
    abstract void quit();
}