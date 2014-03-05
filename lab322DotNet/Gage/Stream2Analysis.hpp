#pragma once
#include <windows.h>
#include <stdio.h>
#include <CsPrototypes.h>
#include "CsExpert.h"

#include "CsAppSupport.h"
#include "CsTchar.h"

namespace GageNative{
	enum GageState{
		Stop = 0,
		Init = 1,
		Start = 2,
		Pause = 3,
		SysError = 4,
		Exit = 5
	};
	typedef struct
	{
		uInt32		u32LoopCount;
		TCHAR		strResultFile[MAX_PATH];
		BOOL		bDoAnalysis;			/* Turn on or off data analysis */
	}CSSTMCONFIG, *PCSSTMCONFIG;

	class Stream2Analysis;
	typedef void(__stdcall *ConsoleCB)(char* str);
	typedef void(__stdcall *TransDataCB)(void* pBuffer,uInt32 size, uInt32 u32SampleBits);
	typedef void(__stdcall *SetupParameters)(Stream2Analysis * stream2Analysis);
	class Stream2Analysis{
	
	public:
		
		char* outputStr;
		int32						i32Status;
		void*						pBuffer1;
		void*						pBuffer2;
		void*						pCurrentBuffer;
		void*						pWorkBuffer;
		uInt32						u32Mode;
		CSHANDLE					hSystem ;
		CSSYSTEMINFO				CsSysInfo;
		CSACQUISITIONCONFIG			CsAcqCfg;
		CSCHANNELCONFIG				csChannelCfg;
		CSTRIGGERCONFIG				csTriggerCfg;
		CSSTMCONFIG					StmConfig;
		GageState					bDone;//
		BOOL						bErrorData;
		BOOL						isFirst;
		uInt32						u32TickStart ;
		long long					llTotalSamples ;
		uInt32						u32TransferSize ;//with tail
		uInt32						u32TransferSizeWithOutTail;
		uInt32						u32TransferSizeInBytes ;
		uInt32						u32ErrorFlag ;
		//int64*						pi64Sums;
		
		void*						form;
		uInt32 resultSize;
		//int16* result;
		Stream2Analysis(ConsoleCB callback, SetupParameters setupFnc, TransDataCB drawCB) :pBuffer1(NULL), pBuffer2(NULL), pCurrentBuffer(NULL), pWorkBuffer(NULL){
			i32Status = CS_SUCCESS;
			//szIniFile = _T("Stream2Analysis.ini");
			outputStr = new char[100];
			hSystem = 0;
			CsAcqCfg = { 0 };
			csChannelCfg = { 0 };
			csTriggerCfg = { 0 };
			CsAcqCfg.u32Size = sizeof(CSACQUISITIONCONFIG);
			csChannelCfg.u32Size = sizeof(CSCHANNELCONFIG);
			csTriggerCfg.u32Size = sizeof(CSTRIGGERCONFIG);

			bDone = GageState::Stop;
			bErrorData = FALSE;
			isFirst = TRUE;
			//u32LoopCount = 0;
			//u32SaveCount = 0;
			u32TickStart = 0;
			llTotalSamples = 0;
			u32TransferSize = 0;
			u32TransferSizeInBytes = 0;
			u32ErrorFlag = 0;
			console = callback;
			transfer = drawCB;
			setup = setupFnc;
		}
		~Stream2Analysis(){ delete[] outputStr; }
		bool Initialize();
		bool Start();
		void Pause();
		void Capture();
		void Exit();

	private:
		ConsoleCB console;
		TransDataCB transfer;
		SetupParameters setup;
		int32 InitializeStream(CSHANDLE hSystem);
		void UpdateProgress( DWORD	dwTickStart,BOOL bAbort, unsigned long long llSamples );
	};
}