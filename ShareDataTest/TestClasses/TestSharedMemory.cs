/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 11/01/2013
 * Time: 9:40
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Runtime.InteropServices;
using System.Threading;


namespace ShareDataTest.TestClasses
{
	/// <summary>
	/// Description of TestSharedMemory.
	/// </summary>
	public class TestSharedMemory
	{
		#region SharedMemory.dll or Wyphon.dll imports
		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern UInt32 CreateSharedMemory([MarshalAs(UnmanagedType.LPTStr)]string name, UInt32 startSize, UInt32 maxSize);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool LockSharedMemory(UInt32 hSharedMemory, UInt32 timeoutInMilliseconds);

//		[DllImport("SharedMemory", CallingConvention = CallingConvention.Cdecl)]
//		public static extern IntPtr ReadSharedMemory(UInt32 hSharedMemory);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern int ReadSharedMemory( UInt32 sharedDataHandle, out IntPtr pData);

		
		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern int WriteSharedMemory( UInt32 hSharedMemory, byte[] data, UInt32 length, UInt32 offset );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern string ReadStringFromSharedMemory(UInt32 hSharedMemory);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern int WriteStringToSharedMemory(UInt32 hSharedMemory, [MarshalAs(UnmanagedType.LPTStr)]string data);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool UnlockSharedMemory(UInt32 hSharedMemory);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool DestroySharedMemory(UInt32 hSharedMemory);
		#endregion SharedMemory.dll or Wyphon.dll imports
		
		#region SharedData.dll imports
//
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern UInt32 CreateSharedDataPartner([MarshalAs(UnmanagedType.LPTStr)]string sharedDataName, [MarshalAs(UnmanagedType.LPTStr)]string applicationName);
//
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool DestroySharedDataPartner(UInt32 hWyphonPartner);
//		
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool ShareData(UInt32 sharedDataPartnerHandle, UInt32 sharedObjectHandle, [MarshalAs(UnmanagedType.LPTStr)]string sharedData, int nrOfBytes);
//
//		[DllImport("SharedData", CallingConvention = CallingConvention.Cdecl)]
//		public static extern bool UnshareData(UInt32 wyphonPartnerHandle, UInt32 sharedObjectHandle);
//
		#endregion SharedData.dll imports

		
		public TestSharedMemory()
		{
		}
		
		
#region TestSharedMemory functions
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


		public static void DoTestSharedMemory() {
			Console.Write("Which channel (0-9) do you want to use: ");
			char input = Console.ReadKey(true).KeyChar;//.ReadLine();
			while ( input != '0' && input != '1' && input != '2' && input != '3' && input != '4' && input != '5' && input != '6' && input != '7' && input != '8' && input != '9' ) {
				input = Console.ReadKey(true).KeyChar;
			}
			string channel = input.ToString();

			UInt32 hSharedMemory = CreateSharedMemory("MyFileMappingObject/" + channel, 1, 64);
			if (hSharedMemory != 0) {
				try {
		
					Console.WriteLine("Talking on channel " + channel + ". Press L or U to try to lock or unlock the semaphore (or 'q' to quit)...");
					
					bool locked = false;
					
					input = Console.ReadKey(true).KeyChar;//.ReadLine();
					while ( input != 'q' ) {
//						Console.WriteLine( locked ? "TRYING TO UNLOCK" : " TRYING TO LOCK" );

						IntPtr pData;
						int nrOfBytesRead;
						
						if ( input == 'u' ) {
							nrOfBytesRead = ReadSharedMemory(hSharedMemory, out pData);
							Console.WriteLine("\tShared buffer now contains [" + MyIntPtrToString(pData, nrOfBytesRead) + "]." );
							Console.Write( (locked ? "*** " : "") + "TRYING TO UNLOCK... " );
							if ( UnlockSharedMemory(hSharedMemory) ) {
								Console.WriteLine( "SUCCESSFULLY UNLOCKED" );						
							}
							else {
								Console.WriteLine( "FAILED TO UNLOCK" );
							}
							locked = false;
						}
						else if ( input == 'l' ) {
							Console.Write( (locked ? "*** " : "") + "TRYING TO LOCK... " );
							if ( LockSharedMemory(hSharedMemory, 5000) ) {
								locked = true;
								Console.WriteLine( "SUCCESSFULLY LOCKED" );

								nrOfBytesRead = ReadSharedMemory(hSharedMemory, out pData);																
								Console.Write("\tShared buffer now contains [" + MyIntPtrToString(pData, nrOfBytesRead) + "].\n\tReplace by: " );
//								Console.Write("    Replace by: " );
								string inputLine = Console.ReadLine();
								//bool written = WriteStringToSharedMemory( hSharedMemory, input);
								bool written = WriteSharedMemory( hSharedMemory, new System.Text.UTF8Encoding().GetBytes(inputLine), (UInt32)inputLine.Length, 0) > 0;
								//WriteSharedMemory( hSharedMemory, new byte[32], 32, (UInt32)inputLine.Length);
								Console.WriteLine( written ? "OK" : "ERROR" );
							}
							else {
								Console.WriteLine( "FAILED TO LOCK" );
							}
						}
						//read the next line
						input = Console.ReadKey(true).KeyChar;
					}
				}
				finally {
					if ( DestroySharedMemory(hSharedMemory) ) {
						Console.WriteLine( "SUCCESSFULLY destroyed shared data" );
					}
					else {
						Console.WriteLine( "FAILED to destroy shared data" );
					}
				}
			}
			else {
				Console.WriteLine( "CreateSharedMemory FAILED!!!" );				
			}
			Thread.Sleep(8000);
		}
#endregion TestSharedMemory functions

	}
}
