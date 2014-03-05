#pragma once
#include "Stream2Analysis.hpp"
#include <iostream>
#include <omp.h>
#define SAMPLERATE 125000000
#define DATELEN 6256 //(10*5*125\16+1)*16

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace GageNative;



namespace GageWrapper{

#pragma unmanaged
void __stdcall SetupGage(Stream2Analysis * stream2Analysis){


		stream2Analysis->i32Status = CsGet(stream2Analysis->hSystem, CS_ACQUISITION, CS_CURRENT_CONFIGURATION, &stream2Analysis->CsAcqCfg);
		stream2Analysis->CsAcqCfg.u32Mode = CS_MODE_OCT | CS_MODE_USER1;
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



#pragma managed

public delegate void OutputDelegate(String^ str);
public delegate void _outputDelegate(char* str);
public delegate void ProceedingDelegate(array<Int16>^ pIntArray, uInt32 size, uInt32 u32SampleBits);
public delegate void _proceedingDelegate(void* pBuffer, uInt32 size, uInt32 u32SampleBits);

public ref class Gage {
	static void __stdcall _OutputGage(char* str){

		Gage ^ gage = Gage::GetSingleton();
		String^ str1 = System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)str);
		gage->output->Invoke(str1);
	};
	static void __stdcall _ProceedGage(void* pBuffer, uInt32 size, uInt32 u32SampleBits){
		Gage ^ gage = Gage::GetSingleton();
		short* pi16Buffer = (short*)pBuffer;
		Marshal::Copy(IntPtr((void *)pBuffer), gage->result, 0, size);

//#pragma omp parallel for
//		for (int channelInx = 0; channelInx < channels; channelInx++){
//			for (int i = 0; i < singleChannelLength; i++){
//				gage->result[channelInx*singleChannelLength + i] = pi16Buffer[channelInx + i*Gage::offset];
//			
//			
//			}
//		}
		//gage->proceed->Invoke(gage->result, channels, singleChannelLength, u32SampleBits);
	};

private:
	Stream2Analysis* stream2Analysis;
	static Gage ^ singleton;
	GCHandle gch;
public:
	static GCHandle gchOutput, gchProceed,gchSetup;
	static Gage^ GetSingleton(){ return singleton; }
	enum class GageState{
		Stop = 0,
		Init = 1,
		Start = 2,
		Pause = 3,
		SysError = 4,
		Exit = 5
	};
	GageState State = GageState::Stop;
	//http://msdn.microsoft.com/en-us/library/367eeye0.aspx
	//Marshal gage delegate
	Gage(OutputDelegate^ output) {
		IntPtr ip;
		singleton = this;
		this->output = output; 
		_outputDelegate^ outputFncPointer = gcnew _outputDelegate(_OutputGage);
		gch = GCHandle::Alloc(outputFncPointer);
		ip = Marshal::GetFunctionPointerForDelegate(outputFncPointer);
		outputFnc = static_cast<ConsoleCB>(ip.ToPointer());
		GC::Collect();

		_proceedingDelegate^ proceedFncPointer = gcnew _proceedingDelegate(_ProceedGage);
		gch = GCHandle::Alloc(proceedFncPointer);
		ip = Marshal::GetFunctionPointerForDelegate(proceedFncPointer);
		proceedFnc = static_cast<TransDataCB>(ip.ToPointer());
		GC::Collect();

		stream2Analysis = new Stream2Analysis(outputFnc, SetupGage, proceedFnc);

	}
	~Gage() { this->!Gage();  }
	!Gage() { delete stream2Analysis; }
	static const UInt16 offset = 8;
	static ConsoleCB outputFnc; 
	static TransDataCB proceedFnc;
	static UInt16 length4SingleChannl;
	UInt16 numOfChannel;
	OutputDelegate^ output;
	ProceedingDelegate^ proceed;
	array<Int16>^ result;
	UINT32 resultSize;
	GageState GetGage(){
		return (GageState)stream2Analysis->bDone;
	}
	void SetGage(GageState newState){
		State = (GageState)stream2Analysis->bDone;
		if (State == GageState::Start && newState == GageState::Exit)
		{
			stream2Analysis->Exit();
		}
		else if ((State == GageState::Init) && newState == GageState::Start){
		
			stream2Analysis->Start();
			stream2Analysis->Capture();
		}
		else if ((State == GageState::Start || State == GageState::Pause) && newState == GageState::Start){
			stream2Analysis->Capture();
		}
		else if ((State == GageState::Start || State == GageState::Pause) && newState == GageState::Pause){
			stream2Analysis->Pause();
		}
		else if (State == GageState::Stop && newState == GageState::Init){
			stream2Analysis->Initialize();
			length4SingleChannl = stream2Analysis->u32TransferSizeWithOutTail / 8;
			result = gcnew array<Int16>(stream2Analysis->u32TransferSizeWithOutTail);
		}
	
	}
};
}