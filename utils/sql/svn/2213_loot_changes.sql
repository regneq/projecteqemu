alter table loottable_entries add `droplimit` tinyint(2) unsigned NOT NULL default 0;
alter table loottable_entries add `mindrop` tinyint(2) unsigned NOT NULL default 0;
alter table lootdrop_entries change `chance` `chance` float not null default 1;
alter table lootdrop_entries add `multiplier` tinyint(2) unsigned NOT NULL default 1;

update lootdrop_entries lde 
inner join loottable_entries lte ON lte.lootdrop_id = lde.lootdrop_id
SET lde.chance = (lte.probability/100)*(lde.chance/100)*100;

update loottable_entries set mindrop = multiplier where probability = 100;
update loottable_entries set droplimit = multiplier where probability = 100;
update loottable_entries set multiplier = 1 where probability = 100;

update loottable_entries set droplimit = 1 where probability != 100;

update lootdrop_entries set chance = 100 where chance > 100;

alter table loottable_entries drop `probability`;