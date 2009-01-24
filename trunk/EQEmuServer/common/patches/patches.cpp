
#include "../debug.h"
#include "patches.h"

#include "Client62.h"
#include "Titanium.h"
#include "Anniversary.h"
#include "Live.h"
#include "SoF.h"

//Uncomment the line below to allow SoF Clients
//#define SoF_Enable

void RegisterAllPatches(EQStreamIdentifier &into) {
	Client62::Register(into);
	Titanium::Register(into);
	Anniversary::Register(into);
	Live::Register(into);
#ifdef SoF_Enable
	SoF::Register(into);
#endif
}

void ReloadAllPatches() {
	Client62::Reload();
	Titanium::Reload();
	Anniversary::Reload();
	Live::Reload();
#ifdef SoF_Enable
	SoF::Reload();
#endif
}



















