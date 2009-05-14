#Some Example NPC 
# Creating an instance and flagging for / zoning to it

sub EVENT_SAY {
	if($text=~/Hail/i) 
	{
		quest::say("Hail to you traveler");
	}
	if($text=~/Instance Status/i) 
	{
		my $i_id = quest::GetInstanceID("templeveeshan", 0);
		quest::say("Instance found: $i_id");
	}
	if($text=~/Instance Create/i) 
	{
		my $i_id = quest::CreateInstance("templeveeshan", 0, 7200);
		quest::AssignToInstance($i_id);
		quest::say("Instance create: $i_id");
	}
	if($text=~/Instance Zone/i) 
	{
		my $i_id = quest::GetInstanceID("templeveeshan", 0);
		if($i_id > 0)
		{
			quest::MovePCInstance(124, $i_id, -499, -2086, -36);
		}
		else
		{
			quest::say("I couldn't find anywhere to send you, sorry.");
		}
	}
	
	if($text=~/Assign Group/i) 
	{
		my $i_id = quest::GetInstanceID("templeveeshan", 0);
		if($i_id > 0)
		{
			quest::say("Assigning your group to instance $i_id.");
			quest::AssignGroupToInstance($i_id);
		}
	}
	
	if($text=~/Assign Raid/i) 
	{
		my $i_id = quest::GetInstanceID("templeveeshan", 0);
		if($i_id > 0)
		{
			quest::say("Assigning your raid to instance $i_id.");
			quest::AssignRaidToInstance($i_id);
		}
	}

	if($text=~/Assign Me Group/i) 
	{
		quest::FlagInstanceByGroupLeader(124, 0);
		quest::say("Assigning you to your group leader's id.");
	}
	
	if($text=~/Assign Me Raid/i) 
	{
		quest::FlagInstanceByRaidLeader(124, 0);
		quest::say("Assigning you to your raid leader's id.");
	}	
}