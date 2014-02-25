#pragma once
#include <windows.h>
#include <stdio.h>
#include <CsPrototypes.h>
#include "CsExpert.h"

#include "CsAppSupport.h"
#include "CsTchar.h"

#using <mscorlib.dll>
using namespace System;
using System::Runtime::InteropServices::Marshal;

namespace lab322_CPlusplus{


#pragma unmanaged
	typedef struct
	{
		uInt32		u32LoopCount;
		TCHAR		strResultFile[MAX_PATH];
		BOOL		bDoAnalysis;			/* Turn on or off data analysis */
	}CSSTMCONFIG, *PCSSTMCONFIG;


	typedef void (*ConsoleCB)(LPCTSTR);  
	typedef void (*DrawCB)(void);  
	static void Draw(){
			
	};
	static void Output(LPCTSTR output){
	
	
	};
	public class Stream2Analysis{
	
	public:
		int32						i32Status;
		void*						pBuffer1;
		void*						pBuffer2;
		void*						pCurrentBuffer;
		void*						pWorkBuffer;
		uInt32						u32Mode;
		CSHANDLE					hSystem ;
		CSSYSTEMINFO				CsSysInfo;
		CSACQUISITIONCONFIG			CsAcqCfg;
		CSSTMCONFIG					StmConfig;
		LPCTSTR						szIniFile;
		BOOL						bDone;
		BOOL						bErrorData;
		uInt32						u32LoopCount ;
		uInt32						u32SaveCount ;
		uInt32						u32TickStart ;
		long long					llTotalSamples ;
		uInt32						u32TransferSize ;
		uInt32						u32TransferSizeInBytes ;
		uInt32						u32ErrorFlag ;
		int64*						pi64Sums;
		
		bool						isRunning;
		void*						form;
		uInt32 resultSize;
		int16* result;
		Stream2Analysis(ConsoleCB callback,DrawCB drawCB):pBuffer1(NULL),pBuffer2(NULL),pCurrentBuffer(NULL),pWorkBuffer(NULL){
			i32Status = CS_SUCCESS;
			szIniFile = _T("Stream2Analysis.ini");

			hSystem = 0;
			isRunning = false;
			bDone = FALSE;
			bErrorData = FALSE;
			u32LoopCount = 0;
			u32SaveCount = 0;
			u32TickStart = 0;
			llTotalSamples = 0;
			u32TransferSize = 0;
			u32TransferSizeInBytes = 0;
			u32ErrorFlag = 0;
			console = callback;
			draw = drawCB;
		}
		bool Initialize();
		void Capture();
		void Exit();

		static void* Run(void * context){
			Stream2Analysis * stream = (Stream2Analysis *) context;
			stream->Initialize();
			while(true){
				stream->Capture();
			}
			return 0;
		};
	private:
		int32 LoadStmConfiguration(LPCTSTR szIniFile, PCSSTMCONFIG pConfig);
		ConsoleCB console;
		DrawCB draw;
		int32 InitializeStream(CSHANDLE hSystem);
		void TransBuffData( void* pBuffer, uInt32 u32Size, uInt32 u32SampleBits );
		void UpdateProgress( DWORD	dwTickStart,BOOL bAbort, unsigned long long llSamples );
	};


#pragma managed
	
	
	public ref  class Stream2AnalysisWrapper{
	
	public:
		Stream2AnalysisWrapper(){
			_stream2Analysis = new Stream2Analysis(&Output,&Draw);
		}
		~Stream2AnalysisWrapper(){};

	
	private:
		Stream2Analysis * _stream2Analysis;
	};
}