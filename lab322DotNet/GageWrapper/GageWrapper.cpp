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
	gage->Initialize();
	gage->Start();
	for (int i = 0; i < 1000;++i){
		gage->Capture();
	} ;
	gage->Exit();

}