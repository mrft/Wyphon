/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 11/01/2013
 * Time: 12:14
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Runtime.InteropServices;
using LocalMessageBroadcast;

namespace ShareDataTest.TestClasses
{
	/// <summary>
	/// Description of TestLocalMessageBroadcastDotNet.
	/// </summary>
	public class TestLocalMessageBroadcastDotNet
	{
		LocalMessageBroadcastPartner localMessageBroadcastPartner;
		
		public TestLocalMessageBroadcastDotNet()
		{
		}
		
#region TestLocalMessageBroadcastDotNet functions
		public void PartnerJoinedHandler(UInt32 partnerId, string partnerName) {
			Console.Out.WriteLine( partnerName + " joined the conversation..." );
		}
		
		public void PartnerLeftHandler(UInt32 partnerId) {
			Console.Out.WriteLine( localMessageBroadcastPartner.GetPartnerName(partnerId) + " left the conversation..." );
		}

		public void DoTestLocalMessageBroadcastDotNet() {
			bool repeat = false;
			do {
				Console.Write("What's your name: ");
				string name = Console.ReadLine();
				Console.WriteLine("Welcome " + name + ". You can say something by typing 's', type 'S' for a message to only 1 partner, q to quit.");
	
				localMessageBroadcastPartner = new LocalMessageBroadcastPartner(name, "TESTCHANNEL");
				localMessageBroadcastPartner.OnPartnerJoined += PartnerJoinedHandler;
				localMessageBroadcastPartner.OnPartnerLeft += PartnerLeftHandler;
				localMessageBroadcastPartner.OnMessage += delegate(UInt32 sendingPartnerId, IntPtr msgData, UInt32 msgLength) {
					Console.Out.WriteLine( localMessageBroadcastPartner.GetPartnerName(sendingPartnerId) + " says: "	+ Marshal.PtrToStringUni(msgData, (int)msgLength / 2) );
				};

				
				if ( localMessageBroadcastPartner != null ) {
					try {
						Console.WriteLine("We have ID = " + localMessageBroadcastPartner.PartnerId.ToString());
						
						char input = Console.ReadKey(true).KeyChar;//.ReadLine();
						while ( input != 'q' ) {
							if ( input == 's' ) {
								Console.Write("" + name + " says: ");
								string msg = Console.ReadLine();
								
								byte[] stringBytes = new System.Text.UnicodeEncoding().GetBytes(msg);
								localMessageBroadcastPartner.BroadcastMessage(stringBytes);
							}
							else if ( input == 'S' ) {
								UInt32 partnerId;
								Console.Write("To which partner ID do you want to speak: ");
								while ( ! UInt32.TryParse( Console.ReadLine(), out partnerId) ) {
									Console.Write("Error, you need to give an integer: ");
								}
								
								Console.Write("" + name + " says: ");
								string msg = Console.ReadLine();
								
								byte[] stringBytes = new System.Text.UnicodeEncoding().GetBytes(msg);
								localMessageBroadcastPartner.SendMessageToSinglePartner(partnerId, stringBytes);								
							}
							
							input = Console.ReadKey(true).KeyChar;
						}
					}
					finally {
						localMessageBroadcastPartner.Dispose();
					}
				}
				else {
					Console.WriteLine( "new LocalMessageBroadcastPartner(...) FAILED!!!" );	
				}
	
				Console.WriteLine( "\nPress 'q' again to really quit or 'r' to restart..." );
				repeat = false;
				char input2 = Console.ReadKey(true).KeyChar;//.ReadLine();
				while ( input2 != 'q' && input2 != 'r' ) {
					input2 = Console.ReadKey(true).KeyChar;
				}
				if (input2 == 'r') {
					repeat = true;
					Console.WriteLine();
				}
				
			} while (repeat);
			//Thread.Sleep(8000);
						
		}

#endregion TestLocalMessageBroadcastDotNet functions

	}
}
