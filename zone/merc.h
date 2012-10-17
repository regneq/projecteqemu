#ifndef MERC_H
#define MERC_H

#include "mob.h"

using namespace std;

struct MercType {
	uint32	Type;
	uint32	ClientVersion;		
};

struct MercData {
	uint32	MercTemplateID;
	uint32	MercType;				// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
	uint32	MercSubType;			// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
	uint32	CostFormula;			// To determine cost to client
	uint32	ClientVersion;				// Only send valid mercs per expansion
	uint8	MercNameType;			// Determines if merc gets random name or default text
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

	int32 GetMercTemplateID() { return _MercTemplateID; }
	int32 GetMercType() { return _MercType; }
	int32 GetMercSubType() { return _MercSubType; }

	void SetMercData (uint32 templateID );
	void SetMercTemplateID( uint32 templateID ) { _MercTemplateID = templateID; }
	void SetMercType( uint32 type ) { _MercType = type; }
	void SetMercSubType( uint32 subtype ) { _MercSubType = subtype; }
	bool Process();

private:
	uint32 _MercTemplateID;
	uint32 _MercType;
	uint32 _MercSubType;
};

#endif // MERC_H