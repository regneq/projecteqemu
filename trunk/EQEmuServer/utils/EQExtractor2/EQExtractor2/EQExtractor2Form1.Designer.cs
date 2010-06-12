namespace EQExtractor2
{
    partial class EQExtractor2Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.InputFileOpenDialog = new System.Windows.Forms.OpenFileDialog();
            this.ConsoleWindow = new System.Windows.Forms.ListBox();
            this.ProcessFileButton = new System.Windows.Forms.Button();
            this.IncludeLabel = new System.Windows.Forms.Label();
            this.DoorCheckBox = new System.Windows.Forms.CheckBox();
            this.SpawnCheckBox = new System.Windows.Forms.CheckBox();
            this.GridCheckBox = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.NPCTypesTextBox = new System.Windows.Forms.MaskedTextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.ZoneIDTextBox = new System.Windows.Forms.MaskedTextBox();
            this.DoorsLabel = new System.Windows.Forms.Label();
            this.DoorsTextBox = new System.Windows.Forms.MaskedTextBox();
            this.SpawnGroupLabel = new System.Windows.Forms.Label();
            this.SpawnGroupTextBox = new System.Windows.Forms.MaskedTextBox();
            this.SpawnEntryLabel = new System.Windows.Forms.Label();
            this.SpawnEntryTextBox = new System.Windows.Forms.MaskedTextBox();
            this.Spawn2Label = new System.Windows.Forms.Label();
            this.Spawn2TextBox = new System.Windows.Forms.MaskedTextBox();
            this.GenerateSQLButton = new System.Windows.Forms.Button();
            this.SQLFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.PacketDumpButton = new System.Windows.Forms.Button();
            this.PacketDumpFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.GridLabel = new System.Windows.Forms.Label();
            this.GridTextBox = new System.Windows.Forms.MaskedTextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.ObjectTextBox = new System.Windows.Forms.MaskedTextBox();
            this.GroundSpawnTextBox = new System.Windows.Forms.MaskedTextBox();
            this.GroundSpawnLabel = new System.Windows.Forms.Label();
            this.ObjectCheckBox = new System.Windows.Forms.CheckBox();
            this.GroundSpawnCheckBox = new System.Windows.Forms.CheckBox();
            this.MerchantCheckBox = new System.Windows.Forms.CheckBox();
            this.MerchantLabel = new System.Windows.Forms.Label();
            this.MerchantTextBox = new System.Windows.Forms.MaskedTextBox();
            this.ProgressBar = new System.Windows.Forms.ProgressBar();
            this.VersionSelector = new System.Windows.Forms.NumericUpDown();
            this.VersionLabel = new System.Windows.Forms.Label();
            this.DumpAAButton = new System.Windows.Forms.Button();
            this.ZoneCheckBox = new System.Windows.Forms.CheckBox();
            this.ZonePointCheckBox = new System.Windows.Forms.CheckBox();
            this.UpdateExistingNPCTypesCheckbox = new System.Windows.Forms.CheckBox();
            this.NPCTypesTintCheckBox = new System.Windows.Forms.CheckBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.VersionSelector)).BeginInit();
            this.SuspendLayout();
            // 
            // InputFileOpenDialog
            // 
            this.InputFileOpenDialog.Filter = "Capture Files (*.pcap)|*.pcap|All files (*.*)|*.*";
            this.InputFileOpenDialog.FileOk += new System.ComponentModel.CancelEventHandler(this.InputFileOpenDialog_FileOk);
            // 
            // ConsoleWindow
            // 
            this.ConsoleWindow.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ConsoleWindow.FormattingEnabled = true;
            this.ConsoleWindow.HorizontalScrollbar = true;
            this.ConsoleWindow.ItemHeight = 14;
            this.ConsoleWindow.Location = new System.Drawing.Point(19, 240);
            this.ConsoleWindow.Name = "ConsoleWindow";
            this.ConsoleWindow.Size = new System.Drawing.Size(929, 186);
            this.ConsoleWindow.TabIndex = 34;
            // 
            // ProcessFileButton
            // 
            this.ProcessFileButton.Location = new System.Drawing.Point(15, 10);
            this.ProcessFileButton.Name = "ProcessFileButton";
            this.ProcessFileButton.Size = new System.Drawing.Size(89, 23);
            this.ProcessFileButton.TabIndex = 0;
            this.ProcessFileButton.Text = "Load .pcap File";
            this.ProcessFileButton.UseVisualStyleBackColor = true;
            this.ProcessFileButton.Click += new System.EventHandler(this.ProcessFileButton_Click);
            // 
            // IncludeLabel
            // 
            this.IncludeLabel.AutoSize = true;
            this.IncludeLabel.Location = new System.Drawing.Point(12, 48);
            this.IncludeLabel.Name = "IncludeLabel";
            this.IncludeLabel.Size = new System.Drawing.Size(45, 13);
            this.IncludeLabel.TabIndex = 1;
            this.IncludeLabel.Text = "Include:";
            // 
            // DoorCheckBox
            // 
            this.DoorCheckBox.AutoSize = true;
            this.DoorCheckBox.Checked = true;
            this.DoorCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.DoorCheckBox.Location = new System.Drawing.Point(96, 44);
            this.DoorCheckBox.Name = "DoorCheckBox";
            this.DoorCheckBox.Size = new System.Drawing.Size(54, 17);
            this.DoorCheckBox.TabIndex = 2;
            this.DoorCheckBox.Text = "Doors";
            this.DoorCheckBox.UseVisualStyleBackColor = true;
            // 
            // SpawnCheckBox
            // 
            this.SpawnCheckBox.AutoSize = true;
            this.SpawnCheckBox.Checked = true;
            this.SpawnCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.SpawnCheckBox.Location = new System.Drawing.Point(182, 44);
            this.SpawnCheckBox.Name = "SpawnCheckBox";
            this.SpawnCheckBox.Size = new System.Drawing.Size(64, 17);
            this.SpawnCheckBox.TabIndex = 3;
            this.SpawnCheckBox.Text = "Spawns";
            this.SpawnCheckBox.UseVisualStyleBackColor = true;
            this.SpawnCheckBox.CheckedChanged += new System.EventHandler(this.SpawnCheckBox_CheckedChanged);
            // 
            // GridCheckBox
            // 
            this.GridCheckBox.AutoSize = true;
            this.GridCheckBox.Checked = true;
            this.GridCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.GridCheckBox.Location = new System.Drawing.Point(268, 44);
            this.GridCheckBox.Name = "GridCheckBox";
            this.GridCheckBox.Size = new System.Drawing.Size(50, 17);
            this.GridCheckBox.TabIndex = 4;
            this.GridCheckBox.Text = "Grids";
            this.GridCheckBox.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 96);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(92, 13);
            this.label2.TabIndex = 12;
            this.label2.Text = "Start Insert IDs at:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(215, 96);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(58, 13);
            this.label3.TabIndex = 15;
            this.label3.Text = "NPCTypes";
            // 
            // NPCTypesTextBox
            // 
            this.NPCTypesTextBox.Enabled = false;
            this.NPCTypesTextBox.HidePromptOnLeave = true;
            this.NPCTypesTextBox.Location = new System.Drawing.Point(279, 92);
            this.NPCTypesTextBox.Mask = "0000000000";
            this.NPCTypesTextBox.Name = "NPCTypesTextBox";
            this.NPCTypesTextBox.PromptChar = ' ';
            this.NPCTypesTextBox.Size = new System.Drawing.Size(61, 20);
            this.NPCTypesTextBox.TabIndex = 16;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 71);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(46, 13);
            this.label4.TabIndex = 8;
            this.label4.Text = "ZoneID:";
            // 
            // ZoneIDTextBox
            // 
            this.ZoneIDTextBox.Enabled = false;
            this.ZoneIDTextBox.HidePromptOnLeave = true;
            this.ZoneIDTextBox.Location = new System.Drawing.Point(96, 67);
            this.ZoneIDTextBox.Mask = "0000000000";
            this.ZoneIDTextBox.Name = "ZoneIDTextBox";
            this.ZoneIDTextBox.PromptChar = ' ';
            this.ZoneIDTextBox.Size = new System.Drawing.Size(100, 20);
            this.ZoneIDTextBox.TabIndex = 9;
            this.ZoneIDTextBox.Validated += new System.EventHandler(this.ZoneIDTextBox_Validated);
            // 
            // DoorsLabel
            // 
            this.DoorsLabel.AutoSize = true;
            this.DoorsLabel.Location = new System.Drawing.Point(106, 96);
            this.DoorsLabel.Name = "DoorsLabel";
            this.DoorsLabel.Size = new System.Drawing.Size(35, 13);
            this.DoorsLabel.TabIndex = 13;
            this.DoorsLabel.Text = "Doors";
            // 
            // DoorsTextBox
            // 
            this.DoorsTextBox.Enabled = false;
            this.DoorsTextBox.HidePromptOnLeave = true;
            this.DoorsTextBox.Location = new System.Drawing.Point(147, 92);
            this.DoorsTextBox.Mask = "0000000000";
            this.DoorsTextBox.Name = "DoorsTextBox";
            this.DoorsTextBox.PromptChar = ' ';
            this.DoorsTextBox.Size = new System.Drawing.Size(61, 20);
            this.DoorsTextBox.TabIndex = 14;
            // 
            // SpawnGroupLabel
            // 
            this.SpawnGroupLabel.AutoSize = true;
            this.SpawnGroupLabel.Location = new System.Drawing.Point(346, 96);
            this.SpawnGroupLabel.Name = "SpawnGroupLabel";
            this.SpawnGroupLabel.Size = new System.Drawing.Size(69, 13);
            this.SpawnGroupLabel.TabIndex = 17;
            this.SpawnGroupLabel.Text = "SpawnGroup";
            // 
            // SpawnGroupTextBox
            // 
            this.SpawnGroupTextBox.Enabled = false;
            this.SpawnGroupTextBox.HidePromptOnLeave = true;
            this.SpawnGroupTextBox.Location = new System.Drawing.Point(421, 92);
            this.SpawnGroupTextBox.Mask = "0000000000";
            this.SpawnGroupTextBox.Name = "SpawnGroupTextBox";
            this.SpawnGroupTextBox.PromptChar = ' ';
            this.SpawnGroupTextBox.Size = new System.Drawing.Size(61, 20);
            this.SpawnGroupTextBox.TabIndex = 18;
            // 
            // SpawnEntryLabel
            // 
            this.SpawnEntryLabel.AutoSize = true;
            this.SpawnEntryLabel.Location = new System.Drawing.Point(488, 96);
            this.SpawnEntryLabel.Name = "SpawnEntryLabel";
            this.SpawnEntryLabel.Size = new System.Drawing.Size(64, 13);
            this.SpawnEntryLabel.TabIndex = 19;
            this.SpawnEntryLabel.Text = "SpawnEntry";
            // 
            // SpawnEntryTextBox
            // 
            this.SpawnEntryTextBox.Enabled = false;
            this.SpawnEntryTextBox.HidePromptOnLeave = true;
            this.SpawnEntryTextBox.Location = new System.Drawing.Point(558, 92);
            this.SpawnEntryTextBox.Mask = "0000000000";
            this.SpawnEntryTextBox.Name = "SpawnEntryTextBox";
            this.SpawnEntryTextBox.PromptChar = ' ';
            this.SpawnEntryTextBox.Size = new System.Drawing.Size(61, 20);
            this.SpawnEntryTextBox.TabIndex = 20;
            // 
            // Spawn2Label
            // 
            this.Spawn2Label.AutoSize = true;
            this.Spawn2Label.Location = new System.Drawing.Point(625, 96);
            this.Spawn2Label.Name = "Spawn2Label";
            this.Spawn2Label.Size = new System.Drawing.Size(46, 13);
            this.Spawn2Label.TabIndex = 21;
            this.Spawn2Label.Text = "Spawn2";
            // 
            // Spawn2TextBox
            // 
            this.Spawn2TextBox.Enabled = false;
            this.Spawn2TextBox.HidePromptOnLeave = true;
            this.Spawn2TextBox.Location = new System.Drawing.Point(677, 92);
            this.Spawn2TextBox.Mask = "0000000000";
            this.Spawn2TextBox.Name = "Spawn2TextBox";
            this.Spawn2TextBox.PromptChar = ' ';
            this.Spawn2TextBox.Size = new System.Drawing.Size(61, 20);
            this.Spawn2TextBox.TabIndex = 22;
            // 
            // GenerateSQLButton
            // 
            this.GenerateSQLButton.Enabled = false;
            this.GenerateSQLButton.Location = new System.Drawing.Point(15, 147);
            this.GenerateSQLButton.Name = "GenerateSQLButton";
            this.GenerateSQLButton.Size = new System.Drawing.Size(89, 23);
            this.GenerateSQLButton.TabIndex = 31;
            this.GenerateSQLButton.Text = "GenerateSQL";
            this.GenerateSQLButton.UseVisualStyleBackColor = true;
            this.GenerateSQLButton.Click += new System.EventHandler(this.GenerateSQLButton_Click);
            // 
            // SQLFileDialog
            // 
            this.SQLFileDialog.Filter = "SQL Files (*.sql)|*.sql|Text Files (*.txt)|*.txt|All files (*.*)|*.*";
            // 
            // PacketDumpButton
            // 
            this.PacketDumpButton.Enabled = false;
            this.PacketDumpButton.Location = new System.Drawing.Point(15, 176);
            this.PacketDumpButton.Name = "PacketDumpButton";
            this.PacketDumpButton.Size = new System.Drawing.Size(89, 23);
            this.PacketDumpButton.TabIndex = 32;
            this.PacketDumpButton.Text = "Packet Dump";
            this.PacketDumpButton.UseVisualStyleBackColor = true;
            this.PacketDumpButton.Click += new System.EventHandler(this.PacketDumpButton_Click);
            // 
            // PacketDumpFileDialog
            // 
            this.PacketDumpFileDialog.Filter = "Text Files (*.txt)|*.txt|All files (*.*)|*.*";
            // 
            // GridLabel
            // 
            this.GridLabel.AutoSize = true;
            this.GridLabel.Location = new System.Drawing.Point(747, 96);
            this.GridLabel.Name = "GridLabel";
            this.GridLabel.Size = new System.Drawing.Size(31, 13);
            this.GridLabel.TabIndex = 23;
            this.GridLabel.Text = "Grids";
            // 
            // GridTextBox
            // 
            this.GridTextBox.Enabled = false;
            this.GridTextBox.HidePromptOnLeave = true;
            this.GridTextBox.Location = new System.Drawing.Point(784, 92);
            this.GridTextBox.Mask = "0000000000";
            this.GridTextBox.Name = "GridTextBox";
            this.GridTextBox.PromptChar = ' ';
            this.GridTextBox.Size = new System.Drawing.Size(61, 20);
            this.GridTextBox.TabIndex = 24;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(98, 122);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(43, 13);
            this.label1.TabIndex = 25;
            this.label1.Text = "Objects";
            // 
            // ObjectTextBox
            // 
            this.ObjectTextBox.Enabled = false;
            this.ObjectTextBox.HidePromptOnLeave = true;
            this.ObjectTextBox.Location = new System.Drawing.Point(147, 118);
            this.ObjectTextBox.Mask = "0000000000";
            this.ObjectTextBox.Name = "ObjectTextBox";
            this.ObjectTextBox.PromptChar = ' ';
            this.ObjectTextBox.Size = new System.Drawing.Size(61, 20);
            this.ObjectTextBox.TabIndex = 26;
            // 
            // GroundSpawnTextBox
            // 
            this.GroundSpawnTextBox.Enabled = false;
            this.GroundSpawnTextBox.HidePromptOnLeave = true;
            this.GroundSpawnTextBox.Location = new System.Drawing.Point(308, 118);
            this.GroundSpawnTextBox.Mask = "0000000000";
            this.GroundSpawnTextBox.Name = "GroundSpawnTextBox";
            this.GroundSpawnTextBox.PromptChar = ' ';
            this.GroundSpawnTextBox.Size = new System.Drawing.Size(61, 20);
            this.GroundSpawnTextBox.TabIndex = 28;
            // 
            // GroundSpawnLabel
            // 
            this.GroundSpawnLabel.AutoSize = true;
            this.GroundSpawnLabel.Location = new System.Drawing.Point(219, 122);
            this.GroundSpawnLabel.Name = "GroundSpawnLabel";
            this.GroundSpawnLabel.Size = new System.Drawing.Size(83, 13);
            this.GroundSpawnLabel.TabIndex = 27;
            this.GroundSpawnLabel.Text = "Ground Spawns";
            // 
            // ObjectCheckBox
            // 
            this.ObjectCheckBox.AutoSize = true;
            this.ObjectCheckBox.Checked = true;
            this.ObjectCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ObjectCheckBox.Location = new System.Drawing.Point(335, 44);
            this.ObjectCheckBox.Name = "ObjectCheckBox";
            this.ObjectCheckBox.Size = new System.Drawing.Size(62, 17);
            this.ObjectCheckBox.TabIndex = 5;
            this.ObjectCheckBox.Text = "Objects";
            this.ObjectCheckBox.UseVisualStyleBackColor = true;
            // 
            // GroundSpawnCheckBox
            // 
            this.GroundSpawnCheckBox.AutoSize = true;
            this.GroundSpawnCheckBox.Checked = true;
            this.GroundSpawnCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.GroundSpawnCheckBox.Location = new System.Drawing.Point(403, 44);
            this.GroundSpawnCheckBox.Name = "GroundSpawnCheckBox";
            this.GroundSpawnCheckBox.Size = new System.Drawing.Size(102, 17);
            this.GroundSpawnCheckBox.TabIndex = 6;
            this.GroundSpawnCheckBox.Text = "Ground Spawns";
            this.GroundSpawnCheckBox.UseVisualStyleBackColor = true;
            // 
            // MerchantCheckBox
            // 
            this.MerchantCheckBox.AutoSize = true;
            this.MerchantCheckBox.Checked = true;
            this.MerchantCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.MerchantCheckBox.Location = new System.Drawing.Point(511, 44);
            this.MerchantCheckBox.Name = "MerchantCheckBox";
            this.MerchantCheckBox.Size = new System.Drawing.Size(95, 17);
            this.MerchantCheckBox.TabIndex = 7;
            this.MerchantCheckBox.Text = "Merchant Lists";
            this.MerchantCheckBox.UseVisualStyleBackColor = true;
            // 
            // MerchantLabel
            // 
            this.MerchantLabel.AutoSize = true;
            this.MerchantLabel.Location = new System.Drawing.Point(385, 122);
            this.MerchantLabel.Name = "MerchantLabel";
            this.MerchantLabel.Size = new System.Drawing.Size(76, 13);
            this.MerchantLabel.TabIndex = 29;
            this.MerchantLabel.Text = "Merchant Lists";
            // 
            // MerchantTextBox
            // 
            this.MerchantTextBox.Enabled = false;
            this.MerchantTextBox.HidePromptOnLeave = true;
            this.MerchantTextBox.Location = new System.Drawing.Point(467, 118);
            this.MerchantTextBox.Mask = "0000000000";
            this.MerchantTextBox.Name = "MerchantTextBox";
            this.MerchantTextBox.PromptChar = ' ';
            this.MerchantTextBox.Size = new System.Drawing.Size(61, 20);
            this.MerchantTextBox.TabIndex = 30;
            // 
            // ProgressBar
            // 
            this.ProgressBar.Location = new System.Drawing.Point(4, 432);
            this.ProgressBar.Name = "ProgressBar";
            this.ProgressBar.Size = new System.Drawing.Size(898, 23);
            this.ProgressBar.TabIndex = 35;
            this.ProgressBar.Visible = false;
            // 
            // VersionSelector
            // 
            this.VersionSelector.Enabled = false;
            this.VersionSelector.Location = new System.Drawing.Point(244, 67);
            this.VersionSelector.Maximum = new decimal(new int[] {
            999,
            0,
            0,
            0});
            this.VersionSelector.Name = "VersionSelector";
            this.VersionSelector.Size = new System.Drawing.Size(39, 20);
            this.VersionSelector.TabIndex = 11;
            this.VersionSelector.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.VersionSelector.ValueChanged += new System.EventHandler(this.VersionSelector_ValueChanged);
            // 
            // VersionLabel
            // 
            this.VersionLabel.AutoSize = true;
            this.VersionLabel.Location = new System.Drawing.Point(202, 71);
            this.VersionLabel.Name = "VersionLabel";
            this.VersionLabel.Size = new System.Drawing.Size(42, 13);
            this.VersionLabel.TabIndex = 10;
            this.VersionLabel.Text = "Version";
            // 
            // DumpAAButton
            // 
            this.DumpAAButton.Enabled = false;
            this.DumpAAButton.Location = new System.Drawing.Point(15, 205);
            this.DumpAAButton.Name = "DumpAAButton";
            this.DumpAAButton.Size = new System.Drawing.Size(89, 23);
            this.DumpAAButton.TabIndex = 33;
            this.DumpAAButton.Text = "Dump AAs";
            this.DumpAAButton.UseVisualStyleBackColor = true;
            this.DumpAAButton.Click += new System.EventHandler(this.DumpAAButton_Click);
            // 
            // ZoneCheckBox
            // 
            this.ZoneCheckBox.AutoSize = true;
            this.ZoneCheckBox.Checked = true;
            this.ZoneCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ZoneCheckBox.Location = new System.Drawing.Point(612, 44);
            this.ZoneCheckBox.Name = "ZoneCheckBox";
            this.ZoneCheckBox.Size = new System.Drawing.Size(84, 17);
            this.ZoneCheckBox.TabIndex = 36;
            this.ZoneCheckBox.Text = "Zone Config";
            this.ZoneCheckBox.UseVisualStyleBackColor = true;
            // 
            // ZonePointCheckBox
            // 
            this.ZonePointCheckBox.AutoSize = true;
            this.ZonePointCheckBox.Checked = true;
            this.ZonePointCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ZonePointCheckBox.Location = new System.Drawing.Point(702, 44);
            this.ZonePointCheckBox.Name = "ZonePointCheckBox";
            this.ZonePointCheckBox.Size = new System.Drawing.Size(83, 17);
            this.ZonePointCheckBox.TabIndex = 37;
            this.ZonePointCheckBox.Text = "Zone Points";
            this.ZonePointCheckBox.UseVisualStyleBackColor = true;
            // 
            // UpdateExistingNPCTypesCheckbox
            // 
            this.UpdateExistingNPCTypesCheckbox.AutoSize = true;
            this.UpdateExistingNPCTypesCheckbox.Location = new System.Drawing.Point(805, 44);
            this.UpdateExistingNPCTypesCheckbox.Name = "UpdateExistingNPCTypesCheckbox";
            this.UpdateExistingNPCTypesCheckbox.Size = new System.Drawing.Size(152, 17);
            this.UpdateExistingNPCTypesCheckbox.TabIndex = 38;
            this.UpdateExistingNPCTypesCheckbox.Text = "Update existing NPC types";
            this.UpdateExistingNPCTypesCheckbox.UseVisualStyleBackColor = true;
            this.UpdateExistingNPCTypesCheckbox.CheckedChanged += new System.EventHandler(this.UpdateExistingNPCTypesCheckbox_CheckedChanged);
            // 
            // NPCTypesTintCheckBox
            // 
            this.NPCTypesTintCheckBox.AutoSize = true;
            this.NPCTypesTintCheckBox.Location = new System.Drawing.Point(805, 69);
            this.NPCTypesTintCheckBox.Name = "NPCTypesTintCheckBox";
            this.NPCTypesTintCheckBox.Size = new System.Drawing.Size(117, 17);
            this.NPCTypesTintCheckBox.TabIndex = 39;
            this.NPCTypesTintCheckBox.Text = "Use npc_types_tint";
            this.toolTip1.SetToolTip(this.NPCTypesTintCheckBox, "Uses the npc_types_tint table instead of the tint columns in the npc_types table." +
                    " Experimental feature");
            this.NPCTypesTintCheckBox.UseVisualStyleBackColor = true;
            // 
            // EQExtractor2Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(960, 457);
            this.Controls.Add(this.NPCTypesTintCheckBox);
            this.Controls.Add(this.UpdateExistingNPCTypesCheckbox);
            this.Controls.Add(this.ZonePointCheckBox);
            this.Controls.Add(this.ZoneCheckBox);
            this.Controls.Add(this.DumpAAButton);
            this.Controls.Add(this.VersionLabel);
            this.Controls.Add(this.VersionSelector);
            this.Controls.Add(this.ProgressBar);
            this.Controls.Add(this.MerchantTextBox);
            this.Controls.Add(this.MerchantLabel);
            this.Controls.Add(this.MerchantCheckBox);
            this.Controls.Add(this.GroundSpawnCheckBox);
            this.Controls.Add(this.ObjectCheckBox);
            this.Controls.Add(this.GroundSpawnTextBox);
            this.Controls.Add(this.GroundSpawnLabel);
            this.Controls.Add(this.ObjectTextBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.GridTextBox);
            this.Controls.Add(this.GridLabel);
            this.Controls.Add(this.PacketDumpButton);
            this.Controls.Add(this.GenerateSQLButton);
            this.Controls.Add(this.Spawn2TextBox);
            this.Controls.Add(this.Spawn2Label);
            this.Controls.Add(this.SpawnEntryTextBox);
            this.Controls.Add(this.SpawnEntryLabel);
            this.Controls.Add(this.SpawnGroupTextBox);
            this.Controls.Add(this.SpawnGroupLabel);
            this.Controls.Add(this.DoorsTextBox);
            this.Controls.Add(this.DoorsLabel);
            this.Controls.Add(this.ZoneIDTextBox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.NPCTypesTextBox);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.GridCheckBox);
            this.Controls.Add(this.SpawnCheckBox);
            this.Controls.Add(this.DoorCheckBox);
            this.Controls.Add(this.IncludeLabel);
            this.Controls.Add(this.ProcessFileButton);
            this.Controls.Add(this.ConsoleWindow);
            this.Name = "EQExtractor2Form1";
            this.Text = "EQExtractor2";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.VersionSelector)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.OpenFileDialog InputFileOpenDialog;
        private System.Windows.Forms.ListBox ConsoleWindow;
        private System.Windows.Forms.Button ProcessFileButton;
        private System.Windows.Forms.Label IncludeLabel;
        private System.Windows.Forms.CheckBox DoorCheckBox;
        private System.Windows.Forms.CheckBox SpawnCheckBox;
        private System.Windows.Forms.CheckBox GridCheckBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.MaskedTextBox NPCTypesTextBox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.MaskedTextBox ZoneIDTextBox;
        private System.Windows.Forms.Label DoorsLabel;
        private System.Windows.Forms.MaskedTextBox DoorsTextBox;
        private System.Windows.Forms.Label SpawnGroupLabel;
        private System.Windows.Forms.MaskedTextBox SpawnGroupTextBox;
        private System.Windows.Forms.Label SpawnEntryLabel;
        private System.Windows.Forms.MaskedTextBox SpawnEntryTextBox;
        private System.Windows.Forms.Label Spawn2Label;
        private System.Windows.Forms.MaskedTextBox Spawn2TextBox;
        private System.Windows.Forms.Button GenerateSQLButton;
        private System.Windows.Forms.SaveFileDialog SQLFileDialog;
        private System.Windows.Forms.Button PacketDumpButton;
        private System.Windows.Forms.SaveFileDialog PacketDumpFileDialog;
        private System.Windows.Forms.Label GridLabel;
        private System.Windows.Forms.MaskedTextBox GridTextBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.MaskedTextBox ObjectTextBox;
        private System.Windows.Forms.MaskedTextBox GroundSpawnTextBox;
        private System.Windows.Forms.Label GroundSpawnLabel;
        private System.Windows.Forms.CheckBox ObjectCheckBox;
        private System.Windows.Forms.CheckBox GroundSpawnCheckBox;
        private System.Windows.Forms.CheckBox MerchantCheckBox;
        private System.Windows.Forms.Label MerchantLabel;
        private System.Windows.Forms.MaskedTextBox MerchantTextBox;
        private System.Windows.Forms.ProgressBar ProgressBar;
        private System.Windows.Forms.NumericUpDown VersionSelector;
        private System.Windows.Forms.Label VersionLabel;
        private System.Windows.Forms.Button DumpAAButton;
        private System.Windows.Forms.CheckBox ZoneCheckBox;
        private System.Windows.Forms.CheckBox ZonePointCheckBox;
        private System.Windows.Forms.CheckBox UpdateExistingNPCTypesCheckbox;
        private System.Windows.Forms.CheckBox NPCTypesTintCheckBox;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}

