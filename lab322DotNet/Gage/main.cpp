#include <stdio.h>
#include <tchar.h>
#include "Stream2Analysis.hpp"


#define SAMPLERATE 125000000
#define DATELEN 6256 //(10*5*125\16+1)*16


using namespace GageNative;
void Output(char* str){

	printf(str);

};

int16* data;
void ProceedingData(void* pBuffer, uInt32 size, uInt32 u32SampleBits){
	if (8 == u32SampleBits) return;

	int16* pi16Buffer = (int16 *)pBuffer;
	const uInt16 offset = 8;
#pragma omp parallel for
	for (int i = 0; i < size; i++){
			data[ i] = pi16Buffer[i];


		}

};


void Setup(Stream2Analysis * stream2Analysis){


	stream2Analysis->i32Status = CsGet(stream2Analysis->hSystem, CS_ACQUISITION, CS_CURRENT_CONFIGURATION, &stream2Analysis->CsAcqCfg);
	stream2Analysis->CsAcqCfg.u32Mode = 8 | CS_MODE_USER1;
	stream2Analysis->CsAcqCfg.i64SampleRate = SAMPLERATE;
	stream2Analysis->CsAcqCfg.i64Depth = DATELEN;
	stream2Analysis->CsAcqCfg.i64SegmentSize = DATELEN;
	stream2Analysis->CsAcqCfg.i64TriggerDelay = 0;
	stream2Analysis->CsAcqCfg.i32SampleOffset = 0;
	stream2Analysis->CsAcqCfg.u32SegmentCount = 1;
	stream2Analysis->i32Status = CsSet(stream2Analysis->hSystem, CS_ACQUISITION, &stream2Analysis->CsAcqCfg);




	stream2Analysis->csChannelCfg.u32ChannelIndex = 1;
	stream2Analysis->i32Status = CsGet(stream2Analysis->hSystem, CS_CHANNEL, CS_CURRENT_CONFIGURATION, &stream2Analysis->csChannelCfg);
	for (int i = 1; i < 9; i++){
		stream2Analysis->csChannelCfg.u32ChannelIndex = i;
		stream2Analysis->csChannelCfg.u32Term = 1;
		stream2Analysis->csChannelCfg.u32InputRange = 2000;
		stream2Analysis->csChannelCfg.u32Impedance = 50;
		stream2Analysis->csChannelCfg.i32DcOffset = 0;
		stream2Analysis->i32Status = CsSet(stream2Analysis->hSystem, CS_CHANNEL, &stream2Analysis->csChannelCfg);
	}



	stream2Analysis->csTriggerCfg.u32TriggerIndex = 1;
	stream2Analysis->i32Status = CsGet(stream2Analysis->hSystem, CS_TRIGGER, CS_CURRENT_CONFIGURATION, &stream2Analysis->csTriggerCfg);
	stream2Analysis->csTriggerCfg.u32Condition = 1;
	stream2Analysis->csTriggerCfg.i32Level = 20;
	stream2Analysis->csTriggerCfg.i32Source = -1;
	stream2Analysis->csTriggerCfg.u32ExtCoupling = 1;
	stream2Analysis->csTriggerCfg.u32ExtTriggerRange = 2000;
	stream2Analysis->csTriggerCfg.u32ExtImpedance = 50;
	stream2Analysis->i32Status = CsSet(stream2Analysis->hSystem, CS_TRIGGER, &stream2Analysis->csTriggerCfg);
}


int _tmain(int argc, _TCHAR* argv[])
{
	Stream2Analysis* gage = new Stream2Analysis(&Output, &Setup, &ProceedingData);
	gage->Initialize();
	bool bDone = TRUE;
	
	data = new int16[gage->u32TransferSizeWithOutTail];
	gage->Start();
	while (bDone){
		/* Are we being asked to quit? */
		if (_kbhit()){
			switch (toupper(_getch())){
				case 27:
					bDone = FALSE;
					break;
				default:
					MessageBeep(MB_ICONHAND);
					break;
			}
		}
		if (gage->bDone == GageState::Start)
			gage->Capture();
		else if (gage->bDone == GageState::Pause)
			gage->Pause();
	}
	gage->Exit();
	delete gage;
	return 0;
}