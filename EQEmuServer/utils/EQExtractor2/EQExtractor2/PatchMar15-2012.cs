using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchMar152012Decoder : PatchNov172011Decoder
    {
        public PatchMar152012Decoder()
        {
            Version = "EQ Client Build Date March 15 2012.";

            ExpectedPPLength = 29688;

            PPZoneIDOffset = 22356;

            PatchConfFileName = "patch_Mar15-2012.conf";
        }               
    }
}