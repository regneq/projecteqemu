//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
// 
using System;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchTestSep012010Decoder : PatchMay122010Decoder
    {
        public PatchTestSep012010Decoder()
        {
            Version = "EQ Client Build Date Test Server September 1 2010.";

            PatchConfFileName = "patch_Sep01-2010.conf";

            ExpectedPPLength = 26640;
        }

        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if((OpCode == OpManager.OpCodeNameToNumber("OP_ZoneEntry")) && (Direction == PacketDirection.ClientToServer))
                return IdentificationStatus.Tentative;

            UInt32 OP_SendAATable = OpManager.OpCodeNameToNumber("OP_SendAATable");

            if ((OpCode == OP_SendAATable) && (Direction == PacketDirection.ServerToClient) &&
                (Size == 120))
                return IdentificationStatus.Yes;

            return IdentificationStatus.No;
        }

        override public MerchantManager GetMerchantData(NPCSpawnList NPCSL)
        {
            List<EQApplicationPacket> PacketList = Packets.PacketList;

            UInt32 OP_ShopRequest = OpManager.OpCodeNameToNumber("OP_ShopRequest");

            UInt32 OP_ShopEnd = OpManager.OpCodeNameToNumber("OP_ShopEnd");

            UInt32 OP_ItemPacket = OpManager.OpCodeNameToNumber("OP_ItemPacket");

            MerchantManager mm = new MerchantManager();

            for (int i = 0; i < PacketList.Count; ++i)
            {
                EQApplicationPacket p = PacketList[i];

                if ((p.Direction == PacketDirection.ServerToClient) && (p.OpCode == OP_ShopRequest))
                {
                    ByteStream Buffer = new ByteStream(p.Buffer);

                    UInt32 MerchantSpawnID = Buffer.ReadUInt32();

                    NPCSpawn npc = NPCSL.GetNPC(MerchantSpawnID);

                    UInt32 NPCTypeID;

                    if (npc != null)
                        NPCTypeID = npc.NPCTypeID;
                    else
                        NPCTypeID = 0;

                    mm.AddMerchant(MerchantSpawnID);

                    for (int j = i + 1; j < PacketList.Count; ++j)
                    {
                        p = PacketList[j];

                        if (p.OpCode == OP_ShopEnd)
                            break;

                        if (p.OpCode == OP_ItemPacket)
                        {
                            Buffer = new ByteStream(p.Buffer);

                            UInt32 StackSize = Buffer.ReadUInt32();             // 00

                            Buffer.SkipBytes(4);

                            UInt32 Slot = Buffer.ReadUInt32();                  // 08

                            Buffer.SkipBytes(1);

                            UInt32 MerchantSlot = Buffer.ReadByte();            // 13

                            UInt32 Price = Buffer.ReadUInt32();                 // 14

                            Buffer.SkipBytes(5);

                            Int32 Quantity = Buffer.ReadInt32();                // 23

                            Buffer.SetPosition(71); // Point to item name

                            string ItemName = Buffer.ReadString(true);

                            string Lore = Buffer.ReadString(true);

                            string IDFile = Buffer.ReadString(true);

                            UInt32 ItemID = Buffer.ReadUInt32();

                            mm.AddMerchantItem(MerchantSpawnID, ItemID, ItemName, MerchantSlot, Quantity);
                        }
                    }
                }
            }

            return mm;
        }
    }


}
