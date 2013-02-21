/*
 * Created by SharpDevelop.
 * User: frederik
 * Date: 14/11/2012
 * Time: 16:30
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
// LocalMessageBroadcast.h
#pragma once

#include <windows.h>

//'using namespace' can gives problems if not all includes are done before this statement
//using namespace System;

namespace LocalMessageBroadcast {


	//CALLBACK DEFINITIONS
	typedef bool ( * LPLocalMessageBroadcastPartnerJoinedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 sendingPartnerId, LPCTSTR sendingPartnerName, void * customData );
	typedef bool ( * LPLocalMessageBroadcastPartnerLeftCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 sendingPartnerId, void * customData);
	typedef bool ( * LPMessageReceivedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 sendingPartnerId, void * msgData, unsigned __int32 msgLength, void * customData );


	extern "C" _declspec(dllexport)
	bool DestroyLocalMessageBroadcastPartner(HANDLE localMessageBroadcastPartnerHandle);
	
	extern "C" _declspec(dllexport)
	HANDLE CreateLocalMessageBroadcastPartner( LPTSTR lpSharedMemoryName, LPTSTR myName
		, void * callbackFuncCustomData
		, LPLocalMessageBroadcastPartnerJoinedCALLBACK pPartnerJoinedCallbackFunc
		, LPLocalMessageBroadcastPartnerLeftCALLBACK pPartnerLeftCallbackFunc
		, LPMessageReceivedCALLBACK pMsgReceivedCallbackFunc 
	);
	
	extern "C" _declspec(dllexport)
	bool BroadcastMessage(HANDLE localMessageBroadcastPartnerHandle, void * data, unsigned __int32 length);

	extern "C" _declspec(dllexport)
	bool SendMessageToSinglePartner(HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 partnerId, void * data, unsigned __int32 length);

	extern "C" _declspec(dllexport)
	LPCTSTR GetBroadcastPartnerName(HANDLE localMessageBroadcastPartnerHandle, unsigned __int32 partnerId);

	extern "C" _declspec(dllexport)
	unsigned __int32 GetBroadcastPartnerId(HANDLE localMessageBroadcastPartnerHandle);

//	/// <summary>
//	///
//	/// </summary>
//	public ref class MyClass {
//		// TODO: Add class methods here
//	};

}
