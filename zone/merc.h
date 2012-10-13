#ifndef MERC_H
#define MERC_H

#include "mob.h"

using namespace std;

struct MercType {
	int32	Type;
	int32	ClientVersion;		
};

struct MercData {
	int32	MercTemplateID;
	int32	MercType;				// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
	int32	MercSubType;			// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
	int32	CostFormula;			// To determine cost to client
	int32	ClientVersion;				// Only send valid mercs per expansion
	int8	MercNameType;			// Determines if merc gets random name or default text
	char	MercNamePrefix[25];
	char	MercNameSuffix[25];
};

class Merc : public Mob {
public:
	Merc(const NPCType* d, float x, float y, float z, float heading);
	virtual ~Merc();

	//abstract virtual function implementations requird by base abstract class
	virtual void Death(Mob* killerMob, sint32 damage, int16 spell_id, SkillType attack_skill);
	virtual void Damage(Mob* from, sint32 damage, int16 spell_id, SkillType attack_skill, bool avoidable = true, sint8 buffslot = -1, bool iBuffTic = false);
	virtual bool Attack(Mob* other, int Hand = SLOT_PRIMARY, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false);
	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return false; }
	virtual Raid* GetRaid() { return 0; }
	virtual Group* GetGroup() { return 0; }

	virtual bool IsMerc() const { return true; }

	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);

	bool IsDead() { return GetHP() < 0;};
};

#endif // MERC_H