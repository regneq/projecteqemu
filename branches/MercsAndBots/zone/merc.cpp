#include "merc.h"
#include "masterentity.h"
#include "NpcAI.h"
#include "../common/packet_dump.h"
#include "../common/eq_packet_structs.h"
#include "../common/eq_constants.h"
#include "../common/skills.h"
#include "spdat.h"
#include "zone.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/rulesys.h"
#include "QuestParserCollection.h"
#include "watermap.h"

Merc::Merc(const NPCType* d, float x, float y, float z, float heading) 
	: Mob(d->name,
	  d->lastname,
	  d->max_hp,
	  d->max_hp,
	  d->gender,
	  d->race,
	  d->class_,
      (bodyType)d->bodytype,
	  d->deity,
	  d->level,
	  d->npc_id,
	  d->size,
	  d->runspeed,
	  heading,
	  x,
	  y,
	  z,
	  d->light,
	  d->texture,
	  d->helmtexture,
	  d->AC,
	  d->ATK,
	  d->STR,
	  d->STA,
	  d->DEX,
	  d->AGI,
	  d->INT,
	  d->WIS,
	  d->CHA,
	  d->haircolor,
	  d->beardcolor,
	  d->eyecolor1,
	  d->eyecolor2,
	  d->hairstyle,
	  d->luclinface,
	  d->beard,
	  d->drakkin_heritage,
	  d->drakkin_tattoo,
	  d->drakkin_details,
	  (int32*)d->armor_tint,
	  0,
	  d->see_invis,			// pass see_invis/see_ivu flags to mob constructor
	  d->see_invis_undead,
	  d->see_hide,
	  d->see_improved_hide,
	  d->hp_regen,
	  d->mana_regen,
	  d->qglobal,
	  d->maxlevel,
	  d->scalerate) 
{
	int r;
	for(r = 0; r <= HIGHEST_SKILL; r++) {
		skills[r] = database.GetSkillCap(GetClass(),(SkillType)r,GetLevel());
	}

	AI_Init();
	AI_Start();
}

Merc::~Merc() {
	entity_list.RemoveMerc(this->GetID());
	UninitializeBuffSlots();
}

void Merc::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	if(ns) {
		Mob::FillSpawnStruct(ns, ForWho);

		ns->spawn.afk = 0;
		ns->spawn.lfg = 0;
		ns->spawn.anon = 0;
		ns->spawn.gm = 0;
		ns->spawn.guildID = 0xFFFFFFFF;		// 0xFFFFFFFF = NO GUILD, 0 = Unknown Guild
		ns->spawn.is_npc = 1;				// 0=no, 1=yes
		ns->spawn.is_pet = 0;
		ns->spawn.guildrank = 0;
		ns->spawn.showhelm = 1;
		ns->spawn.flymode = 0;
		ns->spawn.size = 0;
		ns->spawn.NPC = 1;					// 0=player,1=npc,2=pc corpse,3=npc corpse
		ns->spawn.IsMercenary = 1;
		/*const Item_Struct* item = 0;
		const ItemInst* inst = 0;

		uint32 spawnedmercid = 0;
		spawnedmercid = this->GetID();

		inst = GetBotItem(SLOT_HANDS);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_HANDS]	= item->Material;
				ns->spawn.colors[MATERIAL_HANDS].color = GetEquipmentColor(MATERIAL_HANDS);
			}
		}

		inst = GetBotItem(SLOT_HEAD);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_HEAD] = item->Material;
				ns->spawn.colors[MATERIAL_HEAD].color = GetEquipmentColor(MATERIAL_HEAD);
			}
		}

		inst = GetBotItem(SLOT_ARMS);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_ARMS] = item->Material;
				ns->spawn.colors[MATERIAL_ARMS].color = GetEquipmentColor(MATERIAL_ARMS);
			}
		}

		inst = GetBotItem(SLOT_BRACER01);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_BRACER] = item->Material;
				ns->spawn.colors[MATERIAL_BRACER].color	= GetEquipmentColor(MATERIAL_BRACER);
			}
		}

		inst = GetBotItem(SLOT_BRACER02);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_BRACER] = item->Material;
				ns->spawn.colors[MATERIAL_BRACER].color	= GetEquipmentColor(MATERIAL_BRACER);
			}
		}

		inst = GetBotItem(SLOT_CHEST);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_CHEST]	= item->Material;
				ns->spawn.colors[MATERIAL_CHEST].color = GetEquipmentColor(MATERIAL_CHEST);
			}
		}

		inst = GetBotItem(SLOT_LEGS);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_LEGS] = item->Material;
				ns->spawn.colors[MATERIAL_LEGS].color = GetEquipmentColor(MATERIAL_LEGS);
			}
		}

		inst = GetBotItem(SLOT_FEET);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				ns->spawn.equipment[MATERIAL_FEET] = item->Material;
				ns->spawn.colors[MATERIAL_FEET].color = GetEquipmentColor(MATERIAL_FEET);
			}
		}
		
		inst = GetBotItem(SLOT_PRIMARY);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment[MATERIAL_PRIMARY] = atoi(&item->IDFile[2]);
					ns->spawn.colors[MATERIAL_PRIMARY].color = GetEquipmentColor(MATERIAL_PRIMARY);
			}
		}

		inst = GetBotItem(SLOT_SECONDARY);
		if(inst) {
			item = inst->GetItem();
			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment[MATERIAL_SECONDARY] = atoi(&item->IDFile[2]);
					ns->spawn.colors[MATERIAL_SECONDARY].color = GetEquipmentColor(MATERIAL_SECONDARY);
			}
		}*/
	}
}

bool Merc::Process()
{
	_ZP(Merc_Process);
	
	if(IsStunned() && stunned_timer.Check())
	{
		this->stunned = false;
		this->stunned_timer.Disable();
	}

	if(!GetMercOwner())
		return false;

	if (GetDepop()) {
		return false;
	}

	SpellProcess();

	if(tic_timer.Check())
	{
		//6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if(!IsMoving() && !IsEngaged())
		{
			SendPosition();
		}

		BuffProcess();
	}

	if (IsStunned() || IsMezzed())
		return true;

	// Merc AI
	AI_Process();

	return true;
}

void Merc::AI_Process() {
	if(!IsAIControlled())
		return;

	if(IsCasting())
		return;

	// A bot wont start its AI if not grouped
	if(!GetOwner() || !IsGrouped()) {
		return;
	}

	if(GetAppearance() == eaDead)
		return;

	Mob* MercOwner = GetOwner();

	// The bots need an owner
	if(!MercOwner)
		return;

	try {
		if(MercOwner->CastToClient()->IsDead()) {
			SetTarget(0);
			SetOwnerID(0);
			return;
		}
	}
	catch(...) {
		SetTarget(0);
		SetOwnerID(0);
		return;
	}

	if(!IsEngaged()) {
		if(GetFollowID()) {
			if(MercOwner && MercOwner->CastToClient()->AutoAttackEnabled() && MercOwner->GetTarget() &&
				MercOwner->GetTarget()->IsNPC() && MercOwner->GetTarget()->GetHateAmount(MercOwner)) {
					AddToHateList(MercOwner->GetTarget(), 1);
			}
			else {
				Group* g = GetGroup();

				if(g) {
					for(int counter = 0; counter < g->GroupCount(); counter++) {
						if(g->members[counter]) {
							if(g->members[counter]->IsEngaged() && g->members[counter]->GetTarget()) {
								AddToHateList(g->members[counter]->GetTarget(), 1);
								break;
							}
						}
					}
				}
			}
		}
	}

	if(!IsEngaged()) {
		// Not engaged in combat
		SetTarget(0);

		/*if(!IsMoving() && AIthink_timer->Check() && !spellend_timer.Enabled()) {
			if(GetStance() != MercStancePassive) {
				if(!AI_IdleCastCheck() && !IsCasting())
					Meditate(true);
			}
			else {
				Meditate(true);
			}

		}*/

		if(AImovement_timer->Check()) {
			if(GetFollowID()) {
				Mob* follow = entity_list.GetMob(GetFollowID());

				if(follow) {
					float dist = DistNoRoot(*follow);
					float speed = follow->GetRunspeed();

					if(dist < GetFollowDistance() + 1000) 
						speed = follow->GetWalkspeed();

					SetRunAnimSpeed(0);

					if(dist > GetFollowDistance()) {
						CalculateNewPosition2(follow->GetX(), follow->GetY(), follow->GetZ(), speed);
						//if(rest_timer.Enabled())
						//	rest_timer.Disable();
						return;
					} 
					else
					{						
						if(moved)
						{
							moved=false;
							SendPosition();
							SetMoving(false);
						}
					}
				}
			}
		}
	}
}

void Merc::AI_Init() {
	Mob::AI_Init();
}

void Merc::AI_Start(int32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);

	if (!pAIControlled)
		return;
	
	SetAttackTimer();
}

void Merc::AI_Stop() {
	Mob::AI_Stop();
}

bool Merc::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell)
{

	_ZP(Client_Attack);

	if (!other) {
		SetTarget(NULL);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to Client::Attack() for evaluation!");
		return false;
	}
	
	if(!GetTarget())
		SetTarget(other);
	
	mlog(COMBAT__ATTACKS, "Attacking %s with hand %d %s", other?other->GetName():"(NULL)", Hand, bRiposte?"(this is a riposte)":"");
	
	//SetAttackTimer();
	if (
		   (IsCasting() && GetClass() != BARD && !IsFromSpell)
		|| other == NULL
		|| (other->IsClient() && other->CastToClient()->IsDead())
		|| (GetHP() < 0)
		|| (!IsAttackAllowed(other))
		//|| ((IsClient() && CastToClient()->dead)
		) {
		mlog(COMBAT__ATTACKS, "Attack canceled, invalid circumstances.");
		return false; // Only bards can attack while casting
	}
	/*if(DivineAura() && !GetGM()) {//cant attack while invulnerable unless your a gm
		mlog(COMBAT__ATTACKS, "Attack canceled, Divine Aura is in effect.");
		Message_StringID(MT_DefaultText, DIVINE_AURA_NO_ATK);	//You can't attack while invulnerable!
		return false;
	}*/

	//if (GetFeigned())
	//	return false; // Rogean: How can you attack while feigned? Moved up from Aggro Code.

	
	ItemInst* weapon;
	if (Hand == 14){	// Kaiyodo - Pick weapon from the attacking hand
		//weapon = GetInv().GetItem(SLOT_SECONDARY);
		OffHandAtk(true);
	}
	else{
		//weapon = GetInv().GetItem(SLOT_PRIMARY);
		OffHandAtk(false);
	}

	if(weapon != NULL) {
		if (!weapon->IsWeapon()) {
			mlog(COMBAT__ATTACKS, "Attack canceled, Item %s (%d) is not a weapon.", weapon->GetItem()->Name, weapon->GetID());
			return(false);
		}
		mlog(COMBAT__ATTACKS, "Attacking with weapon: %s (%d)", weapon->GetItem()->Name, weapon->GetID());
	} else {
		mlog(COMBAT__ATTACKS, "Attacking without a weapon.");
	}
	
	// calculate attack_skill and skillinuse depending on hand and weapon
	// also send Packet to near clients
	SkillType skillinuse;
	AttackAnimation(skillinuse, Hand, weapon);
	mlog(COMBAT__ATTACKS, "Attacking with %s in slot %d using skill %d", weapon?weapon->GetItem()->Name:"Fist", Hand, skillinuse);
	
	/// Now figure out damage
	int damage = 0;
	int8 mylevel = GetLevel() ? GetLevel() : 1;
	int32 hate = 0;
	if (weapon) hate = weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt;
	int weapon_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && weapon_damage > 1) hate = weapon_damage;
	
	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if(weapon_damage > 0){

		//try a finishing blow.. if successful end the attack
		if(TryFinishingBlow(other, skillinuse))
			return (true);
		
		int min_hit = 1;
		int max_hit = (2*weapon_damage*GetDamageTable(skillinuse)) / 100;

		if(GetLevel() < 10 && max_hit > 20)
			max_hit = (RuleI(Combat, HitCapPre10));
		else if(GetLevel() < 20 && max_hit > 40)
			max_hit = (RuleI(Combat, HitCapPre20));


		// ***************************************************************
		// *** Calculate the damage bonus, if applicable, for this hit ***
		// ***************************************************************

#ifndef EQEMU_NO_WEAPON_DAMAGE_BONUS

		// If you include the preprocessor directive "#define EQEMU_NO_WEAPON_DAMAGE_BONUS", that indicates that you do not
		// want damage bonuses added to weapon damage at all. This feature was requested by ChaosSlayer on the EQEmu Forums.
		//
		// This is not recommended for normal usage, as the damage bonus represents a non-trivial component of the DPS output
		// of weapons wielded by higher-level melee characters (especially for two-handed weapons).

		int ucDamageBonus = 0;

		if( Hand == 13 && GetLevel() >= 28 && IsWarriorClass() )
		{
			// Damage bonuses apply only to hits from the main hand (Hand == 13) by characters level 28 and above
			// who belong to a melee class. If we're here, then all of these conditions apply.

			ucDamageBonus = GetWeaponDamageBonus( weapon ? weapon->GetItem() : (const Item_Struct*) NULL );

			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		//Live AA - Sinister Strikes *Adds weapon damage bonus to offhand weapon.
		if (Hand==14) {
			if (aabonuses.SecondaryDmgInc || itembonuses.SecondaryDmgInc || spellbonuses.SecondaryDmgInc){
				
				ucDamageBonus = GetWeaponDamageBonus( weapon ? weapon->GetItem() : (const Item_Struct*) NULL );

				min_hit += (int) ucDamageBonus;
				max_hit += (int) ucDamageBonus;
				hate += ucDamageBonus;
			}
		}

		min_hit += min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100;

		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = MakeRandomInt(min_hit, max_hit);

		mlog(COMBAT__DAMAGE, "Damage calculated to %d (min %d, max %d, str %d, skill %d, DMG %d, lv %d)",
			damage, min_hit, max_hit, GetSTR(), GetSkill(skillinuse), weapon_damage, mylevel);

		//check to see if we hit..
		if(!other->CheckHitChance(this, skillinuse, Hand)) {
			mlog(COMBAT__ATTACKS, "Attack missed. Damage set to 0.");
			damage = 0;
		} else {	//we hit, try to avoid it
			other->AvoidDamage(this, damage);
			other->MeleeMitigation(this, damage, min_hit);
			if(damage > 0) {
				ApplyMeleeDamageBonus(skillinuse, damage);
				damage += (itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse) / 100) + GetSkillDmgAmt(skillinuse);
				TryCriticalHit(other, skillinuse, damage);
			}
			mlog(COMBAT__DAMAGE, "Final damage after all reductions: %d", damage);
		}

		//riposte
		bool slippery_attack = false; // Part of hack to allow riposte to become a miss, but still allow a Strikethrough chance (like on Live)
		if (damage == -3)  {
			if (bRiposte) return false;
			else {
				if (Hand == 14) {// Do we even have it & was attack with mainhand? If not, don't bother with other calculations
					//Live AA - SlipperyAttacks 
					//This spell effect most likely directly modifies the actual riposte chance when using offhand attack.
					sint16 OffhandRiposteFail = aabonuses.OffhandRiposteFail + itembonuses.OffhandRiposteFail + spellbonuses.OffhandRiposteFail;
					OffhandRiposteFail *= -1; //Live uses a negative value for this.

					if (OffhandRiposteFail && 
						(OffhandRiposteFail > 99 || (MakeRandomInt(0, 100) < OffhandRiposteFail))) {
						damage = 0; // Counts as a miss
						slippery_attack = true;
					} else 
						DoRiposte(other);
						if (IsDead()) return false;
				}
				else 
					DoRiposte(other);
					if (IsDead()) return false;
			}
		}

		if (((damage < 0) || slippery_attack) && !bRiposte && !IsStrikethrough) { // Hack to still allow Strikethrough chance w/ Slippery Attacks AA
			sint16 bonusStrikeThrough = itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough;
	
			if(bonusStrikeThrough && (MakeRandomInt(0, 100) < bonusStrikeThrough)) {
				Message_StringID(MT_StrikeThrough, STRIKETHROUGH_STRING); // You strike through your opponents defenses!
				Attack(other, Hand, false, true); // Strikethrough only gives another attempted hit
				return false;
			}
		}
	}
	else{
		damage = -5;
	}


	// Hate Generation is on a per swing basis, regardless of a hit, miss, or block, its always the same.
	// If we are this far, this means we are atleast making a swing.
	if (!bRiposte) // Ripostes never generate any aggro.
		other->AddToHateList(this, hate);
	
	///////////////////////////////////////////////////////////
	//////    Send Attack Damage
	///////////////////////////////////////////////////////////
	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);

	if (IsDead()) return false;

	if(damage > 0 && (spellbonuses.MeleeLifetap || itembonuses.MeleeLifetap))
	{
		int lifetap_amt = spellbonuses.MeleeLifetap + itembonuses.MeleeLifetap;
		if(lifetap_amt > 100)
			lifetap_amt = 100;

		lifetap_amt = damage * lifetap_amt / 100;

		mlog(COMBAT__DAMAGE, "Melee lifetap healing for %d damage.", damage);
		//heal self for damage done..
		HealDamage(lifetap_amt);
		
		if (spellbonuses.MeleeLifetap)
			CheckHitsRemaining(0, false,false, SE_MeleeLifetap);
	}
	
	//break invis when you attack
	if(invisible) {
		mlog(COMBAT__ATTACKS, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if(invisible_undead) {
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if(invisible_animals){
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. animals due to melee attack.");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if(hidden || improved_hidden){
		hidden = false;
		improved_hidden = false;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}

	if(GetTarget())
		TriggerDefensiveProcs(weapon, other, Hand, damage);

	if (damage > 0)
		return true;

	else
		return false;
}

void Merc::Damage(Mob* other, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable, sint8 buffslot, bool iBuffTic)
{
	if(IsDead() || IsCorpse())
		return;
	
	if(spell_id==0)
		spell_id = SPELL_UNKNOWN;
	
	// cut all PVP spell damage to 2/3 -solar
	// EverHood - Blasting ourselfs is considered PvP 
	//Don't do PvP mitigation if the caster is damaging himself
	if(other && other->IsClient() && (other != this) && damage > 0) {
		int PvPMitigation = 100;
		if(attack_skill == ARCHERY)
			PvPMitigation = 80;
		else
			PvPMitigation = 67;
		damage = (damage * PvPMitigation) / 100;
	}
			

	//do a majority of the work...
	CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic);
}

void Merc::Death(Mob* killerMob, sint32 damage, int16 spell, SkillType attack_skill)
{
	if(IsDead())
		return;	//cant die more than once...

	int exploss;
	
	mlog(COMBAT__HITS, "Fatal blow dealt by %s with %d damage, spell %d, skill %d", killerMob ? killerMob->GetName() : "Unknown", damage, spell, attack_skill);
	
	//
	// #1: Send death packet to everyone
	//
	uint8 killed_level = GetLevel();
	if(!spell) spell = SPELL_UNKNOWN;

	//
	// #2: figure out things that affect the player dying and mark them dead
	//

	InterruptSpell();
	SetPet(0);
	//SetHorseId(0);
	//dead = true;


	if (killerMob != NULL)
	{
		if (killerMob->IsNPC()) {
            parse->EventNPC(EVENT_SLAY, killerMob->CastToNPC(), this, "", 0);
			int16 emoteid = killerMob->CastToNPC()->GetNPCEmoteID();
			if(emoteid != 0)
				killerMob->CastToNPC()->DoNPCEmote(KILLEDPC,emoteid);
			killerMob->TrySpellOnKill(killed_level,spell);
		}
		
		if(killerMob->IsClient() && (killerMob->CastToClient()->IsDueling())) {

			if (killerMob->IsClient() && killerMob->CastToClient()->IsDueling() && killerMob->CastToClient()->GetDuelTarget() == GetID())
			{
				//if duel opponent killed us...
				killerMob->CastToClient()->SetDueling(false);
				killerMob->CastToClient()->SetDuelTarget(0);
				entity_list.DuelMessage(killerMob,this,false);
			} else {
				//otherwise, we just died, end the duel.
				//Mob* who = entity_list.GetMob(GetDuelTarget());
				//if(who && who->IsClient()) {
				//	who->CastToClient()->SetDueling(false);
				//	who->CastToClient()->SetDuelTarget(0);
				//}
			}
		}
	}

	entity_list.RemoveFromTargets(this);
	hate_list.RemoveEnt(this);
	
	
	//remove ourself from all proximities
	//ClearAllProximities();

	//
	// #3: exp loss and corpse generation
	//

	if(spell != SPELL_UNKNOWN)
	{
		uint32 buff_count = GetMaxTotalSlots();
		for(uint16 buffIt = 0; buffIt < buff_count; buffIt++)
		{
			if(buffs[buffIt].spellid == spell && buffs[buffIt].client)
			{
				exploss = 0;	// no exp loss for pvp dot
				break;
			}
		}
	}

	bool LeftCorpse = false;


		BuffFadeDetrimental();


	//
	// Finally, send em home
	//

	// we change the mob variables, not pp directly, because Save() will copy
	// from these and overwrite what we set in pp anyway
	//

	/*if(LeftCorpse && (GetClientVersionBit() & BIT_SoFAndLater) && RuleB(Character, RespawnFromHover))
	{
		ClearDraggedCorpses();

		RespawnFromHoverTimer.Start(RuleI(Character, RespawnFromHoverTimer) * 1000);

		SendRespawnBinds();
	}
	else
	{*/
		if(isgrouped)
		{
			Group *g = GetGroup();
			if(g)
				g->MemberZoned(this);
		}
	
		//Raid* r = entity_list.GetRaidByClient(this);

		//if(r)
		//	r->MemberZoned(this);

		//dead_timer.Start(5000, true);

		//m_pp.zone_id = m_pp.binds[0].zoneId;
		//m_pp.zoneInstance = 0;
		//database.MoveCharacterToZone(this->CharacterID(), database.GetZoneName(m_pp.zone_id));
		
		Save();
	
		//GoToDeath();
	/*}*/
}

Client* Merc::GetMercOwner() {
	Client* mercOwner = 0;

	if(GetOwnerID()) {
		mercOwner = GetOwner()->CastToClient();
	}

	return mercOwner; 
}

Mob* Merc::GetOwner() {
	Mob* Result = 0;

	Result = entity_list.GetMob(GetOwnerID());

	if(!Result) {
		this->SetOwnerID(0);
	}

	return Result->CastToMob();
}

Merc* Merc::LoadMerc(Client *c, MercTemplate* merc_template, uint32 merchant_id) {
	Merc* merc;

	if(c->GetMercID()) {
		merc_template = c->GetMercData();
	}

	//get mercenary data
	if(merc_template) {
		NPCType* npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));

		sprintf(npc_type->name, "%s", GetRandPetName());

		int8 gender;
		NPC* tar = entity_list.GetNPCByID(merchant_id);
		if(tar) {
			gender = Mob::GetDefaultGender(merc_template->RaceID, tar->GetGender());
		}

		sprintf(npc_type->lastname, "%s's %s", c->GetName(), "Mercenary");
		npc_type->cur_hp = 4000000;
		npc_type->max_hp = 4000000;
		npc_type->race = 1;
		npc_type->gender = gender;
		npc_type->class_ = merc_template->ClassID;
		npc_type->deity= 1;
		npc_type->level = c->GetLevel();
		npc_type->npc_id = 0;
		npc_type->loottable_id = 0;
		npc_type->texture = 1;
		npc_type->light = 0;
		npc_type->runspeed = 0;
		npc_type->d_meele_texture1 = 1;
		npc_type->d_meele_texture2 = 1;
		npc_type->merchanttype = 1;
		npc_type->bodytype = 1;

		npc_type->STR = 150;
		npc_type->STA = 150;
		npc_type->DEX = 150;
		npc_type->AGI = 150;
		npc_type->INT = 150;
		npc_type->WIS = 150;
		npc_type->CHA = 150;
		npc_type->MR = 50;
		npc_type->FR = 50;
		npc_type->CR = 50;
		npc_type->PR = 50;
		npc_type->DR = 50;
		npc_type->Corrup = 15;

		npc_type->findable = 1;

		Merc* merc = new Merc(npc_type, c->GetX(), c->GetY(), c->GetZ(), 0);
	
		merc->SetMercData( merc_template->MercTemplateID );

		return merc;
	}

	return 0;
}

bool Merc::Spawn(Client *owner) {
	if(!owner)
		return false;

	MercTemplate* merc_template = zone->GetMercTemplate(GetMercTemplateID());

	if(!merc_template)
		return false;

	entity_list.AddMerc(this, true, true);

	SendPosition();

	//printf("Spawned Merc with ID %i\n", npc->GetID()); fflush(stdout);

	/*
	uint32 itemID = 0;
	int8 materialFromSlot = 0xFF;
	for(int i=0; i<22; ++i) {
		itemID = GetMercItemBySlot(i);
		if(itemID != 0) {
			materialFromSlot = Inventory::CalcMaterialFromSlot(i);
			if(materialFromSlot != 0xFF) {
				this->SendWearChange(materialFromSlot);
			}
		}
	}
	*/

		
	SendIllusionPacket(merc_template->RaceID, GetGender(), GetTexture(), GetHelmTexture(), GetHairColor(), GetBeardColor(),
		GetEyeColor1(), GetEyeColor2(), GetHairStyle(), GetLuclinFace(), GetBeard(), 0xFF,
		GetDrakkinHeritage(), GetDrakkinTattoo(), GetDrakkinDetails(), GetSize());

	if(!owner->IsGrouped()) {
		Group *g = new Group(owner);
		if(AddMercToGroup(this, g)) {
			entity_list.AddGroup(g);
			database.SetGroupLeaderName(g->GetID(), owner->GetName());
			g->SaveGroupLeaderAA();
			database.SetGroupID(owner->GetName(), g->GetID(), owner->CharacterID());
			database.SetGroupID(GetCleanName(), g->GetID(), GetMercID());
		}
	}
	else {
		AddMercToGroup(this, owner->GetGroup());
		database.SetGroupID(GetCleanName(), owner->GetGroup()->GetID(), GetMercID());
	}

	return true;
}

bool Merc::Suspend() {
	Client* mercOwner;

	if(GetMercOwner()) {
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
		return false;

	if(IsGrouped()) {
		RemoveMercFromGroup(this, GetGroup());
	}

	//Save();
	
	mercOwner->SetMercID(0);
	// Set merc suspended time for client & merc

	Depop();

	return true;
}

bool Merc::Unsuspend() {
	Client* mercOwner;

	if(GetMercOwner()) {
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
		return false;

	if(!GetMercID()) {

		// Get merc, assign it to client & spawn
		Merc* merc = Merc::LoadMerc(mercOwner, mercOwner->GetMercData(), 0);
		//mercOwner->SetMerc(this);
		merc->Spawn(mercOwner);

		// TODO: Populate these packets properly instead of hard coding the data fields.

		// Send Mercenary Status/Timer packet
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryTimer, sizeof(MercenaryStatus_Struct));
		MercenaryStatus_Struct* mss = (MercenaryStatus_Struct*)outapp->pBuffer;
		//mss->MercEntityID = merc->GetID();			// Seen 0 (no merc spawned) or 615843841 and 22779137
		mss->MercEntityID = 1;			// This field needs to be renamed.  If set to 1, it shows stances in the merc window, otherwise it does not.			
		mss->UpdateInterval = 900000;	// Seen 900000 - Matches from 0x6537 packet (15 minutes in ms?)
		mss->MercUnk01 = 180000;		// Seen 180000 - 3 minutes in milleseconds? Maybe next update interval?
		mss->MercState = 5;				// Seen 5 (normal) or 1 (suspended)
		mss->SuspendedTime = 0;			// Seen 0 (not suspended) or c9 c2 64 4f (suspended on Sat Mar 17 11:58:49 2012) - Unix Timestamp
		mercOwner->FastQueuePacket(&outapp);

		// Send Mercenary Assign packet twice - This is actually just WeaponEquip
		outapp = new EQApplicationPacket(OP_MercenaryAssign, sizeof(MercenaryAssign_Struct));
		MercenaryAssign_Struct* mas = (MercenaryAssign_Struct*)outapp->pBuffer;
		mas->MercEntityID = merc->GetID();
		mas->MercUnk01 = 1;		// Values seen on Live
		mas->MercUnk02 = 2;		// Values seen on Live
		mercOwner->FastQueuePacket(&outapp);

		outapp = new EQApplicationPacket(OP_MercenaryAssign, sizeof(MercenaryAssign_Struct));
		MercenaryAssign_Struct* mas2 = (MercenaryAssign_Struct*)outapp->pBuffer;
		mas2->MercEntityID = merc->GetID();
		mas2->MercUnk01 = 0;		// Values seen on Live
		mas2->MercUnk02 = 13;	// Values seen on Live 0xd0
		DumpPacket(outapp);
		mercOwner->FastQueuePacket(&outapp);

		mercOwner->SendMercDataPacket(GetMercTemplateID());
	}

	return true;
}

bool Merc::Dismiss() {
	Client* mercOwner;

	if(GetMercOwner()) {
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
		return false;

	if(IsGrouped()) {
		RemoveMercFromGroup(this, GetGroup());
	}

	//Save();

	mercOwner->SetMerc(0);

	Depop();

	return true;
}

void Merc::Zone() {
	if(HasGroup()) {
		GetGroup()->MemberZoned(this);
	}

	//Save();
	Depop();
}

void Merc::Depop() {
	WipeHateList();
	
	entity_list.RemoveFromHateLists(this);
	
	if(HasGroup())
		RemoveMercFromGroup(this, GetGroup());
	
	SetOwnerID(0);

	p_depop = true;
	
	Mob::Depop(false);
}

bool Merc::RemoveMercFromGroup(Merc* merc, Group* group) {
	bool Result = false;

	if(merc && group) {
		if(merc->HasGroup()) {
			if(!group->IsLeader(merc)) {
				merc->SetFollowID(0);

				if(group->DelMember(merc))
					database.SetGroupID(merc->GetCleanName(), 0, merc->GetMercID());

				if(group->GroupCount() <= 1)
					group->DisbandGroup();
			}
			else {
				for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if(!group->members[i])
						continue;

					group->members[i]->SetFollowID(0);
				}

				group->DisbandGroup();
				database.SetGroupID(merc->GetCleanName(), 0, merc->GetMercID());
			}

			Result = true;
		}
	}

	return Result;
}

bool Merc::AddMercToGroup(Merc* merc, Group* group) {
	bool Result = false;

	if(merc && group) {
		if(!merc->HasGroup()) {
			// Add bot to this group
			if(group->AddMember(merc)) {
				if(group->GetLeader()) {
					merc->SetFollowID(group->GetLeader()->GetID());

					// Need to send this only once when a group is formed with a bot so the client knows it is also the group leader
					if(group->GroupCount() == 2 && group->GetLeader()->IsClient()) {
						group->UpdateGroupAAs();
						Mob *TempLeader = group->GetLeader();
						group->SendUpdate(groupActUpdate, TempLeader);
					}
				}

				Result = true;
			}
		}
	}

	return Result;
}

Merc* Client::GetMerc() {
	if(GetMercID() == 0)
		return(NULL);
	
	Merc* tmp = entity_list.GetMercByID(GetMercID());
	if(tmp == NULL) {
		SetMercID(0);
		return(NULL);
	}
	
	if(tmp->GetOwnerID() != GetID()) {
		SetMercID(0);
		return(NULL);
	}
	
	return(tmp);
}

void Merc::SetMercData( uint32 template_id ) {
	MercTemplate* merc_template = zone->GetMercTemplate(template_id);

	SetMercID(199);   //ToDO: Get ID From Merc table after saving
	SetMercTemplateID( merc_template->MercTemplateID );
	SetMercType( merc_template->MercType );
	SetMercSubType( merc_template->MercSubType );
	SetProficiencyID( merc_template->ProficiencyID );
	SetCostFormula( merc_template->CostFormula );
	SetMercNameType( merc_template->MercNameType );
}

MercTemplate* Zone::GetMercTemplate( uint32 template_id ) {
	return &merc_templates[template_id];
}

void Client::SetMerc(Merc* newmerc) {
	Merc* oldmerc = GetMerc();
	if (oldmerc) {
		oldmerc->SetOwnerID(0);
	}
	if (newmerc == NULL) {
		SetMercID(0);
		m_mercdata.MercTemplateID = 0;
		m_mercdata.MercType = 0;
		m_mercdata.MercSubType = 0;
		m_mercdata.CostFormula = 0;
		m_mercdata.ClientVersion = 0;
		m_mercdata.MercNameType = 0;
		m_mercdata.SuspendedTime = 0;
	} else {
		SetMercID(newmerc->GetID());
		Client* oldowner = entity_list.GetClientByID(newmerc->GetOwnerID());
		if (oldowner)
			oldowner->SetMercID(0);
		newmerc->SetOwnerID(this->GetID());
		m_mercdata.MercTemplateID = newmerc->GetMercTemplateID();
		m_mercdata.MercType = newmerc->GetMercType();
		m_mercdata.MercSubType = newmerc->GetMercSubType();
		m_mercdata.CostFormula = newmerc->GetCostFormula();
		m_mercdata.ClientVersion = 0;
		m_mercdata.MercNameType = newmerc->GetMercNameType();
		m_mercdata.SuspendedTime = 0;
	}
}

void Client::SendMercDataPacket(int32 MercID) {
	// Hard setting some stuff until it can be coded to load properly from the DB/Memory
	int mercCount = 1;
	int stanceCount = 2;
	char mercName[32];	// This actually needs to be null terminated
	strcpy(mercName, GetRandPetName());
	
	uint32 packetSize = sizeof(MercenaryDataUpdate_Struct) + sizeof(MercenaryData_Struct) * mercCount + strlen(mercName);
	
	// This response packet seems to be sent on zoning or camping by client request
	// It is populated with owned merc data only
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, packetSize);
	MercenaryDataUpdate_Struct* mdu = (MercenaryDataUpdate_Struct*)outapp->pBuffer;

	mdu->MercStatus = 0;
	mdu->MercCount = mercCount;

	MercTemplate *mercData = GetMercData();

	for(int i = 0; i < mercCount; i++)
	{
		mdu->MercData[i].MercID = GetMercID();
		mdu->MercData[i].MercType = mercData->MercType;
		mdu->MercData[i].MercSubType = mercData->MercSubType;
		mdu->MercData[i].PurchaseCost = Merc::CalcPurchaseCost(mercData->MercTemplateID, this->GetLevel());
		mdu->MercData[i].UpkeepCost = Merc::CalcUpkeepCost(mercData->MercTemplateID, this->GetLevel());
		mdu->MercData[i].Status = 0;
		mdu->MercData[i].AltCurrencyCost = 0;
		mdu->MercData[i].AltCurrencyUpkeep = 1;
		mdu->MercData[i].AltCurrencyType = 19;
		mdu->MercData[i].MercUnk01 = 0;
		mdu->MercData[i].TimeLeft = 900000;
		mdu->MercData[i].MerchantSlot = 1;
		mdu->MercData[i].MercUnk02 = 1;
		mdu->MercData[i].StanceCount = stanceCount;
		mdu->MercData[i].MercUnk03 = 519044964;
		mdu->MercData[i].MercUnk04 = 1;
		strcpy(mdu->MercData[i].MercName, mercName);
		for (int stanceindex = 0; stanceindex < stanceCount; stanceindex++)
		{
			mdu->MercData[i].Stances[stanceindex].StanceIndex = stanceindex;
			mdu->MercData[i].Stances[stanceindex].Stance = stanceindex + 1;
		}
		mdu->MercData[i].MercUnk05 = 1;
		i++;
	}

	DumpPacket(outapp);
	FastQueuePacket(&outapp); 
}


void NPC::LoadMercTypes(){
	std::string errorMessage;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT DISTINCT MTyp.dbstring, MTyp.clientversion FROM merc_merchant_entries MME, merc_merchant_template_entries MMTE, merc_types MTyp, merc_templates MTem WHERE MME.merchant_id = %i AND MME.merc_merchant_template_id = MMTE.merc_merchant_template_id AND MMTE.merc_template_id = MTem.merc_template_id AND MTem.merc_type_id = MTyp.merc_type_id;", GetNPCTypeID()), TempErrorMessageBuffer, &DatasetResult)) {
		errorMessage = std::string(TempErrorMessageBuffer);
	}
	else {
		while(DataRow = mysql_fetch_row(DatasetResult)) {
			MercType tempMercType;

			tempMercType.Type = atoi(DataRow[0]);
			tempMercType.ClientVersion = atoi(DataRow[1]);

			mercTypeList.push_back(tempMercType);
		}

		mysql_free_result(DatasetResult);
	}

	safe_delete(Query);
	Query = 0;

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error in NPC::LoadMercTypes()");
	}
}

void NPC::LoadMercs(){

	std::string errorMessage;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT DISTINCT MTem.merc_template_id, MTyp.dbstring AS merc_type_id, MTem.dbstring AS merc_subtype_id, 0 AS CostFormula, MTem.clientversion FROM merc_merchant_entries MME, merc_merchant_template_entries MMTE, merc_types MTyp, merc_templates MTem WHERE MME.merchant_id = %i AND MME.merc_merchant_template_id = MMTE.merc_merchant_template_id AND MMTE.merc_template_id = MTem.merc_template_id AND MTem.merc_type_id = MTyp.merc_type_id;", GetNPCTypeID()), TempErrorMessageBuffer, &DatasetResult)) {
		errorMessage = std::string(TempErrorMessageBuffer);
	}
	else {
		while(DataRow = mysql_fetch_row(DatasetResult)) {
			MercData tempMerc;

			tempMerc.MercTemplateID = atoi(DataRow[0]);
			tempMerc.MercType = atoi(DataRow[1]);
			tempMerc.MercSubType = atoi(DataRow[2]);
			tempMerc.CostFormula = atoi(DataRow[3]);
			tempMerc.ClientVersion = atoi(DataRow[4]);

			mercDataList.push_back(tempMerc);
		}

		mysql_free_result(DatasetResult);
	}

	safe_delete(Query);
	Query = 0;

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error in NPC::LoadMercTypes()");
	}
}

int NPC::GetNumMercTypes(int32 clientVersion)
{
	int count = 0;
	std::list<MercType> mercTypeList = GetMercTypesList();

	for(std::list<MercType>::iterator mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end(); mercTypeListItr++) {
		if(mercTypeListItr->ClientVersion <= clientVersion)
			count++;
	}

	return count;
}

int NPC::GetNumMercs(int32 clientVersion)
{
	int count = 0;
	std::list<MercData> mercDataList = GetMercsList();

	for(std::list<MercData>::iterator mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); mercListItr++) {
		if(mercListItr->ClientVersion <= clientVersion)
			count++;
	}

	return count;
}

std::list<MercType> NPC::GetMercTypesList(int32 clientVersion) {
	std::list<MercType> result;

	if(GetNumMercTypes() > 0) {
		for(std::list<MercType>::iterator mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end(); mercTypeListItr++) {
			if(mercTypeListItr->ClientVersion <= clientVersion) {
				MercType mercType;
				mercType.Type = mercTypeListItr->Type;
				mercType.ClientVersion = mercTypeListItr->ClientVersion;
				result.push_back(mercType);
			}
		}		
	}

	return result;
}

std::list<MercData> NPC::GetMercsList(int32 clientVersion) {
	std::list<MercData> result;

	if(GetNumMercs() > 0) {
		for(std::list<MercData>::iterator mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); mercListItr++) {
			if(mercListItr->ClientVersion <= clientVersion) {
				MercData mercData;

				mercData.MercTemplateID = mercListItr->MercTemplateID;
				mercData.MercType = mercListItr->MercType;
				mercData.MercSubType = mercListItr->MercSubType;			
				mercData.CostFormula = mercListItr->CostFormula;		
				mercData.ClientVersion = mercListItr->ClientVersion;
				result.push_back(mercData);
			}
		}		
	}

	return result;
}

int32 Merc::CalcPurchaseCost( uint32 templateID , uint8 level, uint8 currency_type) {
	int32 cost = 0;

	MercTemplate *mercData = zone->GetMercTemplate(templateID);

	if(mercData) {
		if(currency_type == 0) { //calculate cost in coin - cost in gold
			int levels_above_cutoff;
			switch (mercData->CostFormula) {
			case 0:
				levels_above_cutoff = level > 10 ? (level - 10) : 0;
				cost = levels_above_cutoff * 300;
				cost += level >= 10 ? 100 : 0;
				break;
			default:
				break;
			}
		}
		else if(currency_type == 19) {
			cost = 0;
		}
	}

	return cost/100;
}

int32 Merc::CalcUpkeepCost( uint32 templateID , uint8 level, uint8 currency_type) {
	int32 cost = 0;

	MercTemplate *mercData = zone->GetMercTemplate(templateID);

	if(mercData) {
		if(currency_type == 0) { //calculate cost in coin - cost in gold
			int levels_above_cutoff;
			switch (mercData->CostFormula) {
			case 0:
				levels_above_cutoff = level > 10 ? (level - 10) : 0;
				cost = levels_above_cutoff * 300;
				cost += level >= 10 ? 100 : 0;
				break;
			default:
				break;
			}
		}
		else if(currency_type == 19) { // cost in Bayle Marks
			cost = 1;
		}
	}

	return cost/100;
}