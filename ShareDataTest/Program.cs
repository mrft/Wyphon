/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 22/10/2012
 * Time: 21:04
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Threading;
//using SharedMemory;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace ShareDataTest
{
	class Program
	{

		#region Wyphon.dll imports & delegates

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public unsafe delegate bool D3DTextureSharingStartedCallbackDelegate( uint wyphonPartnerHandle, uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint usage, IntPtr descriptionLPTSTR );

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public unsafe delegate bool D3DTextureSharingStoppedCallbackDelegate( uint wyphonPartnerHandle, uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint usage, IntPtr descriptionLPTSTR );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern uint CreateWyphonPartner([MarshalAs(UnmanagedType.LPTStr)]string applicationName,
														IntPtr pD3DTextureSharingStartedCallbackFunc,
														IntPtr pD3DTextureSharingStoppedCallbackFunc
		                                             );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool DestroyWyphonPartner(uint hWyphonPartner);
		
		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool ShareD3DTexture(uint wyphonPartnerHandle, uint sharedTextureHandle, uint width, uint height, uint usage, [MarshalAs(UnmanagedType.LPTStr)]string description);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool UnshareD3DTexture(uint wyphonPartnerHandle, uint sharedTextureHandle);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr GetWyphonPartnerName(uint wyphonPartnerHandle, uint wyphonPartnerId);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		bool GetD3DTextureInfo(uint wyphonPartnerHandle, uint sharedTextureHandle, uint out wyphonPartnerId, uint out width, uint out height, uint out usage, IntPtr description, int maxDescriptionLength );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		bool GetD3DTextureInfo(uint wyphonPartnerHandle, uint sharedTextureHandle, uint out wyphonPartnerId, uint out width, uint out height, uint out usage, IntPtr description, int maxDescriptionLength );
		
		
		
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
		
		
		#endregion Wyphon.dll imports & delegates

		#region LocalMessageBroadcast.dll imports & delegates
//		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//		public unsafe delegate void PartnerJoinedCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId);
//
//		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//		public unsafe delegate void PartnerLeftCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId);
//
//		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//		public unsafe delegate void BroadcastMessageReceivedCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr msgData, uint msgLength);
//
//		[DllImport("LocalMessageBroadcast", CallingConvention = CallingConvention.Cdecl)]
//		public static extern uint CreateLocalMessageBroadcastPartner(
//				[MarshalAs(UnmanagedType.LPTStr)]string localMessageBroadcastName, 
//				[MarshalAs(UnmanagedType.LPTStr)]string applicationName, 
//				IntPtr callbackFuncCustomData,
//				IntPtr pPartnerJoinedCallbackFunc,
//				IntPtr pPartnerLeftCallbackFunc,
//				IntPtr pMsgReceivedCallbackFunc
//			);
//
//		[DllImport("LocalMessageBroadcast", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool DestroyLocalMessageBroadcastPartner(uint hLocalMessageBroadcastPartner);
//
//		[DllImport("LocalMessageBroadcast", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool BroadcastMessage(uint hLocalMessageBroadcastPartner, byte[] data, uint length);
//
//		[DllImport("LocalMessageBroadcast", CallingConvention = CallingConvention.Cdecl)]
//		public static extern IntPtr GetBroadcastPartnerName(uint hLocalMessageBroadcastPartner, uint partnerId);
//
////		[DllImport("LocalMessageBroadcast", CallingConvention = CallingConvention.Cdecl)]
////		public static extern void GetBroadcastPartnerName(uint hLocalMessageBroadcastPartner, uint partnerId, [MarshalAs(UnmanagedType.LPTStr)] System.Text.StringBuilder name);
//		
		#endregion LocalMessageBroadcast.dll imports & delegates


		#region SharedData.dll imports
//
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern uint CreateSharedDataPartner([MarshalAs(UnmanagedType.LPTStr)]string sharedDataName, [MarshalAs(UnmanagedType.LPTStr)]string applicationName);
//
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool DestroySharedDataPartner(uint hWyphonPartner);
//		
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool ShareData(uint sharedDataPartnerHandle, uint sharedObjectHandle, [MarshalAs(UnmanagedType.LPTStr)]string sharedData, int nrOfBytes);
//
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool UnshareData(uint wyphonPartnerHandle, uint sharedObjectHandle);
//
		#endregion SharedData.dll imports

		#region SharedMemory.dll imports
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern uint CreateSharedMemory([MarshalAs(UnmanagedType.LPTStr)]string name, uint startSize, uint maxSize);
//
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool LockSharedMemory(uint hSharedMemory, uint timeoutInMilliseconds);
//
////		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
////		public static extern IntPtr ReadSharedMemory(uint hSharedMemory);
//
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern int ReadSharedMemory( uint sharedDataHandle, out IntPtr pData);
//
//		
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern int WriteSharedMemory( uint hSharedMemory, byte[] data, uint length, uint offset );
//
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern string ReadStringFromSharedMemory(uint hSharedMemory);
//
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern int WriteStringToSharedMemory(uint hSharedMemory, [MarshalAs(UnmanagedType.LPTStr)]string data);
//
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool UnlockSharedMemory(uint hSharedMemory);
//
//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool DestroySharedMemory(uint hSharedMemory);
		#endregion SharedMemory.dll imports

		
		public static void Main(string[] args)
		{
			//Console.WriteLine("Hello World!");
			
			// TODO: Implement Functionality Here
//			Marshal.PtrToStringAnsi( CSharedMemory.GetSomeString() );
//			Console.WriteLine( CSharedMemory.GetSomeString() );
//			SharedMemory.GetAnotherString();

			
			//TestSharedMemory();

			//new Program().TestLocalMessageBroadcast();
			
			//TestSharedData();
			
			TestWyphon();
			
			unsafe {
//				Console.WriteLine( getSomeCharArray() );
//				Console.WriteLine( GetAnotherString() );
				
//				ISharedMemory sd = GetSharedMemory();
//				Console.WriteLine( sd.TestMethod() );
			}
			
		
			
//			Console.Write("Press any key to continue . . . ");
//			Console.ReadKey(true);
		}
		
		static string l2s(List<uint> list) {
			string s = "";
			
			foreach (uint x in list) {
				s += (s.Length == 0 ? "" : ", ") + x;
			}
			return s;
		}

//		void BroadcastMessagePartnerJoinedCallback(uint hLocalMessageBroadcastPartner, uint partnerId) {
////			System.Text.StringBuilder name = new System.Text.StringBuilder();
////			GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId, name);
//
//			//String name = Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) );
//			Console.Out.WriteLine( Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) ) + " joined the conversation..."	);
//		}
//
//		void BroadcastMessagePartnerLeftCallback(uint hLocalMessageBroadcastPartner, uint partnerId) {
//			//System.Text.StringBuilder name = new System.Text.StringBuilder();
//			//GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId, name);
//			
//			Console.Out.WriteLine( Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) ) + " left the conversation..."	);
//		}
//
//		void BroadcastMessageReceivedCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr msgData, uint msgLength) {			
//			//System.Text.StringBuilder name = new System.Text.StringBuilder(512);
//			//GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId, name);
//
//			Console.Out.WriteLine( Marshal.PtrToStringAuto( GetBroadcastPartnerName(hLocalMessageBroadcastPartner, partnerId) ) + " says: " + MyIntPtrToString(msgData, (int)msgLength) );
//		}
//
//		void TestLocalMessageBroadcast() {
//			Console.Write("What's your name: ");
//			string name = Console.ReadLine();
//			Console.WriteLine("Welcome " + name + "! You can talk to the other by typing s, then your message and pressing <enter>");
//			
//			PartnerJoinedCallbackDelegate joinedDelegate = BroadcastMessagePartnerJoinedCallback;
//			PartnerLeftCallbackDelegate leftDelegate = BroadcastMessagePartnerLeftCallback;
//			BroadcastMessageReceivedCallbackDelegate receivedDelegate = BroadcastMessageReceivedCallback;
//			uint hLocalMessageBroadcastPartner = CreateLocalMessageBroadcastPartner(
//					"BroadcastTest", 
//					name, 
//					IntPtr.Zero,
//					Marshal.GetFunctionPointerForDelegate(joinedDelegate),
//					Marshal.GetFunctionPointerForDelegate(leftDelegate),
//					Marshal.GetFunctionPointerForDelegate(receivedDelegate)
//				);
//						
//			if ( hLocalMessageBroadcastPartner > 0 ) {
//				try {
//					char input = Console.ReadKey(true).KeyChar;//.ReadLine();
//					while ( input != 'q' ) {						
//						if ( input == 's' ) {
//							Console.Write( name + " says: ");
//							string s = Console.ReadLine();
//							byte[] stringBytes = new System.Text.UTF8Encoding().GetBytes(s);
//							BroadcastMessage( hLocalMessageBroadcastPartner, stringBytes, (uint)(stringBytes.GetLength(0)) );
//						}
//						
//						input = Console.ReadKey(true).KeyChar;
//					}
//				}
//				finally {
//					if ( DestroyLocalMessageBroadcastPartner(hLocalMessageBroadcastPartner) ) {
//						Console.WriteLine( "SUCCESSFULLY destroyed LocalMessageBroadcast Partner" );
//					}
//					else {
//						Console.WriteLine( "FAILED to destroy LocalMessageBroadcast Partner" );
//					}
//				}
//			}
//			else {
//				Console.WriteLine( "CreateLocalMessageBroadcastPartner FAILED!!!" );				
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
//		
//		static void TestSharedData() {
//			uint hSharedDataPartner = CreateSharedDataPartner("ShareDataTest", "SharedDataTest_" + (new Random()).Next(999999) );
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
		
		
		static void TestWyphon() {
			Console.Write("What's your application name: ");
			string name = Console.ReadLine();
			Console.WriteLine("Welcome " + name + ". You share a texture's info by typing s, and unshare by typing u afterwards<enter>");

			uint hWyphonPartner = CreateWyphonPartner( name, IntPtr.Zero, IntPtr.Zero );
			
			if ( hWyphonPartner > 0 ) {
				try {
					char input = Console.ReadKey(true).KeyChar;//.ReadLine();
					while ( input != 'q' ) {
						if ( input == 's' ) {
							Console.Write("Enter description for " + name + "'s shared texture: ");
							string textureName = Console.ReadLine();
							uint textureHandle = 0;
							uint textureWidth = 0;
							uint textureHeight = 0;
							do {
								Console.Write("Enter handle shared texture: ");
							} while ( ! UInt32.TryParse( Console.ReadLine(), out textureHandle ) );
							do {
								Console.Write("Enter width for shared texture: ");
							} while ( ! UInt32.TryParse( Console.ReadLine(), out textureWidth ) );
							do {
								Console.Write("Enter height for shared texture: ");
							} while ( ! UInt32.TryParse( Console.ReadLine(), out textureHeight ) );
							
							ShareD3DTexture( hWyphonPartner, textureHandle, textureWidth, textureHeight, 999, textureName );
						}
						else if ( input == 'u' ) {
							uint textureHandle = 0;
							do {
								Console.Write("Enter handle for texture you want to UN-share: ");
							} while ( ! UInt32.TryParse( Console.ReadLine(), out textureHandle ) );

							UnshareD3DTexture(hWyphonPartner, textureHandle);
						}
						
						input = Console.ReadKey(true).KeyChar;
					}
				}
				finally {
					if ( DestroyWyphonPartner(hWyphonPartner) ) {
						Console.WriteLine( "SUCCESSFULLY destroyed Wyphon Parter" );
					}
					else {
						Console.WriteLine( "FAILED to destroy Wyphon Parter" );
					}
				}
			}
			else {
				Console.WriteLine( "CreateWyphonPartner FAILED!!!" );				
			}

			Console.WriteLine( "\nPress 'q' again to really quit..." );
			char input2 = Console.ReadKey(true).KeyChar;//.ReadLine();
			while ( input2 != 'q' ) {
			}
			//Thread.Sleep(8000);
						
		}

//		static void TestSharedMemory() {
//			uint hSharedMemory = CreateSharedMemory("MyFileMappingObject", 1, 64);
//			if (hSharedMemory != 0) {
//				try {
//		
//					Console.WriteLine("Press L or U to try to lock or unlock the semaphore (or 'q' to quit)...");
//					
//					bool locked = false;
//					
//					char input = Console.ReadKey(true).KeyChar;//.ReadLine();
//					while ( input != 'q' ) {
////						Console.WriteLine( locked ? "TRYING TO UNLOCK" : " TRYING TO LOCK" );
//
//						IntPtr pData;
//						int nrOfBytesRead;
//						
//						if ( input == 'u' ) {
//							nrOfBytesRead = ReadSharedMemory(hSharedMemory, out pData);
//							Console.WriteLine("\tShared buffer now contains [" + MyIntPtrToString(pData, nrOfBytesRead) + "]." );
//							Console.Write( (locked ? "*** " : "") + "TRYING TO UNLOCK... " );
//							if ( UnlockSharedMemory(hSharedMemory) ) {
//								Console.WriteLine( "SUCCESSFULLY UNLOCKED" );						
//							}
//							else {
//								Console.WriteLine( "FAILED TO UNLOCK" );
//							}
//							locked = false;
//						}
//						else if ( input == 'l' ) {
//							Console.Write( (locked ? "*** " : "") + "TRYING TO LOCK... " );
//							if ( LockSharedMemory(hSharedMemory, 5000) ) {
//								locked = true;
//								Console.WriteLine( "SUCCESSFULLY LOCKED" );
//
//								nrOfBytesRead = ReadSharedMemory(hSharedMemory, out pData);																
//								Console.Write("\tShared buffer now contains [" + MyIntPtrToString(pData, nrOfBytesRead) + "].\n\tReplace by: " );
////								Console.Write("    Replace by: " );
//								string inputLine = Console.ReadLine();
//								//bool written = WriteStringToSharedMemory( hSharedMemory, input);
//								bool written = WriteSharedMemory( hSharedMemory, new System.Text.UTF8Encoding().GetBytes(inputLine), (uint)inputLine.Length, 0) > 0;
//								//WriteSharedMemory( hSharedMemory, new byte[32], 32, (uint)inputLine.Length);
//								Console.WriteLine( written ? "OK" : "ERROR" );
//							}
//							else {
//								Console.WriteLine( "FAILED TO LOCK" );
//							}
//						}
//						//read the next line
//						input = Console.ReadKey(true).KeyChar;
//					}
//				}
//				finally {
//					if ( DestroySharedMemory(hSharedMemory) ) {
//						Console.WriteLine( "SUCCESSFULLY destroyed shared data" );
//					}
//					else {
//						Console.WriteLine( "FAILED to destroy shared data" );
//					}
//				}
//			}
//			else {
//				Console.WriteLine( "CreateSharedMemory FAILED!!!" );				
//			}
//			Thread.Sleep(8000);
//		}
		
		
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
	}
}