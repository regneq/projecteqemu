using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchDec072010Decoder : PatchOct202010Decoder
    {
        public PatchDec072010Decoder()
        {
            Version = "EQ Client Build Date December 7 2010.";

            PatchConfFileName = "patch_Dec7-2010.conf";
            
        }
        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if ((OpCode == OpManager.OpCodeNameToNumber("OP_ZoneEntry")) && (Direction == PacketDirection.ClientToServer))
                return IdentificationStatus.Tentative;

            if ((OpCode == OpManager.OpCodeNameToNumber("OP_SendAATable")) && (Direction == PacketDirection.ServerToClient) &&
                (Size == 120))
                return IdentificationStatus.Yes;

            return IdentificationStatus.No;
        }
    }
}