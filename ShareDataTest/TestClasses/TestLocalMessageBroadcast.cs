/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 11/01/2013
 * Time: 9:44
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Collections.Generic;


namespace ShareDataTest.TestClasses
{
	/// <summary>
	/// Description of TestLocalMessageBroadcast.
	/// </summary>
	public class TestLocalMessageBroadcast
	{
		#region LocalMessageBroadcast.dll imports & delegates
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public unsafe delegate void PartnerJoinedCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public unsafe delegate void PartnerLeftCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public unsafe delegate void BroadcastMessageReceivedCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr msgData, uint msgLength);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern uint CreateLocalMessageBroadcastPartner(
				[MarshalAs(UnmanagedType.LPTStr)]string localMessageBroadcastName, 
				[MarshalAs(UnmanagedType.LPTStr)]string applicationName, 
				IntPtr callbackFuncCustomData,
				IntPtr pPartnerJoinedCallbackFunc,
				IntPtr pPartnerLeftCallbackFunc,
				IntPtr pMsgReceivedCallbackFunc
			);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool DestroyLocalMessageBroadcastPartner(uint hLocalMessageBroadcastPartner);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool BroadcastMessage(uint hLocalMessageBroadcastPartner, byte[] data, uint length);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr GetBroadcastPartnerName(uint hLocalMessageBroadcastPartner, uint partnerId);

//		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
//		public static extern void GetBroadcastPartnerName(uint hLocalMessageBroadcastPartner, uint partnerId, [MarshalAs(UnmanagedType.LPTStr)] System.Text.StringBuilder name);
		
		#endregion LocalMessageBroadcast.dll imports & delegates

		
		#region fields
		
		private PartnerJoinedCallbackDelegate joinedDelegate;
		private PartnerLeftCallbackDelegate leftDelegate;
		private BroadcastMessageReceivedCallbackDelegate receivedDelegate;
		
		#endregion fields
		
		public TestLocalMessageBroadcast()
		{
		}
		
		
#region TestLocalMessageBroadcast functions

		/// <summary>
		/// Simple method that copies the given number of bytes to managed memory and returns a string
		/// </summary>
		/// <param name="intPtr0"></param>
		/// <param name="num1"></param>
		static string MyIntPtrToString(IntPtr p, int nrOfBytes) {
			byte[] bytes = new byte[nrOfBytes + 1];
			bytes[nrOfBytes] = 0;
			if (nrOfBytes > 0) {
				Marshal.Copy(p, bytes, 0, nrOfBytes);
			}
			return new System.Text.UTF8Encoding().GetString(bytes);
		}



		private void BroadcastMessagePartnerJoinedCallback(uint hLocalMessageBroadcastPartner, uint partnerId) {
//			System.Text.StringBuilder name = new System.Text.StringBuilder();
//			GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId, name);

			//String name = Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) );
			Console.Out.WriteLine( Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) ) + " joined the conversation..."	);
		}

		private void BroadcastMessagePartnerLeftCallback(uint hLocalMessageBroadcastPartner, uint partnerId) {
			//System.Text.StringBuilder name = new System.Text.StringBuilder();
			//GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId, name);
			
			Console.Out.WriteLine( Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) ) + " left the conversation..."	);
		}

		private void BroadcastMessageReceivedCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr msgData, uint msgLength) {			
			//System.Text.StringBuilder name = new System.Text.StringBuilder(512);
			//GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId, name);

			Console.Out.WriteLine( Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) ) + " says: " + MyIntPtrToString(msgData, (int)msgLength) );
		}

		public void DoTestLocalMessageBroadcast() {
			Console.Write("What's your name: ");
			string name = Console.ReadLine();
			Console.WriteLine("Welcome " + name + "! You can talk to the other by typing s, then your message and pressing <enter>");
			
			joinedDelegate = BroadcastMessagePartnerJoinedCallback;
			leftDelegate = BroadcastMessagePartnerLeftCallback;
			receivedDelegate = BroadcastMessageReceivedCallback;
			uint hLocalMessageBroadcastPartner = CreateLocalMessageBroadcastPartner(
					"BroadcastTest", 
					name, 
					IntPtr.Zero,
					Marshal.GetFunctionPointerForDelegate(joinedDelegate),
					Marshal.GetFunctionPointerForDelegate(leftDelegate),
					Marshal.GetFunctionPointerForDelegate(receivedDelegate)
				);
						
			if ( hLocalMessageBroadcastPartner > 0 ) {
				try {
					char input = Console.ReadKey(true).KeyChar;//.ReadLine();
					while ( input != 'q' ) {						
						if ( input == 's' ) {
							Console.Write( name + " says: ");
							string s = Console.ReadLine();
							byte[] stringBytes = new System.Text.UTF8Encoding().GetBytes(s);
							BroadcastMessage( hLocalMessageBroadcastPartner, stringBytes, (uint)(stringBytes.GetLength(0)) );
						}
						
						input = Console.ReadKey(true).KeyChar;
					}
				}
				finally {
					if ( DestroyLocalMessageBroadcastPartner(hLocalMessageBroadcastPartner) ) {
						Console.WriteLine( "SUCCESSFULLY destroyed LocalMessageBroadcast Partner" );
					}
					else {
						Console.WriteLine( "FAILED to destroy LocalMessageBroadcast Partner" );
					}
				}
			}
			else {
				Console.WriteLine( "CreateLocalMessageBroadcastPartner FAILED!!!" );				
			}

			Console.WriteLine( "\nPress 'q' again to really quit..." );
			char input2 = Console.ReadKey(true).KeyChar;//.ReadLine();
			while ( input2 != 'q' ) {
			}
			//Thread.Sleep(8000);
			
			
		}
		
//		static void TestSharedData() {
//			uint hSharedDataPartner = CreateLocalMessageBroadcastPartner("ShareDataTest", "SharedDataTest_" + (new Random()).Next(999999), null, BroadcastMessagePartnerJoinedCallback, BroadcastMessagePartnerLeftCallback, BroadcastMessageReceivedCallback );
//			
//			List<uint> l = new List<uint>();
//			
//			if ( hSharedDataPartner > 0 ) {
//				try {
//					char input = Console.ReadKey(true).KeyChar;//.ReadLine();
//					while ( input != 'q' ) {
//						if ( input == 's' ) {
//							uint id;
//							do {
//								Console.Write("Enter INTEGER as id (not " + l2s(l) + "): ");
//							} while ( ! UInt32.TryParse(Console.ReadLine(), out id) );
//							Console.Write("Enter some data: ");
//							string s = Console.ReadLine();
//							ShareData( hSharedDataPartner, id, s, (s.Length + 1) * sizeof(char) );
//							l.Add(id);
//						}
//						else if ( input == 'u' ) {
//							uint id;
//							do {
//								Console.Write("Enter id from (" + l2s(l) + ") that you want to stop sharing: ");
//							} while ( ! UInt32.TryParse(Console.ReadLine(), out id) );
//							
//							UnshareData( hSharedDataPartner, id );
//							l.Remove(id);
//						}
//						
//						input = Console.ReadKey(true).KeyChar;
//					}
//				}
//				finally {
//					if ( DestroySharedDataPartner(hSharedDataPartner) ) {
//						Console.WriteLine( "SUCCESSFULLY destroyed SharedData Parter" );
//					}
//					else {
//						Console.WriteLine( "FAILED to destroy SharedData Parter" );
//					}
//				}
//			}
//			else {
//				Console.WriteLine( "CreateSharedDataPartner FAILED!!!" );				
//			}
//
//			Console.WriteLine( "\nPress 'q' again to really quit..." );
//			char input2 = Console.ReadKey(true).KeyChar;//.ReadLine();
//			while ( input2 != 'q' ) {
//			}
//			//Thread.Sleep(8000);
//			
//			
//		}
#endregion TestLocalMessageBroadcast functions

	}
}
