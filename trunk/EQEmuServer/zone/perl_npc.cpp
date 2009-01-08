/*
 * This file was generated automatically by xsubpp version 1.9508 from the
 * contents of tmp. Do not edit this file, edit tmp instead.
 *
 *		ANY CHANGES MADE HERE WILL BE LOST!
 *
 */


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

typedef const char Const_char;

#include "npc.h"

#ifdef THIS	 /* this macro seems to leak out on some systems */
#undef THIS		
#endif


XS(XS_NPC_SignalNPC); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SignalNPC)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SignalNPC(THIS, _signal_id)");
	{
		NPC *		THIS;
		int		_signal_id = (int)SvIV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SignalNPC(_signal_id);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_CheckNPCFactionAlly); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_CheckNPCFactionAlly)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::CheckNPCFactionAlly(THIS, other_faction)");
	{
		NPC *		THIS;
		FACTION_VALUE		RETVAL;
		dXSTARG;
		sint32		other_faction = (sint32)SvIV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->CheckNPCFactionAlly(other_faction);
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_AddItem); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_AddItem)
{
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: NPC::AddItem(THIS, itemid, charges, slot= 0)");
	{
		NPC *		THIS;
		int32		itemid = (int32)SvUV(ST(1));
		int8		charges = (int8)SvUV(ST(2));
		int8		slot;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (items < 4)
			slot = 0;
		else {
			slot = (int8)SvUV(ST(3));
		}

		THIS->AddItem(itemid, charges, slot);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_AddLootTable); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_AddLootTable)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::AddLootTable(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->AddLootTable();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_RemoveItem); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_RemoveItem)
{
	dXSARGS;
	if (items < 2 || items > 4)
		Perl_croak(aTHX_ "Usage: NPC::RemoveItem(THIS, item_id, quantity= 0, slot= 0)");
	{
		NPC *		THIS;
		uint32		item_id = (uint32)SvUV(ST(1));
		int16		quantity;
		int16		slot;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (items < 3)
			quantity = 0;
		else {
			quantity = (int16)SvUV(ST(2));
		}

		if (items < 4)
			slot = 0;
		else {
			slot = (int16)SvUV(ST(3));
		}

		THIS->RemoveItem(item_id, quantity, slot);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_ClearItemList); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_ClearItemList)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::ClearItemList(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->ClearItemList();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_AddCash); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_AddCash)
{
	dXSARGS;
	if (items != 5)
		Perl_croak(aTHX_ "Usage: NPC::AddCash(THIS, in_copper, in_silver, in_gold, in_platinum)");
	{
		NPC *		THIS;
		int16		in_copper = (int16)SvUV(ST(1));
		int16		in_silver = (int16)SvUV(ST(2));
		int16		in_gold = (int16)SvUV(ST(3));
		int16		in_platinum = (int16)SvUV(ST(4));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->AddCash(in_copper, in_silver, in_gold, in_platinum);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_RemoveCash); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_RemoveCash)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::RemoveCash(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->RemoveCash();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_CountLoot); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_CountLoot)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::CountLoot(THIS)");
	{
		NPC *		THIS;
		int32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->CountLoot();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetLoottableID); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetLoottableID)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetLoottableID(THIS)");
	{
		NPC *		THIS;
		int32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetLoottableID();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetCopper); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetCopper)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetCopper(THIS)");
	{
		NPC *		THIS;
		uint32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetCopper();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetSilver); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetSilver)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetSilver(THIS)");
	{
		NPC *		THIS;
		uint32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetSilver();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetGold); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetGold)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetGold(THIS)");
	{
		NPC *		THIS;
		uint32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetGold();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetPlatinum); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetPlatinum)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetPlatinum(THIS)");
	{
		NPC *		THIS;
		uint32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetPlatinum();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_SetCopper); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetCopper)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetCopper(THIS, amt)");
	{
		NPC *		THIS;
		uint32		amt = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetCopper(amt);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_SetSilver); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetSilver)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetSilver(THIS, amt)");
	{
		NPC *		THIS;
		uint32		amt = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetSilver(amt);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_SetGold); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetGold)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetGold(THIS, amt)");
	{
		NPC *		THIS;
		uint32		amt = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetGold(amt);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_SetPlatinum); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetPlatinum)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetPlatinum(THIS, amt)");
	{
		NPC *		THIS;
		uint32		amt = (uint32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetPlatinum(amt);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_SetGrid); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetGrid)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetGrid(THIS, grid_)");
	{
		NPC *		THIS;
		int16		grid_ = (int16)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetGrid(grid_);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_SetSp2); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetSp2)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetSp2(THIS, sg2)");
	{
		NPC *		THIS;
		int32		sg2 = (int32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetSp2(sg2);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_GetWaypointMax); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetWaypointMax)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetWaypointMax(THIS)");
	{
		NPC *		THIS;
		int16		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetWaypointMax();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetGrid); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetGrid)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetGrid(THIS)");
	{
		NPC *		THIS;
		sint16		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetGrid();
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetSp2); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetSp2)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetSp2(THIS)");
	{
		NPC *		THIS;
		int32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetSp2();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetNPCFactionID); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetNPCFactionID)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetNPCFactionID(THIS)");
	{
		NPC *		THIS;
		sint32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetNPCFactionID();
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetPrimaryFaction); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetPrimaryFaction)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetPrimaryFaction(THIS)");
	{
		NPC *		THIS;
		sint32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetPrimaryFaction();
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_GetNPCHate); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetNPCHate)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::GetNPCHate(THIS, in_ent)");
	{
		NPC *		THIS;
		sint32		RETVAL;
		dXSTARG;
		Mob*		in_ent;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV*)SvRV(ST(1)));
			in_ent = INT2PTR(Mob *,tmp);
		}
		else
			Perl_croak(aTHX_ "in_ent is not of type Mob");
		if(in_ent == NULL)
			Perl_croak(aTHX_ "in_ent is NULL, avoiding crash.");

		RETVAL = THIS->GetNPCHate(in_ent);
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_IsOnHatelist); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_IsOnHatelist)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::IsOnHatelist(THIS, p)");
	{
		NPC *		THIS;
		bool		RETVAL;
		Mob*		p;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV*)SvRV(ST(1)));
			p = INT2PTR(Mob *,tmp);
		}
		else
			Perl_croak(aTHX_ "p is not of type Mob");
		if(p == NULL)
			Perl_croak(aTHX_ "p is NULL, avoiding crash.");

		RETVAL = THIS->IsOnHatelist(p);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_NPC_SetNPCFactionID); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetNPCFactionID)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetNPCFactionID(THIS, in)");
	{
		NPC *		THIS;
		sint32		in = (sint32)SvIV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetNPCFactionID(in);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_GetMaxDMG); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetMaxDMG)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetMaxDMG(THIS)");
	{
		NPC *		THIS;
		int16		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetMaxDMG();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_IsAnimal); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_IsAnimal)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::IsAnimal(THIS)");
	{
		NPC *		THIS;
		bool		RETVAL;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->IsAnimal();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_NPC_GetPetSpellID); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetPetSpellID)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetPetSpellID(THIS)");
	{
		NPC *		THIS;
		int16		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetPetSpellID();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_SetPetSpellID); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetPetSpellID)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetPetSpellID(THIS, amt)");
	{
		NPC *		THIS;
		int16		amt = (int16)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetPetSpellID(amt);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_GetMaxDamage); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetMaxDamage)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::GetMaxDamage(THIS, tlevel)");
	{
		NPC *		THIS;
		int32		RETVAL;
		dXSTARG;
		int8		tlevel = (int8)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetMaxDamage(tlevel);
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_SetTaunting); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetTaunting)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::SetTaunting(THIS, tog)");
	{
		NPC *		THIS;
		bool		tog = (bool)SvTRUE(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetTaunting(tog);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_PickPocket); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_PickPocket)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::PickPocket(THIS, thief)");
	{
		NPC *		THIS;
		Client*		thief;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (sv_derived_from(ST(1), "Client")) {
			IV tmp = SvIV((SV*)SvRV(ST(1)));
			thief = INT2PTR(Client *,tmp);
		}
		else
			Perl_croak(aTHX_ "thief is not of type Client");
		if(thief == NULL)
			Perl_croak(aTHX_ "thief is NULL, avoiding crash.");

		THIS->PickPocket(thief);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_StartSwarmTimer); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_StartSwarmTimer)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::StartSwarmTimer(THIS, duration)");
	{
		NPC *		THIS;
		int32		duration = (int32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->StartSwarmTimer(duration);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_DoClassAttacks); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_DoClassAttacks)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::DoClassAttacks(THIS, target)");
	{
		NPC *		THIS;
		Mob *		target;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
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

		THIS->DoClassAttacks(target);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_GetMaxWp); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetMaxWp)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetMaxWp(THIS)");
	{
		NPC *		THIS;
		int		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetMaxWp();
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_NPC_DisplayWaypointInfo); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_DisplayWaypointInfo)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::DisplayWaypointInfo(THIS, to)");
	{
		NPC *		THIS;
		Client *		to;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (sv_derived_from(ST(1), "Client")) {
			IV tmp = SvIV((SV*)SvRV(ST(1)));
			to = INT2PTR(Client *,tmp);
		}
		else
			Perl_croak(aTHX_ "to is not of type Client");
		if(to == NULL)
			Perl_croak(aTHX_ "to is NULL, avoiding crash.");

		THIS->DisplayWaypointInfo(to);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_CalculateNewWaypoint); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_CalculateNewWaypoint)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::CalculateNewWaypoint(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->CalculateNewWaypoint();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_AssignWaypoints); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_AssignWaypoints)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::AssignWaypoints(THIS, grid)");
	{
		NPC *		THIS;
		int32		grid = (int32)SvUV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->AssignWaypoints(grid);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_SetWaypointPause); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SetWaypointPause)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::SetWaypointPause(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->SetWaypointPause();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_UpdateWaypoint); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_UpdateWaypoint)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::UpdateWaypoint(THIS, wp_index)");
	{
		NPC *		THIS;
		int		wp_index = (int)SvIV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->UpdateWaypoint(wp_index);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_StopWandering); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_StopWandering)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::StopWandering(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->StopWandering();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_ResumeWandering); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_ResumeWandering)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::ResumeWandering(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->ResumeWandering();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_PauseWandering); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_PauseWandering)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: NPC::PauseWandering(THIS, pausetime)");
	{
		NPC *		THIS;
		int		pausetime = (int)SvIV(ST(1));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->PauseWandering(pausetime);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_MoveTo); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_MoveTo)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: NPC::MoveTo(THIS, mtx, mty, mtz)");
	{
		NPC *		THIS;
		float		mtx = (float)SvNV(ST(1));
		float		mty = (float)SvNV(ST(2));
		float		mtz = (float)SvNV(ST(3));

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->MoveTo(mtx, mty, mtz);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_NextGuardPosition); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_NextGuardPosition)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::NextGuardPosition(THIS)");
	{
		NPC *		THIS;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->NextGuardPosition();
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_SaveGuardSpot); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_SaveGuardSpot)
{
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: NPC::SaveGuardSpot(THIS, iClearGuardSpot= false)");
	{
		NPC *		THIS;
		bool		iClearGuardSpot;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (items < 2)
			iClearGuardSpot = false;
		else {
			iClearGuardSpot = (bool)SvTRUE(ST(1));
		}

		THIS->SaveGuardSpot(iClearGuardSpot);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_IsGuarding); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_IsGuarding)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::IsGuarding(THIS)");
	{
		NPC *		THIS;
		bool		RETVAL;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->IsGuarding();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_NPC_AI_SetRoambox); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_AI_SetRoambox)
{
	dXSARGS;
	if (items < 6 || items > 7)
		Perl_croak(aTHX_ "Usage: NPC::AI_SetRoambox(THIS, iDist, iMaxX, iMinX, iMaxY, iMinY, iDelay= 2500)");
	{
		NPC *		THIS;
		float		iDist = (float)SvNV(ST(1));
		float		iMaxX = (float)SvNV(ST(2));
		float		iMinX = (float)SvNV(ST(3));
		float		iMaxY = (float)SvNV(ST(4));
		float		iMinY = (float)SvNV(ST(5));
		int32		iDelay;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		if (items < 7)
			iDelay = 2500;
		else {
			iDelay = (int32)SvUV(ST(6));
		}

		THIS->AI_SetRoambox(iDist, iMaxX, iMinX, iMaxY, iMinY, iDelay);
	}
	XSRETURN_EMPTY;
}

XS(XS_NPC_GetNPCSpellsID); /* prototype to pass -Wmissing-prototypes */
XS(XS_NPC_GetNPCSpellsID)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::GetNPCSpellsID(THIS)");
	{
		NPC *		THIS;
		int32		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "NPC")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(NPC *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type NPC");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetNPCSpellsID();
		XSprePUSH; PUSHu((UV)RETVAL);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_NPC); /* prototype to pass -Wmissing-prototypes */
XS(boot_NPC)
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

		newXSproto(strcpy(buf, "SignalNPC"), XS_NPC_SignalNPC, file, "$$");
		newXSproto(strcpy(buf, "CheckNPCFactionAlly"), XS_NPC_CheckNPCFactionAlly, file, "$$");
		newXSproto(strcpy(buf, "AddItem"), XS_NPC_AddItem, file, "$$$;$");
		newXSproto(strcpy(buf, "AddLootTable"), XS_NPC_AddLootTable, file, "$");
		newXSproto(strcpy(buf, "RemoveItem"), XS_NPC_RemoveItem, file, "$$;$$");
		newXSproto(strcpy(buf, "ClearItemList"), XS_NPC_ClearItemList, file, "$");
		newXSproto(strcpy(buf, "AddCash"), XS_NPC_AddCash, file, "$$$$$");
		newXSproto(strcpy(buf, "RemoveCash"), XS_NPC_RemoveCash, file, "$");
		newXSproto(strcpy(buf, "CountLoot"), XS_NPC_CountLoot, file, "$");
		newXSproto(strcpy(buf, "GetLoottableID"), XS_NPC_GetLoottableID, file, "$");
		newXSproto(strcpy(buf, "GetCopper"), XS_NPC_GetCopper, file, "$");
		newXSproto(strcpy(buf, "GetSilver"), XS_NPC_GetSilver, file, "$");
		newXSproto(strcpy(buf, "GetGold"), XS_NPC_GetGold, file, "$");
		newXSproto(strcpy(buf, "GetPlatinum"), XS_NPC_GetPlatinum, file, "$");
		newXSproto(strcpy(buf, "SetCopper"), XS_NPC_SetCopper, file, "$$");
		newXSproto(strcpy(buf, "SetSilver"), XS_NPC_SetSilver, file, "$$");
		newXSproto(strcpy(buf, "SetGold"), XS_NPC_SetGold, file, "$$");
		newXSproto(strcpy(buf, "SetPlatinum"), XS_NPC_SetPlatinum, file, "$$");
		newXSproto(strcpy(buf, "SetGrid"), XS_NPC_SetGrid, file, "$$");
		newXSproto(strcpy(buf, "SetSp2"), XS_NPC_SetSp2, file, "$$");
		newXSproto(strcpy(buf, "GetWaypointMax"), XS_NPC_GetWaypointMax, file, "$");
		newXSproto(strcpy(buf, "GetGrid"), XS_NPC_GetGrid, file, "$");
		newXSproto(strcpy(buf, "GetSp2"), XS_NPC_GetSp2, file, "$");
		newXSproto(strcpy(buf, "GetNPCFactionID"), XS_NPC_GetNPCFactionID, file, "$");
		newXSproto(strcpy(buf, "GetPrimaryFaction"), XS_NPC_GetPrimaryFaction, file, "$");
		newXSproto(strcpy(buf, "GetNPCHate"), XS_NPC_GetNPCHate, file, "$$");
		newXSproto(strcpy(buf, "IsOnHatelist"), XS_NPC_IsOnHatelist, file, "$$");
		newXSproto(strcpy(buf, "SetNPCFactionID"), XS_NPC_SetNPCFactionID, file, "$$");
		newXSproto(strcpy(buf, "GetMaxDMG"), XS_NPC_GetMaxDMG, file, "$");
		newXSproto(strcpy(buf, "IsAnimal"), XS_NPC_IsAnimal, file, "$");
		newXSproto(strcpy(buf, "GetPetSpellID"), XS_NPC_GetPetSpellID, file, "$");
		newXSproto(strcpy(buf, "SetPetSpellID"), XS_NPC_SetPetSpellID, file, "$$");
		newXSproto(strcpy(buf, "GetMaxDamage"), XS_NPC_GetMaxDamage, file, "$$");
		newXSproto(strcpy(buf, "SetTaunting"), XS_NPC_SetTaunting, file, "$$");
		newXSproto(strcpy(buf, "PickPocket"), XS_NPC_PickPocket, file, "$$");
		newXSproto(strcpy(buf, "StartSwarmTimer"), XS_NPC_StartSwarmTimer, file, "$$");
		newXSproto(strcpy(buf, "DoClassAttacks"), XS_NPC_DoClassAttacks, file, "$$");
		newXSproto(strcpy(buf, "GetMaxWp"), XS_NPC_GetMaxWp, file, "$");
		newXSproto(strcpy(buf, "DisplayWaypointInfo"), XS_NPC_DisplayWaypointInfo, file, "$$");
		newXSproto(strcpy(buf, "CalculateNewWaypoint"), XS_NPC_CalculateNewWaypoint, file, "$");
		newXSproto(strcpy(buf, "AssignWaypoints"), XS_NPC_AssignWaypoints, file, "$$");
		newXSproto(strcpy(buf, "SetWaypointPause"), XS_NPC_SetWaypointPause, file, "$");
		newXSproto(strcpy(buf, "UpdateWaypoint"), XS_NPC_UpdateWaypoint, file, "$$");
		newXSproto(strcpy(buf, "StopWandering"), XS_NPC_StopWandering, file, "$");
		newXSproto(strcpy(buf, "ResumeWandering"), XS_NPC_ResumeWandering, file, "$");
		newXSproto(strcpy(buf, "PauseWandering"), XS_NPC_PauseWandering, file, "$$");
		newXSproto(strcpy(buf, "MoveTo"), XS_NPC_MoveTo, file, "$$$$");
		newXSproto(strcpy(buf, "NextGuardPosition"), XS_NPC_NextGuardPosition, file, "$");
		newXSproto(strcpy(buf, "SaveGuardSpot"), XS_NPC_SaveGuardSpot, file, "$;$");
		newXSproto(strcpy(buf, "IsGuarding"), XS_NPC_IsGuarding, file, "$");
		newXSproto(strcpy(buf, "AI_SetRoambox"), XS_NPC_AI_SetRoambox, file, "$$$$$$;$");
		newXSproto(strcpy(buf, "GetNPCSpellsID"), XS_NPC_GetNPCSpellsID, file, "$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES

