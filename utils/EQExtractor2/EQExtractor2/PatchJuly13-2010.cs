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
    class PatchJuly132010Decoder : PatchMay122010Decoder
    {
        public PatchJuly132010Decoder()
        {
            Version = "EQ Client Build Date July 13 2010. (Including Test Server June 24 to July 8 2010).";

            ExpectedPPLength = 26640;
        }

        public override void RegisterExplorers()
        {
            base.RegisterExplorers();

            //OpManager.RegisterExplorer("OP_ZoneEntry", ExploreZoneEntry);
            OpManager.RegisterExplorer("OP_CastSpell", ExploreCastSpell);
            //OpManager.RegisterExplorer("OP_SpawnAppearance", ExploreSpawnAppearance);
        }

        

        public void ExploreCastSpell(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 Slot = Buffer.ReadUInt32();
            UInt32 SpellID = Buffer.ReadUInt32();

            OutputStream.WriteLine("Casting spell {0} from slot {1}", SpellID, Slot);

            OutputStream.WriteLine("");
        }

        
    }
}
