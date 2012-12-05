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
	typedef bool ( * LPPartnerJoinedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * customData );
	typedef bool ( * LPPartnerLeftCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * customData);
	typedef bool ( * LPMessageReceivedCALLBACK)( HANDLE localMessageBroadcastPartnerHandle, unsigned int sendingPartnerId, void * msgData, unsigned int msgLength, void * customData );


	extern "C" _declspec(dllexport)
	bool DestroyLocalMessageBroadcastPartner(HANDLE localMessageBroadcastPartnerHandle);
	
	extern "C" _declspec(dllexport)
	HANDLE CreateLocalMessageBroadcastPartner( LPTSTR lpSharedMemoryName, LPTSTR myName
		, void * callbackFuncCustomData
		, LPPartnerJoinedCALLBACK pPartnerJoinedCallbackFunc
		, LPPartnerLeftCALLBACK pPartnerLeftCallbackFunc
		, LPMessageReceivedCALLBACK pMsgReceivedCallbackFunc 
	);
	
	extern "C" _declspec(dllexport)
	bool BroadcastMessage(HANDLE localMessageBroadcastPartnerHandle, void * data, unsigned int length);

	extern "C" _declspec(dllexport)
	LPCTSTR GetBroadcastPartnerName(HANDLE localMessageBroadcastPartnerHandle, unsigned int partnerId);

//	/// <summary>
//	///
//	/// </summary>
//	public ref class MyClass {
//		// TODO: Add class methods here
//	};

}
