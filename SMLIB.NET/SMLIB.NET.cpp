// This is the main DLL file.

#include "stdafx.h"

#include <windows.h>
#include "SMLIB.NET.h"
//#include <_vcclrit.h>

using namespace System;

namespace PESMLIB
{
	bool ManagedWrapper::minitialize() {
		bool retval = TRUE;
		/*try {
           retval =  (bool)__crt_dll_initialize();
		} catch(System::Exception* e) {
			Console::WriteLine(e->Message);
			retval = false;
		}*/
		return retval;
	}
	bool ManagedWrapper::mterminate() {
		bool retval = TRUE;
		/*
		try {
            retval = (bool)__crt_dll_terminate();
		} catch(System::Exception* e) {
						Console::WriteLine(e->Message);
			retval = false;
		}*/
		return retval;
	}
}