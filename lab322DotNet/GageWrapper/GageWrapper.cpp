// GageWrapper.cpp : main project file.

#include "stdafx.h"
#include "Stream2Analysis.hpp"
#include "Gage.hpp"
using namespace System;
using namespace GageWrapper;



static void print(System::String^ str) {

	Console::Write(str);
}


int _tmain(int argc, _TCHAR* argv[]){
	OutputDelegate^ output = gcnew OutputDelegate(print);
	Gage^ gage = gcnew Gage(output);
	gage->SetGage(Gage::GageState::Init);
	for (int i = 0; i < 10000000000;++i){
		gage->SetGage(Gage::GageState::Start);
	} ;
	gage->SetGage(Gage::GageState::Stop);

}