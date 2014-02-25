#include "Stream2Analysis.hpp"

using namespace GageNative;
#define	TAIL_ADJUST	32					/* number of bytes at end of data which holds the timestamp values */
#define OUT_FILE	"BufferSums.txt"	/* name of the output file */
#define LOOP_COUNT	1000
//#define STM_SECTION _T("StmConfig")		/* section name in ini file */
#include <stdio.h>
#include <stdlib.h>

void DisplayErrorString(const int32 i32Status){

	

};


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
			//DisplayErrorString(i32Status);
			_itoa(i32Status, outputStr, 16);
			console(_T("\nSystem Error Code :")); console(outputStr);
			return (i32Status);
		}
		/*
			Check if selected system supports Expert Stream
			And set the correct image to be used.
		*/
		CsGet(hSystem, CS_PARAMS, CS_EXTENDED_BOARD_OPTIONS, &i64ExtendedOptions);

		if (i64ExtendedOptions & CS_BBOPTIONS_STREAM)
		{
			//_ftprintf(stdout, _T("\nSelecting Expert Stream from image 1."));
			console(_T("\nSelecting Expert Stream from image 1."));
			CsAcqCfg.u32Mode |= CS_MODE_USER1;
		}
		else if ((i64ExtendedOptions >> 32) & CS_BBOPTIONS_STREAM)
		{
			//_ftprintf(stdout, _T("\nSelecting Expert Stream from image 2."));
			console(_T("\nSelecting Expert Stream from image 2."));
			CsAcqCfg.u32Mode |= CS_MODE_USER2;
		}
		else
		{
			//_ftprintf(stdout, _T("\nCurrent system does not support Expert Stream."));
			//_ftprintf(stdout, _T("\nApplication terminated."));
			console(_T("\nCurrent system does not support Expert Stream."));
			console(_T("\nApplication terminated."));
			return CS_MISC_ERROR;
		}
	
	/*
		Sets the Acquisition values down the driver, without any validation, 
		for the Commit step which will validate system configuration.
	*/

		i32Status = CsSet(hSystem, CS_ACQUISITION, &CsAcqCfg);
		if (CS_FAILED(i32Status))
		{
			//DisplayErrorString(i32Status);
			_itoa(i32Status, outputStr, 16);
			console(_T("\nSystem Error Code :")); console(outputStr);
			return CS_MISC_ERROR;
		}

		return CS_SUCCESS; // Success
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
			//printf ("\rTotal data: %0.2f MS, Rate: %6.2f MS/s, Elapsed Time: %u:%02u:%02u  ", llSamples / 1000000.0, dRate, h, m, s);
			
			sprintf(outputStr, "\rTotal data: %0.2f MS, Rate: %6.2f MS/s, Elapsed Time: %u:%02u:%02u  ", llSamples / 1000000.0, dRate, h, m, s);
			console(outputStr);
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
				//DisplayErrorString(i32Status);
				_itoa(i32Status, outputStr, 16);
				console(_T("\nSystem Error Code :")); console(outputStr);
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
				//DisplayErrorString(i32Status);
				_itoa(i32Status, outputStr, 16);
				console(_T("\nSystem Error Code :")); console(outputStr);
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
			//_ftprintf(stdout, _T("\nBoard Name: %s"), CsSysInfo.strBoardName);
			console(_T("\nBoard Name:")); console(CsSysInfo.strBoardName);

			setup(this);

			CsDo(hSystem, ACTION_COMMIT);
		



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
			u32TransferSizeWithOutTail = u32TransferSize - TAIL_ADJUST;
			u32TransferSizeInBytes = u32TransferSize * CsAcqCfg.u32SampleSize;

			i32Status = CsStmAllocateBuffer(hSystem, 0,  u32TransferSizeInBytes, &pBuffer1);
			if (CS_FAILED(i32Status))
			{
				_ftprintf (stderr, _T("\nUnable to allocate memory for stream buffer 1.\n"));
				CsFreeSystem(hSystem);
				return (-1);
			}

			i32Status = CsStmAllocateBuffer(hSystem, 0,  u32TransferSizeInBytes, &pBuffer2);
			if (CS_FAILED(i32Status))
			{
				_ftprintf (stderr, _T("\nUnable to allocate memory for stream buffer 2.\n"));
				CsStmFreeBuffer(hSystem, 0, pBuffer1);
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
				//DisplayErrorString(i32Status);
				_itoa(i32Status, outputStr, 16);
				console(_T("\nSystem Error Code :")); console(outputStr);

				CsStmFreeBuffer(hSystem, 0, pBuffer1);
				CsStmFreeBuffer(hSystem, 0, pBuffer2);
				CsFreeSystem(hSystem);
				return (-1);
			}
}
bool Stream2Analysis::Start(){

	i32Status = CsDo(hSystem, ACTION_START);
	if (CS_FAILED(i32Status))
	{
		//DisplayErrorString(i32Status);
		_itoa(i32Status, outputStr, 16);
		console(_T("\nSystem Error Code :")); console(outputStr);

		CsStmFreeBuffer(hSystem, 0, pBuffer1);
		CsStmFreeBuffer(hSystem, 0, pBuffer2);
		CsFreeSystem(hSystem);
		return (-1);
	}
	this->u32TickStart = GetTickCount();
}
void Stream2Analysis::Exit(){
	console(_T("\nSystem Exiting"));
	
	CsDo(hSystem, ACTION_ABORT);


	//if ( StmConfig.bDoAnalysis )
	//{
	//	if ( u32SaveCount < u32LoopCount )
	//	{
	//		//pi64Sums[u32SaveCount++] = SumBufferData(pWorkBuffer, u32TransferSize - TAIL_ADJUST, CsAcqCfg.u32SampleBits);
	//	}
	//}

	CsStmFreeBuffer(hSystem, 0, pBuffer1);
	CsStmFreeBuffer(hSystem, 0, pBuffer2);

	i32Status = CsFreeSystem(hSystem);
	
}

void Stream2Analysis::Capture(){

		if (isFirst)
		{
			pCurrentBuffer = pBuffer1;
		}
		else
		{
			pCurrentBuffer = pBuffer2;
		}
		isFirst = !isFirst;

		i32Status = CsStmTransferToBuffer(hSystem, 0, pCurrentBuffer, u32TransferSize);
		if (CS_FAILED(i32Status))
		{
			_itoa(i32Status, outputStr, 16);
			console(_T("\nSystem Error Code :")); console(outputStr);

			return;
		}

		//if ( StmConfig.bDoAnalysis )
		//{
			if ( NULL != pWorkBuffer ){
				this->transfer(pWorkBuffer, u32TransferSizeWithOutTail, CsAcqCfg.u32SampleBits);
			}
		//}
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
			//DisplayErrorString(i32Status);
			_itoa(i32Status, outputStr, 16);
			console(_T("\nSystem Error Code :")); console(outputStr);

			bDone = TRUE;
		}

		//if (isFirst)
		//{
		//	u32TickStart =  GetTickCount();
		//}

		pWorkBuffer = pCurrentBuffer;

		llTotalSamples += u32TransferSize;

		UpdateProgress(u32TickStart, FALSE, llTotalSamples);
}
