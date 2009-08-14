sub EVENT_SAY 
{ 
	if($text=~/mob/i)
	{
		my $mob_list = $entity_list->GetMobList();
		while($mob_list)
		{
			my $cur = $mob_list->GetData();
			if($cur)
			{
				$cur->Shout("I'm a mob!");
			}
			$mob_list = $mob_list->GetNext();
		}
	}
	
	if($text=~/client/i)
	{
		my $client_list = $entity_list->GetClientList();
		while($client_list)
		{
			my $cur = $client_list->GetData();
			if($cur)
			{
				$cur->Shout("I'm a client!");
			}
			$client_list = $client_list->GetNext();
		}
	}	
	
	if($text=~/npc/i)
	{
		my $npc_list = $entity_list->GetNPCList();
		while($npc_list)
		{
			my $cur = $npc_list->GetData();
			if($cur)
			{
				$cur->Shout("I'm a npc!");
			}
			$npc_list = $npc_list->GetNext();
		}
	}

	if($text=~/corpse/i)
	{
		my $corpse_list = $entity_list->GetCorpseList();
		my $index = 0;
		my $index_two = 0;
		while($corpse_list)
		{
			my $cur = $corpse_list->GetData();
			if($cur)
			{
				if($cur->GetOwnerName() eq $client->GetName())
				{
					$index_two++;
				}
				$index++;
			}
			$corpse_list = $corpse_list->GetNext();
		}
		quest::say("There are $index corpses in the zone and $index_two of them belong to you.");
	}
	
	if($text=~/summon/i)
	{
		my $corpse_list = $entity_list->GetCorpseList();
		while($corpse_list)
		{
			my $cur = $corpse_list->GetData();
			if($cur)
			{
				if($cur->GetOwnerName() eq $client->GetName())
				{
					$cur->Summon($client, 1773);
				}
			}
			$corpse_list = $corpse_list->GetNext();
		}
	}
	
	if($text=~/rez/i)
	{
		my $corpse_list = $entity_list->GetCorpseList();
		while($corpse_list)
		{
			my $cur = $corpse_list->GetData();
			if($cur)
			{
				if($cur->GetOwnerName() eq $client->GetName())
				{
					$cur->CastRezz(994, $npc);
					return;
				}
			}
			$corpse_list = $corpse_list->GetNext();
		}
	}	
}

sub EVENT_AGGRO_SAY
{
	if($text=~/hate/i)
	{
		my $hate_list = $npc->GetHateList();
		while($hate_list)
		{
			my $cur = $hate_list->GetData();
			if($cur)
			{
				my $h_ent = $cur->GetEnt();
				my $h_dmg = $cur->GetDamage();
				my $h_hate = $cur->GetHate();
				if($h_ent)
				{
					my $h_ent_name = $h_ent->GetName();
					quest::say("$h_ent_name is on my hate list with $h_hate and $h_dmg");
				}
			}
			$hate_list = $hate_list->GetNext();
		}
	}
}

