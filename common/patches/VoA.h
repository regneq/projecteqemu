#ifndef VoA_H_
#define VoA_H_

#include "../StructStrategy.h"

class EQStreamIdentifier;

namespace VoA {
	
	//these are the only public member of this namespace.
	extern void Register(EQStreamIdentifier &into);
	extern void Reload();
	
	
	
	//you should not directly access anything below.. 
	//I just dont feel like making a seperate header for it.
	
	class Strategy : public StructStrategy {
	public:
		Strategy();
		
	protected:
		
		virtual std::string Describe() const;
		
		//magic macro to declare our opcode processors
		#include "SSDeclare.h"
		#include "VoA_ops.h"
	};
	
};



#endif /*VoA_H_*/
