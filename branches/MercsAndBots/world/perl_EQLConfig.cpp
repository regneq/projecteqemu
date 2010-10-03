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

typedef const char Const_char;

#ifdef EMBPERL
#include "../common/debug.h"
#include "EQWParser.h"
#include "EQLConfig.h"

#ifdef THIS	 /* this macro seems to leak out on some systems */
#undef THIS		
#endif


XS(XS_EQLConfig_GetName); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_GetName)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: EQLConfig::GetName(THIS)");
	{
		EQLConfig *		THIS;
		Const_char *		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetName();
		sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_GetStaticCount); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_GetStaticCount)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: EQLConfig::GetStaticCount(THIS)");
	{
		EQLConfig *		THIS;
		int		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetStaticCount();
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_IsConnected); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_IsConnected)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: EQLConfig::IsConnected(THIS)");
	{
		EQLConfig *		THIS;
		bool		RETVAL;

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->IsConnected();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_DeleteLauncher); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_DeleteLauncher)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: EQLConfig::DeleteLauncher(THIS)");
	{
		EQLConfig *		THIS;

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->DeleteLauncher();
	}
	XSRETURN_EMPTY;
}

XS(XS_EQLConfig_RestartZone); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_RestartZone)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: EQLConfig::RestartZone(THIS, zone_ref)");
	{
		EQLConfig *		THIS;
		Const_char *		zone_ref = (Const_char *)SvPV_nolen(ST(1));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->RestartZone(zone_ref);
	}
	XSRETURN_EMPTY;
}

XS(XS_EQLConfig_StopZone); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_StopZone)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: EQLConfig::StopZone(THIS, zone_ref)");
	{
		EQLConfig *		THIS;
		Const_char *		zone_ref = (Const_char *)SvPV_nolen(ST(1));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->StopZone(zone_ref);
	}
	XSRETURN_EMPTY;
}

XS(XS_EQLConfig_StartZone); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_StartZone)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: EQLConfig::StartZone(THIS, zone_ref)");
	{
		EQLConfig *		THIS;
		Const_char *		zone_ref = (Const_char *)SvPV_nolen(ST(1));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		THIS->StartZone(zone_ref);
	}
	XSRETURN_EMPTY;
}

XS(XS_EQLConfig_BootStaticZone); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_BootStaticZone)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: EQLConfig::BootStaticZone(THIS, short_name, port)");
	{
		EQLConfig *		THIS;
		bool		RETVAL;
		Const_char *		short_name = (Const_char *)SvPV_nolen(ST(1));
		uint16		port = (uint16)SvUV(ST(2));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->BootStaticZone(short_name, port);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_ChangeStaticZone); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_ChangeStaticZone)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: EQLConfig::ChangeStaticZone(THIS, short_name, port)");
	{
		EQLConfig *		THIS;
		bool		RETVAL;
		Const_char *		short_name = (Const_char *)SvPV_nolen(ST(1));
		uint16		port = (uint16)SvUV(ST(2));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->ChangeStaticZone(short_name, port);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_DeleteStaticZone); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_DeleteStaticZone)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: EQLConfig::DeleteStaticZone(THIS, short_name)");
	{
		EQLConfig *		THIS;
		bool		RETVAL;
		Const_char *		short_name = (Const_char *)SvPV_nolen(ST(1));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->DeleteStaticZone(short_name);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_SetDynamicCount); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_SetDynamicCount)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: EQLConfig::SetDynamicCount(THIS, count)");
	{
		EQLConfig *		THIS;
		bool		RETVAL;
		int		count = (int)SvIV(ST(1));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->SetDynamicCount(count);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_GetDynamicCount); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_GetDynamicCount)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: EQLConfig::GetDynamicCount(THIS)");
	{
		EQLConfig *		THIS;
		int		RETVAL;
		dXSTARG;

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetDynamicCount();
		XSprePUSH; PUSHi((IV)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_ListZones); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_ListZones)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: EQLConfig::ListZones(THIS)");
	{
		EQLConfig *		THIS;
		vector<string>		RETVAL;

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->ListZones();
		ST(0) = sv_newmortal();
	{
			U32 ix_RETVAL;
			/* pop crap off the stack we dont really want */
			POPs;
			POPs;
			/* grow the stack to the number of elements being returned */
			EXTEND(SP, RETVAL.size());
			for (ix_RETVAL = 0; ix_RETVAL < RETVAL.size(); ix_RETVAL++) {
					const string &it = RETVAL[ix_RETVAL];
					ST(ix_RETVAL) = sv_newmortal();
					sv_setpvn(ST(ix_RETVAL), it.c_str(), it.length());
			}
			/* hackish, but im over it. The normal xsubpp return will be right below this */
			XSRETURN(RETVAL.size());
	}
	}
	XSRETURN(1);
}

XS(XS_EQLConfig_GetZoneDetails); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQLConfig_GetZoneDetails)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: EQLConfig::GetZoneDetails(THIS, zone_ref)");
	{
		EQLConfig *		THIS;
		map<string,string>		RETVAL;
		Const_char *		zone_ref = (Const_char *)SvPV_nolen(ST(1));

		if (sv_derived_from(ST(0), "EQLConfig")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			THIS = INT2PTR(EQLConfig *,tmp);
		}
		else
			Perl_croak(aTHX_ "THIS is not of type EQLConfig");
		if(THIS == NULL)
			Perl_croak(aTHX_ "THIS is NULL, avoiding crash.");

		RETVAL = THIS->GetZoneDetails(zone_ref);
		ST(0) = sv_newmortal();
		if (RETVAL.begin()!=RETVAL.end())
		{		
				//NOTE: we are leaking the original ST(0) right now
				HV *hv = newHV();
				sv_2mortal((SV*)hv);
				ST(0) = newRV((SV*)hv);
				
				map<string,string>::const_iterator cur, end;
				cur = RETVAL.begin();
				end = RETVAL.end();
				for(; cur != end; cur++) {
						/* get the element from the hash, creating if needed (will be needed) */
						SV**ele = hv_fetch(hv, cur->first.c_str(), cur->first.length(), TRUE);
						if(ele == NULL) {
								Perl_croak(aTHX_ "Unable to create a hash element for RETVAL");
								break;
						}
						/* put our string in the SV associated with this element in the hash */
						sv_setpvn(*ele, cur->second.c_str(), cur->second.length());
				}
		}

















	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_EQLConfig); /* prototype to pass -Wmissing-prototypes */
XS(boot_EQLConfig)
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

		newXSproto(strcpy(buf, "GetName"), XS_EQLConfig_GetName, file, "$");
		newXSproto(strcpy(buf, "GetStaticCount"), XS_EQLConfig_GetStaticCount, file, "$");
		newXSproto(strcpy(buf, "IsConnected"), XS_EQLConfig_IsConnected, file, "$");
		newXSproto(strcpy(buf, "DeleteLauncher"), XS_EQLConfig_DeleteLauncher, file, "$");
		newXSproto(strcpy(buf, "RestartZone"), XS_EQLConfig_RestartZone, file, "$$");
		newXSproto(strcpy(buf, "StopZone"), XS_EQLConfig_StopZone, file, "$$");
		newXSproto(strcpy(buf, "StartZone"), XS_EQLConfig_StartZone, file, "$$");
		newXSproto(strcpy(buf, "BootStaticZone"), XS_EQLConfig_BootStaticZone, file, "$$$");
		newXSproto(strcpy(buf, "ChangeStaticZone"), XS_EQLConfig_ChangeStaticZone, file, "$$$");
		newXSproto(strcpy(buf, "DeleteStaticZone"), XS_EQLConfig_DeleteStaticZone, file, "$$");
		newXSproto(strcpy(buf, "SetDynamicCount"), XS_EQLConfig_SetDynamicCount, file, "$$");
		newXSproto(strcpy(buf, "GetDynamicCount"), XS_EQLConfig_GetDynamicCount, file, "$");
		newXSproto(strcpy(buf, "ListZones"), XS_EQLConfig_ListZones, file, "$");
		newXSproto(strcpy(buf, "GetZoneDetails"), XS_EQLConfig_GetZoneDetails, file, "$$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES

