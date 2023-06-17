import java.util.*;
import java.util.regex.Pattern;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeModel;

public class Main{
    private static int ds;
    public static int getDs() {
        return ds;
    }

    public static void setDs(int ds) {
        Main.ds = ds;
    }

    private static int op;
    public static int getOp() {
        return op;
    }

    public static void setOp(int op) {
        Main.op = op;
    }

    private static int state = State.ORDER;
    public static int getState() {
        return state;
    }

    public static void setState(int state) {
        Main.state = state;
    }

    private static Tree bpt = new BPT();

    public static JFrame jf = new JFrame();
    public static JScrollPane jfScroll = null;
    public static JTree jt = new JTree();
    public static TreeModel tm = null;

    public static void main(String args[]){
        Scanner scanner = new Scanner(System.in);
        jf.setVisible(false);

        System.out.println(ConsoleColors.CYAN_BOLD + "Welcome!" + ConsoleColors.RESET);
        printStateInfo();

        while(scanner.hasNextLine()){
            run(scanner.nextLine().trim());

            if(state == State.QUIT){
                bpt.quit();
            }
        }
        scanner.close();
    }

    private static void printStateInfo(){
        System.out.println(ConsoleColors.PURPLE_BOLD);
        if(state == State.OP){
            System.out.println(ConsoleColors.RED_BOLD + "(enter c to change order of b+tree or h to show manual or q to quit)" + ConsoleColors.RESET);
            System.out.println("please select an operation on b+tree:");
            System.out.println("(1)insert (2)lookup (3)display (4)display(GUI)");
        } else if(state == State.KEY){
            System.out.println(ConsoleColors.RED_BOLD + "(enter c to change operation or h to show manual or q to quit)" + ConsoleColors.RESET);
            if(op == Operation.INSERT){
                System.out.println("please enter keys(separated by space, e.g., 1 3 5 7 9) to insert into b+tree:");
            } else if(op == Operation.LOOKUP){
                System.out.println("please enter a key to lookup from b+tree:");
            }
        } else if(state == State.ORDER){
            System.out.println(ConsoleColors.RED_BOLD + "(enter h to show manual or q to quit)" + ConsoleColors.RESET);
            System.out.println("please enter a num to set the order of b+tree:");
        } else if(state == State.RANGEEXPR){
            System.out.println(ConsoleColors.RED_BOLD + "(enter c to change operation or h to show manual or q to quit)" + ConsoleColors.RESET);
            if(op == Operation.LOOKUP){
                System.out.println("please enter an expression for range searching in b+tree:");
            }
            System.out.println("for example: = 6, > 6, >= 6, < 6, >= 6, 32 <= key < 100, 22 < key < 101");
        }
        System.out.println(ConsoleColors.RESET);
    }

    private static void usage(String input){
        if(input != null){
            System.out.println(ConsoleColors.RED_BOLD + "Invalid input: " + input + ConsoleColors.RESET);
            System.out.println(ConsoleColors.CYAN_BOLD + "Usage: please only use suggested options or inputs or enter h to show manual" + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.CYAN_BOLD + "Usage:");
            System.out.println("\t (1) " + ConsoleColors.RESET + "When you are entering the order of b+tree, "  + ConsoleColors.YELLOW_BOLD + "must be >= 1, "  +  
                                    "e.g, 1, 2, 3, ..." + ConsoleColors.RESET + ConsoleColors.CYAN_BOLD);
            System.out.println("\t (2) " + ConsoleColors.RESET + "When you are selecting the operation of b+tree, " + ConsoleColors.YELLOW_BOLD + "valid operation are 1, 2, 3 and 4 only");
            System.out.println("\t\t 1 is insert");
            System.out.println("\t\t 2 is single search or range search depends on the expression");
            System.out.println("\t\t 3 is display");
            System.out.println("\t\t 4 is display in GUI" + ConsoleColors.RESET + ConsoleColors.CYAN_BOLD);
            System.out.println("\t (3) " + ConsoleColors.RESET + "When you are entering the keys to insert into b+tree, ");
            System.out.println("\t\t it could be one or more keys, note that multiple keys" + ConsoleColors.YELLOW_BOLD + "should separated by space, e.g., 32, 64 23 12 18 66 70 80" + ConsoleColors.RESET + ConsoleColors.CYAN_BOLD);
            System.out.println("\t (4) " + ConsoleColors.RESET + "When you are entering the expression to search from b+tree, ");
            System.out.println("\t\t it could be single search or range search, the format as follow:");
            System.out.println("\t\t\t " + ConsoleColors.YELLOW_BOLD + "- single search, = x, x = {integer}, e.g., = 23");
            System.out.println("\t\t\t - range search, op x or a op key op b, x and a and b = {integer}, op = {<, <=, >, >=}, e.g., >= 32, < 32, 60 <= key <= 100" + ConsoleColors.RESET);
        }
    }

    private static boolean inputValidation(String input, List<Integer> num_list){
        if(state == State.OP){
            if(input.equals("c") || input.equals("q") ||
                input.equals("h")){
                    return true;
            } else {
                try {
                    int num = Integer.parseInt(input);
                    if(num != 1 && num != 2 && num != 3 && num != 4)
                        return false;
                    return true;
                } catch (NumberFormatException e) {
                    // System.out.println("input string is not numeric");
                }
                return false;
            }
        } else if(state == State.KEY){
            if(input.equals("c") || input.equals("q") ||
                input.equals("h")){
                    return true;
            } else {
                try {
                    StringTokenizer st = new StringTokenizer(input, " ");
                    while(st.hasMoreTokens()){
                        num_list.add(Integer.parseInt(st.nextToken()));
                    }
                    // Integer.parseInt(input);
                    return true;
                } catch (NumberFormatException e) {
                    // System.out.println("input string is not numeric");
                }

                return false;
            }
        } else if(state == State.ORDER){
                if(input.equals("q") || input.equals("h")){
                        return true;
                } else {
                    try {
                        int order = Integer.parseInt(input);
                        if(order <= 0)
                            return false;
                        return true;
                    } catch (NumberFormatException e) {
                        // System.out.println("input string is not numeric");
                    }

                    return false;
                }
        } else if(state == State.RANGEEXPR){
            if(input.equals("c") || input.equals("q") ||
                input.equals("h")){
                return true;
            } else {
                // String rangeExprRegex = "^[><=]=?\\s+-?\\d+(\\.\\d+)?$";
                String rangeExprRegex = "^[><=]=?\\s+-?\\d+(\\.\\d+)?|\\d+(\\.\\d+)?\\s+-?[><=]=?\\s+-?key\\s+-?[><=]=?\\s+-?\\d+(\\.\\d+)?$";
                boolean valid = Pattern.matches(rangeExprRegex, input);  

                return valid;
            }
        }

        return true;
    }

    private static void run(String input){
        DefaultMutableTreeNode n = null;
        TreeModel tm = null;

        if(state == State.ORDER){
            if(!inputValidation(input, null)){
                usage(input);
            } else {
                if(input.equals("q")){
                    setState(State.QUIT);
                } else if(input.equals("h")){
                    usage(null);
                } else {
                    // setState(State.PS);
                    setState(State.OP);
                    bpt.init(Integer.parseInt(input));
                }
            }
        } else if(state == State.OP){
            if(!inputValidation(input, null)){
                usage(input);
            } else {
                if(input.equals("c")){
                    setState(State.ORDER);
                } else if(input.equals("q")){
                    setState(State.QUIT);
                } else if(input.equals("h")){
                    usage(null);
                } else {
                    setOp(Integer.parseInt(input));
                    switch(Integer.parseInt(input)){
                        case Operation.INSERT:
                            setState(State.KEY);
                            break;
                        case Operation.LOOKUP:
                            setState(State.RANGEEXPR);
                            break;
                        case Operation.DISPLAY:
                            setState(State.OP);
                            bpt.display(bpt.getRoot(), 0);
                            break;
                        case Operation.DISPLAYGUI:
                            setState(State.OP);
                            n = ((BPT) bpt).displayGUI(bpt.getRoot(), null);
                            tm = new DefaultTreeModel(n);
                            jt.setModel(tm);

                            if(jfScroll == null){
                                jfScroll = new JScrollPane(jt,
                                                            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, 
                                                            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
                            } else {
                                jf.remove(jfScroll);
                                jfScroll = new JScrollPane(jt,
                                                            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, 
                                                            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
                            }

                            // expand all paths
                            for (int i = 0; i < jt.getRowCount(); i++) {
                                jt.expandRow(i);
                                jt.scrollRowToVisible(i);;
                            }

                            jf.add(jfScroll);
                            jf.setSize(500, 500);
                            jf.setVisible(true);
                            break;
                        default:
                            break;
                    }
                }
            }
        } else if(state == State.KEY){
            List<Integer> num_list = new ArrayList<Integer>();

            if(!inputValidation(input, num_list)){
                usage(input);
            } else {
                if(input.equals("c")){
                    setState(State.OP);
                } else if(input.equals("q")){
                    setState(State.QUIT);
                } else if(input.equals("h")){
                    usage(null);
                } else {
                    setState(State.OP);
                    for(Integer num : num_list){
                        System.out.println("Insert key " + num + ":");
                        bpt.insert(num);
                        bpt.display(bpt.getRoot(), 0);
                        System.out.println();
                    }
                }
            }
        } else if(state == State.RANGEEXPR){
            if(!inputValidation(input, null)){
                usage(input);
            } else {
                LookupPair ret = null;
                List<Integer> result = null;
                List<Integer> overflow = null;

                if(input.equals("c")){
                    setState(State.OP);
                } else if(input.equals("q")){
                    setState(State.QUIT);
                } else if(input.equals("h")){
                    usage(null);
                } else {
                    setState(State.OP);

                    ret = bpt.lookup(input);
                    result = ret.getResult();
                    overflow = ret.getOverflow();

                    if(result.size() == 0 && overflow.size() == 0){
                        System.out.println(ConsoleColors.RED_BOLD);
                        System.out.println("Empty");
                        System.out.println(ConsoleColors.RESET);
                    } else {
                        if(result.size() != 0){
                            System.out.println(ConsoleColors.GREEN_BOLD);
                            System.out.print("Result: ");
                            for(int i = 0; i < result.size(); i++){
                                if(i == result.size() - 1){
                                    System.out.print(result.get(i));
                                    break;
                                }
                                System.out.print(result.get(i) + ",");
                            }
                            System.out.println(ConsoleColors.RESET);
                        } else {
                            System.out.println(ConsoleColors.RED_BOLD);
                            System.out.println("No result!");
                            System.out.println(ConsoleColors.RESET);
                        }

                        if(overflow.size() != 0){
                            System.out.println(ConsoleColors.GREEN_BOLD);
                            System.out.print("Duplicated keys: ");
                            System.out.print("{");
                            for(int i = 0; i < overflow.size(); i++){
                                if(i == overflow.size() - 1){
                                    System.out.print(overflow.get(i));
                                    break;
                                }
                                System.out.print(overflow.get(i) + ",");
                            }
                            System.out.print("}");
                            System.out.println(ConsoleColors.RESET);
                        } else {
                            System.out.println(ConsoleColors.RED_BOLD);
                            System.out.println("No duplicated keys!");
                            System.out.println(ConsoleColors.RESET);
                        }
                    }
                }
            }
        }

        printStateInfo();
    }
}