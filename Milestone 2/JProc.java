/* Idris Soule, Michael Pang
** Java Process run C program
*/


//import java.util.*;
import java.io.*;
public class JProc {
public JProc()
{} //default constructor

public static void main(String args[])
{
      if(args.length != 1){
        System.out.println("Usage: java JProc <process>");
        System.exit(0);
      }
     String procName = args[0];
        try
        {            
            Runtime rt = Runtime.getRuntime();
            Process proc = rt.exec(procName);
            InputStream stderr = proc.getInputStream();
            InputStreamReader isr = new InputStreamReader(stderr);
            BufferedReader br = new BufferedReader(isr);
           
            String line = null;
            while ( (line = br.readLine()) != null)
                System.out.println(line);
         
            proc.waitFor();
        } catch (Throwable t) {
            t.printStackTrace();
          }
}
}  
