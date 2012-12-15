
#include "../debug.h"
#include "patches.h"

#include "Client62.h"
#include "Titanium.h"
#include "Anniversary.h"
#include "Underfoot.h"
#include "SoF.h"
#include "SoD.h"
#include "HoT.h"
#include "VoA.h"
#include "RoF.h"

void RegisterAllPatches(EQStreamIdentifier &into) {
	Client62::Register(into);
	Titanium::Register(into);
	SoF::Register(into);
	SoD::Register(into);
	Underfoot::Register(into);
	//HoT::Register(into);
	//VoA::Register(into);
	//RoF::Register(into);
}

void ReloadAllPatches() {
	Client62::Reload();
	Titanium::Reload();
	SoF::Reload();
	SoD::Reload();
	Underfoot::Reload();
	//HoT::Reload();
	//VoA::Reload();
	//RoF::Reload();
}
