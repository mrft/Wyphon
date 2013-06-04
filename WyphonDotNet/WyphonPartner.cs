/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 5/12/2012
 * Time: 8:32
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Wyphon
{
		
	/// <summary>
	/// Wyphon allows to share texture info (and in future other graphics elements like vertexbuffers).
	/// 
	/// This relies on the fact that Direct3D 9ex or higher, allows to share textures 
	/// between devices if the textures are declared in a specific way.
	/// 
	/// The responsibility to create these sharable textures lies with the individual applications.
	/// 
	/// Wyphon is merely a way to easily inform all of the other 'Wyphon Partners' of the textures
	/// one application is willing to share.
	/// </summary>
	public class WyphonPartner : IDisposable
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

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern UInt32 GetPartnerIdByName(UInt32 wyphonPartnerHandle, [MarshalAs(UnmanagedType.LPTStr)]string partnerName);
		
		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern UInt32 GetShareHandleByDescription( UInt32 wyphonPartnerHandle, UInt32 wyphonPartnerId, [MarshalAs( UnmanagedType.LPTStr )]string textureName );

		#endregion Wyphon.dll imports & delegates
		
		#region fields
		UInt32 wyphonPartnerHandle = 0;
		
		//by storing them as class variables, we make sure the garbage collector doesn't remove these too early
		private WyphonPartnerJoinedCallbackDelegate wyphonPartnerJoinedCallbackDelegate;
		private WyphonPartnerLeftCallbackDelegate wyphonPartnerLeftCallbackDelegate;
		private D3DTextureSharingStartedCallbackDelegate d3dTextureSharingStartedCallbackDelegate;
		private D3DTextureSharingStoppedCallbackDelegate d3dTextureSharingStoppedCallbackDelegate;
		#endregion fields
		
		// What the event handlers should look like
		public delegate void WyphonPartnerJoinedHandler(UInt32 partnerId, string partnerName);
		public delegate void WyphonPartnerLeftHandler(UInt32 partnerId);

		public delegate void WyphonPartnerD3DTextureSharedHandler(UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, string description);
		public delegate void WyphonPartnerD3DTextureUnsharedHandler(UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, string description);
		
		// Public events that one can subscribe to
		public event WyphonPartnerJoinedHandler WyphonPartnerJoinedEvent;
		public event WyphonPartnerLeftHandler WyphonPartnerLeftEvent;

		public event WyphonPartnerD3DTextureSharedHandler WyphonPartnerD3DTextureSharedEvent;
		public event WyphonPartnerD3DTextureUnsharedHandler WyphonPartnerD3DTextureUnsharedEvent;

		//Example if you want to subscribe to an event fired by Wyphon
		//wyphon.Toggle += new Wyphon.WyphonPartherJoinedHandler(DoubleBufferChanged);
		
		/// <summary>
		/// 
		/// </summary>
		/// <param name="name">the name of our application that is communicated to the other partners</param>
		public WyphonPartner(string name) {
			wyphonPartnerJoinedCallbackDelegate = WyphonPartnerJoinedCallback;
			wyphonPartnerLeftCallbackDelegate = WyphonPartnerLeftCallback;
			d3dTextureSharingStartedCallbackDelegate = D3DTextureSharingStartedCallback;
			d3dTextureSharingStoppedCallbackDelegate = D3DTextureSharingStoppedCallback;
			
			wyphonPartnerHandle = CreateWyphonPartner(name, IntPtr.Zero, 
			                                          Marshal.GetFunctionPointerForDelegate(wyphonPartnerJoinedCallbackDelegate), 
			                                          Marshal.GetFunctionPointerForDelegate(wyphonPartnerLeftCallbackDelegate), 
			                                          Marshal.GetFunctionPointerForDelegate(d3dTextureSharingStartedCallbackDelegate), 
			                                          Marshal.GetFunctionPointerForDelegate(d3dTextureSharingStoppedCallbackDelegate)
			                                         );
			
		}
		
		
		public void WyphonPartnerJoinedCallback(UInt32 hLocalMessageBroadcastPartner, UInt32 partnerId, IntPtr partnerNameLPTSTR, IntPtr customData) {
			//Only if there are any subscribers
			if (WyphonPartnerJoinedEvent != null) {
				try {
					WyphonPartnerJoinedEvent.Invoke(partnerId,  Marshal.PtrToStringAuto(partnerNameLPTSTR));
				} catch (Exception e) {
					//catch all in order not to fuck up Wyphon
					//the implementor should make sure this desn't throw exceptions in the first place					
				}

			}
		}

		public void WyphonPartnerLeftCallback(UInt32 hLocalMessageBroadcastPartner, UInt32 partnerId, IntPtr customData) {
			//Only if there are any subscribers
			if (WyphonPartnerLeftEvent != null) {
				try {
					WyphonPartnerLeftEvent.Invoke(partnerId);
				} catch (Exception e) {
					//catch all in order not to fuck up Wyphon
					//the implementor should make sure this desn't throw exceptions in the first place					
				}
			}
		}
		
		public void D3DTextureSharingStartedCallback( UInt32 wyphonPartnerHandle, UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, IntPtr descriptionLPTSTR ) {
			//Only if there are any subscribers
			if (WyphonPartnerD3DTextureSharedEvent != null) {
				try {
					WyphonPartnerD3DTextureSharedEvent.Invoke(sendingPartnerId, sharedTextureHandle, width, height, format, usage, Marshal.PtrToStringAuto(descriptionLPTSTR));
				} catch (Exception e) {
					//catch all in order not to fuck up Wyphon
					//the implementor should make sure this desn't throw exceptions in the first place					
				}
			}
		}

		public void D3DTextureSharingStoppedCallback( UInt32 wyphonPartnerHandle, UInt32 sendingPartnerId, UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, IntPtr descriptionLPTSTR ) {
			//Only if there are any subscribers
			if (WyphonPartnerD3DTextureUnsharedEvent != null) {
				try {
					WyphonPartnerD3DTextureUnsharedEvent.Invoke(sendingPartnerId, sharedTextureHandle, width, height, format, usage, Marshal.PtrToStringAuto(descriptionLPTSTR));
				} catch (Exception e) {
					//catch all in order not to fuck up Wyphon
					//the implementor should make sure this desn't throw exceptions in the first place					
				}

			}
		}
		
		public bool ShareD3DTexture(UInt32 sharedTextureHandle, UInt32 width, UInt32 height, UInt32 format, UInt32 usage, string description) {
			return ShareD3DTexture(wyphonPartnerHandle, sharedTextureHandle, width, height, format, usage, description);
		}

		public bool UnshareD3DTexture(UInt32 sharedTextureHandle) {
			return UnshareD3DTexture(wyphonPartnerHandle, sharedTextureHandle);
		}
		
		public UInt32 PartnerId {
			get { return GetPartnerId(wyphonPartnerHandle); }
		}
		
		public string GetPartnerName(UInt32 partnerId) {
			return Marshal.PtrToStringAuto( GetWyphonPartnerName(wyphonPartnerHandle, partnerId) );
		}


		public UInt32 GetPartnerIdByName( string partnerName ) {
			return GetPartnerIdByName( wyphonPartnerHandle, partnerName );
		}

		public UInt32 GetShareHandleByDescription( UInt32 wyphonPartnerId, string textureName ) {
			return GetShareHandleByDescription( wyphonPartnerHandle, wyphonPartnerId, textureName );
		}

		public bool GetD3DTextureInfo( UInt32 sharedTextureHandle, out UInt32 wyphonPartnerId, out UInt32 width, out UInt32 height, out UInt32 format, out UInt32 usage, out string description ) {
			//descr.
			char[] chars = new char[2048];
			IntPtr descrPtr = System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(chars, 0);
			bool retVal =  GetD3DTextureInfo( wyphonPartnerHandle, sharedTextureHandle, out wyphonPartnerId, out width, out height, out format, out usage, ref descrPtr, 2048 );


			//TODO fill description string with the bytes in descrPtr...
			//using System.Text;
			//string desc = new string( chars );
			description = new string( chars );

			return retVal;
		}

		/// <summary>
		/// Shortcut method to get a texture's info by passing the applicatio name and textue description
		/// </summary>
		/// <param name="sharedTextureHandle"></param>
		/// <param name="wyphonPartnerId"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="format"></param>
		/// <param name="usage"></param>
		/// <param name="description"></param>
		/// <returns></returns>
		public bool GetD3DTextureInfo( string partnerName, string sharedTextureName, out UInt32 width, out UInt32 height, out UInt32 format, out UInt32 usage ) {
			
			UInt32 pId = GetPartnerIdByName( partnerName );
			UInt32 shareHandle = GetShareHandleByDescription( pId, sharedTextureName );
			string description;

			return GetD3DTextureInfo( shareHandle, out pId, out width, out height, out format, out usage, out description );
		}


		public void Dispose() {			
			if (wyphonPartnerHandle != 0) {
				DestroyWyphonPartner(wyphonPartnerHandle);
				wyphonPartnerHandle = 0;
			}
		}
	}
}