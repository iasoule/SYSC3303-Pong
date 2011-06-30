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
import javax.swing.JTextField;
import java.io.*;
import java.net.*;

public class View extends JFrame implements ActionListener, Runnable{

	private final static int X=600;
	private final static int Y=600;
	private boolean gameStart = false;
	private JMenuItem add, exit, single, twoplayer;
	Socket connected;
	private ServerSocket serverSocket;


	boolean connection = false;
	private View.sender sender;

	private PongPanel pPanel; 
	private ButtonPanel bPanel;
	private ScorePanel sPanel;
	Thread ViewThread;
	Thread SenderThread;
	boolean receivemsg =false;

	private boolean stopThread = false;

	private boolean addPong = false;

	private boolean reset = false;

	public View(){
		super("SYSC 3303 Pong");
		this.view_initalize();

		try {
			serverSocket = new ServerSocket(1027);
		} catch (IOException e) {
			e.printStackTrace(System.err);
			System.exit(1);
		}
		sender = new View.sender();
		ViewThread = new Thread(this);
		SenderThread = new Thread(sender);

		ViewThread.start();
		//SenderThread.start();

		try {
			ViewThread.join();
//			SenderThread.join();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	/**
	 * To know each reference of each class
	 * @param controller The reference from the controller class
	 * @param model The reference from the Model class
	 * It will also initalize the default view 
	 */


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

		add.addActionListener(this);
		exit.addActionListener(this);


		bPanel = new View.ButtonPanel();
		sPanel = new View.ScorePanel();
		pPanel = new View.PongPanel();

		//pPanel.addKeyListener(this);
		//bPanel.addKeyListener(controller);
		//sPanel.addKeyListener(controller);


		Container cp = getContentPane();
		cp.add( pPanel, BorderLayout.CENTER );
		cp.add( sPanel, BorderLayout.NORTH );
		cp.add( bPanel, BorderLayout.SOUTH );

		pPanel.grabFocus();


		//model.setResolution(this.getX(),this.getY(),this.getWidth(),this.getHeight());

		//System.out.println(this.getX() + "Y" + this.getY() + "W" + this.getWidth() + "H" +this.getHeight());
		//System.out.println(this.getX() + " Y " + this.getY() + " H "+ this.getWidth()+ " L " +this.getHeight());
		this.setVisible(true);
	}
	/**
	 * @return the reference of the pPanel
	 */
	public View.PongPanel getPanel(){
		return pPanel;
	}
	/**
	 * @return The reference of the score Panel
	 */
	public View.ScorePanel getScore(){
		return sPanel;
	}

	public class ScorePanel extends JPanel
	{
		private JTextField top, bottom;
		/**
		 * A setup of the score panel
		 */
		public ScorePanel()
		{
			add( new  JLabel("Top") );
			top = new JTextField(10);
			top.setEditable(false);
			//top.addKeyListener(View.this.controller);
			add( top );
			add(  new JLabel( "Bottom" ) );
			bottom = new JTextField(10);
			//bottom.addKeyListener(View.this.controller);
			bottom.setEditable(false);
			add( bottom );

		}
		/**
		 * A update method to update the score 
		 * @param playerScore the current player score
		 * @param opponentScore the current AI score or the opponent score
		 */ 
		public void scoreUpdate(int playerScore,int opponentScore){
			top.setText("Player Score: "+playerScore);
			bottom.setText("Opponent Score: "+opponentScore);
		}


	}

	public class PongPanel extends JPanel 
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

		private int pongCreation = 0;

		// private Controller controller;

		private Thread displayThread;

		private boolean ballCreation = false;
		public PongPanel()
		{
			/*Decide problem*/

			setBackground(Color.black);
			//addMouseListener(controller);



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

			g.setColor( Color.white);

			if(this.ballCreation){
				g.fillOval( this.currentBallX,this.currentBallY, this.currentBallRadius, this.currentBallRadius) ;
				//g.fillOval( 250,250, 30, 30) ;
			}
			if(this.pongCreation == 1){	
				g.fillRect(this.pongOpponentLeftcornerX, this.pongOpponentLeftcornerY, this.pongOpponentLength, this.pongOpponentHeight);
			}
			if(this.pongCreation >= 2){	
				g.fillRect(this.pongOpponentLeftcornerX, this.pongOpponentLeftcornerY, this.pongOpponentLength, this.pongOpponentHeight);
				g.fillRect(this.pongLeftcornerX, this.pongLeftcornerY, this.pongLength, this.pongHeight);
			}
		}

		/**
		 * A setup method to notify the view which allow the change of the screen
		 */



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

	public class ButtonPanel extends JPanel
	{
		private JButton start, reset;

		public ButtonPanel()
		{
			start = new JButton("Start");
			reset = new JButton("Reset");

			add(start);
			add(reset);

			start.addActionListener(View.this);
			reset.addActionListener(View.this);
			//start.addKeyListener(this);
			//reset.addKeyListener(this);
		}

	}

	@Override
	public void actionPerformed(ActionEvent event) {
		// TODO Auto-generated method stub
		String source = event.getActionCommand();
		if (source.equals("Start"))
		{
			//model.getPong().start();
			gameStart = true;
		}
		else if (source.equals("Reset"))
		{
			reset = true;
			//model.getPong().reset();
			//model.getPong().resetScore();
		}
		else if (source.equals("Add Pong")){
			//model.getPong().addPong();
			addPong= true;
		}
		else if (source.equals("Exit")){
			this.stopThread = true;
			System.exit(0);
		}

	}
	public static void main(String [ ] args)
	{	     
		View view = new View();
	}
	DataInputStream in;
	@Override	
	public void run() {
		// TODO Auto-generated method stub
		// TODO Auto-generated method stub
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
				
				System.out.printf("Read %d bytes.\n", num_bytes);

				String input_string = new String(input_buffer);
				System.out.println("DEBUG => " + input_string);
				
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
					if(pPanel.pongCreation > 1 ){
						pPanel.paintPongNotify(data[1], data[2], data[3], data[4]);
					}

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
				else if(recv_instruction.equals("exit")){
					System.exit(0);
				}
			}


		} 
		catch (IOException e) {
			System.out.println("disconnected");
			System.exit(1);
		}

	}
	public class sender implements Runnable{
		private Socket streamSocket;
		private DataInputStream in;
		private DataOutputStream out;
		@Override
		public void run() {
			// TODO Auto-generated method stub
			
			try {
				// Bind a socket to any available port on the local host machine. 
				//streamSocket = new Socket("134.117.58.130", 1024);
				streamSocket = new Socket(JOptionPane.showInputDialog("IP Address"), 1027);
			} catch (UnknownHostException e1) {
				JOptionPane.showMessageDialog(null, "Unknown host name");
				System.exit(1);
			} catch (IOException e2) {
				JOptionPane.showMessageDialog(null, "Invaild IP address");
				System.exit(1);
			}

			try {
				out = new DataOutputStream(new BufferedOutputStream(streamSocket.getOutputStream()));
				//in = new DataInputStream(new BufferedInputStream( streamSocket.getInputStream()));
			} catch (IOException e2) {
				System.err.println("Couldn't get I/O connection");
				System.exit(1);
			}    	
			while(!stopThread){
				if(View.this.gameStart){
					try {
						out.writeBytes("start;");
						View.this.gameStart = false;
						out.flush();

					} catch (IOException e) {
						System.err.println("Couldn't get I/O for the connection");
						System.exit(1);
					}
				}
				else if(View.this.addPong){
					try {
						out.writeBytes("addPong;");
						View.this.addPong = false;
						out.flush();

					} catch (IOException e) {
						System.err.println("Couldn't get I/O for the connection");
						System.exit(1);
					}
				}
				else if(View.this.reset){
					try {
						out.writeBytes("reset;");
						View.this.reset = false;
						out.flush();
					} catch (IOException e) {
						System.err.println("Couldn't get I/O for the connection");
						System.exit(1);
					}
				}
				else if(receivemsg){
					try {
						out.writeBytes("receive;");
						receivemsg = false;
						out.flush();
					} catch (IOException e) {
						System.err.println("Couldn't get I/O for the connection");
						System.exit(1);
					}
				}
			}
		}
	}
}



