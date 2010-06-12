using System;
using System.Text;
using System.IO;
using System.Collections.Generic;
using zlib;
using MyUtils;
using EQPacket;

namespace SharpPcap.Test.EQCollector
{

    public class NPCType
    {
        public NPCType(UInt32 DBID, string Name, Byte Level, uint Gender, float Size, Byte Face, float WalkSpeed, float RunSpeed, UInt32 Race,
                       UInt32 BodyType, Byte HairColor, Byte BeardColor, Byte EyeColor1, Byte EyeColor2, Byte HairStyle, Byte Beard,
                       UInt32 DrakkinHeritage, UInt32 DrakkinTattoo, UInt32 DrakkinDetails, UInt32 Deity, Byte Class, Byte EquipChest2,
                       Byte Helm, string LastName)
        {
            this.DBID = DBID;
            this.Name = Name;
            this.Level = Level;
            this.Gender = Gender;
            this.Size = Size;
            this.Face = Face;
            this.WalkSpeed = WalkSpeed;
            this.RunSpeed = RunSpeed;
            this.Race = Race;
            this.BodyType = BodyType;
            this.HairColor = HairColor;
            this.BeardColor = BeardColor;
            this.EyeColor1 = EyeColor1;
            this.EyeColor2 = EyeColor2;
            this.HairStyle = HairStyle;
            this.Beard = Beard;
            this.DrakkinHeritage = DrakkinHeritage;
            this.DrakkinTattoo = DrakkinTattoo;
            this.DrakkinDetails = DrakkinDetails;
            this.Deity = Deity;
            this.Class = Class;
            this.EquipChest2 = EquipChest2;
            this.Helm = Helm;
            this.LastName = LastName;
        }

        public UInt32 DBID;
        public string Name = "";
        public Byte Level = 0;
        public uint Gender = 0;
        public float Size = 0;
        public Byte Face = 0;
        public float WalkSpeed = 0;
        public float RunSpeed = 0;
        public UInt32 Race = 0;
        public UInt32 BodyType = 0;
        public Byte HairColor = 0;
        public Byte BeardColor = 0;
        public Byte EyeColor1 = 0;
        public Byte EyeColor2 = 0;
        public Byte HairStyle = 0;
        public Byte Beard = 0;
        public UInt32 DrakkinHeritage = 0;
        public UInt32 DrakkinTattoo = 0;
        public UInt32 DrakkinDetails = 0;
        public UInt32 Deity = 0;
        public Byte Class = 0;
        public Byte EquipChest2 = 0;
        public Byte Helm = 0;
        public string LastName = "";
                        
    }

    public class Position
    {
        public Position(float x, float y, float z, float heading)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.heading = heading;
        }
        public float x, y, z, heading;
    }
    public class NPCSpawn
    {
        public NPCSpawn(UInt32 SpawnID, UInt32 Spawn2DBID, string Name)
        {
            this.SpawnID = SpawnID;
            this.Spawn2DBID = Spawn2DBID;
            this.Name = Name;
        }

        public bool DoesHaveHighResWaypoints()
        {
            return HasHighResWaypoints;
        }

        public void SetHasHighResWaypoints()
        {
            HasHighResWaypoints = true;
        }

        public void AddWaypoint(Position p, bool HighRes)
        {
            Waypoints.Add(p);
            if (HighRes)
                HasHighResWaypoints = true;
        }

        public UInt32 SpawnID;
        public UInt32 Spawn2DBID;
        public string Name;
        bool HasHighResWaypoints = false;
        public List<Position> Waypoints = new List<Position>();
    }

    public  class ReadFile
    {

        static int PacketsSeen = 0;
        static PacketManager pm = new PacketManager();

        public class NPCSpawnList
        {
            public List<NPCSpawn> _NPCSpawnList = new List<NPCSpawn>();

            public bool DoesHaveHighResWaypoints(UInt32 SpawnID)
            {
                foreach (NPCSpawn s in _NPCSpawnList)
                {
                    if (s.SpawnID == SpawnID)
                    {
                        if (s.DoesHaveHighResWaypoints())
                            return true;
                        else
                            return false;
                    }
                }
                return false;
            }

            public void AddNPCSpawn(UInt32 DBID, UInt32 SpawnID, string Name)
            {
                NPCSpawn NewSpawn = new NPCSpawn(DBID, SpawnID, Name);

                _NPCSpawnList.Add(NewSpawn);
            }

            public void AddWaypoint(UInt32 SpawnID, Position p, bool HighRes)
            {
                foreach (NPCSpawn n in _NPCSpawnList)
                {
                    if (n.SpawnID == SpawnID)
                    {
                        n.AddWaypoint(p, HighRes);
                        return;
                    }
                }
            }
        }

        public class NPCTypeList
        {
            List<NPCType> _NPCTypeList = new List<NPCType>();

            public void AddNPCType(NPCType NewNPC)
            {
                _NPCTypeList.Add(NewNPC);
            }

            public UInt32 FindNPCType(string Name, Byte Level, uint Gender, float Size, Byte Face, float WalkSpeed, float RunSpeed, UInt32 Race,
                           UInt32 BodyType, Byte HairColor, Byte BeardColor, Byte EyeColor1, Byte EyeColor2, Byte HairStyle, Byte Beard,
                           UInt32 DrakkinHeritage, UInt32 DrakkinTattoo, UInt32 DrakkinDetails, UInt32 Deity, Byte Class, Byte EquipChest2,
                           Byte Helm, string LastName)
            {
                foreach (NPCType n in _NPCTypeList)
                {
                    if (n.Name != Name)
                        continue;

                    if (n.Level != Level)
                        continue;

                    if (n.Gender != Gender)
                        continue;

                    if (n.Size != Size)
                        continue;

                    if (n.Face != Face)
                        continue;

                    if (n.WalkSpeed != WalkSpeed)
                        continue;

                    if (n.RunSpeed != RunSpeed)
                        continue;

                    if (n.Race != Race)
                        continue;

                    if (n.BodyType != BodyType)
                        continue;

                    if (n.HairColor != HairColor)
                        continue;

                    if (n.BeardColor != BeardColor)
                        continue;

                    if (n.EyeColor1 != EyeColor1)
                        continue;

                    if (n.EyeColor2 != EyeColor2)
                        continue;

                    if (n.HairStyle != HairStyle)
                        continue;

                    if (n.Beard != Beard)
                        continue;

                    if (n.DrakkinHeritage != DrakkinHeritage)
                        continue;

                    if (n.DrakkinTattoo != DrakkinTattoo)
                        continue;

                    if (n.DrakkinDetails != DrakkinDetails)
                        continue;

                    if (n.Deity != Deity)
                        continue;

                    if (n.Class != Class)
                        continue;

                    if (n.EquipChest2 != EquipChest2)
                        continue;

                    if (n.Helm != Helm)
                        continue;

                    if (n.LastName != LastName)
                        continue;

                    //Console.WriteLine("Found NPCType already in list, returning {0}", n.DBID);

                    return n.DBID;

                }

                return 0;
            }
        }



        
     
        public static void Main(string[] args)
        {
            

            // Print SharpPcap version
            string ver = SharpPcap.Version.VersionString;
            Console.WriteLine("--", ver);

            //Console.WriteLine();
            //Console.Write("-- Please enter an input capture file name: ");
            //string capFile = Console.ReadLine();

            //string capFile = "c:\\temp\\livearena2010b.pcap";
            //string capFile = "c:\\temp\\liveguildlogon.pcap";
            //string capFile = "c:\\temp\\livehighpasshold1.pcap";
            //string capFile = "c:\\temp\\livewestfreeport.pcap";
            //string capFile = "c:\\temp\\livewf30mins.pcap";
            string capFile = "c:\\temp\\livewfspawns2.pcap";

            PcapDevice device;

            try
            {
                // Get an offline device
                device = new OfflinePcapDevice( capFile );

                // Open the device
                device.Open();
            } 
            catch(Exception e)
            {
                Console.WriteLine("Caught exception when opening file" + e.ToString());
                return;
            }

            //Register our handler function to the 'packet arrival' event
            device.OnPacketArrival += 
                new PacketArrivalEventHandler( device_OnPacketArrival );

            //Console.WriteLine();
            Console.WriteLine("-- Capturing from '{0}', hit 'Ctrl-C' to exit...", capFile);

            // Start capture 'INFINTE' number of packets
            // This method will return when EOF reached.
            device.Capture();

            // Close the pcap device
            device.Close();
            //Console.WriteLine("-- End of file reached.");
            //pm.DumpCache();

            List<byte[]> NewZonePacket = pm.GetPacketsOfType("OP_NewZone");
            if (NewZonePacket.Count != 1)
            {
                Console.WriteLine("Got {0} OP_NewZone packets when expecting 1. Quitting.");
                return;
            }
            MemoryStream nzms = new MemoryStream(NewZonePacket[0].GetUpperBound(0));
            nzms.Write(NewZonePacket[0], 0, NewZonePacket[0].GetUpperBound(0));
            nzms.Seek(0, System.IO.SeekOrigin.Begin);

            BinaryReader nzbinReader = new BinaryReader(nzms);

            nzbinReader.ReadBytes(704);

            string ZoneName = "";

            char zc;

            zc = nzbinReader.ReadChar();

            while (zc != '\0')
            {
                ZoneName = ZoneName + zc;
                zc = nzbinReader.ReadChar();
            }
            Console.WriteLine("-- Zone short name is {0}", ZoneName);

            List<byte[]> SpawnDoorPacket = pm.GetPacketsOfType("OP_SpawnDoor");
            if (SpawnDoorPacket.Count > 0)
            {
                Console.WriteLine("--");
                Console.WriteLine("-- Doors");
                Console.WriteLine("--");
                int DoorDBID = 501000;

                Console.WriteLine("DELETE from doors where id >= {0} and id <= {1};", DoorDBID, DoorDBID + 1000);

                Console.WriteLine("-- There are {0} OP_SpawnDoor packets. Size is {1}", SpawnDoorPacket.Count, SpawnDoorPacket[0].Length);
                //Console.WriteLine(Utils.HexDump(SpawnDoorPacket[0]));

                int DoorCount = SpawnDoorPacket[0].Length / 92;

                MemoryStream sdms = new MemoryStream(SpawnDoorPacket[0].GetUpperBound(0));
                sdms.Write(SpawnDoorPacket[0], 0, SpawnDoorPacket[0].GetUpperBound(0));
                sdms.Seek(0, System.IO.SeekOrigin.Begin);

                BinaryReader sdbinReader = new BinaryReader(sdms);

                for (int d = 0; d < DoorCount; ++d)
                {
                    char[] DoorNameArray = new char[32];

                    DoorNameArray = sdbinReader.ReadChars(32);

                    int e = 0;
                    string DoorName = "";

                    while (DoorNameArray[e] > 0)
                    {
                        //Console.WriteLine("DoorNameArray[{0}] = {1}", e, DoorNameArray[e]);
                        DoorName += DoorNameArray[e++];
                    }
                    
                    
                    //Console.WriteLine("Door name is {0}", DoorName);

                    float YPos = sdbinReader.ReadSingle();
                    float XPos = sdbinReader.ReadSingle();
                    float ZPos = sdbinReader.ReadSingle();
                    float Heading = sdbinReader.ReadSingle();

                    //if(Heading >= 256)
                    //    Heading = Heading % 256;


                    UInt32 Incline = sdbinReader.ReadUInt32();
                    Int32 Size = sdbinReader.ReadInt32();
                    sdbinReader.ReadUInt32(); // Skip Unknown
                    Byte DoorID = sdbinReader.ReadByte();
                    Byte OpenType = sdbinReader.ReadByte();
                    Byte StateAtSpawn = sdbinReader.ReadByte();
                    Byte InvertState = sdbinReader.ReadByte();
                    Int32 DoorParam = sdbinReader.ReadInt32();

                    //Console.WriteLine("Door: {0}, Loc: {1}, {2}, {3}, Heading: {4}, Incline: {5}, Size: {6}, DoorID {7}, OpenType {8}, StateAtSpawn {9}, InvertState {10}, DoorParam {11}",
                    //    DoorName, XPos, YPos, ZPos, Heading, Incline, Size, DoorID, OpenType, StateAtSpawn, InvertState, DoorParam);

                    string DoorQuery = "INSERT INTO doors(`id`, `doorid`, `zone`, `name`, `pos_y`, `pos_x`, `pos_z`, `heading`, `opentype`, `doorisopen`, `door_param`, `invert_state`, `incline`, `size`) VALUES(";
                    DoorQuery += DoorDBID++ + ", " + DoorID + ", '" + ZoneName + "', '" + DoorName + "', " + YPos + ", " + XPos + ", " + ZPos + ", " + Heading + ", " + OpenType + ", " + StateAtSpawn + ", " + DoorParam + ", " + InvertState + ", " + Incline + ", " + Size + ");";

                    Console.WriteLine(DoorQuery);

                    sdbinReader.ReadUInt32();
                    sdbinReader.ReadUInt32();

                    sdbinReader.ReadByte();
                    sdbinReader.ReadByte();
                    sdbinReader.ReadByte();
                    sdbinReader.ReadByte();
                    sdbinReader.ReadByte();
                    sdbinReader.ReadByte();
                    sdbinReader.ReadByte();
                    sdbinReader.ReadByte();
                    sdbinReader.ReadBytes(8);






                }


            }

            Console.WriteLine("-- There are {0} OP_ZoneEntry", pm.PacketTypeCountByName("OP_ZoneEntry"));

            List<byte[]> SpawnPackets = pm.GetPacketsOfType("OP_ZoneEntry");



            //foreach (byte[] Payload in SpawnPackets)
            //{
            //    Console.WriteLine("Payload is {0} and has size {1}", Payload, Payload.Length);
            //    Console.WriteLine(Utils.HexDump(Payload));
            //}

            UInt32 SpawnDBID = 501000;
            UInt32 SpawnGroupID = 501000;
            UInt32 SpawnEntryID = 501000;
            UInt32 Spawn2ID = 501000;

            Console.WriteLine("DELETE from npc_types where id >= {0} and id <= {1};", SpawnDBID, SpawnDBID + 1000);
            Console.WriteLine("DELETE from spawngroup where id >= {0} and id <= {1};", SpawnGroupID, SpawnGroupID + 1000);
            Console.WriteLine("DELETE from spawnentry where spawngroupID >= {0} and spawngroupID <= {1};", SpawnEntryID, SpawnEntryID + 1000);
            Console.WriteLine("DELETE from spawn2 where id >= {0} and id <= {1};", Spawn2ID, Spawn2ID + 1000);

            Console.WriteLine("--");
            Console.WriteLine("-- Spawns");
            Console.WriteLine("--");

            NPCTypeList NPCTL = new NPCTypeList();
            NPCSpawnList NPCSL = new NPCSpawnList();

            for (int i = 1; i < SpawnPackets.Count; ++i)
            {
                //Console.WriteLine(Utils.HexDump(SpawnPackets[1]));
                MemoryStream ms = new MemoryStream(SpawnPackets[i].GetUpperBound(0));
                ms.Write(SpawnPackets[i], 0, SpawnPackets[i].GetUpperBound(0));
                ms.Seek(0, System.IO.SeekOrigin.Begin);

                BinaryReader binReader = new BinaryReader(ms);

                string SpawnName = "";

                char c;

                c = binReader.ReadChar();

                while (c != '\0')
                {
                    if(!((c >= '0') && (c <= '9')))
                        SpawnName = SpawnName + c;

                    c = binReader.ReadChar();
                }

                UInt32 SpawnID = binReader.ReadUInt32();

                //Console.WriteLine("SpawnID: {0} Name {1}", SpawnID, SpawnName);

                //Console.WriteLine("SpawnID {0} {1}", SpawnID, SpawnName);
                Byte Level = binReader.ReadByte();
                float UnkSize = binReader.ReadSingle();
                Byte IsNPC = binReader.ReadByte();
                UInt32 Bitfield = binReader.ReadUInt32();

                if (IsNPC != 1)
                    continue;

                uint Showname = (Bitfield >> 28) & 1;
                uint TargetableWithHotkey = (Bitfield >> 27) & 1;
                uint Targetable = (Bitfield >> 26) & 1;
                uint ShowHelm = (Bitfield >> 24) & 1;
                uint Gender = (Bitfield >> 20) & 3;

                Byte OtherData = binReader.ReadByte();
                //Console.WriteLine("Otherdata is {0}", OtherData.ToString("x"));

                binReader.ReadSingle(); // Skip Unknown
                binReader.ReadSingle(); // Skip Unknown

                if ((OtherData & 1) > 0)
                {
                    //Console.WriteLine("Otherdata is {0} Skipping Stuff.", OtherData.ToString("x"));
                    Byte b;
                    do
                        b = binReader.ReadByte();
                    while (b > 0);

                    do
                        b = binReader.ReadByte();
                    while (b > 0);

                    do
                        b = binReader.ReadByte();
                    while (b > 0);

                    for (int j = 0; j < 53; ++j)
                        binReader.ReadByte();
                }


                float Size = binReader.ReadSingle();
                Byte Face = binReader.ReadByte();
                float WalkSpeed = binReader.ReadSingle();
                float RunSpeed = binReader.ReadSingle();
                UInt32 Race = binReader.ReadUInt32();

                Byte PropCount = binReader.ReadByte();

                UInt32 BodyType = 0;

                if (PropCount >= 1)
                {
                    //Console.WriteLine("Propcount is {0}", PropCount);
                    BodyType = binReader.ReadUInt32();
                    for (int j = 1; j < PropCount; ++j)
                        binReader.ReadUInt32();
                }

                binReader.ReadByte();   // Skip HP %
                Byte HairColor = binReader.ReadByte();
                Byte BeardColor = binReader.ReadByte();
                Byte EyeColor1 = binReader.ReadByte();
                Byte EyeColor2 = binReader.ReadByte();
                Byte HairStyle = binReader.ReadByte();
                Byte Beard = binReader.ReadByte();

                UInt32 DrakkinHeritage = binReader.ReadUInt32();
                UInt32 DrakkinTattoo = binReader.ReadUInt32();
                UInt32 DrakkinDetails = binReader.ReadUInt32();

                binReader.ReadByte();   // Skip Holding

                UInt32 Deity = binReader.ReadUInt32();

                binReader.ReadUInt32(); // Skip GuildID 
                binReader.ReadUInt32(); // Skip GuildRank
                
                Byte Class = binReader.ReadByte();

                binReader.ReadByte();   // PVP
                binReader.ReadByte();   // Standstate
                binReader.ReadByte();   // Light
                binReader.ReadByte();   // Flymode
                Byte EquipChest2 = binReader.ReadByte();

                if (EquipChest2 == 255)
                    EquipChest2 = 0;

                binReader.ReadByte();   // Unknown
                binReader.ReadByte();   // Unknown

                Byte Helm = binReader.ReadByte();

                string LastName = "";

                c = binReader.ReadChar();

                while (c != '\0')
                {
                    LastName = LastName + c;
                    c = binReader.ReadChar();
                }

                binReader.ReadUInt32(); // Skip AATitle
                binReader.ReadByte();   // Unknown
                UInt32 PetOwnerID = binReader.ReadUInt32(); // PetOWnerID
                if (PetOwnerID > 0)
                {
                    //Console.WriteLine("Skipping Pet. {0}", SpawnName);
                    continue;
                }
                binReader.ReadByte();   // Unknown

                binReader.ReadUInt32(); // Unknown
                binReader.ReadUInt32(); // Unknown
                binReader.ReadUInt32(); // Unknown
                binReader.ReadUInt32(); // Unknown
                binReader.ReadUInt32(); // Unknown
                binReader.ReadUInt32(); // Unknown

                UInt32 Position1 = binReader.ReadUInt32();
                UInt32 Position2 = binReader.ReadUInt32();
                UInt32 Position3 = binReader.ReadUInt32();
                UInt32 Position4 = binReader.ReadUInt32();
                UInt32 Position5 = binReader.ReadUInt32();

                Int32 Yint = (Int32)(Position3 & 0x7FFFF);
                if ((Yint & 0x40000) > 0)
                    Yint = -(0x7FFFF - Yint + 1);

                float YPos = (float)Yint / (float)(1 << 3);

                Int32 IntHeading = (Int32)(Position4 & 0xFFF);

                float Heading = (float)IntHeading / (float)(1 << 3);

                Int32 Xint = (Int32)(Position4 >> 12) & 0x7FFFF;

                if ((Xint & 0x4000) > 0)
                    Xint = -(0x7FFFF - Xint + 1);

                float XPos = (float)Xint / (float)(1 << 3);

                Int32 Zint = (Int32)(Position5 & 0x7FFFF);

                if ((Zint & 0x4000) > 0)
                    Zint = -(0x7FFFF - Zint + 1);

                float ZPos = (float)Zint / (float)(1 << 3);

                //Console.WriteLine("Location: {0}, {1}, {2}", XPos, YPos, ZPos);
                binReader.ReadBytes(36);    // Skip Color

                if (Race <= 12 || Race == 128 || Race == 130 || Race == 330 || Race == 522)
                {
                    for (int ii = 0; ii < 9; ii++)
                    {
                        binReader.ReadUInt32(); // ItemID
                        binReader.ReadUInt32(); // Equip1
                        binReader.ReadUInt32(); // Equip0
                    }
                }

                if ((OtherData & 4) > 0)
                {
                    // Skip Title
                    c = binReader.ReadChar();

                    while (c != '\0')
                        c = binReader.ReadChar();
                }

                if ((OtherData & 8) > 0)
                {
                    // Skip Suffix
                    c = binReader.ReadChar();

                    while (c != '\0')
                        c = binReader.ReadChar();
                    
                }

                // unknowns
                binReader.ReadBytes(8);


                Byte isMercenary = binReader.ReadByte();

                if (isMercenary > 0)
                {
                    //Console.WriteLine("Found Mercenary {0} {1}, skipping", SpawnName, LastName);
                    continue;
                }
                
                UInt32 ExistingDBID = NPCTL.FindNPCType(SpawnName, Level, Gender, Size, Face, WalkSpeed, RunSpeed, Race,
                       BodyType, HairColor, BeardColor, EyeColor1, EyeColor2, HairStyle, Beard,
                       DrakkinHeritage, DrakkinTattoo, DrakkinDetails, Deity, Class, EquipChest2,
                       Helm, LastName);

                if (ExistingDBID == 0)
                {
                    //Console.WriteLine("No matching NPCType for {0} already in list, adding it.", SpawnName);
                    NPCType NewNPCType = new NPCType(SpawnDBID, SpawnName, Level, Gender, Size, Face, WalkSpeed, RunSpeed, Race,
                       BodyType, HairColor, BeardColor, EyeColor1, EyeColor2, HairStyle, Beard,
                       DrakkinHeritage, DrakkinTattoo, DrakkinDetails, Deity, Class, EquipChest2,
                       Helm, LastName);

                    NPCTL.AddNPCType(NewNPCType);

                    ExistingDBID = SpawnDBID++;

                    string NPCTypesQuery = "INSERT INTO npc_types(`id`, `name`, `lastname`, `level`, `gender`, `size`, `runspeed`,`race`, `class`, `bodytype`, `hp`, `texture`, `helmtexture`, `face`, `luclin_hairstyle`, `luclin_haircolor`, `luclin_eyecolor`, `luclin_eyecolor2`,`luclin_beard`, `luclin_beardcolor`) VALUES(";

                    NPCTypesQuery += ExistingDBID + ", '" + SpawnName + "', " + "'" + LastName + "', " + Level + ", " + Gender + ", " + Size + ", ";
                    NPCTypesQuery += RunSpeed + ", " + Race + ", " + Class + ", " + BodyType + ", " + Level * (10 + Level) + ", ";
                    NPCTypesQuery += EquipChest2 + ", " + Helm + ", " + Face + ", " + HairStyle + ", " + HairColor + ", " + EyeColor1 + ", ";
                    NPCTypesQuery += EyeColor2 + ", " + Beard + ", " + BeardColor + ");";

                    Console.WriteLine(NPCTypesQuery);
                }
                //else
                //   Console.WriteLine("Matching NPCType for {0} already in list with DBID {1}.", SpawnName, ExistingDBID);

                NPCSL.AddNPCSpawn(SpawnID, Spawn2ID, SpawnName);

                Position p = new Position(XPos, YPos, ZPos, Heading);

                NPCSL.AddWaypoint(SpawnID, p, false);

                string SpawnGroupQuery = "INSERT INTO spawngroup(`id`, `name`, `spawn_limit`, `dist`, `max_x`, `min_x`, `max_y`, `min_y`, `delay`) VALUES(";
                SpawnGroupQuery += SpawnGroupID + ", '" + ZoneName + SpawnGroupID + "', 0, 0, 0, 0, 0, 0, 0);";

                string SpawnEntryQuery = "INSERT INTO spawnentry(`spawngroupID`, `npcID`, `chance`) VALUES(";
                SpawnEntryQuery += SpawnEntryID + ", " + ExistingDBID + ", " + "100);";

                string Spawn2EntryQuery = "INSERT INTO spawn2(`id`, `spawngroupID`, `zone`, `version`, `x`, `y`, `z`, `heading`, `respawntime`, `variance`, `pathgrid`, `_condition`, `cond_value`, `enabled`) VALUES(";
                Spawn2EntryQuery += Spawn2ID + ", " + SpawnGroupID + ", '" + ZoneName + "', 0, " + XPos + ", " + YPos + ", " + ZPos + ", ";
                Spawn2EntryQuery += Heading + ", 640, 0, 0, 0, 1, 1);";

                //SpawnDBID++;
                SpawnGroupID++;
                SpawnEntryID++;
                Spawn2ID++;

                //Console.WriteLine("Spawn name is {0}, SpawnID {1}, Level {2}, IsNPC {3}, Gender {4}, Otherdata: {5}", SpawnName, SpawnID, Level, IsNPC, Gender, OtherData.ToString("x"));
                //Console.WriteLine("      Size {0}, Face {1}, WalkSpeed {2} RunSpeed {3} Race {4} Bodytype {5} Class {6}", Size, Face, WalkSpeed, RunSpeed, Race, BodyType, Class);
                //Console.WriteLine("      EquipChest2 is {0} LastName is {1}", EquipChest2, LastName);
                //Console.WriteLine("      Loc is {0}, {1}, {2}", XPos, YPos, ZPos);

                
                Console.WriteLine(SpawnGroupQuery);
                Console.WriteLine(SpawnEntryQuery);
                Console.WriteLine(Spawn2EntryQuery);
               
            }

            List<byte[]> VariableUpdatePackets = pm.GetPacketsOfType("OP_VariableUpdate");
            Console.WriteLine("-- There are {0} OP_VariableUpdate packets.", VariableUpdatePackets.Count);

            //Console.WriteLine(Utils.HexDump(VariableUpdatePackets[0]));

            foreach (byte[] up in VariableUpdatePackets)
            {
                MemoryStream vums = new MemoryStream(up.GetUpperBound(0) + 1);
                vums.Write(up, 0, up.GetUpperBound(0) + 1);
                vums.Seek(0, System.IO.SeekOrigin.Begin);

                BinaryReader vubinReader = new BinaryReader(vums);

                byte[] bb = vubinReader.ReadBytes(13);

                BitStream bs = new BitStream(bb, 13);

                UInt32 VSpawnID = bs.readUInt(16);
                UInt32 VFlags = bs.readUInt(6);

                float Vy = (float)bs.readInt(19) / (float)(1 << 3);
                float Vx = (float)bs.readInt(19) / (float)(1 << 3);
                float Vz = (float)bs.readInt(19) / (float)(1 << 3);
                float VHeading = (float)bs.readInt(12) / (float)(1 << 3);

                //Console.WriteLine("Loc is ", Vx, Vy, Vz, VHeading);

                //Console.WriteLine("VSpawnID is {0}, Loc: {1}, {2}, {3} Heading {4}", VSpawnID, Vx, Vy, Vz, VHeading);

                Position wp = new Position(Vx, Vy, Vz, VHeading);

                NPCSL.AddWaypoint(VSpawnID, wp, true);
            }

            List<byte[]> UpdatePackets = pm.GetPacketsOfType("OP_MobUpdate");
            Console.WriteLine("-- There are {0} OP_MobUpdate packets.", UpdatePackets.Count);

            foreach (byte[] up in UpdatePackets)
            {
                //Console.WriteLine(Utils.HexDump(up));

                MemoryStream mums = new MemoryStream(up.GetUpperBound(0) + 1);
                mums.Write(up, 0, up.GetUpperBound(0) + 1);
                mums.Seek(0, System.IO.SeekOrigin.Begin);

                BinaryReader mubinReader = new BinaryReader(mums);

                UInt16 UpdateSpawnID = mubinReader.ReadUInt16();

                

                //Console.WriteLine("Position in stream is {0}", mums.Position);
                UInt32 Word1 = mubinReader.ReadUInt32();
                UInt32 Word2 = mubinReader.ReadUInt32();
                //Console.WriteLine("Position in stream is {0}", mums.Position);
                //Console.WriteLine("Length of stream is {0}", mums.Length);

                UInt16 Word3 = mubinReader.ReadUInt16();


                Int32 UpdateIntY = (Int32)(Word1 & 0x7FFFF);

                if ((UpdateIntY & 0x40000) > 0)
                    UpdateIntY = -(0x7FFFF - UpdateIntY + 1);

                float UpdateYPos = (float)UpdateIntY / (float)(1 << 3);

                // Z is in the top 13 bits of Word1 and the bottom 6 of Word2

                UInt32 ZPart1 = Word1 >> 19;    // ZPart1 now has low order bits of Z in bottom 13 bits
                UInt32 ZPart2 = Word2 & 0x3F;   // ZPart2 now has low order bits of Z in bottom 6 bits

                ZPart2 = ZPart2 << 13;

                Int32 UpdateIntZ = (Int32)(ZPart1 | ZPart2); // Combine the two parts

                if ((UpdateIntZ & 0x40000) > 0)
                    UpdateIntZ = -(0x7FFFF - UpdateIntZ + 1);

                float UpdateZPos = (float)UpdateIntZ / (float)(1 << 3);

                Int32 UpdateIntX = (Int32)(Word2 >> 6) & 0x7FFFF;

                if ((UpdateIntX & 0x40000) > 0)
                    UpdateIntX = -(0x7FFFF - UpdateIntX + 1);


                float UpdateXPos = (float)UpdateIntX / (float)(1 << 3);

                Int32 UpdateIntHeading = (Int32)(Word3 & 0xFFF);

                float UpdateHeading = (float)UpdateIntHeading / (float)(1 << 3);

                //Console.WriteLine("SpawnID is {0} Position: {1}, {2}, {3} Heading {4}", UpdateSpawnID, UpdateXPos, UpdateYPos, UpdateZPos, UpdateHeading);
                if (NPCSL.DoesHaveHighResWaypoints(UpdateSpawnID))
                {
                    //Console.WriteLine("-- Skipping low res waypoint update for spawn {0} ({1}, {2}, {3})", UpdateSpawnID, UpdateXPos, UpdateYPos, UpdateZPos);
                    continue;
                }
                //Console.WriteLine("-- Adding low res waypoint update for spawn {0}", UpdateSpawnID);
                Position wp = new Position(UpdateXPos, UpdateYPos, UpdateZPos, UpdateHeading);

                NPCSL.AddWaypoint(UpdateSpawnID, wp, false);
            }

            UInt32 GridDBID = 501000;
            UInt32 ZoneID = 383;

            Console.WriteLine("DELETE from grid WHERE id >= {0} AND id <= {1};", GridDBID, GridDBID + 1000);
            Console.WriteLine("DELETE from grid_entries WHERE gridid >= {0} AND gridid <= {1};", GridDBID, GridDBID + 1000);
            foreach (NPCSpawn ns in NPCSL._NPCSpawnList)
            {
                if (ns.Waypoints.Count > 1)
                {
                    float LastHeading = -12345;

                    Console.WriteLine("-- Spawn {0} has {1} waypoints (including it's spawn point)", ns.SpawnID, ns.Waypoints.Count);
                    Console.WriteLine("INSERT into grid(`id`, `zoneid`, `type`, `type2`) VALUES({0}, {1}, {2}, {3}); -- {4}",
                                       GridDBID, ZoneID, 3, 2, ns.Name);
                    Console.WriteLine("UPDATE spawn2 set pathgrid = {0} WHERE id = {1};", GridDBID, ns.Spawn2DBID);

                    int WPNum = 1;
                    foreach (Position p in ns.Waypoints)
                    {
                        if ((LastHeading != -12345) && (Math.Abs(p.heading - LastHeading) < 1))
                        {
                            //Console.WriteLine("-- Skipping waypoint as heading is the same. {0} vs {1}", LastHeading, p.heading);
                        }
                        else
                        {
                            Console.WriteLine("INSERT into grid_entries (`gridid`, `zoneid`, `number`, `x`, `y`, `z`, `heading`, `pause`) VALUES({0}, {1}, {2}, {3}, {4}, {5}, {6}, {7});",
                                               GridDBID, ZoneID, WPNum++, p.x, p.y, p.z, p.heading, 0);
                        }
                        LastHeading = p.heading;
                        //Console.WriteLine("   {0}, {1}, {2} Heading: {3}", p.x, p.y, p.z, p.heading);
                    }
                    ++GridDBID;
                }
            }
            
            

            //Console.Write("Hit 'Enter' to exit...");
            //Console.ReadLine();
        }

        /// <summary>
        /// Prints the source and dest MAC addresses of each received Ethernet frame
        /// </summary>
        private static void device_OnPacketArrival(object sender, CaptureEventArgs e)
        {
            //if (PacketsSeen > 10)
            //    return;

            if(e.Packet.LinkLayerType == PacketDotNet.LinkLayers.Ethernet)
            {
                var packet = PacketDotNet.Packet.ParsePacket(e.Packet);
                var ethernetPacket = (PacketDotNet.EthernetPacket)packet;

                var udpPacket = PacketDotNet.UdpPacket.GetEncapsulated(packet);

                if (udpPacket != null)
                {
                    var ipPacket = (PacketDotNet.IpPacket)udpPacket.ParentPacket;
                    System.Net.IPAddress srcIp = ipPacket.SourceAddress;
                    System.Net.IPAddress dstIp = ipPacket.DestinationAddress;

                    //Console.WriteLine("UDP packet.");
                    //Console.WriteLine("Source IP {0}:{1} Dest IP {2}:{3}", srcIp, udpPacket.SourcePort, dstIp, udpPacket.DestinationPort);
                    //Console.WriteLine(HexDump(udpPacket.Bytes));

                    byte[] Payload = udpPacket.PayloadData;

                    pm.ProcessPacket(srcIp, dstIp, udpPacket.SourcePort, udpPacket.DestinationPort, Payload, false);
                    ++PacketsSeen;

                    
                }
            }
        }
    }
}

