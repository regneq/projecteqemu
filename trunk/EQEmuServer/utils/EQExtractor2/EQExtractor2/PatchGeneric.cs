using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    public enum IdentificationStatus { No, Tentative, Yes };

    class PatchSpecficDecoder
    {
        virtual public string GetVersion()
        {
            return "Unsupported EQ Client Version.";
        }

        virtual public int ExpectedPPLength()
        {
            return 0;
        }

        virtual public int GetPPZoneIDOffset()
        {
            return 0;
        }

        virtual public string GetPatchConfFileName()
        {
            return "";
        }

        virtual public bool UnsupportedVersion()
        {
            return ExpectedPPLength() == 0;
        }

        virtual public bool Init(string ConfDirectory, ref string ErrorMessage)
        {
            return false;
        }

        virtual public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            return IdentificationStatus.No;
        }

        virtual public List<Door> GetDoors()
        {
            List<Door> DoorList = new List<Door>();

            return DoorList;
        }

        virtual public UInt16 GetZoneNumber()
        {
            return 0;
        }

        virtual public int VerifyPlayerProfile()
        {
            return 0;
        }

        virtual public MerchantManager GetMerchantData(NPCSpawnList NPCSL)
        {
            return null;
        }

        virtual public List<ZonePoint> GetZonePointList()
        {
            List<ZonePoint> ZonePointList = new List<ZonePoint>();

            return ZonePointList;
        }

        virtual public NewZoneStruct GetZoneData()
        {
            NewZoneStruct NewZone = new NewZoneStruct();

            return NewZone;
        }

        virtual public List<ZoneEntryStruct> GetSpawns()
        {
            List<ZoneEntryStruct> ZoneSpawns = new List<ZoneEntryStruct>();

            return ZoneSpawns;
        }

        virtual public List<PositionUpdate> GetHighResolutionMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            return Updates;
        }

        virtual public List<PositionUpdate> GetLowResolutionMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            return Updates;
        }

        virtual public List<GroundSpawnStruct> GetGroundSpawns()
        {
            List<GroundSpawnStruct> GroundSpawns = new List<GroundSpawnStruct>();

            return GroundSpawns;
        }

        virtual public List<UInt32> GetFindableSpawns()
        {
            List<UInt32> FindableSpawnList = new List<UInt32>();

            return FindableSpawnList;
        }

        virtual public string GetZoneName()
        {
            return "";
        }

        virtual public bool DumpAAs(string FileName)
        {
            return false;
        }
        
        public void GivePackets(PacketManager pm)
        {
            Packets = pm;
        }

        public List<byte[]> GetPacketsOfType(string OpCodeName, PacketDirection Direction)
        {
            List<byte[]> ReturnList = new List<byte[]>();

            if (OpManager == null)
                return ReturnList;

            UInt32 OpCodeNumber = OpManager.OpCodeNameToNumber(OpCodeName);

            foreach (EQApplicationPacket app in Packets.PacketList)
            {
                if ((app.OpCode == OpCodeNumber) && (app.Direction == Direction) && (app.Locked))
                    ReturnList.Add(app.Buffer);
            }

            return ReturnList;
        }
        public bool DumpPackets(string FileName)
        {

            StreamWriter PacketDumpStream;

            try
            {
                PacketDumpStream = new StreamWriter(FileName);
            }
            catch
            {
                return false;
            }

            string Direction = "";

            foreach (EQApplicationPacket p in Packets.PacketList)
            {

                if (p.Direction == PacketDirection.ServerToClient)
                    Direction = "[Server->Client]";
                else
                    Direction = "[Client->Server]";

                PacketDumpStream.WriteLine("[OPCode: 0x" + p.OpCode.ToString("x") + "] " + (OpManager != null ? OpManager.OpCodeToName(p.OpCode) : "OP_Unknown") + " " + Direction + " [Size: " + p.Buffer.Length + "]");
                PacketDumpStream.WriteLine(Utils.HexDump(p.Buffer));
            }

            PacketDumpStream.Close();

            return true;
        }
        public int PacketTypeCountByName(string OPCodeName)
        {
            UInt32 OpCodeNumber = OpManager.OpCodeNameToNumber(OPCodeName);

            int Count = 0;

            foreach (EQApplicationPacket app in Packets.PacketList)
            {
                if (app.OpCode == OpCodeNumber)
                    ++Count;
            }


            return Count;
        }
        
        protected PacketManager Packets;

        public OpCodeManager OpManager;
    }
}
