/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2004  EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
		are required to give you total support for your newly bought product;
		without even the implied warranty of MERCHANTABILITY or FITNESS FOR
		A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "features.h"
#ifdef EMBPERL_XS_CLASSES
#include "../common/debug.h"
#include "embperl.h"

#include <string>
#include "mob.h"
#include "spells.h"
#include "buff.h"

#ifdef THIS		/* this macro seems to leak out on some systems */
#undef THIS
#endif

XS(XS_Spell_GetSpellID);
XS(XS_Spell_GetSpellID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellID(THIS)");
	{
		Spell *             THIS;
		bool                RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetSpellID();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetCaster); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetCaster)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetCaster(THIS, Caster)");
	{
		Spell *         THIS;
		Mob *           caster;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");
			
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV*)SvRV(ST(1)));
			caster = INT2PTR(Mob *,tmp);
		}
		else
			Perl_croak(aTHX_ "caster is not of type Mob");
		if(caster == NULL)
			Perl_croak(aTHX_ "caster is NULL, avoiding crash.");			

		THIS->SetCaster(caster);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetCasterID);
XS(XS_Spell_GetCasterID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCasterID(THIS)");
	{
		Spell *             THIS;
		uint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetCasterID();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetTarget); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetTarget)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetTarget(THIS, Caster)");
	{
		Spell *         THIS;
		Mob *           target;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");
			
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV*)SvRV(ST(1)));
			target = INT2PTR(Mob *,tmp);
		}
		else
			Perl_croak(aTHX_ "target is not of type Mob");
		if(target == NULL)
			Perl_croak(aTHX_ "target is NULL, avoiding crash.");			

		THIS->SetTarget(target);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetTargetID);
XS(XS_Spell_GetTargetID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTargetID(THIS)");
	{
		Spell *             THIS;
		uint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetTargetID();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}


XS(XS_Spell_SetSpellSlot); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetSpellSlot)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetSpellSlot(THIS, Slot)");
	{
		Spell *         THIS;
		sint32          slot = (sint32)SvIV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetSpellSlot(slot);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetSpellSlot);
XS(XS_Spell_GetSpellSlot) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellSlot(THIS)");
	{
		Spell *             THIS;
		sint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetSpellSlot();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetManaCost); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetManaCost)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetManaCost(THIS, Mana)");
	{
		Spell *         THIS;
		sint32          mana = (sint32)SvIV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetManaCost(mana);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetManaCost);
XS(XS_Spell_GetManaCost) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetManaCost(THIS)");
	{
		Spell *             THIS;
		sint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetManaCost();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetCastTime); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetCastTime)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetCastTime(THIS, CastTime)");
	{
		Spell *         THIS;
		sint32          cast_time = (sint32)SvIV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetCastTime(cast_time);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetCastTime);
XS(XS_Spell_GetCastTime) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCastTime(THIS)");
	{
		Spell *             THIS;
		sint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetCastTime();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetInventorySpellSlot); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetInventorySpellSlot)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetInventorySpellSlot(THIS, InventorySpellSlot)");
	{
		Spell *         THIS;
		uint32          inv_slot = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetInventorySpellSlot(inv_slot);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetInventorySpellSlot);
XS(XS_Spell_GetInventorySpellSlot) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetInventorySpellSlot(THIS)");
	{
		Spell *             THIS;
		uint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetInventorySpellSlot();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetTimerID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetTimerID)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetTimerID(THIS, TimerID)");
	{
		Spell *         THIS;
		uint32          timer = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetTimerID(timer);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetTimerID);
XS(XS_Spell_GetTimerID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTimerID(THIS)");
	{
		Spell *             THIS;
		uint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetTimerID();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetTimerIDDuration); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetTimerIDDuration)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetTimerIDDuration(THIS, duration)");
	{
		Spell *         THIS;
		uint32          duration = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetTimerIDDuration(duration);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetTimerIDDuration);
XS(XS_Spell_GetTimerIDDuration) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTimerIDDuration(THIS)");
	{
		Spell *             THIS;
		uint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetTimerIDDuration();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetSpellType); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetSpellType)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetSpellType(THIS, type)");
	{
		Spell *         THIS;
		uint32          type = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetSpellType((SpellClass)type);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetSpellType);
XS(XS_Spell_GetSpellType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellType(THIS)");
	{
		Spell *             THIS;
		uint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetSpellType();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_SetCasterLevel); /* prototype to pass -Wmissing-prototypes */
XS(XS_Spell_SetCasterLevel)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::SetCasterLevel(THIS, level)");
	{
		Spell *         THIS;
		uint32          level = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");			

		THIS->SetCasterLevel(level);
	}
	XSRETURN_EMPTY;
}

XS(XS_Spell_GetCasterLevel);
XS(XS_Spell_GetCasterLevel) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCasterLevel(THIS)");
	{
		Spell *             THIS;
		uint32              RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetCasterLevel();
		XSprePUSH; 
		PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Spell_IsCustomSpell);
XS(XS_Spell_IsCustomSpell) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::IsCustomSpell(THIS)");
	{
		Spell *             THIS;
		bool                RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->IsCustomSpell();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_CopySpell);
XS(XS_Spell_CopySpell) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::CopySpell(THIS)");
	{
		Spell *             THIS;
		Spell *             RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->CopySpell();
		sv_setref_pv(ST(0), "Spell", (void*)RETVAL);
		XPUSHs(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSpellAttribute);
XS(XS_Spell_GetSpellAttribute) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellAttribute(THIS, field)");
	{
		Spell *             THIS;
		const char *        field = (const char *)SvPV_nolen(ST(1));
		std::string         RETSTR;
		const char *        RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETSTR = THIS->GetSpellAttribute(field);
		RETVAL = RETSTR.c_str();
		sv_setpv(TARG, RETVAL); 
		XSprePUSH; 
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_SetSpellAttribute);
XS(XS_Spell_SetSpellAttribute) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Spell::SetSpellAttribute(THIS, field, attribute)");
	{
		Spell *             THIS;
		const char *        field = (const char *)SvPV_nolen(ST(1));
		const char *        attribute = (const char *)SvPV_nolen(ST(2));
		dXSTARG;

		if (sv_derived_from(ST(0), "Spell")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(Spell *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type Spell");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetSpellAttribute(attribute, field);
	}
	XSRETURN_EMPTY;
}

#ifdef __cplusplus
extern "C"
#endif

XS(boot_Spell);
XS(boot_Spell) 
{
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if(items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");
	char buf[128];

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK ;

		newXSproto(strcpy(buf, "GetSpellID"), XS_Spell_GetSpellID, file, "$");
		newXSproto(strcpy(buf, "GetCasterID"), XS_Spell_GetCasterID, file, "$");
		newXSproto(strcpy(buf, "SetCaster"), XS_Spell_SetCaster, file, "$$");
		newXSproto(strcpy(buf, "GetTargetID"), XS_Spell_GetTargetID, file, "$");
		newXSproto(strcpy(buf, "SetTarget"), XS_Spell_SetTarget, file, "$$");
		newXSproto(strcpy(buf, "GetSpellSlot"), XS_Spell_GetSpellSlot, file, "$");
		newXSproto(strcpy(buf, "SetSpellSlot"), XS_Spell_SetSpellSlot, file, "$$");
		newXSproto(strcpy(buf, "GetManaCost"), XS_Spell_GetManaCost, file, "$");
		newXSproto(strcpy(buf, "SetManaCost"), XS_Spell_SetManaCost, file, "$$");
		newXSproto(strcpy(buf, "GetCastTime"), XS_Spell_GetCastTime, file, "$");
		newXSproto(strcpy(buf, "SetCastTime"), XS_Spell_SetCastTime, file, "$$");
		newXSproto(strcpy(buf, "GetInventorySpellSlot"), XS_Spell_GetInventorySpellSlot, file, "$");
		newXSproto(strcpy(buf, "SetInventorySpellSlot"), XS_Spell_SetInventorySpellSlot, file, "$$");
		newXSproto(strcpy(buf, "GetTimerID"), XS_Spell_GetTimerID, file, "$");
		newXSproto(strcpy(buf, "SetTimerID"), XS_Spell_SetTimerID, file, "$$");
		newXSproto(strcpy(buf, "GetTimerIDDuration"), XS_Spell_GetTimerIDDuration, file, "$");
		newXSproto(strcpy(buf, "SetTimerIDDuration"), XS_Spell_SetTimerIDDuration, file, "$$");
		newXSproto(strcpy(buf, "GetSpellType"), XS_Spell_GetSpellType, file, "$");
		newXSproto(strcpy(buf, "SetSpellType"), XS_Spell_SetSpellType, file, "$$");
		newXSproto(strcpy(buf, "GetCasterLevel"), XS_Spell_GetCasterLevel, file, "$");
		newXSproto(strcpy(buf, "SetCasterLevel"), XS_Spell_SetCasterLevel, file, "$$");
		newXSproto(strcpy(buf, "IsCustomSpell"), XS_Spell_IsCustomSpell, file, "$");
		newXSproto(strcpy(buf, "CopySpell"), XS_Spell_CopySpell, file, "$");
		newXSproto(strcpy(buf, "GetSpellAttribute"), XS_Spell_GetSpellAttribute, file, "$$");
		newXSproto(strcpy(buf, "SetSpellAttribute"), XS_Spell_SetSpellAttribute, file, "$$$");

	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES