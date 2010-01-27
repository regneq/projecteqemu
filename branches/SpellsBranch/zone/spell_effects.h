//Tried to get more than enough parameters for everything possibly needed for spell casting.
//The return type is whether or not we need a client affecting action packet (type 0x04)
//Some spells do if they're successful to work correctly, but most don't. The 0x04 can cause issues
//With the way we do buff packets so we only send it for the ones that truely do like bind affinity or shadow step.
bool Handle_SE_Blank(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_BlankWithPacket(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_CurrentHP(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Invisibility(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_CurrentMana(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_AddFaction(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Blind(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Stun(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Charm(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Fear(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_BindAffinity(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Gate(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_CancelMagic(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_InvisVsUndead(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_InvisVsAnimals(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Mez(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_SummonItem(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_SummonPet(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_DiseaseCounter(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_PoisonCounter(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_DivineAura(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Destroy(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Rune(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Levitate(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);
bool Handle_SE_Illusion(const Spell *spell_to_cast, Mob *caster, const uint32 effect_id_index, const float partial, ItemInst **summoned_item, Buff *buff_in_use, sint32 buff_slot);



