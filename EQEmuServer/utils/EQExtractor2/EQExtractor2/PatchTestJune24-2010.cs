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
        override public int ExpectedPPLength()
        {
            return 26640;
        }

        override public string GetVersion()
        {
            return "EQ Client Build Date Test Server June 24 2010.";
        }
    }
}
