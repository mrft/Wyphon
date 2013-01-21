/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 11/01/2013
 * Time: 9:38
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace LocalMessageBroadcast
{
	/// <summary>
	/// LocalMessageBroadcastPartner allows to create a 'partner' 
	/// that can be notified of the existence of other partners on the same 'channel'.
	/// 
	/// Every partner is allowed to send messages, and they will be delivered to all 
	/// other partners.
	/// </summary>
	public class LocalMessageBroadcastPartner : IDisposable
	{
		#region LocalMessageBroadcast.dll imports & delegates
		
		//HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, LPCTSTR sendingPartnerName, void * customData
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void PartnerJoinedCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr partnerNameLPTSTR, IntPtr customData);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void PartnerLeftCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr customData);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private unsafe delegate void BroadcastMessageReceivedCallbackDelegate(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr msgData, uint msgLength, IntPtr customData);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern uint CreateLocalMessageBroadcastPartner(
				[MarshalAs(UnmanagedType.LPTStr)]string localMessageBroadcastName, 
				[MarshalAs(UnmanagedType.LPTStr)]string applicationName, 
				IntPtr callbackFuncCustomData,
				IntPtr pPartnerJoinedCallbackFunc,
				IntPtr pPartnerLeftCallbackFunc,
				IntPtr pMsgReceivedCallbackFunc
			);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool DestroyLocalMessageBroadcastPartner(uint hLocalMessageBroadcastPartner);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool BroadcastMessage(uint hLocalMessageBroadcastPartner, byte[] data, uint length);

        [DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool SendMessageToSinglePartner(uint hLocalMessageBroadcastPartner, uint partnerId, byte[] data, uint length);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr GetBroadcastPartnerName(uint hLocalMessageBroadcastPartner, uint partnerId);

		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
		private static extern uint GetPartnerId(uint wyphonPartnerHandle);

//		[DllImport("Wyphon", CallingConvention = CallingConvention.Cdecl)]
//		public static extern void GetBroadcastPartnerName(uint hLocalMessageBroadcastPartner, uint partnerId, [MarshalAs(UnmanagedType.LPTStr)] System.Text.StringBuilder name);
		
		#endregion LocalMessageBroadcast.dll imports & delegates

		
		#region fields
		
		uint localMessageBroadcastPartnerHandle = 0;
		
		//by storing them as class variables, we make sure the garbage collector doesn't remove these too early
		private PartnerJoinedCallbackDelegate partnerJoinedCallbackDelegate;
		private PartnerLeftCallbackDelegate partnerLeftCallbackDelegate;
		private BroadcastMessageReceivedCallbackDelegate messageReceivedCallbackDelegate;
		#endregion fields
		
		// What the event handlers should look like
		public delegate void PartnerJoinedHandler(uint partnerId, string partnerName);
		public delegate void PartnerLeftHandler(uint partnerId);

		public delegate void MessageReceivedHandler(uint sendingPartnerId, IntPtr msgData, uint msgLength);

		// Public events that one can subscribe to
		public event PartnerJoinedHandler OnPartnerJoined;
		public event PartnerLeftHandler OnPartnerLeft;

		public event MessageReceivedHandler OnMessage;

		//Example if you want to subscribe to an event fired by Wyphon
		//partner.OnMessage += new MessageReceivedHandler(DoubleBufferChanged);

		
		
		public LocalMessageBroadcastPartner(string partnerName, string channel) {
			partnerJoinedCallbackDelegate = PartnerJoinedCallback;
			partnerLeftCallbackDelegate = PartnerLeftCallback;
			messageReceivedCallbackDelegate = MessageReceivedCallback;
			
			localMessageBroadcastPartnerHandle = CreateLocalMessageBroadcastPartner(channel, partnerName, IntPtr.Zero, 
			                                          Marshal.GetFunctionPointerForDelegate(partnerJoinedCallbackDelegate), 
			                                          Marshal.GetFunctionPointerForDelegate(partnerLeftCallbackDelegate), 
			                                          Marshal.GetFunctionPointerForDelegate(messageReceivedCallbackDelegate)
			                                         );
			
		}

		
		private void PartnerJoinedCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr partnerNameLPTSTR, IntPtr customData) {
			if (OnPartnerJoined != null) {
				try {
					OnPartnerJoined.Invoke(partnerId, Marshal.PtrToStringAuto(partnerNameLPTSTR));
				} catch (Exception e) {
					//catch all in order not to fuck up LocalMessageBroadcast
					//the implementor should make sure this desn't throw exceptions in the first place
				}
			}
		}
		
		public void PartnerLeftCallback(uint hLocalMessageBroadcastPartner, uint partnerId, IntPtr customData) {
			if (OnPartnerLeft != null) {
				try {
					OnPartnerLeft.Invoke(partnerId);
				} catch (Exception e) {
					//catch all in order not to fuck up LocalMessageBroadcast
					//the implementor should make sure this desn't throw exceptions in the first place
				}
			}			
		}

		public void MessageReceivedCallback(uint hLocalMessageBroadcastPartner, uint sendingPartnerId, IntPtr msgData, uint msgLength, IntPtr customData) {
			if (OnMessage != null) {
				try {
					OnMessage.Invoke(sendingPartnerId, msgData, msgLength);
				} catch (Exception e) {
					//catch all in order not to fuck up LocalMessageBroadcast
					//the implementor should make sure this desn't throw exceptions in the first place
				}
			}
		}

		public uint PartnerId {
			get { return GetPartnerId(localMessageBroadcastPartnerHandle); }
		}
		
		public string GetPartnerName(uint partnerId) {
			return Marshal.PtrToStringAuto( GetBroadcastPartnerName(localMessageBroadcastPartnerHandle, partnerId) );
		}
		
		public bool BroadcastMessage(byte[] data) {
			return BroadcastMessage(localMessageBroadcastPartnerHandle, data, (uint) data.Length);
		}

        public bool SendMessageToSinglePartner(uint partnerId, byte[] data)
        {
            return SendMessageToSinglePartner(localMessageBroadcastPartnerHandle, partnerId, data, (uint)data.Length);
        }

		public void Dispose() {
			if (localMessageBroadcastPartnerHandle != 0) {
				DestroyLocalMessageBroadcastPartner(localMessageBroadcastPartnerHandle);
				localMessageBroadcastPartnerHandle = 0;
			}
		}

	}
}