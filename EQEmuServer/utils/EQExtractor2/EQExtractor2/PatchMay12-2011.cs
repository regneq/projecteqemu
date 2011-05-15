using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchMay122011Decoder : PatchMarch152011Decoder
    {
        public PatchMay122011Decoder()
        {
            Version = "EQ Client Build Date May 12 2011.";

            PatchConfFileName = "patch_May12-2011.conf";

            ExpectedPPLength = 28536;

            PPZoneIDOffset = 21204;            
        }

        override public PositionUpdate Decode_OP_NPCMoveUpdate(byte[] UpdatePacket)
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            BitStream bs = new BitStream(UpdatePacket, 13);

            PosUpdate.SpawnID = bs.readUInt(16);

            UInt32 Unknown = bs.readUInt(16);

            UInt32 VFlags = bs.readUInt(6);

            PosUpdate.p.y = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.x = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.z = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.heading = (float)bs.readInt(12) / (float)(1 << 3);

            PosUpdate.HighRes = true;

            return PosUpdate;
        }

        override public PositionUpdate Decode_OP_MobUpdate(byte[] MobUpdatePacket)
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            ByteStream Buffer = new ByteStream(MobUpdatePacket);

            PosUpdate.SpawnID = Buffer.ReadUInt16();

            Buffer.SkipBytes(2);

            UInt32 Word1 = Buffer.ReadUInt32();

            UInt32 Word2 = Buffer.ReadUInt32();

            UInt16 Word3 = Buffer.ReadUInt16();

            PosUpdate.p.y = Utils.EQ19ToFloat((Int32)(Word1 & 0x7FFFF));

            // Z is in the top 13 bits of Word1 and the bottom 6 of Word2

            UInt32 ZPart1 = Word1 >> 19;    // ZPart1 now has low order bits of Z in bottom 13 bits
            UInt32 ZPart2 = Word2 & 0x3F;   // ZPart2 now has high order bits of Z in bottom 6 bits

            ZPart2 = ZPart2 << 13;

            PosUpdate.p.z = Utils.EQ19ToFloat((Int32)(ZPart1 | ZPart2));

            PosUpdate.p.x = Utils.EQ19ToFloat((Int32)(Word2 >> 6) & 0x7FFFF);

            PosUpdate.p.heading = Utils.EQ19ToFloat((Int32)(Word3 & 0xFFF));

            PosUpdate.HighRes = false;

            return PosUpdate;
        }

        override public List<ZonePoint> GetZonePointList()
        {
            List<ZonePoint> ZonePointList = new List<ZonePoint>();

            List<byte[]> ZonePointPackets = GetPacketsOfType("OP_SendZonepoints", PacketDirection.ServerToClient);

            if (ZonePointPackets.Count < 1)
            {
                return ZonePointList;
            }

            // Assume there is only 1 packet and process the first one.

            ByteStream Buffer = new ByteStream(ZonePointPackets[0]);

            UInt32 Entries = Buffer.ReadUInt32();

            if (Entries == 0)
                return ZonePointList;

            float x, y, z, Heading;

            UInt32 Number;

            UInt16 ZoneID, Instance;

            ZonePointList = new List<ZonePoint>();

            for (int i = 0; i < Entries; ++i)
            {
                Number = Buffer.ReadUInt32();

                y = Buffer.ReadSingle();

                x = Buffer.ReadSingle();

                z = Buffer.ReadSingle();

                Heading = Buffer.ReadSingle();

                if (Heading != 999)
                    Heading = Heading / 2;

                ZoneID = Buffer.ReadUInt16();

                Instance = Buffer.ReadUInt16();

                Buffer.SkipBytes(8);    // Skip the last UInt32

                ZonePoint NewZonePoint = new ZonePoint(Number, ZoneID, Instance, x, y, z, x, y, z, Heading, ZoneID);

                ZonePointList.Add(NewZonePoint);
            }

            return ZonePointList;
        }
    }
}