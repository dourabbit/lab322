#include "Stream2Analysis.hpp"

using namespace lab322_CPlusplus;
#define	TAIL_ADJUST	32					/* number of bytes at end of data which holds the timestamp values */
#define OUT_FILE	"BufferSums.txt"	/* name of the output file */
#define LOOP_COUNT	1000
#define STM_SECTION _T("StmConfig")		/* section name in ini file */


void DisplayErrorString(const int32 i32Status){};


int32 Stream2Analysis::InitializeStream(CSHANDLE hSystem)
{
		int32	i32Status = CS_SUCCESS;
		int64	i64ExtendedOptions = 0;	

		CSACQUISITIONCONFIG CsAcqCfg = {0};

		CsAcqCfg.u32Size = sizeof(CSACQUISITIONCONFIG);

		/*
			Get user's acquisition Data
		*/
		i32Status = CsGet(hSystem, CS_ACQUISITION, CS_CURRENT_CONFIGURATION, &CsAcqCfg);
		if (CS_FAILED(i32Status))
		{
			DisplayErrorString(i32Status);
			return (i32Status);
		}
		/*
			Check if selected system supports Expert Stream
			And set the correct image to be used.
		*/
		CsGet(hSystem, CS_PARAMS, CS_EXTENDED_BOARD_OPTIONS, &i64ExtendedOptions);

		if (i64ExtendedOptions & CS_BBOPTIONS_STREAM)
		{
			_ftprintf(stdout, _T("\nSelecting Expert Stream from image 1."));
			CsAcqCfg.u32Mode |= CS_MODE_USER1;
		}
		else if ((i64ExtendedOptions >> 32) & CS_BBOPTIONS_STREAM)
		{
			_ftprintf(stdout, _T("\nSelecting Expert Stream from image 2."));
			CsAcqCfg.u32Mode |= CS_MODE_USER2;
		}
		else
		{
			_ftprintf(stdout, _T("\nCurrent system does not support Expert Stream."));
			_ftprintf(stdout, _T("\nApplication terminated."));
			return CS_MISC_ERROR;
		}
	
	/*
		Sets the Acquisition values down the driver, without any validation, 
		for the Commit step which will validate system configuration.
	*/

		i32Status = CsSet(hSystem, CS_ACQUISITION, &CsAcqCfg);
		if (CS_FAILED(i32Status))
		{
			DisplayErrorString(i32Status);
			return CS_MISC_ERROR;
		}

		return CS_SUCCESS; // Success
}
int32 Stream2Analysis::LoadStmConfiguration(LPCTSTR szIniFile, PCSSTMCONFIG pConfig)
{
	TCHAR	szDefault[MAX_PATH];
	TCHAR	szString[MAX_PATH];
	TCHAR	szFilePath[MAX_PATH];
	int		nDummy;

	CSSTMCONFIG CsStmCfg;	
	/*
		Set defaults in case we can't read the ini file
	*/
	CsStmCfg.u32LoopCount	= LOOP_COUNT; 
	strcpy(CsStmCfg.strResultFile, _T(OUT_FILE));
	
	if (NULL == pConfig)
	{
		return (CS_INVALID_PARAMETER);
	}

	GetFullPathName(szIniFile, MAX_PATH, szFilePath, NULL);

	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(szFilePath))
	{
		*pConfig = CsStmCfg;
		return (CS_USING_DEFAULTS);
	}

	if (0 == GetPrivateProfileSection(STM_SECTION, szString, 100, szFilePath))
	{
		*pConfig = CsStmCfg;
		return (CS_USING_DEFAULTS);
	}

	nDummy = (int)CsStmCfg.u32LoopCount;
	CsStmCfg.u32LoopCount = GetPrivateProfileInt(STM_SECTION, _T("LoopCount"), nDummy, szFilePath);

	nDummy = 1;
	CsStmCfg.bDoAnalysis = (0 != GetPrivateProfileInt(STM_SECTION, _T("DoAnalysis"), nDummy, szFilePath));

	_stprintf(szDefault, _T("%s"), CsStmCfg.strResultFile);
	GetPrivateProfileString(STM_SECTION, _T("ResultsFile"), szDefault, szString, MAX_PATH, szFilePath);
	_tcscpy(CsStmCfg.strResultFile, szString);

	*pConfig = CsStmCfg;
	return (CS_SUCCESS);
}
void Stream2Analysis::TransBuffData( void* pBuffer, uInt32 u32Size, uInt32 u32SampleBits )
{
	uInt32 i;
	uInt8* pu8Buffer = NULL;
	int16* pi16Buffer = NULL;
	
	int64 i64Sum = 0;
	if ( 8 == u32SampleBits )
	{
		pu8Buffer = (uInt8 *)pBuffer;
		for (i = 0; i < u32Size; i++)
		{
			i64Sum += pu8Buffer[i];
		}
	}
	else
	{
		pi16Buffer = (int16 *)pBuffer;
		int offset = 0;
		this->result = pi16Buffer;
		this->draw();
	}

}


void Stream2Analysis::UpdateProgress( DWORD	dwTickStart,BOOL bAbort, unsigned long long llSamples )
{
	static	DWORD	dwTickLastUpdate = 0;
	uInt32	h, m, s;
	DWORD	dwElapsed;
	DWORD	dwTickNow = GetTickCount();

	if (bAbort || (dwTickNow - dwTickLastUpdate > 300))
	{
		double dRate;
		dwTickLastUpdate = dwTickNow;
		dwElapsed = dwTickNow - dwTickStart;
		
		if (dwElapsed)
		{
			dRate = (llSamples / 1000000.0) / (dwElapsed / 1000.0);
			h = m = s = 0;

			if (dwElapsed >= 1000)
			{
				if ((s = dwElapsed / 1000) >= 60)	// Seconds
				{
					
					if ((m = s / 60) >= 60)			// Minutes
					{
						if ((h = m / 60) > 0)		// Hours
							m %= 60;
					}

					s %= 60;
				}
			}
			printf ("\rTotal data: %0.2f MS, Rate: %6.2f MS/s, Elapsed Time: %u:%02u:%02u  ", llSamples / 1000000.0, dRate, h, m, s);
		}
	}
}
bool	Stream2Analysis::Initialize(){
			this->isRunning = true;
	
			/*
				Initializes the CompuScope boards found in the system. If the
				system is not found a message with the error code will appear.
				Otherwise i32Status will contain the number of systems found.
			*/
			i32Status = CsInitialize();

			if (CS_FAILED(i32Status))
			{
				DisplayErrorString(i32Status);
				return (-1);
			}
			/*
				Get the system. This sample program only supports one system. If
				2 systems or more are found, the first system that is found
				will be the system that will be used. hSystem will hold a unique
				system identifier that is used when referencing the system.
			*/
			i32Status = CsGetSystem(&hSystem, 0, 0, 0, 0);

			if (CS_FAILED(i32Status))
			{
				DisplayErrorString(i32Status);
				return (-1);
			}
			/*
				Get System information. The u32Size field must be filled in
				 prior to calling CsGetSystemInfo
			*/
			CsSysInfo.u32Size = sizeof(CSSYSTEMINFO);
			i32Status = CsGetSystemInfo(hSystem, &CsSysInfo);

			/*
				Display the system name from the driver
			*/
			_ftprintf(stdout, _T("\nBoard Name: %s"), CsSysInfo.strBoardName);

			/* 
				In this example we're only using 1 trigger source
			*/	
			i32Status = CsAs_ConfigureSystem(hSystem, (int)CsSysInfo.u32ChannelCount, 1, (LPCTSTR)szIniFile, &u32Mode);

			if (CS_FAILED(i32Status))
			{
				if (CS_INVALID_FILENAME == i32Status)
				{
			/*
						Display message but continue on using defaults.
			*/
					_ftprintf(stdout, _T("\nCannot find %s - using default parameters."), szIniFile);
				}
				else
				{	
			/*
						Otherwise the call failed.  If the call did fail we should free the CompuScope
						system so it's available for another application
			*/
					DisplayErrorString(i32Status);
					CsFreeSystem(hSystem);
					return(-1);
				}
			}
			/*
				If the return value is greater than  1, then either the application, 
				acquisition, some of the Channel and / or some of the Trigger sections
				were missing from the ini file and the default parameters were used. 
			*/
			if (CS_USING_DEFAULT_ACQ_DATA & i32Status)
				_ftprintf(stdout, _T("\nNo ini entry for acquisition. Using defaults."));

			if (CS_USING_DEFAULT_CHANNEL_DATA & i32Status)
				_ftprintf(stdout, _T("\nNo ini entry for one or more Channels. Using defaults for missing items."));

			if (CS_USING_DEFAULT_TRIGGER_DATA & i32Status)
				_ftprintf(stdout, _T("\nNo ini entry for one or more Triggers. Using defaults for missing items."));

			/*
				Load application specific information from the ini file
			*/
			i32Status = LoadStmConfiguration(szIniFile, &StmConfig);
			if (CS_FAILED(i32Status))
			{
				DisplayErrorString(i32Status);
				CsFreeSystem(hSystem);
				return (-1);
			}
			if (CS_USING_DEFAULTS == i32Status)
			{
				_ftprintf(stdout, _T("\nNo ini entry for Stm configuration. Using defaults."));
			}
			/*
				Streaming Configuration

				Validate if the board supports hardware streaming  If  it is not supported, 
				we'll exit gracefully.
			*/
			i32Status = InitializeStream(hSystem);
			if (CS_FAILED(i32Status))
			{
				// Error string is displayed in InitializeStream
				CsFreeSystem(hSystem);
				return (-1);
			}

			CsAcqCfg.u32Size = sizeof(CSACQUISITIONCONFIG);
			/*
				Get user's acquisition data to use for various parameters for transfer
			*/
			i32Status = CsGet(hSystem, CS_ACQUISITION, CS_CURRENT_CONFIGURATION, &CsAcqCfg);
			if (CS_FAILED(i32Status))
			{
				DisplayErrorString(i32Status);
				CsFreeSystem(hSystem);
				return (-1);
			}

			pi64Sums = (int64 *)calloc(StmConfig.u32LoopCount, sizeof(int64));

			if (NULL == pi64Sums)
			{
				_ftprintf (stderr, _T("\nUnable to allocate memory to hold analysis results.\n"));
				CsFreeSystem(hSystem);
				return (-1);
			}

			/*
				We need to allocate a buffer for transferring the data. Buffer is allocated as void with
				a size of length * number of channels * sample size. All channels in the mode are transferred
				within the same buffer, so the mode tells us the number of channels.  Currently, TAIL_ADJUST 
				samples are placed at the end of each segment. These samples contain timestamp information for the 
				segemnt.  The buffer must be allocated by a call to CsStmAllocateBuffer.  This routine will
				allocate a buffer suitable for streaming.  In this program we're allocating 2 streaming buffers
				so we can transfer to one while doing analysis on the other.
			*/

			u32TransferSize = (uInt32)(CsAcqCfg.i64SegmentSize * (CsAcqCfg.u32Mode & CS_MASKED_MODE) + TAIL_ADJUST);
			u32TransferSizeInBytes = u32TransferSize * CsAcqCfg.u32SampleSize;

			i32Status = CsStmAllocateBuffer(hSystem, 0,  u32TransferSizeInBytes, &pBuffer1);
			if (CS_FAILED(i32Status))
			{
				_ftprintf (stderr, _T("\nUnable to allocate memory for stream buffer 1.\n"));
				free(pi64Sums);
				CsFreeSystem(hSystem);
				return (-1);
			}

			i32Status = CsStmAllocateBuffer(hSystem, 0,  u32TransferSizeInBytes, &pBuffer2);
			if (CS_FAILED(i32Status))
			{
				_ftprintf (stderr, _T("\nUnable to allocate memory for stream buffer 2.\n"));
				CsStmFreeBuffer(hSystem, 0, pBuffer1);
				free(pi64Sums);
				CsFreeSystem(hSystem);
				return (-1);
			}

		/*
			Commit the values to the driver.  This is where the values get sent to the
			hardware.  Any invalid parameters will be caught here and an error returned.
		*/
			i32Status = CsDo(hSystem, ACTION_COMMIT);
			if (CS_FAILED(i32Status))
			{
				DisplayErrorString(i32Status);
				free(pi64Sums);
				CsStmFreeBuffer(hSystem, 0, pBuffer1);
				CsStmFreeBuffer(hSystem, 0, pBuffer2);
				CsFreeSystem(hSystem);
				return (-1);
			}


}

void Stream2Analysis::Exit(){

	
	CsDo(hSystem, ACTION_ABORT);


	if ( StmConfig.bDoAnalysis )
	{
		if ( u32SaveCount < u32LoopCount )
		{
			//pi64Sums[u32SaveCount++] = SumBufferData(pWorkBuffer, u32TransferSize - TAIL_ADJUST, CsAcqCfg.u32SampleBits);
		}
	}

	CsStmFreeBuffer(hSystem, 0, pBuffer1);
	CsStmFreeBuffer(hSystem, 0, pBuffer2);

	i32Status = CsFreeSystem(hSystem);

	if (bErrorData)
	{
		_ftprintf (stdout, _T("\nStream aborted on error.\n"));
	}
	if (u32SaveCount > 0)
	{
		//SaveResults(pi64Sums, u32SaveCount, StmConfig.strResultFile);
	}
	free(pi64Sums);

}

void Stream2Analysis::Capture(){

		if (u32LoopCount & 1)
		{
			pCurrentBuffer = pBuffer2;
		}
		else
		{
			pCurrentBuffer = pBuffer1;
		}

		i32Status = CsStmTransferToBuffer(hSystem, 0, pCurrentBuffer, u32TransferSize);
		if (CS_FAILED(i32Status))
		{
			DisplayErrorString(i32Status);
			return;
		}

		if ( StmConfig.bDoAnalysis )
		{
			if ( NULL != pWorkBuffer ){
				TransBuffData(pWorkBuffer, u32TransferSize - TAIL_ADJUST, CsAcqCfg.u32SampleBits);
			}
		}
/*
		Wait for the DMA transfer on the current buffer to complete so we can loop 
		back around to start a new one. Calling thread will sleep until
		the transfer completes
*/
		i32Status = CsStmGetTransferStatus( hSystem, 0, 5000, &u32ErrorFlag, NULL, NULL );
		if ( CS_SUCCEEDED(i32Status) )
		{
			if ( CS_STM_TRANSFER_INPROGRESS == i32Status )
			{
				_ftprintf (stdout, _T("\nStream transfer timeout. !!!"));
				bDone = TRUE;
			}
			if ( STM_TRANSFER_ERROR_FIFOFULL & u32ErrorFlag )
			{
				_ftprintf (stdout, _T("\nStream Fifo full !!!"));
				bDone = TRUE;
			}
		}
		else if ( CS_STM_TRANSFER_ABORTED == i32Status )
		{
			_ftprintf (stdout, _T("\nStream transfer aborted. !!!"));
			bDone = TRUE;
		}
		else /* some other error */
		{
			DisplayErrorString(i32Status);
			bDone = TRUE;
		}

		if ( 0 == u32LoopCount )
		{
			u32TickStart =  GetTickCount();
		}

		pWorkBuffer = pCurrentBuffer;

		llTotalSamples += u32TransferSize;
		u32LoopCount = 1;

		/* Are we being asked to quit? */
		if (_kbhit())
		{
			switch (toupper(_getch()))
			{
			case 27: 
				bDone = TRUE;
				break;
			default:
				MessageBeep(MB_ICONHAND);
				break;
			}
		}

		UpdateProgress(u32TickStart, FALSE, llTotalSamples);
}
