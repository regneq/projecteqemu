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
	for (sint16 i=3000; i<=3003; i++){
		const ItemInst* inst = m_inv[i];
		if (inst) {
			items[i-3000]=inst->GetItem()->ID;
			charges[i-3000]=inst->GetCharges();
			DeleteItemInInventory(i);
		}
	}
	
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
		
	int xy = with->CountLoot();
	
	for(int y=0; y < 4; y++) {
		if (xy >= 20)
			break;
		xy++;
		//NPC* npc=with->CastToNPC();
		const Item_Struct* item2 = database.GetItem(items[y]);
		if (item2) {
			//if was not no drop item, let the NPC have it
			if(GetGM() || item2->NoDrop != 0)
				with->AddLootDrop(item2, &with->itemlist, charges[y], true, true);
			//else 
			//	with->AddLootDrop(item2, NULL, charges[y], false, true);
			
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
	Trader_Struct* outints2 = database.LoadTraderItem(this->CharacterID());
	for(int i=0;i<160;i=i+2){
		if(i==0){
			outints->itemcost[i]=outints2->itemcost[i];
			outints->itemid[i+1]=outints2->itemid[i];
		}
		else{
			outints->itemcost[(i/2)]=outints2->itemcost[(i/2)];
			outints->itemid[i+1]=outints2->itemid[(i/2)];
		}
	}
	outints->code=11;
	QueuePacket(outapp);
	safe_delete(outapp);
	safe_delete(outints2);
}

void Client::SendTraderPacket(Client* trader){
	EQApplicationPacket* outapp= new EQApplicationPacket(OP_BecomeTrader,sizeof(BecomeTrader_Struct));
	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;
	bts->code=1;
	bts->id=trader->GetID();
	//outapp->Deflate();
	this->QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Trader_StartTrader(){
	Trader=true;
	EQApplicationPacket* outapp2= new EQApplicationPacket(OP_Trader,sizeof(Trader_ShowItems_Struct));
	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp2->pBuffer;
	sis->code=1;
	sis->traderid=this->GetID();
	QueuePacket(outapp2);
	safe_delete(outapp2);
	EQApplicationPacket* outapp= new EQApplicationPacket(OP_BecomeTrader,sizeof(BecomeTrader_Struct));
	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;
	bts->code=1;
	bts->id=this->GetID();
	entity_list.QueueCloseClients(this,outapp,false,15000);
	safe_delete(outapp);
}
void Client::Trader_EndTrader(){
	database.DeleteTraderItem(this->CharacterID());
	EQApplicationPacket* outapp= new EQApplicationPacket(OP_BecomeTrader,sizeof(BecomeTrader_Struct));
	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;
	bts->code=0;
	bts->id=this->GetID();
	entity_list.QueueCloseClients(this,outapp,false,5000);
	safe_delete(outapp);
	EQApplicationPacket* outapp2= new EQApplicationPacket(OP_Trader,sizeof(Trader_ShowItems_Struct));
	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp2->pBuffer;
	sis->code=2;
	sis->traderid=0;
	QueuePacket(outapp2);
	safe_delete(outapp2);
	this->withcustomer=false;
	this->Trader=false;
}
void Client::SendTraderItem(int32 item_id,int16 quantity){
	string packet;
	sint16 freeslotid=0;
	const Item_Struct* item = database.GetItem(item_id);
	if(!item){
		printf("Bogus item deleted in Client::SendTraderItem!\n");
		return;
	}
	
	ItemInst* inst = database.CreateItem(item, quantity);
	if (inst) {
		freeslotid = m_inv.FindFreeSlot(false, true, inst->GetItem()->Size);
		PutItemInInventory(freeslotid, *inst);
		SendItemPacket(freeslotid, inst, ItemPacketTrade);
		safe_delete(inst);
	}
}



void Client::BulkSendTraderInventory(int32 char_id) {
  const Item_Struct *item;
  Trader_Struct* outints2 = database.LoadTraderItem(char_id);
  for (int8 i=0;i<80;i++) {
		if(outints2->itemid[i]==0)
			continue;
		else
			item=database.GetItem(outints2->itemid[i]);
		
		if (item && (item->NoDrop!=0)) {
			ItemInst* inst = database.CreateItem(item);
			if (inst) {
				inst->SetPrice(outints2->itemcost[i]);
				//inst->SetUnknown5(outints2->itemid[i]);
				SendItemPacket(30, inst, ItemPacketMerchant);
				safe_delete(inst);
			}
		}
	}
}


int16 Client::FindTraderItemCharges(int32 item_id){
	const ItemInst* item= NULL;
	int16 slotid=0;
	for(int i=0;i<8;i++){
		item=this->GetInv().GetItem(22+i);
		if(item && item->GetItem()->ID==17899){ //Traders Satchel
			for(int x=0;x<10;x++){
				slotid=(((22+i+3)*10)+x+1);
				item=this->GetInv().GetItem(slotid);
				if(item && item->GetItem()->ID==item_id)
					return item->GetCharges();
			}
		}
	}
	return 9999;
}
GetItems_Struct* Client::GetTraderItems(){
	const ItemInst* item= NULL;
//	int16 charges=0;
	int16 slotid=0;
	GetItems_Struct* gis= new GetItems_Struct;
	memset(gis,0,sizeof(GetItems_Struct));
	int8 ndx=0;
	for(int i=0;i<8;i++){
		item=this->GetInv().GetItem(22+i);
		if(item && item->GetItem()->ID==17899){ //Traders Satchel
			for(int x=0;x<10;x++){
				slotid=(((22+i+3)*10)+x+1);
				item=this->GetInv().GetItem(slotid);
				if(item){
					gis->items[ndx]=item->GetItem()->ID;
					ndx++;
				}
			}
		}
	}
	return gis;
}
int16 Client::FindTraderItem(int32 item_id,int16 quantity){
	const ItemInst* item= NULL;
//	int16 charges=0;
	int16 slotid=0;
	for(int i=0;i<8;i++){
		item=this->GetInv().GetItem(22+i);
		if(item && item->GetItem()->ID==17899){ //Traders Satchel
			for(int x=0;x<10;x++){
				slotid=(((22+i+3)*10)+x+1);
				item=this->GetInv().GetItem(slotid);
				if(item && item->GetItem()->ID==item_id && (item->GetCharges()>=quantity || (item->GetCharges()==0 && quantity==1))){
					return slotid;
				}
			}
		}
	}
	printf("Could NOT find a match for Item: %i with a quantity of: %i on Trader: %s\n",item_id,quantity,this->GetName());
	return 0;
}
void Client::NukeTraderItem(int16 slot,int16 charges,int16 quantity,Client* customer,int16 traderslot){
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TraderDelItem,sizeof(TraderDelItem_Struct));
	TraderDelItem_Struct* tdis = (TraderDelItem_Struct*)outapp->pBuffer;
	tdis->quantity=0xFFFFFFFF;
	tdis->unknown=0xFFFFFFFF;
	tdis->slotid=slot;
	if(quantity<=20 && charges>quantity){
		for(int y=0;y<quantity;y++)
			this->QueuePacket(outapp);
	}
	else{
		EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_MoveItem,sizeof(MoveItem_Struct));
		MoveItem_Struct* mis=(MoveItem_Struct*)outapp2->pBuffer;
		mis->from_slot=slot;
		mis->to_slot=0xFFFFFFFF;
		mis->number_in_stack=0;
		this->QueuePacket(outapp2);
		safe_delete(outapp2);
		customer->TraderUpdate(traderslot,this->GetID());
	}
	safe_delete(outapp);
}
void Client::TraderUpdate(int16 slot_id,int32 trader_id){
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TraderItemUpdate,sizeof(TraderItemUpdate_Struct));
	TraderItemUpdate_Struct* tus=(TraderItemUpdate_Struct*)outapp->pBuffer;
	tus->charges=0xFFFF;
	tus->fromslot=slot_id;
	tus->toslot=0xFF;
	tus->traderid=trader_id;
	tus->unknown0=0;
	QueuePacket(outapp);
	safe_delete(outapp);
}
void Client::FindAndNukeTraderItem(int32 item_id,int16 quantity,Client* customer,int16 traderslot){
	const ItemInst* item= NULL;
	int16 charges=0;
	int16 slotid=FindTraderItem(item_id,quantity);
	if(slotid>0){
		item=this->GetInv().GetItem(slotid);
		if(item)
			charges=this->GetInv().GetItem(slotid)->GetCharges();
		if(item && item->GetItem()->ID==item_id && (charges>=quantity || (charges==0 && quantity==1))){
			this->DeleteItemInInventory(slotid,quantity);
			Trader_Struct* getslot = database.LoadTraderItem(this->CharacterID());
			int8 count=0;
			bool testslot=true;
			for(int y=0;y<80;y++){
				if(testslot && getslot->itemid[y]==item_id){
					database.DeleteTraderItem(this->CharacterID(),y);
					testslot=false;
				}
				else if(getslot->itemid[y]>0)
					count++;
			}
			if(count==0)
				Trader_EndTrader();
			NukeTraderItem(slotid,charges,quantity,customer,traderslot);
			return;
		}
	}
	printf("Could NOT find a match for Item: %i with a quantity of: %i on Trader: %s\n",item_id,quantity,this->GetName());
}
void Client::ReturnTraderReq(const EQApplicationPacket* app,int16 traderitemcharges){
	TraderBuy_Struct* tbs=(TraderBuy_Struct*)app->pBuffer;
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TraderBuy,sizeof(TraderBuy_Struct));
	TraderBuy_Struct* outtbs  = (TraderBuy_Struct*)outapp->pBuffer;
	memcpy(outtbs,tbs,app->size);
	outtbs->price=(tbs->price*traderitemcharges);
	outtbs->quantity=traderitemcharges;
	outtbs->traderid=this->GetID();
	this->QueuePacket(outapp);
	safe_delete(outapp);
}
void Client::BuyTraderItem(TraderBuy_Struct* tbs,Client* trader,const EQApplicationPacket* app){
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Trader,sizeof(TraderBuy_Struct));
	TraderBuy_Struct* outtbs  = (TraderBuy_Struct*)outapp->pBuffer;
	outtbs->itemid=tbs->itemid;
	outtbs->price=tbs->price;
	int16 traderitemcharges=trader->FindTraderItemCharges(tbs->itemid);
	if(traderitemcharges<=0)
		traderitemcharges=1;
	else if(traderitemcharges==9999){
		Message(15,"Item not found!");
		return;
	}
	const Item_Struct* item2=database.GetItem(tbs->itemid);
	if(!item2)
	{
		safe_delete(outapp);
		return;
	}
	if(traderitemcharges<tbs->quantity)
		outtbs->quantity=traderitemcharges;
	else
		outtbs->quantity=tbs->quantity;
	ReturnTraderReq(app,outtbs->quantity);
	outtbs->traderid=this->GetID();
	outtbs->slot_num=tbs->slot_num;
	outtbs->unknown0=0x0A;
	strncpy(outtbs->itemname-4,item2->Name,64);
	int traderslot=0;
	SendTraderItem(outtbs->itemid,outtbs->quantity);

	EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_MoneyUpdate,sizeof(MoneyUpdate_Struct));
	MoneyUpdate_Struct* mus= (MoneyUpdate_Struct*)outapp2->pBuffer;
	int32 itemcost=tbs->price;
	this->TakeMoneyFromPP(tbs->price);
	mus->platinum=(int)itemcost/1000;
	itemcost-=(mus->platinum*1000);
	mus->gold=(int)itemcost/100;
	itemcost-=(mus->gold*100);
	mus->silver=(int)itemcost/10;
	itemcost-=(mus->silver*10);
	mus->copper=itemcost;
	//trader->AddMoneyToPP(tbs->price,true);
	trader->AddMoneyToPP(mus->copper,mus->silver,mus->gold,mus->platinum,false);
	mus->platinum=trader->GetPlatinum();
	mus->gold=trader->GetGold();
	mus->silver=trader->GetSilver();
	mus->copper=trader->GetCopper();
	traderslot=trader->FindTraderItem(tbs->itemid,outtbs->quantity);
	trader->QueuePacket(outapp2);
	trader->FindAndNukeTraderItem(tbs->itemid,outtbs->quantity,this,tbs->slot_num);
	trader->QueuePacket(outapp);
	safe_delete(outapp);
	//safe_delete(outapp2);
}
void Client::SendBazaarWelcome(){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (database.RunQuery(query,MakeAnyLenString(&query, "select count(distinct char_id),count(char_id) from trader"),errbuf,&result)){
		if(mysql_num_rows(result)==1){
			row = mysql_fetch_row(result);
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_Bazaar,sizeof(BazaarWelcome_Struct));
			memset(outapp->pBuffer,0,outapp->size);
			BazaarWelcome_Struct* bws = (BazaarWelcome_Struct*)outapp->pBuffer;
			bws->beginning.action=9;
			bws->items=atoi(row[1]);
			bws->traders=atoi(row[0]);
			this->QueuePacket(outapp);
			safe_delete(outapp);
		}
		mysql_free_result(result);
	}
	
	safe_delete_array(query);
}
void Client::SendBazaarResults(int32 trader_id,int32 class_,int32 race,int32 stat,int32 slot,int32 type,char name[64],int32 minprice,int32 maxprice){
	char errbuf[MYSQL_ERRMSG_SIZE];
    char* query = 0;
	string search,values;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char tmp[100]={0};
	values.append("count(item_id),trader.*,items.name");
	search.append("where trader.item_id=items.id");
	if(trader_id>0){
		Client* trader=entity_list.GetClientByID(trader_id);
		if(trader){
			sprintf(tmp," and trader.char_id=%i",trader->CharacterID());
			search.append(tmp);
		}
			
	}
	string searchresults;
	if(minprice!=0){
		sprintf(tmp," and trader.item_cost>=%i",minprice);
		search.append(tmp);
	}
	if(maxprice!=0){
		sprintf(tmp," and trader.item_cost<=%i",maxprice);
		search.append(tmp);
	}
	if(strlen(name)>0){
		sprintf(tmp," and items.name like '%%%s%%'",name);
		search.append(tmp);
	}
	if(class_!=0xFFFFFFFF){
			sprintf(tmp," and mid(reverse(bin(items.classes)),%i,1)=1",class_);
			search.append(tmp);
	}
	if(race!=0xFFFFFFFF){
			sprintf(tmp," and mid(reverse(bin(items.races)),%i,1)=1",race);
			search.append(tmp);
	}
	if(slot!=0xFFFFFFFF){
			sprintf(tmp," and mid(reverse(bin(items.slots)),%i,1)=1",slot+1);
			search.append(tmp);
	}
	if(type!=0xFFFFFFFF){
		switch(type){
			case 31:
				sprintf(tmp," and items.itemclass=2");
				search.append(tmp);
				break;
			case 46:
				sprintf(tmp," and items.spellid>0 and items.spellid<65000");
				search.append(tmp);
				break;
			case 47:
				sprintf(tmp," and items.spellid=998");
				search.append(tmp);
				break;
			case 48:
				sprintf(tmp," and items.spellid>=1298 and items.spellid<=1307");
				search.append(tmp);
				break;
			case 49:
				sprintf(tmp," and items.focusid>0");
				search.append(tmp);
				break;
			default:
				sprintf(tmp," and items.itemtype=%i",type);
				search.append(tmp);
		}
	}
	if(stat!=0xFFFFFFFF){
		if(stat==14){
			search.append(" and items.ac>0");
			values.append(",items.ac");
		}
		else if(stat==2){
			search.append(" and items.aagi>0");
			values.append(",items.aagi");
		}
		else if(stat==6){
			search.append(" and items.acha>0");
			values.append(",items.acha");
		}
		else if(stat==3){
			search.append(" and items.adex>0");
			values.append(",items.adex");
		}
		else if(stat==4){
			search.append(" and items.aint>0");
			values.append(",items.aint");
		}
		else if(stat==1){
			search.append(" and items.asta>0");
			values.append(",items.asta");
		}
		else if(stat==0){
			search.append(" and items.astr>0");
			values.append(",items.astr");
		}
		else if(stat==5){
			search.append(" and items.awis>0");
			values.append(",items.awis");
		}
		else if(stat==8){
			search.append(" and items.cr>0");
			values.append(",items.cr");
		}
		else if(stat==11){
			search.append(" and items.dr>0");
			values.append(",items.dr");
		}
		else if(stat==9){
			search.append(" and items.fr>0");
			values.append(",items.fr");
		}
		else if(stat==7){
			values.append(",items.mr");
			search.append(" and items.mr>0");
		}
		else if(stat==10){
			search.append(" and items.pr>0");
			values.append(",items.pr");
		}
		else if(stat==13){
			search.append(" and items.hp>0");
			values.append(",items.hp");
		}
		else if(stat==12){
			search.append(" and items.mana>0");
			values.append(",items.mana");
		}
	}
	if (database.RunQuery(query,MakeAnyLenString(&query, "select %s from trader,items %s group by items.id limit 50",values.c_str(),search.c_str()),errbuf,&result)){
		safe_delete_array(query);
		int size=0;
		int32 id=0;
		//BazaarSearchResults_Struct* brs= new BazaarSearchResults_Struct;
		if(mysql_num_rows(result)==0){
			EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_Bazaar,sizeof(BazaarReturnDone_Struct));
			BazaarReturnDone_Struct* brds = (BazaarReturnDone_Struct*)outapp2->pBuffer;
			brds->traderid=id;
			brds->type=0x0C;
			brds->unknown8=0xFFFFFFFF;
			brds->unknown12=0xFFFFFFFF;
			brds->unknown16=0xFFFFFFFF;
			this->QueuePacket(outapp2);
			safe_delete(outapp2);
			mysql_free_result(result);
			return;
		}
		size=mysql_num_rows(result)*sizeof(BazaarSearchResults_Struct);
		//char *buffer=(char*)malloc(size);
		//char *bufptr=buffer;
		uchar *buffer=new uchar[size];
		uchar *bufptr=buffer;
		memset(buffer,0,size);
		int action=7;
		int32 cost=0;
		int32 item_id=0;
		char name[64]={0};
		int count=0;
		int32 statvalue=0;
		while ((row = mysql_fetch_row(result))) {
			memcpy(bufptr,&action, sizeof(int32));
			bufptr+=sizeof(int32);
			count=atoi(row[0]);
			memcpy(bufptr,&count, sizeof(int32));
			bufptr+=sizeof(int32);
			item_id=atoi(row[2]);
			memcpy(bufptr,&item_id, sizeof(int32));
			bufptr+=sizeof(int32);
			Client* trader2=entity_list.GetClientByCharID(atoi(row[1]));
			if(trader2){
				id=trader2->GetID();
				memcpy(bufptr,&id, sizeof(int32));
				bufptr+=sizeof(int32);
			}
			else{
				printf("Unable to find trader: %i\n",atoi(row[1]));
				memcpy(bufptr,&id, sizeof(int32));
				bufptr+=sizeof(int32);
			}
			cost=atoi(row[3]);
			memcpy(bufptr,&cost, sizeof(int32));
			bufptr+=sizeof(int32);
			statvalue=atoi(row[6]);
			memcpy(bufptr,&statvalue, sizeof(int32));
			bufptr+=sizeof(int32);
			sprintf(name,"%s(%i)",row[5],count);
			memcpy(bufptr,&name, strlen(name));
			bufptr+=64;
		}
		mysql_free_result(result);
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Bazaar,size);
		memcpy(outapp->pBuffer,buffer,size);
		this->QueuePacket(outapp);
		safe_delete(outapp);
		//free(buffer);
		safe_delete_array(buffer);
		EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_Bazaar,sizeof(BazaarReturnDone_Struct));
		BazaarReturnDone_Struct* brds = (BazaarReturnDone_Struct*)outapp2->pBuffer;
		brds->traderid=id;
		brds->type=0x0C;
		brds->unknown8=0xFFFFFFFF;
		brds->unknown12=0xFFFFFFFF;
		brds->unknown16=0xFFFFFFFF;
		this->QueuePacket(outapp2);
		safe_delete(outapp2);
		
	}
	else{
		printf("Failed to retrieve Bazaar Search!! %s\n",query);
		safe_delete_array(query);
		return;
	}
}
