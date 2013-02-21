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
		private unsafe delegate void WyphonPartnerJoinedCallbackDelegate(UInt32 hLocalMessageBroadcastPartner, UInt32 partnerId, IntPtr partnerNameLPTSTR, IntPtr customData);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void WyphonPartnerLeftCallbackDelegate(UInt32 hLocalMessageBroadcastPartner, UInt32 partnerId, IntPtr customData);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void D3DTextureSharingStartedCallbackDelegate( UInt32 wyphonPartnerHandle, UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, IntPtr descriptionLPTSTR );

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void D3DTextureSharingStoppedCallbackDelegate( UInt32 wyphonPartnerHandle, UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, IntPtr descriptionLPTSTR );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern UInt32 CreateWyphonPartner([MarshalAs(UnmanagedType.LPTStr)]string applicationName,
														IntPtr pCallbackFuncCustomData,
														IntPtr pPartnerJoinedCallbackFunc,
														IntPtr pPartnerLeftCallbackFunc,
														IntPtr pD3DTextureSharingStartedCallbackFunc,
														IntPtr pD3DTextureSharingStoppedCallbackFunc
		                                             );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool DestroyWyphonPartner(UInt32 hWyphonPartner);
		
		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool ShareD3DTexture(UInt32 wyphonPartnerHandle, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, [MarshalAs(UnmanagedType.LPTStr)]string description);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool UnshareD3DTexture(UInt32 wyphonPartnerHandle, UInt32 sharedTextureHandle);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr GetWyphonPartnerName(UInt32 wyphonPartnerHandle, UInt32 wyphonPartnerId);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool GetD3DTextureInfo(UInt32 wyphonPartnerHandle, UInt32 sharedTextureHandle, out UInt32 wyphonPartnerId, out UInt32 width, out UInt32 height, out UInt32 format, out UInt32 usage, ref IntPtr description, int maxDescriptionLength );

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern UInt32 GetPartnerId(UInt32 wyphonPartnerHandle);
		
		#endregion Wyphon.dll imports & delegates


        private static WyphonPartnerJoinedCallbackDelegate wpjDelegate = WyphonPartnerJoinedCallback;
        private static WyphonPartnerLeftCallbackDelegate wplDelegate = WyphonPartnerLeftCallback;


        private static D3DTextureSharingStartedCallbackDelegate shareDelegate = D3DTextureSharingStartedCallback;
        private static D3DTextureSharingStoppedCallbackDelegate unshareDelegate = D3DTextureSharingStoppedCallback;


		public TestWyphon()
		{
		}
		
#region TestWyphon functions

		public static unsafe void WyphonPartnerJoinedCallback(UInt32 hLocalMessageBroadcastPartner, UInt32 partnerId, IntPtr partnerNameLPTSTR, IntPtr customData) {
            Console.WriteLine( " >>>>>>>> Partner with id " + partnerId + " JOINED. Its name is " + Marshal.PtrToStringAuto(partnerNameLPTSTR) );			
		}
		
		public static unsafe void WyphonPartnerLeftCallback(UInt32 hLocalMessageBroadcastPartner, UInt32 partnerId, IntPtr customData) {
            Console.WriteLine( " >>>>>>>> Partner with id " + partnerId + " LEFT." );
		}

        public static unsafe void D3DTextureSharingStartedCallback(UInt32 wyphonPartnerHandle, UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, IntPtr descriptionLPTSTR) {
            Console.WriteLine(" >>>>>>>> Partner with id " + sendingPartnerId + " STARTED sharing texture " + sharedTextureHandle + " " + width + "x" + height + ":" + format + " " + Marshal.PtrToStringAuto(descriptionLPTSTR) );
        }

        public static unsafe void D3DTextureSharingStoppedCallback(UInt32 wyphonPartnerHandle, UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, IntPtr descriptionLPTSTR) {
            Console.WriteLine(" >>>>>>>> Partner with id " + sendingPartnerId + " STOPPED sharing texture " + sharedTextureHandle + " " + width + "x" + height + ":" + format + " " + Marshal.PtrToStringAuto(descriptionLPTSTR) );
        }



		public static void DoTestWyphon() {
			Console.Write("What's your application name: ");
			string name = Console.ReadLine();
			Console.WriteLine("Welcome " + name + ". You share a texture's info by typing s, and unshare by typing u afterwards<enter>");


            UInt32 hWyphonPartner = CreateWyphonPartner(name, IntPtr.Zero, Marshal.GetFunctionPointerForDelegate(wpjDelegate), Marshal.GetFunctionPointerForDelegate(wplDelegate), Marshal.GetFunctionPointerForDelegate(shareDelegate), Marshal.GetFunctionPointerForDelegate(unshareDelegate) );
			
			if ( hWyphonPartner > 0 ) {
				try {
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
							
							ShareD3DTexture( hWyphonPartner, textureHandle, textureWidth, textureHeight, 20, 999, textureName );
						}
						else if ( input == 'u' ) {
							UInt32 textureHandle = 0;
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
