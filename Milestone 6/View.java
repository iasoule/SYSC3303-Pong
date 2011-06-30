import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.StringTokenizer;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import java.io.*;
import java.net.*;

/** 
    VIEW under FEDORA - REDHAT LINUX
    Idris Soule, Michael Pang
**/

public class View extends JFrame implements Runnable{

	private final static int X=600;
	private final static int Y=600;
	private boolean gameStart = false;
	private JMenuItem add, exit, single, twoplayer;
	private Socket connected;
	private ServerSocket serverSocket;


	boolean connection = false;

	private PongPanel pPanel; 
	private CountDownPanel cPanel;
	private ScorePanel sPanel;
	Thread ViewThread;
	boolean receivemsg =false;

	private boolean stopThread = false;

	private boolean addPong = false;

	private boolean reset = false;

	public View(int port){
		super("SYSC 3303 Pong");
		this.view_initalize();

		try {
			serverSocket = new ServerSocket(port);
		} catch (IOException e) {
			e.printStackTrace(System.err);
			System.exit(1);
		}
		ViewThread = new Thread(this);
		ViewThread.start();

		try {
			ViewThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	public static void main(String[] args)
	{	     
		View view = new View(Integer.parseInt(args[0]));
	}


	/**
	 * To setup all the default view requirement
	 * in this case, all the frame will be setup and the listner will also be setup in this function
	 */
	public void view_initalize(){

		this.setSize(X,Y);

		add = new JMenuItem("Add Pong");
		exit = new JMenuItem("Exit");

		JMenu fmenu = new JMenu("File");
		fmenu.add( add );
		fmenu.add( exit );


		JMenuBar menubar = new JMenuBar();
		menubar.add( fmenu );
		setJMenuBar( menubar );

		cPanel = new View.CountDownPanel();
		sPanel = new View.ScorePanel();
		pPanel = new View.PongPanel();

		Container cp = getContentPane();
		cp.add( pPanel, BorderLayout.CENTER );
		cp.add( sPanel, BorderLayout.NORTH );
		cp.add( cPanel, BorderLayout.SOUTH );

		pPanel.grabFocus();


		this.setVisible(true);
	}

	public void run() {
		DataInputStream in;
		DataOutputStream out;
		boolean doneread = false;
		try {
			int bytesRead;
			int max_data = 5;
			connected = this.serverSocket.accept();
			in = new DataInputStream(new BufferedInputStream(connected.getInputStream()));
			byte input_buffer[] = new byte[26];
			while (!serverSocket.isClosed()){

				int num_bytes = in.read(input_buffer,0,25);
				
				

				String input_string = new String(input_buffer);
			
				
				if(num_bytes < 0){
					JOptionPane.showMessageDialog(null, "Client Disconnected ...");
					System.exit(0);
				}
				
				StringTokenizer st = new StringTokenizer(input_string, ";");
				
				String recv_instruction = "";
				String analysis = "";
				int[] data = new int[6];
				int index =1;
				for (int i = 0;i <max_data && st.hasMoreTokens(); i++) {
					if(i == 0){
						recv_instruction = st.nextToken();
					}
					else {
						String s = st.nextToken();
						if(!s.startsWith("x", 0)){
							data[index] = Integer.parseInt(s);
							index++;
						}
					}
				}
				
				if(recv_instruction.equals("pong")){
					if(pPanel.pongCreation < 2){
						pPanel.pongCreated();
						if (pPanel.pongCreation == 1){
							pPanel.paintOpponentPongNotify(data[1], data[2], data[3], data[4]);
						}
						else {
							pPanel.paintPongNotify(data[1], data[2], data[3], data[4]);
						}
					}
					
				}
				else if (recv_instruction.equals("player1")){ //L/R
				    if(pPanel.pongCreation > 1 )
						pPanel.paintPongNotify(data[1], data[2], data[3], data[4]);

				}
				else if (recv_instruction.equals("player2")){ //L/R	
						pPanel.paintOpponentPongNotify(data[1], data[2], data[3], data[4]);
				}
				else if (recv_instruction.equals("ball")){
					if(pPanel.ballCreation){
						pPanel.paintBallNotify(data[1], data[2], data[3]);
					}
					else{
						pPanel.ballCreated();
						pPanel.paintBallNotify(data[1], data[2], data[3]);
					}
				}
				else if (recv_instruction.equals("score")){
					this.sPanel.scoreUpdate(data[1], data[2]);
				}
				
				else if(recv_instruction.equals("reset")){
					this.reset = true;
				}
				else if(recv_instruction.equals("Exit")){
					System.exit(0);
				}
				else if(recv_instruction.equals("count")){
					cPanel.countDown("Waiting for another player. Time remaining:", data[1]);
				}
				
				
			}


		} 
		catch (IOException e) {
			System.exit(1);
		}

	}
}

	class ScorePanel extends JPanel
	{
		private JTextField top, bottom;
		/**
		 * A setup of the score panel
		 */
		public ScorePanel()
		{
			add( new  JLabel("Top") );
			top = new JTextField(20);
			top.setEditable(false);
			add( top );
			add(  new JLabel( "Bottom" ) );
			bottom = new JTextField(20);
			bottom.setEditable(false);
			add( bottom );

		}
		/**
		 * A update method to update the score 
		 * @param playerScore the current player score
		 * @param opponentScore the current AI score or the opponent score
		 */ 
		public void scoreUpdate(int playerScore,int opponentScore){
			top.setText("Score: "+playerScore);
			bottom.setText("Score: "+opponentScore);
			this.repaint();
		}

	}

	class CountDownPanel extends JPanel
	{
		private JTextField count;

		public CountDownPanel()
		{
			count = new JTextField(40);
			count.setText("No Connection!");
			add(count);
		}
	        public void countDown(String msg, int i){
			this.repaint();
			if (i > 0)
			    count.setText(msg + i);
			else 
			    count.setText("Game Begin");
		}
	}


class PongPanel extends JPanel 
	{ 

		private final static int MAX_PONGS_PER_SIDE = 1;
		private final static int MAX_PONGS = MAX_PONGS_PER_SIDE * 2;

		private int nPongs;
		private int currentlocation;
		private int currentX;
		private int currentY;

		private int currentBallX;
		private int currentBallY;
		private int currentBallRadius;

		private int pongLeftcornerX;
		private int pongLeftcornerY;
		private int pongLength;
		private int pongHeight;

		private int pongOpponentLeftcornerX;
		private int pongOpponentLeftcornerY;
		private int pongOpponentLength;
		private int pongOpponentHeight;
		public int pongCreation = 0;

		private Thread displayThread;
		public boolean ballCreation = false;

		public PongPanel()
		{
			/*Decide problem*/
			setBackground(Color.black);
			}
		/**
		 * A setup method to notify the view about ball is created
		 */
		public void ballCreated (){
			ballCreation = true;
		}

		/**
		 * A setup method to notify the view everytime the pong is created
		 */
		public void pongCreated(){
			pongCreation++;
		}

		public void paintComponent(Graphics g)
		{
			super.paintComponent(g);
			g.setColor(Color.green);

			if(this.ballCreation){
				g.fillOval( this.currentBallX,this.currentBallY, this.currentBallRadius, this.currentBallRadius) ;
			}
			g.setColor(Color.ORANGE);
			if(this.pongCreation == 1){	
				g.fillRect(this.pongOpponentLeftcornerX, this.pongOpponentLeftcornerY, this.pongOpponentLength, this.pongOpponentHeight);
			}
			if(this.pongCreation >= 2){	
				g.fillRect(this.pongOpponentLeftcornerX, this.pongOpponentLeftcornerY, this.pongOpponentLength, this.pongOpponentHeight);
				g.fillRect(this.pongLeftcornerX, this.pongLeftcornerY, this.pongLength, this.pongHeight);
			}
		}

		/**
		 * A setup function of the current ball location
		 * @param x The ball location
		 * @param y The ball location 
		 * @param radius the ball radius
		 */
		public void paintBallNotify(int x,int y,int radius){
			this.currentBallX = x;
			this.currentBallY =y;
			this.currentBallRadius = radius;
			repaint();
		}
		/**
		 * A setup function of the user pong location
		 * @param leftCornerX the user pong location
		 * @param leftCornerY the user pong location
		 * @param length the user pong length
		 * @param height the user pong height
		 */

		public void paintPongNotify(int leftCornerX, int leftCornerY,int length, int height){
			this.pongLeftcornerX = leftCornerX;
			this.pongLeftcornerY = leftCornerY;
			this.pongLength = length;
			this.pongHeight = height;
			repaint();
		}
		/**
		 * A function to setup the AI or opponent location
		 * @param leftCornerX the AI/opponent pong location
		 * @param leftCornerY the AI/opponent pong location
		 * @param length the AI/opponent pong length
		 * @param heigh7 the AI/opponent pong height
		 */
		public void paintOpponentPongNotify(int leftCornerX, int leftCornerY,int length, int height){
			this.pongOpponentLeftcornerX = leftCornerX;
			this.pongOpponentLeftcornerY = leftCornerY;
			this.pongOpponentLength = length;
			this.pongOpponentHeight = height;
			repaint();
		}
	}

