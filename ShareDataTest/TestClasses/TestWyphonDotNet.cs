/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 11/01/2013
 * Time: 10:50
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using Wyphon;

namespace ShareDataTest.TestClasses
{
	/// <summary>
	/// Description of TestWyphonDotNet.
	/// </summary>
	public class TestWyphonDotNet
	{
		public TestWyphonDotNet()
		{
		}
		
#region TestWyphonDotNet functions
		public unsafe void WyphonDotNetPartnerJoinedCallback(UInt32 partnerId, string partnerName) {
			Console.WriteLine("DotNet Partner joined. Its name is " + partnerName + " and its id = " + partnerId);
		}
		
		public unsafe void WyphonDotNetPartnerLeftCallback(UInt32 partnerId) {
			Console.WriteLine("DotNet Partner left. Its name is <whatever>" + " and its id = " + partnerId);			
		}

		public void DoTestWyphonDotNet() {
			bool repeat = false;
			do {
				Console.Write("What's your application name: ");
				string name = Console.ReadLine();
				Console.WriteLine("Welcome " + name + ". You share a texture's info by typing s, and unshare by typing u afterwards<enter>");
	
				WyphonPartner wp = new WyphonPartner(name);
				wp.WyphonPartnerJoinedEvent += WyphonDotNetPartnerJoinedCallback;
				wp.WyphonPartnerLeftEvent += WyphonDotNetPartnerLeftCallback;
				wp.WyphonPartnerD3DTextureSharedEvent += 
					delegate(UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, string description) {
						Console.WriteLine("DotNet new shared texture by partner " + sendingPartnerId + ". Its handle = " + sharedTextureHandle +  " and its other data = " + width + "x" + height + ":fmt" + format + ":usg"+ usage + " : " + description);
					};
				wp.WyphonPartnerD3DTextureUnsharedEvent += 
					delegate(UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, string description) {
						Console.WriteLine("DotNet STOPPED SHARING texture by partner " + sendingPartnerId + ". Its handle = " + sharedTextureHandle +  " and its other data = " + width + "x" + height + ":" + usage + " : " + description);
					};
				
				if ( wp != null ) {
					try {
						Console.WriteLine("We have ID = " + wp.PartnerId);
						
						char input = Console.ReadKey(true).KeyChar;//.ReadLine();
						while ( input != 'q' ) {
							if ( input == 's' ) {
								Console.Write("Enter description for " + name + "'s shared texture: ");
								string textureName = Console.ReadLine();
								UInt32 textureHandle = 0;
								UInt32 textureWidth = 0;
								UInt32 textureHeight = 0;
								do {
									Console.Write("Enter handle shared texture: ");
								} while ( ! UInt32.TryParse( Console.ReadLine(), out textureHandle ) );
								do {
									Console.Write("Enter width for shared texture: ");
								} while ( ! UInt32.TryParse( Console.ReadLine(), out textureWidth ) );
								do {
									Console.Write("Enter height for shared texture: ");
								} while ( ! UInt32.TryParse( Console.ReadLine(), out textureHeight ) );
								
								wp.ShareD3DTexture(textureHandle, textureWidth, textureHeight, 21, 999, textureName);
								//ShareD3DTexture( hWyphonPartner, textureHandle, textureWidth, textureHeight, 999, textureName );
							}
							else if ( input == 'u' ) {
								UInt32 textureHandle = 0;
								do {
									Console.Write("Enter handle for texture you want to UN-share: ");
								} while ( ! UInt32.TryParse( Console.ReadLine(), out textureHandle ) );
	
								wp.UnshareD3DTexture(textureHandle);
								//UnshareD3DTexture(hWyphonPartner, textureHandle);
							}
							else if ( input == 'i' ) {
								Console.WriteLine( "We will try to find the info of a specific texture by name..." );
								string appName = "";
								do {
									Console.Write( "Enter name of the application that shared the texture: " );
									appName = Console.ReadLine( );
								} while ( appName.Length == 0 );

								string textureName = "";
								do {
									Console.Write( "Enter description of the texture: " );
									textureName = Console.ReadLine( );
								} while ( textureName.Length == 0 );


								UInt32 width = 0;
								UInt32 height = 0;
								UInt32 format = 0;
								UInt32 usage = 0;

								bool retreived = wp.GetD3DTextureInfo( appName, textureName, out width, out height, out format, out usage );

								Console.WriteLine( ( retreived ? "We found this texture's info:" : "We were unable to find this texture's info" ) + " - " +
													width + "x" + height + " fmt:" + format + " usg:" + usage );

							}
							
							input = Console.ReadKey(true).KeyChar;
						}
					}
					finally {
						
						wp.Dispose();
					}
				}
				else {
					Console.WriteLine( "CreateWyphonPartner FAILED!!!" );				
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

#endregion TestWyphonDotNet functions

	}
}
