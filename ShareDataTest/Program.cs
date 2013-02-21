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
using System.Runtime.InteropServices;
using System.Collections.Generic;

using ShareDataTest.TestClasses;

//using Wyphon;

namespace ShareDataTest
{
	class Program
	{

		public static void Main(string[] args)
		{
			bool repeat = true;
			do {
				Console.Write(	"What do you want to do?\n" +
								"  0. EXIT the application\n" +
								"  1. WyphonDotNet\n" +
								"  2. LocalMessageBroadcastDotnet\n" +
								"  3. Wyphon\n" +
								"  4. LocalMessageBroadcast\n" +
								"  5. SharedMemory\n"
				             );
				
				bool inexistingChoice;
				do {
					ConsoleKeyInfo key = Console.ReadKey(true);
					inexistingChoice = false; //unless proven otherwise
					
					switch ( key.KeyChar ) {
						case '0':
							repeat = false;
							break;
						case '1':
							Console.WriteLine( "---- BEGIN WyphonDotNet Test" );
							new TestWyphonDotNet().DoTestWyphonDotNet();
							Console.WriteLine( "---- END WyphonDotNet Test" );
							break;
						case '2':
							Console.WriteLine( "---- BEGIN LocalMessageBroadcastDotnet Test" );
							new TestLocalMessageBroadcastDotNet().DoTestLocalMessageBroadcastDotNet();
							Console.WriteLine( "---- END LocalMessageBroadcastDotnet Test" );
							break;
						case '3':
							Console.WriteLine( "---- BEGIN Wyphon Test" );
							TestWyphon.DoTestWyphon();
							Console.WriteLine( "---- END Wyphon Test" );
							break;
						case '4':
							Console.WriteLine( "---- BEGIN LocalMessageBroadcast Test" );
							new TestLocalMessageBroadcast().DoTestLocalMessageBroadcast();
							Console.WriteLine( "---- END LocalMessageBroadcast Test" );
							break;
						case '5':
							Console.WriteLine( "---- BEGIN SharedMemory Test" );
							TestSharedMemory.DoTestSharedMemory();
							Console.WriteLine( "---- END SharedMemory Test" );
							break;
						default:
							inexistingChoice = true;
							break;
					}
				} while ( inexistingChoice );
				
			} while ( repeat );

			
			
			//TestSharedMemory();

			//new Program().TestLocalMessageBroadcast();
			
			//TestSharedData();
			
			//TestWyphon();
			
			Console.WriteLine( "Exiting application..." );
		}
		
		static string l2s(List<UInt32> list) {
			string s = "";
			
			foreach (UInt32 x in list) {
				s += (s.Length == 0 ? "" : ", ") + x;
			}
			return s;
		}



		

		
		
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