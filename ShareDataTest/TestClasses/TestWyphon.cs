/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 11/01/2013
 * Time: 9:46
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Threading;
using System.Runtime.InteropServices;


namespace ShareDataTest.TestClasses
{
	/// <summary>
	/// Description of TestWyphon.
	/// </summary>
	public class TestWyphon
	{

		#region Wyphon.dll imports & delegates

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void WyphonPartnerJoinedCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr partnerNameLPTSTR, IntPtr customData);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void WyphonPartnerLeftCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr customData);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void D3DTextureSharingStartedCallbackDelegate( uint wyphonPartnerHandle, uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint format, uint usage, IntPtr descriptionLPTSTR );

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void D3DTextureSharingStoppedCallbackDelegate( uint wyphonPartnerHandle, uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint format, uint usage, IntPtr descriptionLPTSTR );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern uint CreateWyphonPartner([MarshalAs(UnmanagedType.LPTStr)]string applicationName,
														IntPtr pCallbackFuncCustomData,
														IntPtr pPartnerJoinedCallbackFunc,
														IntPtr pPartnerLeftCallbackFunc,
														IntPtr pD3DTextureSharingStartedCallbackFunc,
														IntPtr pD3DTextureSharingStoppedCallbackFunc
		                                             );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool DestroyWyphonPartner(uint hWyphonPartner);
		
		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool ShareD3DTexture(uint wyphonPartnerHandle, uint sharedTextureHandle, uint width, uint height, uint format, uint usage, [MarshalAs(UnmanagedType.LPTStr)]string description);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool UnshareD3DTexture(uint wyphonPartnerHandle, uint sharedTextureHandle);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr GetWyphonPartnerName(uint wyphonPartnerHandle, uint wyphonPartnerId);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool GetD3DTextureInfo(uint wyphonPartnerHandle, uint sharedTextureHandle, out uint wyphonPartnerId, out uint width, out uint height, out uint format, out uint usage, ref IntPtr description, int maxDescriptionLength );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern uint GetPartnerId(uint wyphonPartnerHandle);
		
		#endregion Wyphon.dll imports & delegates


		public TestWyphon()
		{
		}
		
#region TestWyphon functions

		public unsafe void WyphonPartnerJoinedCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr partnerNameLPTSTR, IntPtr customData) {
			
		}
		
		public unsafe void WyphonPartnerLeftCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr customData) {
			
		}

		public static void DoTestWyphon() {
			Console.Write("What's your application name: ");
			string name = Console.ReadLine();
			Console.WriteLine("Welcome " + name + ". You share a texture's info by typing s, and unshare by typing u afterwards<enter>");

			uint hWyphonPartner = CreateWyphonPartner( name, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero );
			
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
							
							ShareD3DTexture( hWyphonPartner, textureHandle, textureWidth, textureHeight, 20, 999, textureName );
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

			Console.WriteLine( "Done..\n" );

//			Console.WriteLine( "\nPress 'q' again to really quit..." );
//			char input2 = Console.ReadKey(true).KeyChar;//.ReadLine();
//			while ( input2 != 'q' ) {
//			}
			//Thread.Sleep(8000);
						
		}
#endregion TestWyphon functions

	}
}
