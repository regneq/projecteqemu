using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchTestSep222010Decoder : PatchTestSep012010Decoder
    {
        public PatchTestSep222010Decoder()
        {
            Version = "EQ Client Build Date Test Server September 22 2010.";

            PatchConfFileName = "patch_Sep22-2010.conf";

            ExpectedPPLength = 26728;
        }

        override public List<Door> GetDoors()
        {
            List<Door> DoorList = new List<Door>();

            List<byte[]> SpawnDoorPacket = GetPacketsOfType("OP_SpawnDoor", PacketDirection.ServerToClient);

            if ((SpawnDoorPacket.Count == 0) || (SpawnDoorPacket[0].Length == 0))
                return DoorList;

            int DoorCount = SpawnDoorPacket[0].Length / 96;

            ByteStream Buffer = new ByteStream(SpawnDoorPacket[0]);

            for (int d = 0; d < DoorCount; ++d)
            {
                string DoorName = Buffer.ReadFixedLengthString(32, false);

                float YPos = Buffer.ReadSingle();

                float XPos = Buffer.ReadSingle();

                float ZPos = Buffer.ReadSingle();

                float Heading = Buffer.ReadSingle();

                UInt32 Incline = Buffer.ReadUInt32();

                Int32 Size = Buffer.ReadInt32();

                Buffer.SkipBytes(4); // Skip Unknown

                Byte DoorID = Buffer.ReadByte();

                Byte OpenType = Buffer.ReadByte();

                Byte StateAtSpawn = Buffer.ReadByte();

                Byte InvertState = Buffer.ReadByte();

                Int32 DoorParam = Buffer.ReadInt32();

                // Skip past the trailing unknowns in the door struct, moving to the next door in the packet.

                Buffer.SkipBytes(28);

                string DestZone = "NONE";

                Door NewDoor = new Door(DoorName, YPos, XPos, ZPos, Heading, Incline, Size, DoorID, OpenType, StateAtSpawn, InvertState,
                                        DoorParam, DestZone, 0, 0, 0, 0);

                DoorList.Add(NewDoor);

            }
            return DoorList;
        }

        override public Item DecodeItemPacket(byte[] PacketBuffer)
        {
            ByteStream Buffer = new ByteStream(PacketBuffer);

            Item NewItem = new Item();

            //NewItem.StackSize = Buffer.ReadUInt32();             // 00
            //Buffer.SkipBytes(4);
            //NewItem.Slot = Buffer.ReadUInt32();                  // 08
            //Buffer.SkipBytes(1);
            Buffer.SetPosition(30);
            NewItem.MerchantSlot = Buffer.ReadByte();            // 13
            NewItem.Price = Buffer.ReadUInt32();                 // 14
            Buffer.SkipBytes(5);
            NewItem.Quantity = Buffer.ReadInt32();               // 23
            Buffer.SetPosition(96);
            NewItem.Name = Buffer.ReadString(true);
            NewItem.Lore = Buffer.ReadString(true);
            NewItem.IDFile = Buffer.ReadString(true);
            NewItem.ID = Buffer.ReadUInt32();

            return NewItem;
        }

        public override void RegisterExplorers()
        {

        }

    }
}