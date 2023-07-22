TestMap1_MapScripts::
	.byte 0

TestMap1_GivePokemon::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_STARTER, TestMap1_GivePokemon_2
	msgbox TestMap1_GivePokemon_Text_1
	setvar VAR_TEMP_1, SPECIES_MAGIKARP
	givecustommon SPECIES_MAGIKARP, 5, ITEM_NONE, 0, NUM_NATURES, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, MOVE_SPLASH, MOVE_BUBBLE, MOVE_TACKLE, MOVE_NONE, 0
	bufferspeciesname STR_VAR_1, SPECIES_MAGIKARP
	playfanfare MUS_OBTAIN_ITEM
	message TestMap1_GivePokemon_Text_2
	waitmessage
	waitfanfare
	setflag FLAG_RECEIVED_STARTER
	setflag FLAG_SYS_POKEMON_GET
	msgbox TestMap1_GivePokemon_Text_3, MSGBOX_YESNO
	compare VAR_RESULT, 1
	goto_if_eq TestMap1_GivePokemon_5
TestMap1_GivePokemon_1:
	releaseall
	return

TestMap1_GivePokemon_2:
	msgbox TestMap1_GivePokemon_Text_0
	goto TestMap1_GivePokemon_1

TestMap1_GivePokemon_5:
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto TestMap1_GivePokemon_1


TestMap1_GivePokemon_Text_0:
	.string "Good luck on your adventure!$"

TestMap1_GivePokemon_Text_1:
	.string "Here's a POKéMON!$"

TestMap1_GivePokemon_Text_2:
	.string "{PLAYER} received {STR_VAR_1}!$"

TestMap1_GivePokemon_Text_3:
	.string "Nickname {STR_VAR_1}?$"
