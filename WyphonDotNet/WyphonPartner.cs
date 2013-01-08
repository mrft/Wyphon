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
		
		#region fields
		uint wyphonPartnerHandle = 0;
		
		#endregion fields
		
		// What the event handlers should look like
		public delegate void WyphonPartnerJoinedHandler(uint partnerId, string partnerName);
		public delegate void WyphonPartnerLeftHandler(uint partnerId);

		public delegate void WyphonPartnerD3DTextureSharedHandler(uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint format, uint usage, string description);
		public delegate void WyphonPartnerD3DTextureUnsharedHandler(uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint format, uint usage, string description);
		
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
			WyphonPartnerJoinedCallbackDelegate jcd = WyphonPartnerJoinedCallback;
			WyphonPartnerLeftCallbackDelegate lcd = WyphonPartnerLeftCallback;
			D3DTextureSharingStartedCallbackDelegate d3dtsbcd = D3DTextureSharingStartedCallback;
			D3DTextureSharingStoppedCallbackDelegate d3dtsecd = D3DTextureSharingStoppedCallback;
			
			wyphonPartnerHandle = CreateWyphonPartner(name, IntPtr.Zero, Marshal.GetFunctionPointerForDelegate(jcd), Marshal.GetFunctionPointerForDelegate(lcd), Marshal.GetFunctionPointerForDelegate(d3dtsbcd), Marshal.GetFunctionPointerForDelegate(d3dtsecd));
			
		}
		
		
		public void WyphonPartnerJoinedCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr partnerNameLPTSTR, IntPtr customData) {
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

		public void WyphonPartnerLeftCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr customData) {
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
		
		public void D3DTextureSharingStartedCallback( uint wyphonPartnerHandle, uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint format, uint usage, IntPtr descriptionLPTSTR ) {
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

		public void D3DTextureSharingStoppedCallback( uint wyphonPartnerHandle, uint sendingPartnerId, uint sharedTextureHandle, uint width, uint height, uint format, uint usage, IntPtr descriptionLPTSTR ) {
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
		
		public bool ShareD3DTexture(uint sharedTextureHandle, uint width, uint height, uint format, uint usage, string description) {
			return ShareD3DTexture(wyphonPartnerHandle, sharedTextureHandle, width, height, format, usage, description);
		}

		public bool UnshareD3DTexture(uint sharedTextureHandle) {
			return UnshareD3DTexture(wyphonPartnerHandle, sharedTextureHandle);
		}
		
		public uint PartnerId {
			get { return GetPartnerId(wyphonPartnerHandle); }
		}
		
		public void Dispose() {			
			if (wyphonPartnerHandle != 0) {
				DestroyWyphonPartner(wyphonPartnerHandle);
				wyphonPartnerHandle = 0;
			}
		}
	}
}