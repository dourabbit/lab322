#pragma once
#include "Stream2Analysis.hpp"
#include <iostream>

#define SAMPLERATE 125000000
#define DATELEN 6256 //(10*5*125\16+1)*16

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace GageNative;



namespace GageWrapper{
#pragma unmanaged
void __stdcall SetupGage(Stream2Analysis * stream2Analysis){


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



#pragma managed

public delegate void OutputDelegate(String^ str);
public delegate void _outputDelegate(char* str);
public delegate void ProceedingDelegate(array<uInt16>^ pIntArray, uInt32 size, uInt32 u32SampleBits);
public delegate void _proceedingDelegate(void* pBuffer, uInt32 size, uInt32 u32SampleBits);

public ref class Gage {
	static void __stdcall _OutputGage(char* str){

		Gage ^ gage = Gage::GetSingleton();
		String^ str1 = System::Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)str);
		gage->output->Invoke(str1);
	};
	static void __stdcall _ProceedGage(void* pBuffer, uInt32 size, uInt32 u32SampleBits){
		Gage ^ gage = Gage::GetSingleton();
		//String^ str1 = System::Runtime::InteropServices::Marshal::ptr;
		
		//gage->proceed->Invoke(pIntArray,size,u32SampleBits);
		int16* pi16Buffer = (int16 *)pBuffer;
		for (int i = 0; i < size; i++)
			gage->result[i] = pi16Buffer[i];
	};

private:
	Stream2Analysis* stream2Analysis;
	static Gage ^ singleton;
	GCHandle gch;
public:
	static GCHandle gchOutput, gchProceed,gchSetup;
	static Gage^ GetSingleton(){ return singleton; }
	bool isRunning = false;
	//http://msdn.microsoft.com/en-us/library/367eeye0.aspx
	//Marshal gage delegate
	Gage(OutputDelegate^ output) {//,  ProceedingDelegate^ drawCB
		IntPtr ip;
		singleton = this;
		this->output = output; //this->proceed = drawCB;

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
	~Gage() { this->!Gage(); }
	!Gage() { delete stream2Analysis; }
	static ConsoleCB outputFnc; 
	static TransDataCB proceedFnc;
	OutputDelegate^ output;
	//ProceedingDelegate^ proceed;
	array<Int16>^ result;
	UINT32 resultSize;
	bool Initialize() {	
		stream2Analysis->Initialize();
		resultSize = stream2Analysis->u32TransferSizeWithOutTail;
		result = gcnew array<Int16>(stream2Analysis->u32TransferSizeWithOutTail);
		return true;
	}
	bool Start() { this->isRunning = true; return stream2Analysis->Start(); }
	void Capture(){ this->isRunning = true; stream2Analysis->Capture(); }
	void Exit(){ this->isRunning = false; stream2Analysis->Exit(); }
};
}