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
    class PatchTestJune242010Decoder : PatchMay122010Decoder
    {
        
        private const string Version = "EQ Client Build Date Test Server June 24 2010.";

        // Opcodes seem to be unchanged, only PP length seems to have changed.

        private const string PatchConfFile = "patch_May12-2010.conf";

        private const int PPLength = 26640;

        private const int PPZoneIDOffset = 19396;

        override public string GetVersion()
        {
            return Version;
        }

        override public bool UnsupportedVersion()
        {
            return false;
        }

        override public bool Init(string ConfDirectory, ref string ErrorMessage)
        {
            OpManager = new OpCodeManager();

            if (!OpManager.Init(ConfDirectory + "\\" + PatchConfFile, ref ErrorMessage))
                return false;

            return true;
        }

        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if ((OpCode == OpManager.OpCodeNameToNumber("OP_ZoneEntry")) && (Direction == PacketDirection.ClientToServer))
                return IdentificationStatus.Tentative;

            if ((OpCode == OpManager.OpCodeNameToNumber("OP_PlayerProfile")) && (Direction == PacketDirection.ServerToClient) &&
                (Size == PPLength))
                return IdentificationStatus.Yes;

            return IdentificationStatus.No;
        }

        override public int VerifyPlayerProfile()
        {
            List<byte[]> PlayerProfilePacket = GetPacketsOfType("OP_PlayerProfile", PacketDirection.ServerToClient);

            if (PlayerProfilePacket.Count == 0)
            {
                return 0;
            }
            else
            {
                if (PlayerProfilePacket[0].Length != PPLength)
                {
                    return 0;
                }
            }

            return PPLength;
        }

        override public UInt16 GetZoneNumber()
        {
            // A return value of zero from this method should be intepreted as 'Unable to identify patch version'.

            List<byte[]> PlayerProfilePacket = GetPacketsOfType("OP_PlayerProfile", PacketDirection.ServerToClient);

            if (PlayerProfilePacket.Count == 0)
            {
                return 0;
            }
            else
            {
                if (PlayerProfilePacket[0].Length != PPLength)
                {
                    return 0;
                }
            }

            return BitConverter.ToUInt16(PlayerProfilePacket[0], PPZoneIDOffset);
        }
    }
}
