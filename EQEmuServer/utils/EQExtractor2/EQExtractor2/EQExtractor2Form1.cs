//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
// 

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using SharpPcap;    
using zlib;
using MyUtils;
using EQPacket;
using EQApplicationLayer;

namespace EQExtractor2
{
    

    public partial class EQExtractor2Form1 : Form
    {
        string Version = "EQExtractor2 Version 2.0.2 SVN";
        //static PacketManager pm;
        static int PacketsSeen = 0;
        static long BytesRead = 0;
        static long CaptureFileSize = 0;
        string ZoneName;

        StreamWriter SQLStream;
        EQStreamProcessor StreamProcessor;

        public EQExtractor2Form1()
        {
            InitializeComponent();
#if DEBUG
            Version += " (Debug Build)";
#else
            Version += " (Release Build)";
#endif
            Text = Version;

            ConsoleWindow.Items.Add("EQExtractor2 Initialised.");
                        
            // Print SharpPcap version
            string ver = SharpPcap.Version.VersionString;
            Log("");
            Log("Instructions:");
            Log("Generate a .pcap file using Wireshark. To do this, park a character in the zone you want to collect in.");
            Log("Camp to character select. Start Wireshark capturing. Zone your character in and just sit around for a");
            Log("while, or go and inspect merchant inventories if you want to collect those. When finished, stop the");
            Log("Wireshark capture and save it (File/Save As).");
            Log("");
            Log("Load the .pcap file into this program, select the check boxes and set the starting SQL INSERT IDs as required.");
            Log("Click on Generate SQL to ... generate SQL. Review it before sourcing as DELETEs are auto-generated.");
            Log("The Packet Dump button will give you a hex dump of all the EQ packets.");
            Log("");            
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void InputFileOpenDialog_FileOk(object sender, CancelEventArgs e)
        {

        }

        public void Log(string Message)
        {
            ConsoleWindow.Items.Add(Message);
            ConsoleWindow.SelectedIndex = ConsoleWindow.Items.Count - 1;
            Application.DoEvents();
        }

        private void ProcessFileButton_Click(object sender, EventArgs e)
        {
            if (InputFileOpenDialog.ShowDialog() != DialogResult.OK)
                return;

            GenerateSQLButton.Enabled = false;

            PacketDumpButton.Enabled = false;

            DumpAAButton.Enabled = false;

            SharpPcap.OfflinePcapDevice device;

            try
            {
                // Get an offline device
                string CapFile = InputFileOpenDialog.FileName;

                device = new SharpPcap.OfflinePcapDevice(CapFile);

                // Open the device
                device.Open();
            }
            catch
            {                
                Log("Error: File does not exist or not in .pcap format.");
                return;
            }
                        
            StreamProcessor = new EQStreamProcessor();                        

            if (!StreamProcessor.Init(Application.StartupPath, this.Log))
            {
                Log("Fatal error initialising Stream Processor. No decoders could be initialised (mostly likely misplaced patch_XXXX.conf files.");
                return;
            }
                        
            device.OnPacketArrival +=
                new PacketArrivalEventHandler(device_OnPacketArrival);

            BytesRead = 0;
            PacketsSeen = 0;
            
            ConsoleWindow.Items.Add("-- Capturing from '" + InputFileOpenDialog.FileName);
            ProgressBar.Value = 0;
            ProgressBar.Show();
                        
            ProcessFileButton.Enabled = false;

            CaptureFileSize = device.FileSize;
            
            device.Capture();
                        
            device.Close();

            ConsoleWindow.Items.Add("End of file reached. Processed " + PacketsSeen + " packets and " + BytesRead + " bytes.");

            ProgressBar.Hide();
            
            if (StreamProcessor.Packets.ErrorsInStream)
                Log("There were errors encountered in the packet stream. Data may be incomplete.");

            ConsoleWindow.SelectedIndex = ConsoleWindow.Items.Count - 1;
            
            ProcessFileButton.Enabled = true;
            
            StreamProcessor.PCAPFileReadFinished();

            PacketDumpButton.Enabled = true;

            Log("Stream recognised as " + StreamProcessor.GetDecoderVersion());

            int PPLength = StreamProcessor.VerifyPlayerProfile();

            if (PPLength == 0)
            {
                Log("Unable to find player profile packet, or packet not of correct size.");
                DumpAAButton.Enabled = false;
                GenerateSQLButton.Enabled = false;
                return;
            }
            else
            {
                Log("Found player profile packet of the expected length (" + PPLength + ").");
            }

            ZoneName = StreamProcessor.GetZoneName();
            
            UInt32 ZoneNumber = StreamProcessor.GetZoneNumber();

            Log("Zonename is " + StreamProcessor.GetZoneName());

            Log("Zone number is " + ZoneNumber);

            ZoneIDTextBox.Text = ZoneNumber.ToString();

            ZoneIDTextBox.Enabled = true;
            DoorsTextBox.Enabled = true;
            NPCTypesTextBox.Enabled = true;
            SpawnEntryTextBox.Enabled = true;
            SpawnGroupTextBox.Enabled = true;
            Spawn2TextBox.Enabled = true;
            GridTextBox.Enabled = true;
            ObjectTextBox.Enabled = true;
            GroundSpawnTextBox.Enabled = true;
            MerchantTextBox.Enabled = true;
            VersionSelector.Enabled = true;
            GenerateSQLButton.Enabled = true;
            PacketDumpButton.Enabled = true;
            DumpAAButton.Enabled = true;

            RecalculateBaseInsertIDs();
                        
            StreamProcessor.GenerateZonePointList();
        }

        private void RecalculateBaseInsertIDs()
        {
            UInt32 ZoneNumber = Convert.ToUInt32(ZoneIDTextBox.Text);
            NPCTypesTextBox.Text = ((ZoneNumber * 1000) + (VersionSelector.Value * 100)).ToString();
            SpawnEntryTextBox.Text = NPCTypesTextBox.Text;
            SpawnGroupTextBox.Text = NPCTypesTextBox.Text;
            Spawn2TextBox.Text = NPCTypesTextBox.Text;
            GridTextBox.Text = NPCTypesTextBox.Text;
            ObjectTextBox.Text = NPCTypesTextBox.Text;
            GroundSpawnTextBox.Text = NPCTypesTextBox.Text;
            MerchantTextBox.Text = NPCTypesTextBox.Text;

            DoorsTextBox.Text = ((VersionSelector.Value * 1000)).ToString();
        }

        private void device_OnPacketArrival(object sender, SharpPcap.CaptureEventArgs e)
        {            
            if (e.Packet.LinkLayerType == PacketDotNet.LinkLayers.Ethernet)
            {
                PacketDotNet.Packet packet;

                long TotalPacketSize = e.Packet.Data.Length;
                BytesRead += TotalPacketSize;
                ++PacketsSeen;

                if ((PacketsSeen > 0) && ((PacketsSeen % 10000) == 0))
                {         
                    ConsoleWindow.SelectedIndex = ConsoleWindow.Items.Count - 1;
                    int Progress = (int)((float)BytesRead / (float)CaptureFileSize * 100);
                    ProgressBar.Value = Progress;
                 
                    Application.DoEvents();
                }

                try
                {
                    packet = PacketDotNet.Packet.ParsePacket(e.Packet);
                }
                catch
                {
                    return;
                }

                var ethernetPacket = (PacketDotNet.EthernetPacket)packet;

                var udpPacket = PacketDotNet.UdpPacket.GetEncapsulated(packet);

                if (udpPacket != null)
                {
                    var ipPacket = (PacketDotNet.IpPacket)udpPacket.ParentPacket;
                    System.Net.IPAddress srcIp = ipPacket.SourceAddress;
                    System.Net.IPAddress dstIp = ipPacket.DestinationAddress;
                    
                    byte[] Payload = udpPacket.PayloadData;

                    Int32 l = udpPacket.Length - udpPacket.Header.GetLength(0);

                    if (l > 0)
                    {
                        Array.Resize(ref Payload, l);

                        StreamProcessor.ProcessPacket(srcIp, dstIp, udpPacket.SourcePort, udpPacket.DestinationPort, Payload);
                    }
                }
            }
        }
        
        public void WriteSQL(string Message)
        {
            SQLStream.WriteLine(Message);
         }

        private void GenerateSQLButton_Click(object sender, EventArgs e)
        {
            if (SQLFileDialog.ShowDialog() != DialogResult.OK)
                return;

            string SQLFile = SQLFileDialog.FileName;

            try
            {
                SQLStream = new StreamWriter(SQLFile);
            }
            catch
            {
                Log("Unable to open file " + SQLFile + " for writing.");
                return;
            }

            UInt32 SpawnDBID = Convert.ToUInt32(NPCTypesTextBox.Text);
            UInt32 SpawnGroupID = Convert.ToUInt32(SpawnGroupTextBox.Text);
            UInt32 SpawnEntryID = Convert.ToUInt32(SpawnEntryTextBox.Text);
            UInt32 Spawn2ID = Convert.ToUInt32(Spawn2TextBox.Text);
            UInt32 GridDBID = Convert.ToUInt32(GridTextBox.Text);
            UInt32 MerchantDBID = Convert.ToUInt32(MerchantTextBox.Text);
            int DoorDBID = Convert.ToInt32(DoorsTextBox.Text);
            UInt32 GroundSpawnDBID = Convert.ToUInt32(GroundSpawnTextBox.Text);
            UInt32 ObjectDBID = Convert.ToUInt32(ObjectTextBox.Text);

            UInt32 ZoneID = Convert.ToUInt32(ZoneIDTextBox.Text);


            WriteSQL("-- SQL created by " + Version);
            WriteSQL("--");
            WriteSQL("-- Using Decoder: " + StreamProcessor.GetDecoderVersion());
            WriteSQL("--");
            WriteSQL("-- Change these variables if required");
            WriteSQL("--");
            WriteSQL("set @StartingNPCTypeID = " + SpawnDBID + ";");
            WriteSQL("set @StartingSpawnGroupID = " + SpawnGroupID + ";");
            WriteSQL("set @StartingSpawnEntryID = " + SpawnEntryID + ";");
            WriteSQL("set @StartingSpawn2ID = " + Spawn2ID + ";");
            WriteSQL("set @StartingGridID = " + GridDBID + ";");
            WriteSQL("set @StartingMerchantID = " + MerchantDBID + ";");
            WriteSQL("set @BaseDoorID = " + DoorDBID + ";");
            WriteSQL("set @StartingGroundSpawnID = " + GroundSpawnDBID + ";");
            WriteSQL("set @StartingObjectID = " + ObjectDBID + ";");
            WriteSQL("--");
            WriteSQL("--");
            
            if(ZoneCheckBox.Checked)
                StreamProcessor.GenerateZoneSQL(this.WriteSQL);

            if(ZonePointCheckBox.Checked)
                StreamProcessor.GenerateZonePointSQL(ZoneName, this.WriteSQL);
            
            UInt32 SpawnVersion = (UInt32)VersionSelector.Value;

            if (DoorCheckBox.Checked)
            {                
                Log("Starting to generate SQL for Doors.");
                StreamProcessor.GenerateDoorsSQL(ZoneName, DoorDBID, SpawnVersion, this.WriteSQL);
                Log("Finished generating SQL for Doors.");                
            }
                        
            Log("Starting to generate SQL for Spawns and/or Grids.");

            StreamProcessor.GenerateSpawnSQL(SpawnCheckBox.Checked, GridCheckBox.Checked, MerchantCheckBox.Checked, ZoneName, ZoneID, SpawnVersion, UpdateExistingNPCTypesCheckbox.Checked, NPCTypesTintCheckBox.Checked, this.WriteSQL);

            Log("Finished generating SQL for Spawns and/or Grids.");

            if (GroundSpawnCheckBox.Checked || ObjectCheckBox.Checked)
            {
                Log("Starting to generate SQL for Ground Spawns and/or Objects.");

                StreamProcessor.GenerateObjectSQL(GroundSpawnCheckBox.Checked, ObjectCheckBox.Checked, SpawnVersion, this.WriteSQL);

                Log("Finished generating SQL for Ground Spawns and/or Objects.");
            }
            
            //EQStreamProcessor.GenerateMerchantSQL(pm, this.WriteSQL);
            SQLStream.Close();  
        }

        private void DisableAllControls()
        {

            foreach (Control c in this.Controls)
            {
                if ((c is Button) || (c is TextBox) || (c is MaskedTextBox) || (c is CheckBox))
                    c.Enabled = false;
                
            }

        }

        private void EnableAllControls()
        {

            foreach (Control c in this.Controls)
                c.Enabled = true;
            
            GenerateSQLButton.Enabled = StreamProcessor.StreamRecognised();

            DumpAAButton.Enabled = StreamProcessor.StreamRecognised();
        }

        private void PacketDumpButton_Click(object sender, EventArgs e)
        {
            if (PacketDumpFileDialog.ShowDialog() == DialogResult.OK)
            {
                Log("Packets dump in progress...");

                DisableAllControls();

                if (StreamProcessor.DumpPackets(PacketDumpFileDialog.FileName))
                    Log("Packets dumped successfully.");
                else
                    Log("Packet dump failed.");

                EnableAllControls();

            }
                
        }

        private void SpawnCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (!SpawnCheckBox.Checked)
            {
                GridCheckBox.Checked = false;
                GridCheckBox.Enabled = false;
            }
            else
                GridCheckBox.Enabled = true;
        }

        private void DumpAAButton_Click(object sender, EventArgs e)
        {
            if (PacketDumpFileDialog.ShowDialog() == DialogResult.OK)
            {
                Log("AA dump in progress...");

                DisableAllControls();

                if (StreamProcessor.DumpAAs(PacketDumpFileDialog.FileName))
                    Log("AAs dumped successfully.");
                else
                    Log("AA dump failed.");

                EnableAllControls();
            }
        }

        private void VersionSelector_ValueChanged(object sender, EventArgs e)
        {
            RecalculateBaseInsertIDs();
        }

        private void ZoneIDTextBox_Validated(object sender, EventArgs e)
        {
            RecalculateBaseInsertIDs();
        }

        private void UpdateExistingNPCTypesCheckbox_CheckedChanged(object sender, EventArgs e)
        {
            if (UpdateExistingNPCTypesCheckbox.Checked)
            {
                SpawnCheckBox.Checked = false;
                SpawnCheckBox.Enabled = false;
                GridCheckBox.Checked = false;
                GridCheckBox.Enabled = false;
                MerchantCheckBox.Checked = false;
                MerchantCheckBox.Enabled = false;
                ZoneCheckBox.Checked = false;
                ZonePointCheckBox.Checked = false;
                GridCheckBox.Checked = false;
                DoorCheckBox.Checked = false;
                ObjectCheckBox.Checked = false;
                GroundSpawnCheckBox.Checked = false;
            }
            else
            {
                SpawnCheckBox.Enabled = true;
                GridCheckBox.Enabled = true;
                MerchantCheckBox.Enabled = true;
            }
        }

        

        
    }
    
}

