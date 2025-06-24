// SMLIBNET.h

#pragma once


namespace PESMLIB
{
	public __gc class ManagedWrapper 
	{
	public:
		ManagedWrapper() { minitialize(); }
		~ManagedWrapper() { mterminate(); }
		static bool minitialize();
		static bool mterminate();
	};
}

