/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

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
#include "../common/debug.h"
#include "masterentity.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/rulesys.h"

#ifdef EMBPERL
#include "embparser.h"
#endif

// ##########################################
// Trade implementation
// ##########################################

Trade::Trade(Mob* in_owner)
{
	owner = in_owner;
	Reset();
}

Trade::~Trade()
{
	Reset();
}

void Trade::Reset()
{
	state = TradeNone;
	with_id = 0;
	pp=0; gp=0; sp=0; cp=0;
}

void Trade::SetTradeCash(uint32 in_pp, uint32 in_gp, uint32 in_sp, uint32 in_cp)
{
	pp=in_pp; gp=in_gp; sp=in_sp; cp=in_cp;
}

// Initiate a trade with another mob
// initiate_with specifies whether to start trade with other mob as well
void Trade::Start(uint32 mob_id, bool initiate_with)
{
	Reset();
	state = Trading;
	with_id = mob_id;
	
	// Autostart on other mob?
	if (initiate_with) {
		Mob* with = With();
		if (with)
			with->trade->Start(owner->GetID(), false);
	}
}

// Add item from a given slot to trade bucket (automatically does bag data too)
void Trade::AddEntity(int16 from_slot_id, int16 trade_slot_id)
{
	if (!owner || !owner->IsClient()) {
		// This should never happen
		LogFile->write(EQEMuLog::Debug, "Programming error: NPC's should not call Trade::AddEntity()");
		return;
	}
	
	// If one party accepted the trade then an item was added, their state needs to be reset
	owner->trade->state = Trading;
	Mob* with = With();
	if (with)
		with->trade->state = Trading;
	
	// Item always goes into trade bucket from cursor
	Client* client = owner->CastToClient();
	const ItemInst* inst = client->GetInv().GetItem(SLOT_CURSOR);
	if (!inst) {
		client->Message(13, "Error: Could not find item on your cursor!");
		return;
	}
	
	_log(TRADING__HOLDER, "%s added item '%s' to trade slot %i", owner->GetName(), inst->GetItem()->Name, trade_slot_id);
	
	ItemInst* inst2 = client->GetInv().GetItem(trade_slot_id);
	int new_charges = 0;
	if (!inst2 || !inst2->GetItem()) {
		// Send all item data to other client
		SendItemData(inst, trade_slot_id);
		// Move item on cursor to the trade slots
		client->PutItemInInventory(trade_slot_id, *inst);
	}
	else
	{
		if (client->GetInv().GetItem(SLOT_CURSOR)->GetID() != client->GetInv().GetItem(trade_slot_id)->GetID()) {
			client->Kick();
			return;
		}
		new_charges = (inst2->GetCharges()+inst->GetCharges());
		if (new_charges < inst2->GetItem()->StackSize)
		{
			inst2->SetCharges(new_charges);
			new_charges = 0;
		}
		else
		{
			new_charges = inst->GetCharges()-(inst2->GetItem()->StackSize-inst2->GetCharges()); //Leftover charges = charges - difference
			inst2->SetCharges(inst2->GetItem()->StackSize);
		}
		SendItemData(inst2, trade_slot_id);
	}
	if (new_charges > 0)
		client->GetInv().GetItem(from_slot_id)->SetCharges(new_charges);
	else
		client->DeleteItemInInventory(from_slot_id);//, (ItemInst&)trade_inst);
}

// Retrieve mob the owner is trading with
// Done like this in case 'with' mob goes LD and Mob* becomes invalid
Mob* Trade::With()
{
	return entity_list.GetMob(with_id);
}

// Private Method: Send item data for trade item to other person involved in trade
void Trade::SendItemData(const ItemInst* inst, sint16 dest_slot_id)
{
	// @merth: This needs to be redone with new item classes
	Mob* mob = With();
	if (!mob->IsClient())
		return; // Not sending packets to NPCs!
	
	Client* with = mob->CastToClient();
	Client* trader = owner->CastToClient();
	if (with && with->IsClient()) {
		with->SendItemPacket(dest_slot_id -IDX_TRADE,inst,ItemPacketTradeView);
		if (inst->GetItem()->ItemClass == 1) {
			for (int16 i=0; i<10; i++) {
				int16 bagslot_id = Inventory::CalcSlotId(dest_slot_id, i);
				const ItemInst* bagitem = trader->GetInv().GetItem(bagslot_id);
				if (bagitem) {
					with->SendItemPacket(bagslot_id-IDX_TRADE,bagitem,ItemPacketTradeView);
				}
			}
		}
		
		//safe_delete(outapp);
	}
}

// Audit trade: The part logged is what travels owner -> with
void Trade::LogTrade()
{
	Mob* with = With();
	if (!owner->IsClient() || !with)
		return; // Should never happen
	
	Client* trader = owner->CastToClient();
	bool logtrade = false;
	int admin_level = 0;
	uint8 item_count = 0;
	
	if (zone->tradevar != 0) {
		for (int16 i=3000; i<=3007; i++) {
			if (trader->GetInv().GetItem(i))
				item_count++;
		}
		
		if (((this->cp + this->sp + this->gp + this->pp)>0) || (item_count>0))
			admin_level = trader->Admin();
		else
			admin_level = 999;
		
		if (zone->tradevar == 7) {
			logtrade = true;
		}
		else if ((admin_level>=10) && (admin_level<20)) {
			if ((zone->tradevar<8) && (zone->tradevar>5))
				logtrade = true;
		}
		else if (admin_level<=20) {
			if ((zone->tradevar<8) && (zone->tradevar>4))
				logtrade = true;
		}
		else if (admin_level<=80) {
			if ((zone->tradevar<8) && (zone->tradevar>3))
				logtrade = true;
		}
		else if (admin_level<=100){
			if ((zone->tradevar<9) && (zone->tradevar>2))
				logtrade = true;
		}
		else if (admin_level<=150){
			if (((zone->tradevar<8) && (zone->tradevar>1)) || (zone->tradevar==9))
				logtrade = true;
		}
		else if (admin_level<=255){
			if ((zone->tradevar<8) && (zone->tradevar>0))
				logtrade = true;	
		}
	}
	
	if (logtrade == true) {
		char logtext[1000] = {0};
		uint32 cash = 0;
		bool comma = false;
		
		// Log items offered by owner
		cash = this->cp + this->sp + this->gp + this->pp;
		if ((cash>0) || (item_count>0)) {
			sprintf(logtext, "%s gave %s ", trader->GetName(), with->GetName());
			
			if (item_count > 0) {
				strcat(logtext, "items {");
				
				for (int16 i=3000; i<=3007; i++) {
					const ItemInst* inst = trader->GetInv().GetItem(i);
					
					if (!comma)
						comma = true;
					else {
						if (inst)
							strcat(logtext, ",");
					}
					
					if (inst) {
						char item_num[15] = {0};
						sprintf(item_num, "%i", inst->GetItem()->ID);
						strcat(logtext, item_num);
						
						if (inst->IsType(ItemClassContainer)) {
							for (uint8 j=0; j<10; j++) {
								inst = trader->GetInv().GetItem(i, j);
								if (inst) {
									strcat(logtext, ",");
									sprintf(item_num, "%i", inst->GetItem()->ID);
									strcat(logtext, item_num);
								}
							}
						}
					}
				}
			}
			
			if (cash > 0) {	
				char money[100] = {0};
				sprintf(money, " %ipp, %igp, %isp, %icp", trader->trade->pp, trader->trade->gp, trader->trade->sp, trader->trade->cp);
				strcat(logtext, money);
			}
			
			database.logevents(trader->AccountName(), trader->AccountID(),
				trader->Admin(), trader->GetName(), with->GetName(), "Trade", logtext, 6);
		}
	}
}

#if (EQDEBUG >= 9)
void Trade::DumpTrade()
{
	Mob* with = With();
	LogFile->write(EQEMuLog::Debug, "Dumping trade data: '%s' in TradeState %i with '%s'",
		this->owner->GetName(), state, ((with==NULL)?"(null)":with->GetName()));
	
	if (!owner->IsClient())
		return;
	
	Client* trader = owner->CastToClient();
	for (int16 i=3000; i<=3007; i++) {
		const ItemInst* inst = trader->GetInv().GetItem(i);
		
		if (inst) {
			LogFile->write(EQEMuLog::Debug, "Item %i (Charges=%i, Slot=%i, IsBag=%s)",
				inst->GetItem()->ID, inst->GetCharges(),
				i, ((inst->IsType(ItemClassContainer)) ? "True" : "False"));
			
			if (inst->IsType(ItemClassContainer)) {
				for (uint8 j=0; j<10; j++) {
					inst = trader->GetInv().GetItem(i, j);
					if (inst) {
						LogFile->write(EQEMuLog::Debug, "\tBagItem %i (Charges=%i, Slot=%i)",
							inst->GetItem()->ID, inst->GetCharges(),
							Inventory::CalcSlotId(i, j));
					}
				}
			}
		}
	}
	
	LogFile->write(EQEMuLog::Debug, "\tpp:%i, gp:%i, sp:%i, cp:%i", pp, gp, sp, cp);
}
#endif

void Client::ResetTrade() {
	const Item_Struct* TempItem = 0;
	ItemInst* ins;
	int x;
	AddMoneyToPP(trade->cp, trade->sp, trade->gp, trade->pp, true);
	for(x=3000; x <= 3007; x++)
	{
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem)
		{
			int freeslotid = GetInv().FindFreeSlot(ins->IsType(ItemClassContainer), true, TempItem->Size);
			if (freeslotid == SLOT_INVALID)
			{
				DropInst(ins);
			}
			else
			{
				PutItemInInventory(freeslotid, *ins);
				SendItemPacket(freeslotid, ins, ItemPacketTrade);
			}
			DeleteItemInInventory(x);
		}
	}
}

// Place items into inventory of NPC specified
void Client::FinishTrade(NPC* with){
	int32 items[4]={0};
	int8 charges[4]={0};

#ifdef EQBOTS

    bool botCanWear[4];
	bool BotCanWear;
	for (sint16 i=3000; i<=3003; i++){
        BotCanWear = false;
        botCanWear[i-3000] = BotCanWear;

		const ItemInst* inst = m_inv[i];
		if (inst) {
			items[i-3000]=inst->GetItem()->ID;
			charges[i-3000]=inst->GetCharges();
		}
		//EQoffline: will give the items to the bots and change the bot stats
		if(inst && with->IsBot() && with->BotOwner == this->CastToMob()) {
			const Item_Struct *mWeaponItem = inst->GetItem();
			if(mWeaponItem && inst->IsEquipable(with->GetBaseRace(), with->GetClass()) && (with->GetLevel() >= mWeaponItem->ReqLevel)) { // Angelox
				BotCanWear = true;
                botCanWear[i-3000] = BotCanWear;

				for(int j=0;j<22;j++) {
					if(inst->IsSlotAllowed(j)) {
                        if(j==1) { // earrings
							if(database.GetBotItemBySlot(with->GetNPCTypeID(), 4) == 0) {
								database.SetBotItemInSlot(with->GetNPCTypeID(), 4, inst->GetID());
							}
							else if(database.GetBotItemBySlot(with->GetNPCTypeID(), 1) == 0) {
								database.SetBotItemInSlot(with->GetNPCTypeID(), 1, inst->GetID());
							}
							else {
								const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 1));
								const ItemInst* insttmp = new ItemInst(itmtmp,0);
								PushItemOnCursor(*insttmp, true);
								database.RemoveBotItemBySlot(with->GetNPCTypeID(), 1);
								database.SetBotItemInSlot(with->GetNPCTypeID(), 1, inst->GetID());
							}
							break;
						}
                        else if(j==9) { // bracers
							if(database.GetBotItemBySlot(with->GetNPCTypeID(), 10) == 0) {
								database.SetBotItemInSlot(with->GetNPCTypeID(), 10, inst->GetID());
							}
							else if(database.GetBotItemBySlot(with->GetNPCTypeID(), 9) == 0) {
								database.SetBotItemInSlot(with->GetNPCTypeID(), 9, inst->GetID());
							}
							else {
								const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 9));
								const ItemInst* insttmp = new ItemInst(itmtmp,0);
								PushItemOnCursor(*insttmp, true);
								database.RemoveBotItemBySlot(with->GetNPCTypeID(), 9);
								database.SetBotItemInSlot(with->GetNPCTypeID(), 9, inst->GetID());
							}
							break;
						}
                        else if(j==13 || j==14) { // melee weapons
							const Item_Struct* itmwp = database.GetItem(inst->GetID());
							if((database.GetBotItemBySlot(with->GetNPCTypeID(), 13) == 0)  && inst->IsSlotAllowed(13)) {
								database.SetBotItemInSlot(with->GetNPCTypeID(), 13, inst->GetID());
								if((itmwp->ItemType == ItemType2HS) || (itmwp->ItemType == ItemType2HB) || (itmwp->ItemType == ItemType2HPierce)) {
									if(database.GetBotItemBySlot(with->GetNPCTypeID(), 14) != 0) {
										const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 14));
										const ItemInst* insttmp = new ItemInst(itmtmp,0);
										PushItemOnCursor(*insttmp, true);
										database.RemoveBotItemBySlot(with->GetNPCTypeID(), 14);
									}
								}
							}
							else if((database.GetBotItemBySlot(with->GetNPCTypeID(), 13) != 0)  && inst->IsSlotAllowed(13)) {
								if((itmwp->ItemType == ItemType2HS) || (itmwp->ItemType == ItemType2HB) || (itmwp->ItemType == ItemType2HPierce)) {
									const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 13));
									const ItemInst* insttmp = new ItemInst(itmtmp,0);
									PushItemOnCursor(*insttmp, true);
									database.RemoveBotItemBySlot(with->GetNPCTypeID(), 13);
									database.SetBotItemInSlot(with->GetNPCTypeID(), 13, inst->GetID());
									if((database.GetBotItemBySlot(with->GetNPCTypeID(), 14) != 0)) {
										const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 14));
										const ItemInst* insttmp = new ItemInst(itmtmp,0);
										PushItemOnCursor(*insttmp, true);
										database.RemoveBotItemBySlot(with->GetNPCTypeID(), 14);
									}
								}
								else if((database.GetBotItemBySlot(with->GetNPCTypeID(), 14) == 0)  && inst->IsSlotAllowed(14)) {
									if(inst->IsWeapon() && with->GetLevel() < 13) {
										with->Say("I don't have the requered level to use two weapons.");
										PushItemOnCursor(*inst, true);
										DeleteItemInInventory(i);
										return;
									}
									database.SetBotItemInSlot(with->GetNPCTypeID(), 14, inst->GetID());
									const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 13));
									if(itmtmp && (itmtmp->ItemType == ItemType2HS) || (itmtmp->ItemType == ItemType2HB) || (itmtmp->ItemType == ItemType2HPierce)) {
										const ItemInst* insttmp = new ItemInst(itmtmp,0);
										PushItemOnCursor(*insttmp, true);
										database.RemoveBotItemBySlot(with->GetNPCTypeID(), 13);
									}
								}
								else if((database.GetBotItemBySlot(with->GetNPCTypeID(), 14) != 0)  && inst->IsSlotAllowed(14)) {
									const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 14));
									const ItemInst* insttmp = new ItemInst(itmtmp,0);
									PushItemOnCursor(*insttmp, true);
									database.RemoveBotItemBySlot(with->GetNPCTypeID(), 14);
									database.SetBotItemInSlot(with->GetNPCTypeID(), 14, inst->GetID());
								}
								else {
									with->Say("Use #bot inventory remove 13 to swap primary weapons when dual wielding.");
									PushItemOnCursor(*inst, true);
									DeleteItemInInventory(i);
									return;
								}
							}
							else if((database.GetBotItemBySlot(with->GetNPCTypeID(), 14) == 0)  && inst->IsSlotAllowed(14)) {
								if(inst->IsWeapon() && with->GetLevel() < 13) {
									with->Say("I don't have the required level to use two weapons.");
									PushItemOnCursor(*inst, true);
									DeleteItemInInventory(i);
									return;
								}
								database.SetBotItemInSlot(with->GetNPCTypeID(), 14, inst->GetID());
								const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 13));
								if(itmtmp && ((itmtmp->ItemType == ItemType2HS) || (itmtmp->ItemType == ItemType2HB) || (itmtmp->ItemType == ItemType2HPierce))) {
									const ItemInst* insttmp = new ItemInst(itmtmp,0);
									PushItemOnCursor(*insttmp, true);
									database.RemoveBotItemBySlot(with->GetNPCTypeID(), 13);
								}
							}
							else if((database.GetBotItemBySlot(with->GetNPCTypeID(), 14) != 0)  && inst->IsSlotAllowed(14)) {
								const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 14));
								const ItemInst* insttmp = new ItemInst(itmtmp,0);
								PushItemOnCursor(*insttmp, true);
								database.RemoveBotItemBySlot(with->GetNPCTypeID(), 14);
								database.SetBotItemInSlot(with->GetNPCTypeID(), 14, inst->GetID());
							}
							break;
						}
                        else if(j==15 || j==16) { // rings
							if(database.GetBotItemBySlot(with->GetNPCTypeID(), 16) == 0) {
								database.SetBotItemInSlot(with->GetNPCTypeID(), 16, inst->GetID());
							}
							else if(database.GetBotItemBySlot(with->GetNPCTypeID(), 15) == 0) {
								database.SetBotItemInSlot(with->GetNPCTypeID(), 15, inst->GetID());
							}
							else {
								const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), 15));
								const ItemInst* insttmp = new ItemInst(itmtmp,0);
								PushItemOnCursor(*insttmp, true);
								database.RemoveBotItemBySlot(with->GetNPCTypeID(), 15);
								database.SetBotItemInSlot(with->GetNPCTypeID(), 15, inst->GetID());
							}
							break;
						}					
						if(database.GetBotItemBySlot(with->GetNPCTypeID(), j) != 0) {
							const Item_Struct* itmtmp = database.GetItem(database.GetBotItemBySlot(with->GetNPCTypeID(), j));
							const ItemInst* insttmp = new ItemInst(itmtmp,0);
							PushItemOnCursor(*insttmp, true);
							database.RemoveBotItemBySlot(with->GetNPCTypeID(), j);
						}
						database.SetBotItemInSlot(with->GetNPCTypeID(), j, inst->GetID());
						break;
					}
				}
				with->CalcBotStats();
			}
		}
        if(inst) {
			if(with->IsBot() && !botCanWear[i-3000]) {
				PushItemOnCursor(*inst, true);
			}
            DeleteItemInInventory(i);
        }
	}
	if(!with->IsBot()) {

#else //EQBOTS

    for (sint16 i=3000; i<=3003; i++) {
        const ItemInst* inst = m_inv[i];
        if (inst) {
            items[i-3000]=inst->GetItem()->ID;
            charges[i-3000]=inst->GetCharges();
            DeleteItemInInventory(i);
        }
    }

#endif //EQBOTS

	//dont bother with this crap unless we have a quest...
	//pets can have quests! (especially charmed NPCs)
	bool did_quest = false;
#ifdef EMBPERL
	if(((PerlembParser *)parse)->HasQuestSub(with->GetNPCTypeID(), "EVENT_ITEM")) {
#else
	if(parse->HasQuestFile(with->GetNPCTypeID())) {
#endif
		char temp1[100];
		memset(temp1,0x0,100);
		char temp2[100];
		memset(temp2,0x0,100);
		for ( int z=0; z < 4; z++ ) {
			snprintf(temp1, 100, "item%d.%d", z+1,with->GetNPCTypeID());
			snprintf(temp2, 100, "%d",items[z]);
			parse->AddVar(temp1,temp2);
//			memset(temp1,0x0,100);
//			memset(temp2,0x0,100);
			snprintf(temp1, 100, "item%d.charges.%d", z+1,with->GetNPCTypeID());
			snprintf(temp2, 100, "%d",charges[z]);
			parse->AddVar(temp1,temp2);
//			memset(temp1,0x0,100);
//			memset(temp2,0x0,100);
		}
		snprintf(temp1, 100, "copper.%d",with->GetNPCTypeID());
		snprintf(temp2, 100, "%i",trade->cp);
		parse->AddVar(temp1,temp2);
//		memset(temp1,0x0,100);
//		memset(temp2,0x0,100);
		snprintf(temp1, 100, "silver.%d",with->GetNPCTypeID());
		snprintf(temp2, 100, "%i",trade->sp);
		parse->AddVar(temp1,temp2);
//		memset(temp1,0x0,100);
//		memset(temp2,0x0,100);
		snprintf(temp1, 100, "gold.%d",with->GetNPCTypeID());
		snprintf(temp2, 100, "%i",trade->gp);
		parse->AddVar(temp1,temp2);
//		memset(temp1,0x0,100);
//		memset(temp2,0x0,100);
		snprintf(temp1, 100, "platinum.%d",with->GetNPCTypeID());
		snprintf(temp2, 100, "%i",trade->pp);
		parse->AddVar(temp1,temp2);
//		memset(temp1,0x0,100);
//		memset(temp2,0x0,100);
		parse->Event(EVENT_ITEM, with->GetNPCTypeID(), NULL, with, this);
		did_quest = true;
	}
	if(RuleB(TaskSystem, EnableTaskSystem)) {
		int Cash = trade->cp + (trade->sp * 10) + (trade->gp * 100) + (trade->pp * 1000);
		if(UpdateTasksOnDeliver(items, Cash, with->GetNPCTypeID())) {
			if(!with->IsMoving()) 
				with->FaceTarget(this);
		}
	}
//		Message(0, "Normal NPC: keeping items.");
		
	//else, we do not have a quest, give the items to the NPC
	if(did_quest) {
		//only continue if we are a charmed NPC
		if(!with->HasOwner() || with->GetPetType() != petCharmed)
			return;
	}

#ifdef EQBOTS

	}

#endif //EQBOTS
		
	int xy = with->CountLoot();
	
	for(int y=0; y < 4; y++) {
		if (xy >= 20)
			break;
		xy++;
		//NPC* npc=with->CastToNPC();
		const Item_Struct* item2 = database.GetItem(items[y]);
		if (item2) {

#ifdef EQBOTS

			//if was not no drop item, let the NPC have it
			if((GetGM() && !with->IsBot()) || ((item2->NoDrop != 0) && !with->IsBot()))
				with->AddLootDrop(item2, &with->itemlist, charges[y], true, true);
			// franck-add: you can give nodrop items to bots
			else if(with->IsBot() && botCanWear[y]) {
				with->AddLootDrop(item2, &with->itemlist, charges[y], true, true);
                with->Say("Thank you for the %s , %s.", item2->Name, with->BotOwner->GetName());
			}
			else if(with->IsBot() && !botCanWear[y]) {
				with->Say("I can't use this %s!", item2->Name);
			}

#else //EQBOTS

            //if was not no drop item, let the NPC have it
			if(GetGM() || item2->NoDrop != 0)
				with->AddLootDrop(item2, &with->itemlist, charges[y], true, true);
			//else 
			//	with->AddLootDrop(item2, NULL, charges[y], false, true);

#endif //EQBOTS

		}
	}

}


void Client::FinishTrade(Client* other)
{
	sint16 slot_id;
	if (!other)
		return;

	mlog(TRADING__CLIENT, "Finishing trade with client %s", other->GetName());
	
	// Move each trade slot into free inventory slot
	for (sint16 i=3000; i<=3007; i++){
		const ItemInst* inst = m_inv[i];
		if(inst == NULL)
			continue;

		mlog(TRADING__CLIENT, "Giving %s (%d) in slot %d to %s", inst->GetItem()->Name, inst->GetItem()->ID, i, other->GetName());
		
		if (inst->GetItem()->NoDrop != 0 || other == this) {
			slot_id = other->GetInv().FindFreeSlot(inst->IsType(ItemClassContainer), true, inst->GetItem()->Size);

			mlog(TRADING__CLIENT, "Trying to put %s (%d) into slot %d", inst->GetItem()->Name, inst->GetItem()->ID, slot_id);
			if (other->PutItemInInventory(slot_id, *inst, true)) {
				mlog(TRADING__CLIENT, "Item  %s (%d) successfully transfered, deleting from trade slot.", inst->GetItem()->Name, inst->GetItem()->ID);
			} else {
				PushItemOnCursor(*inst, true);
				mlog(TRADING__ERROR, "Unable to give item %d (%d) to %s, returning to giver.", inst->GetItem()->Name, inst->GetItem()->ID, other->GetName());
			}
			DeleteItemInInventory(i);
		} else {
			PushItemOnCursor(*inst, true);
			DeleteItemInInventory(i);
		}
	}
	
	// Money @merth: look into how NPC's receive cash
	this->AddMoneyToPP(other->trade->cp, other->trade->sp, other->trade->gp, other->trade->pp, true);
	
	//Do not reset the trade here, done by the caller.
}

bool Client::CheckTradeLoreConflict(Client* other)
{
	if (!other)
		return true;
	// Move each trade slot into free inventory slot
	for (sint16 i=3000; i<=3007; i++){
		const ItemInst* inst = m_inv[i];

		if (inst && inst->GetItem()) {
			if (other->CheckLoreConflict(inst->GetItem()))
				return true;
		}
	}
	return false;
}

void Client::Trader_ShowItems(){
	EQApplicationPacket* outapp= new EQApplicationPacket(OP_Trader, sizeof(Trader_Struct));

	Trader_Struct* outints = (Trader_Struct*)outapp->pBuffer;
	Trader_Struct* TraderItems = database.LoadTraderItem(this->CharacterID());

	for(int i = 0; i < 80; i++){
		outints->ItemCost[i] = TraderItems->ItemCost[i];
		outints->Items[i] = TraderItems->Items[i];
	}
	outints->Code = BazaarTrader_ShowItems;

	QueuePacket(outapp);
	_pkt(TRADING__PACKETS, outapp);
	safe_delete(outapp);
	safe_delete(TraderItems);
}

void Client::SendTraderPacket(Client* Trader) {

	EQApplicationPacket* outapp= new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->Code = BazaarTrader_StartTraderMode;

	bts->ID = Trader->GetID();

	QueuePacket(outapp);

	_pkt(TRADING__PACKETS, outapp);

	safe_delete(outapp);
}

void Client::Trader_CustomerBrowsing(Client *Customer) {

	EQApplicationPacket* outapp= new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));

	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp->pBuffer;

	sis->Code = BazaarTrader_CustomerBrowsing;

	sis->TraderID = Customer->GetID();

	QueuePacket(outapp);
}


void Client::Trader_StartTrader() {

	Trader=true;

	EQApplicationPacket* outapp= new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));

	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp->pBuffer;

	sis->Code = BazaarTrader_StartTraderMode;

	sis->TraderID = this->GetID();

	QueuePacket(outapp);

	_pkt(TRADING__PACKETS, outapp);

	safe_delete(outapp);

	// Notify other clients we are now in trader mode
	//
	outapp= new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->Code = 1;

	bts->ID = this->GetID();

	entity_list.QueueClients(this, outapp, false);

	_pkt(TRADING__PACKETS, outapp);

	safe_delete(outapp);
}

void Client::Trader_EndTrader() {

	database.DeleteTraderItem(this->CharacterID());

	// Notify other clients we are no longer in trader mode.
	//
	EQApplicationPacket* outapp= new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->Code = 0;

	bts->ID = this->GetID();

	entity_list.QueueClients(this, outapp, false);

	_pkt(TRADING__PACKETS, outapp);

	safe_delete(outapp);

	outapp= new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));

	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp->pBuffer;

	sis->Code = BazaarTrader_EndTraderMode;

	sis->TraderID = BazaarTrader_EndTraderMode;

	QueuePacket(outapp);

	_pkt(TRADING__PACKETS, outapp);

	safe_delete(outapp);

	WithCustomer(0);

	this->Trader = false;
}

void Client::SendTraderItem(int32 ItemID, int16 Quantity) {

	string Packet;
	sint16 FreeSlotID=0;

	const Item_Struct* item = database.GetItem(ItemID);

	if(!item){
		_log(TRADING__CLIENT, "Bogus item deleted in Client::SendTraderItem!\n");
		return;
	}
	
	ItemInst* inst = database.CreateItem(item, Quantity);

	if (inst) {
		FreeSlotID = m_inv.FindFreeSlot(false, true, inst->GetItem()->Size);

		PutItemInInventory(FreeSlotID, *inst);
		Save();
		
		SendItemPacket(FreeSlotID, inst, ItemPacketTrade);

		safe_delete(inst);
	}
}

void Client::SendSingleTraderItem(int32 CharID, int SerialNumber) {

	ItemInst* inst= database.LoadSingleTraderItem(CharID, SerialNumber);
	if(inst) {
		SendItemPacket(30, inst, ItemPacketMerchant);
		safe_delete(inst);
	}

}

void Client::BulkSendTraderInventory(int32 char_id) {
	const Item_Struct *item;

	TraderCharges_Struct* TraderItems = database.LoadTraderItemWithCharges(char_id);

	for (int8 i = 0;i < 80; i++) {
		if(TraderItems->ItemID[i] == 0) {
			continue;
		}
		else
			item=database.GetItem(TraderItems->ItemID[i]);
		
		if (item && (item->NoDrop!=0)) {
			ItemInst* inst = database.CreateItem(item);
			if (inst) {
				inst->SetSerialNumber(TraderItems->SerialNumber[i]);
				if(TraderItems->Charges[i] > 0)
					inst->SetCharges(TraderItems->Charges[i]);

				if(inst->IsStackable()) {
					inst->SetMerchantCount(TraderItems->Charges[i]);
					inst->SetMerchantSlot(TraderItems->SerialNumber[i]);
				}

				inst->SetPrice(TraderItems->ItemCost[i]);
				SendItemPacket(30, inst, ItemPacketMerchant);
				safe_delete(inst);
			}
			else
				_log(TRADING__CLIENT, "Client::BulkSendTraderInventory NULL inst pointer");
		}
		else
			_log(TRADING__CLIENT, "Client::BulkSendTraderInventory NULL item pointer or item is NODROP %8X",item);
	}
	safe_delete(TraderItems);
}

ItemInst* Client::FindTraderItemBySerialNumber(sint32 SerialNumber){

	ItemInst* item = NULL;
	int16 SlotID = 0;
	for(int i = 0; i < 8;i++){
		item = this->GetInv().GetItem(22 + i);
		if(item && item->GetItem()->ID == 17899){ //Traders Satchel
			for(int x = 0; x < 10; x++){
				SlotID = (((22 + i + 3) * 10) + x + 1);
				item = this->GetInv().GetItem(SlotID);
				if(item) {
					if(item->GetSerialNumber() == SerialNumber) 
						return item;
				}
			}
		}
	}
	_log(TRADING__CLIENT, "Client::FindTraderItemBySerialNumber Couldn't find item! Serial No. was %i", SerialNumber);

	return NULL;
}


GetItems_Struct* Client::GetTraderItems(){

	const ItemInst* item = NULL;
	int16 SlotID = 0;

	GetItems_Struct* gis= new GetItems_Struct;

	memset(gis,0,sizeof(GetItems_Struct));

	int8 ndx = 0;

	for(int i = 0; i < 8; i++){
		item = this->GetInv().GetItem(22 + i);
		if(item && item->GetItem()->ID == 17899){ //Traders Satchel
			for(int x = 0; x < 10; x++){
				SlotID = (((22 + i +3 ) *10) + x + 1);

				item = this->GetInv().GetItem(SlotID);

				if(item){
					gis->Items[ndx] = item->GetItem()->ID;
					gis->SerialNumber[ndx] = item->GetSerialNumber();
					gis->Charges[ndx] = item->GetCharges();
					ndx++;
				}
			}
		}
	}
	return gis;
}

int16 Client::FindTraderItem(sint32 SerialNumber, int16 Quantity){

	const ItemInst* item= NULL;
	int16 SlotID = 0;
	for(int i = 0; i < 8;i++){
		item = this->GetInv().GetItem(22+i);
		if(item && item->GetItem()->ID == 17899){ //Traders Satchel
			for(int x = 0; x < 10; x++){
				SlotID= (((22 + i + 3) * 10) + x + 1);

				item = this->GetInv().GetItem(SlotID);

				if(item && item->GetSerialNumber() == SerialNumber && 
				   (item->GetCharges() >= Quantity || (item->GetCharges() <= 0 && Quantity == 1))){

					return SlotID;
				}
			}
		}
	}
	_log(TRADING__CLIENT, "Could NOT find a match for Item: %i with a quantity of: %i on Trader: %s\n", 
			      SerialNumber , Quantity, this->GetName());

	return 0;
}

void Client::NukeTraderItem(int16 Slot,sint16 Charges,int16 Quantity,Client* Customer,int16 TraderSlot, int SerialNumber) {

	if(!Customer) return;
	_log(TRADING__CLIENT, "NukeTraderItem(Slot %i, Charges %i, Quantity %i", Slot, Charges, Quantity);
	if(Quantity < Charges)
		Customer->SendSingleTraderItem(this->CharacterID(), SerialNumber);
	else {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_TraderDelItem,sizeof(TraderDelItem_Struct));
		TraderDelItem_Struct* tdis = (TraderDelItem_Struct*)outapp->pBuffer;

		tdis->Unknown000 = 0;
		tdis->TraderID = Customer->GetID();
		tdis->ItemID = SerialNumber;
		tdis->Unknown012 = 0;

		_pkt(TRADING__PACKETS, outapp);

		Customer->QueuePacket(outapp);
		safe_delete(outapp);

	}
	// This updates the trader. Removes it from his trading bags.
	//
	m_inv.DeleteItem(Slot, Quantity);
	const ItemInst* Inst = m_inv[Slot];

	if(Inst)
		database.SaveInventory(CharacterID(), Inst, Slot);

	EQApplicationPacket* outapp2;

	if(Quantity < Charges)
		outapp2 = new EQApplicationPacket(OP_DeleteItem,sizeof(MoveItem_Struct));
	else
		outapp2 = new EQApplicationPacket(OP_MoveItem,sizeof(MoveItem_Struct));

	MoveItem_Struct* mis = (MoveItem_Struct*)outapp2->pBuffer;
	mis->from_slot = Slot;
	mis->to_slot = 0xFFFFFFFF;
	mis->number_in_stack = 0xFFFFFFFF;

	if(Quantity >= Charges)
		Quantity = 1;

	for(int i = 0; i < Quantity; i++) {
		_pkt(TRADING__PACKETS, outapp2);

		this->QueuePacket(outapp2);
	}
	safe_delete(outapp2);

}
void Client::TraderUpdate(int16 SlotID,int32 TraderID){
	// This method is no longer used.

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TraderItemUpdate,sizeof(TraderItemUpdate_Struct));
	TraderItemUpdate_Struct* tus=(TraderItemUpdate_Struct*)outapp->pBuffer;
	tus->Charges = 0xFFFF;
	tus->FromSlot = SlotID;
	tus->ToSlot = 0xFF;
	tus->TraderID = TraderID;
	tus->Unknown000 = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::FindAndNukeTraderItem(int32 SerialNumber, int16 Quantity, Client* Customer, int16 TraderSlot){

	const ItemInst* item= NULL;
	bool Stackable = false;
	sint16 Charges=0;

	int16 SlotID = FindTraderItem(SerialNumber, Quantity);
	if(SlotID > 0){

		item = this->GetInv().GetItem(SlotID);

		if(item) {
			Charges = this->GetInv().GetItem(SlotID)->GetCharges();

			Stackable = item->IsStackable();

			if(!Stackable) 
				Quantity = (Charges > 0) ? Charges : 1;

		}
		_log(TRADING__CLIENT, "FindAndNuke %s, Charges %i, Quantity %i", item->GetItem()->Name, Charges, Quantity);
		if(item && (Charges <= Quantity || (Charges <= 0 && Quantity==1) || !Stackable)){
			this->DeleteItemInInventory(SlotID, Quantity);

			TraderCharges_Struct* GetSlot = database.LoadTraderItemWithCharges(this->CharacterID());

			int8 Count = 0;

			bool TestSlot = true;

			for(int y = 0;y < 80;y++){

				if(TestSlot && GetSlot->SerialNumber[y] == SerialNumber){

					database.DeleteTraderItem(this->CharacterID(),y);
					NukeTraderItem(SlotID, Charges, Quantity, Customer, TraderSlot, GetSlot->SerialNumber[y]);
					TestSlot=false;
				}
				else if(GetSlot->ItemID[y] > 0)
					Count++;
			}
			if(Count == 0) 
				Trader_EndTrader();

			return;
		}
		else if(item) {
			database.UpdateTraderItemCharges(this->CharacterID(), item->GetSerialNumber(), Charges-Quantity);

			NukeTraderItem(SlotID, Charges, Quantity, Customer, TraderSlot, item->GetSerialNumber());

			return;

		}
	}
	_log(TRADING__CLIENT, "Could NOT find a match for Item: %i with a quantity of: %i on Trader: %s\n",SerialNumber, 
			      Quantity,this->GetName());
}

void Client::ReturnTraderReq(const EQApplicationPacket* app, sint16 TraderItemCharges){

	TraderBuy_Struct* tbs = (TraderBuy_Struct*)app->pBuffer;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));

	TraderBuy_Struct* outtbs  = (TraderBuy_Struct*)outapp->pBuffer;

	memcpy(outtbs, tbs, app->size);

	outtbs->Price = (tbs->Price * TraderItemCharges);

	outtbs->Quantity = TraderItemCharges;

	outtbs->TraderID = this->GetID();

	outtbs->AlreadySold = 0;

	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::TradeRequestFailed(const EQApplicationPacket* app) {

	TraderBuy_Struct* tbs = (TraderBuy_Struct*)app->pBuffer;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));

	TraderBuy_Struct* outtbs = (TraderBuy_Struct*)outapp->pBuffer;

	memcpy(outtbs, tbs, app->size);

	outtbs->AlreadySold = 0xFFFFFFFF;

	outtbs->TraderID = 0xFFFFFFFF;;

	QueuePacket(outapp);

	safe_delete(outapp);
}


static void BazaarAuditTrail(const char *Seller, const char *Buyer, const char *ItemName, int Quantity, int TotalCost) {

	const char *AuditQuery="INSERT INTO `trader_audit` (`time`, `seller`, `buyer`, `itemname`, `quantity`, `totalcost`) "
			  "VALUES (NOW(), '%s', '%s', '%s', %i, %i)";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	if(!database.RunQuery(query, MakeAnyLenString(&query, AuditQuery, Seller, Buyer, ItemName, Quantity, TotalCost), errbuf))
		_log(TRADING__CLIENT, "Audit write error: %s : %s", query, errbuf);

	safe_delete_array(query);
}



void Client::BuyTraderItem(TraderBuy_Struct* tbs,Client* Trader,const EQApplicationPacket* app){

	if(!Trader) return;

	if(!Trader->IsTrader()) {
		TradeRequestFailed(app);
		return;
	}

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Trader,sizeof(TraderBuy_Struct));

	TraderBuy_Struct* outtbs  = (TraderBuy_Struct*)outapp->pBuffer;

	outtbs->ItemID = tbs->ItemID;

	const ItemInst* BuyItem = Trader->FindTraderItemBySerialNumber(tbs->ItemID);

	if(!BuyItem) {
		_log(TRADING__CLIENT, "Unable to find item on trader.");
		TradeRequestFailed(app);	
		safe_delete(outapp);
		return;
	}

	_log(TRADING__CLIENT, "Buyitem: Name: %s, IsStackable: %i, Requested Quantity: %i, Charges on Item %i",
			      BuyItem->GetItem()->Name, BuyItem->IsStackable(), tbs->Quantity, BuyItem->GetCharges());
	// If the item is not stackable, then we can only be buying one of them.
	if(!BuyItem->IsStackable()) 
		 outtbs->Quantity = tbs->Quantity;
	else {
		// Stackable items, arrows, diamonds, etc
		int ItemCharges = BuyItem->GetCharges();
		// ItemCharges for stackables should not be <= 0
		if(ItemCharges <= 0)
			outtbs->Quantity = 1;
		// If the purchaser requested more than is in the stack, just sell them how many are actually in the stack.
		else if(ItemCharges < (sint16)tbs->Quantity)
			outtbs->Quantity = ItemCharges;
		else
			outtbs->Quantity = tbs->Quantity;
	}

	_log(TRADING__CLIENT, "Actual quantity that will be traded is %i", outtbs->Quantity);

	if((tbs->Price * outtbs->Quantity) <= 0) {
		Message(13, "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1");
		Trader->Message(13, "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1");
		LogFile->write(EQEMuLog::Error, "Bazaar: Zero price transaction between %s and %s aborted."
						"Item: %s, Charges: %i, TBS: Qty %i, Price: %i",
						GetName(), Trader->GetName(),
						BuyItem->GetItem()->Name, BuyItem->GetCharges(), tbs->Quantity, tbs->Price);
		TradeRequestFailed(app);
		safe_delete(outapp);
		return;
	}

	ReturnTraderReq(app, outtbs->Quantity);

	outtbs->TraderID = this->GetID();

	outtbs->Action = BazaarBuyItem;

	strncpy(outtbs->ItemName, BuyItem->GetItem()->Name, 64);

	int TraderSlot = 0;

	if(BuyItem->IsStackable())
		SendTraderItem(BuyItem->GetItem()->ID, outtbs->Quantity);
	else
		SendTraderItem(BuyItem->GetItem()->ID, BuyItem->GetCharges());


	EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_MoneyUpdate,sizeof(MoneyUpdate_Struct));

	MoneyUpdate_Struct* mus= (MoneyUpdate_Struct*)outapp2->pBuffer;
	
	int32 TotalCost = tbs->Price * outtbs->Quantity;

	outtbs->Price = TotalCost;

	this->TakeMoneyFromPP(TotalCost);

	mus->platinum = TotalCost / 1000;

	TotalCost -= (mus->platinum * 1000);

	mus->gold = TotalCost / 100;

	TotalCost -= (mus->gold * 100);

	mus->silver = (int) TotalCost / 10;

	TotalCost -= (mus->silver * 10);

	mus->copper = TotalCost;

	Trader->AddMoneyToPP(mus->copper,mus->silver,mus->gold,mus->platinum,false);

	mus->platinum = Trader->GetPlatinum();
	mus->gold = Trader->GetGold();
	mus->silver = Trader->GetSilver();
	mus->copper = Trader->GetCopper();

	TraderSlot = Trader->FindTraderItem(tbs->ItemID, outtbs->Quantity);

	Trader->QueuePacket(outapp2);

	_pkt(TRADING__PACKETS, outapp2);

	if(RuleB(Bazaar, AuditTrail))
		BazaarAuditTrail(Trader->GetName(), GetName(), BuyItem->GetItem()->Name, outtbs->Quantity, outtbs->Price);

	Trader->FindAndNukeTraderItem(tbs->ItemID, outtbs->Quantity, this, 0);

	Trader->QueuePacket(outapp);

	_pkt(TRADING__PACKETS, outapp);

	safe_delete(outapp);
	safe_delete(outapp2);

}

void Client::SendBazaarWelcome(){

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	MYSQL_RES *result;

	MYSQL_ROW row;

	if (database.RunQuery(query,MakeAnyLenString(&query, "select count(distinct char_id),count(char_id) from trader"),errbuf,&result)){
		if(mysql_num_rows(result)==1){

			row = mysql_fetch_row(result);

			EQApplicationPacket* outapp = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarWelcome_Struct));

			memset(outapp->pBuffer,0,outapp->size);

			BazaarWelcome_Struct* bws = (BazaarWelcome_Struct*)outapp->pBuffer;

			bws->Beginning.Action = BazaarWelcome;

			bws->Items = atoi(row[1]);

			bws->Traders = atoi(row[0]);

			QueuePacket(outapp);

			safe_delete(outapp);
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}

void Client::SendBazaarResults(int32 TraderID, int32 Class_, int32 Race, int32 ItemStat, int32 Slot, int32 Type,
			       char Name[64], int32 MinPrice, int32 MaxPrice) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* Query = 0;
	string Search, Values;
	MYSQL_RES *Result;
	MYSQL_ROW Row;
	char Tmp[100] = {0};

	Values.append("count(item_id),trader.*,items.name");

	Search.append("where trader.item_id=items.id");

	if(TraderID > 0){
		Client* Trader = entity_list.GetClientByID(TraderID);

		if(Trader){
			sprintf(Tmp," and trader.char_id=%i",Trader->CharacterID());
			Search.append(Tmp);
		}
			
	}
	string SearchrResults;

	if(MinPrice != 0){
		sprintf(Tmp, " and trader.item_cost>=%i", MinPrice);
		Search.append(Tmp);
	}
	if(MaxPrice != 0){
		sprintf(Tmp, " and trader.item_cost<=%i", MaxPrice);
		Search.append(Tmp);
	}
	if(strlen(Name) > 0){
		sprintf(Tmp, " and items.name like '%%%s%%'", Name);
		Search.append(Tmp);
	}
	if(Class_ != 0xFFFFFFFF){
			sprintf(Tmp, " and mid(reverse(bin(items.classes)),%i,1)=1", Class_);
			Search.append(Tmp);
	}
	if(Race!=0xFFFFFFFF){
			sprintf(Tmp, " and mid(reverse(bin(items.races)),%i,1)=1", Race);
			Search.append(Tmp);
	}
	if(Slot!=0xFFFFFFFF){
			sprintf(Tmp, " and mid(reverse(bin(items.slots)),%i,1)=1", Slot + 1);
			Search.append(Tmp);
	}
	if(Type!=0xFFFFFFFF){
		switch(Type){

			case 0:
				// 1H Slashing
				Search.append(" and items.itemtype=0 and damage>0");
				break;
			case 31:
				Search.append(" and items.itemclass=2");
				break;
			case 46:
				Search.append(" and items.spellid>0 and items.spellid<65000");
				break;
			case 47:
				Search.append(" and items.spellid=998");
				break;
			case 48:
				Search.append(" and items.spellid>=1298 and items.spellid<=1307");
				break;
			case 49:
				Search.append(" and items.focusid>0");
				break;
			default:
				sprintf(Tmp, " and items.itemtype=%i", Type);
				Search.append(Tmp);
		}
	}
		
	switch(ItemStat) {

		case STAT_AC:
			Search.append(" and items.ac>0");
			Values.append(",items.ac");
			break;

		case STAT_AGI:
			Search.append(" and items.aagi>0");
			Values.append(",items.aagi");
			break;

		case STAT_CHA:
			Search.append(" and items.acha>0");
			Values.append(",items.acha");
			break;

		case STAT_DEX:
			Search.append(" and items.adex>0");
			Values.append(",items.adex");
			break;

		case STAT_INT:
			Search.append(" and items.aint>0");
			Values.append(",items.aint");
			break;

		case STAT_STA:
			Search.append(" and items.asta>0");
			Values.append(",items.asta");
			break;

		case STAT_STR:
			Search.append(" and items.astr>0");
			Values.append(",items.astr");
			break;
		
		case STAT_WIS:
			Search.append(" and items.awis>0");
			Values.append(",items.awis");
			break;

		case STAT_COLD:
			Search.append(" and items.cr>0");
			Values.append(",items.cr");
			break;
		
		case STAT_DISEASE:
			Search.append(" and items.dr>0");
			Values.append(",items.dr");
			break;
	
		case STAT_FIRE:
			Search.append(" and items.fr>0");
			Values.append(",items.fr");
			break;

		case STAT_MAGIC:
			Values.append(",items.mr");
			Search.append(" and items.mr>0");
			break;

		case STAT_POISON:
			Search.append(" and items.pr>0");
			Values.append(",items.pr");
			break;

		case STAT_HP:
			Search.append(" and items.hp>0");
			Values.append(",items.hp");
			break;

		case STAT_MANA:
			Search.append(" and items.mana>0");
			Values.append(",items.mana");
			break;

		default:
			Values.append(",0");
			break;
	}

	Values.append(",sum(charges), items.stackable ");

	if (database.RunQuery(Query,MakeAnyLenString(&Query, "select %s from trader,items %s group by items.id,charges,char_id limit %i",
						     Values.c_str(),Search.c_str(), RuleI(Bazaar, MaxSearchResults)),errbuf,&Result)){

		_log(TRADING__CLIENT, "SRCH: %s", Query);
		safe_delete_array(Query);

		int Size = 0;
		int32 ID = 0;

		if(mysql_num_rows(Result) == 0){
			EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
			BazaarReturnDone_Struct* brds = (BazaarReturnDone_Struct*)outapp2->pBuffer;
			brds->TraderID = ID;
			brds->Type = BazaarSearchDone;
			brds->Unknown008 = 0xFFFFFFFF;
			brds->Unknown012 = 0xFFFFFFFF;
			brds->Unknown016 = 0xFFFFFFFF;
			this->QueuePacket(outapp2);
			_pkt(TRADING__PACKETS,outapp2);
			safe_delete(outapp2);
			mysql_free_result(Result);
			return;
		}
		Size = mysql_num_rows(Result) * sizeof(BazaarSearchResults_Struct);
		uchar *buffer = new uchar[Size];
		uchar *bufptr = buffer;
		memset(buffer, 0, Size);

		int Action = BazaarSearchResults;
		int32 Cost = 0;
		sint32 SerialNumber = 0;
		char Name[64] = {0};
		int Count = 0;
		int32 StatValue=0;

		while ((Row = mysql_fetch_row(Result))) {
			VARSTRUCT_ENCODE_TYPE(int32, bufptr, Action);
			Count = atoi(Row[0]);
			VARSTRUCT_ENCODE_TYPE(int32, bufptr, Count);
			SerialNumber = atoi(Row[3]);
			VARSTRUCT_ENCODE_TYPE(sint32, bufptr, SerialNumber);
			Client* Trader2=entity_list.GetClientByCharID(atoi(Row[1]));
			if(Trader2){
				ID = Trader2->GetID();
				VARSTRUCT_ENCODE_TYPE(int32, bufptr, ID);
			}
			else{
				_log(TRADING__CLIENT, "Unable to find trader: %i\n",atoi(Row[1]));
				VARSTRUCT_ENCODE_TYPE(int32, bufptr, 0);
			}
			Cost = atoi(Row[5]);
			VARSTRUCT_ENCODE_TYPE(int32, bufptr, Cost);
			StatValue = atoi(Row[8]);
			VARSTRUCT_ENCODE_TYPE(int32, bufptr, StatValue);
			bool Stackable = atoi(Row[10]);
			if(Stackable) {
				int Charges = atoi(Row[9]);
				sprintf(Name, "%s(%i)", Row[7], Charges);
			}
			else
				sprintf(Name,"%s(%i)",Row[7], Count);

			memcpy(bufptr,&Name, strlen(Name));

			bufptr += 64;
		}
		mysql_free_result(Result);

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_BazaarSearch, Size);

		memcpy(outapp->pBuffer, buffer, Size);

		this->QueuePacket(outapp);

		_pkt(TRADING__PACKETS,outapp);

		safe_delete(outapp);
		safe_delete_array(buffer);

		EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
		BazaarReturnDone_Struct* brds = (BazaarReturnDone_Struct*)outapp2->pBuffer;

		brds->TraderID = ID;
		brds->Type = BazaarSearchDone;

		brds->Unknown008 = 0xFFFFFFFF;
		brds->Unknown012 = 0xFFFFFFFF;
		brds->Unknown016 = 0xFFFFFFFF;

		this->QueuePacket(outapp2);

		_pkt(TRADING__PACKETS,outapp2);
		safe_delete(outapp2);
		
	}
	else{
		_log(TRADING__CLIENT, "Failed to retrieve Bazaar Search!! %s\n", Query);
		safe_delete_array(Query);
		return;
	}
}
