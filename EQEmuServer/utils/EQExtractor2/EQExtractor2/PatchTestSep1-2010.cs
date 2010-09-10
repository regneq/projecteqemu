//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
// 
using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
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
                return IdentificationStatus.Yes;
            
            return IdentificationStatus.No;
        }                

        override public Item DecodeItemPacket(byte[] PacketBuffer)
        {
            ByteStream Buffer = new ByteStream(PacketBuffer);

            Item NewItem = new Item();                      

            NewItem.StackSize = Buffer.ReadUInt32();             // 00
            Buffer.SkipBytes(4);
            NewItem.Slot = Buffer.ReadUInt32();                  // 08
            Buffer.SkipBytes(1);
            NewItem.MerchantSlot = Buffer.ReadByte();            // 13
            NewItem.Price = Buffer.ReadUInt32();                 // 14
            Buffer.SkipBytes(5);
            NewItem.Quantity = Buffer.ReadInt32();               // 23
            Buffer.SetPosition(71);
            NewItem.Name = Buffer.ReadString(true);
            NewItem.Lore = Buffer.ReadString(true);
            NewItem.IDFile = Buffer.ReadString(true);
            NewItem.ID = Buffer.ReadUInt32();

            return NewItem;
        }

        public override void RegisterExplorers()
        {
            OpManager.RegisterExplorer("OP_CharInventory", ExploreCharInventoryPacket);
        }
         
        public void ExploreCharInventoryPacket(StreamWriter OutputStream, byte[] PacketBuffer)
        {            
            OutputStream.WriteLine("\r\nExploreCharInventoryPacket Called!\r\n");         

            ByteStream Buffer = new ByteStream(PacketBuffer);

            UInt32 ItemCount = Buffer.ReadUInt32();
            OutputStream.WriteLine("There are {0} items in the inventory.\r\n", ItemCount );

            for (int i = 0; i < ItemCount; ++i)
            {
                ExploreSubItem(OutputStream, ref Buffer);             
            }            
        }

        void  ExploreSubItem(StreamWriter OutputStream, ref ByteStream Buffer)
        {
            int BufferPosition = Buffer.GetPosition();
            
            Buffer.SkipBytes(8);

            byte Area = Buffer.ReadByte();
            UInt16 MainSlot = Buffer.ReadUInt16();
            UInt16 SubSlot = Buffer.ReadUInt16();
            Buffer.SkipBytes(54);
            string Name = Buffer.ReadString(true);

            OutputStream.WriteLine("Area: {0} Main Slot {1} Sub Slot {2} Name {3}\r\n", Area, MainSlot, SubSlot, Name);

            Buffer.ReadString(true);    // Lore
            Buffer.ReadString(true);    // IDFile
            Buffer.SkipBytes(236);  // Item Body Struct
            Buffer.ReadString(true);    // Charm File
            Buffer.SkipBytes(64);   // Item Secondary Body Struct
            Buffer.ReadString(true);    // Filename
            Buffer.SkipBytes(76);   // Item Tertiary Body Struct
            Buffer.SkipBytes(30);   // Click Effect Struct
            Buffer.ReadString(true);    // Clickname
            Buffer.SkipBytes(4);    // clickunk7
            Buffer.SkipBytes(30);   // Proc Effect Struct
            Buffer.ReadString(true);    // Proc Name
            Buffer.SkipBytes(4);    // unknown5            
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6            
            Buffer.SkipBytes(103);   // Item Quaterary Body Struct - 4 (we want to read the SubLength field at the end)

            UInt32 SubLengths = Buffer.ReadUInt32();
                        
            for (int i = 0; i < SubLengths; ++i)
            {
                Buffer.SkipBytes(4);
                ExploreSubItem(OutputStream, ref Buffer);               
            }
        }
    }
}
