// SMLIBNET.h

#pragma once


namespace PESMLIB
{
	public ref class ManagedWrapper 
	{
	public:
		ManagedWrapper() { minitialize(); }
		~ManagedWrapper() { mterminate(); }
		static bool minitialize();
		static bool mterminate();
	};
}

