#include "Main.h"

namespace Hooks {


    //PopUpDialog
    void __stdcall PopUpDialog(unsigned int iClientID, unsigned int buttonClicked) {
        returncode = DEFAULT_RETURNCODE;

        //ConPrint(L"PopUpDialog %u %u\n", iClientID, buttonClicked);
        PopUp::HandleButtonClick(iClientID, buttonClicked);
    }

    //CharacterSelect
    void __stdcall CharacterSelect(struct CHARACTER_ID const &cId, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;
      
        //InfoCardUpdate
        ClientController::Send_ControlMsg(true, iClientID, L"_INFOCARDUPDATE CharacterSelect");

        //Update BaseState
        ClientController::Send_ControlMsg(false, iClientID, L"_ResetBaseState");       

        //NewPlayerMessage
        Tools::HkNewPlayerMessage(iClientID, cId);
    }

    // LaunchComplete
    void __stdcall LaunchComplete(unsigned int iBaseID, unsigned int iShip) {
        returncode = DEFAULT_RETURNCODE;

        //Get ClientID
        uint iClientID = HkGetClientIDByShip(iShip);
        if (!iClientID)
            return;

        //AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::DataGrab::CharnameToFLHOOKUSER_FILE(iClientID);
            AntiCheat::SpeedAC::Init(iClientID);
            AntiCheat::TimingAC::Init(iClientID);
            AntiCheat::PowerAC::Init(iClientID);
        }
		
        //Update InfoCards
        ClientController::Send_ControlMsg(true, iClientID, L"_INFOCARDUPDATE LaunchComplete");
        
        //Update BaseState
        ClientController::Send_ControlMsg(false, iClientID, L"_InSpaceState");

        //Show WelcomePopUp
        if (Modules::GetModuleState("WelcomeMSG"))
        {
            PopUp::WelcomeBox(iClientID);
        }
		
        //ProxyUndocking
        if (Modules::GetModuleState("CarrierModule"))
        {
            Docking::ClearCarrier(iClientID);
            Docking::HandleUndocking(iClientID);
        }

        //Test
        //int iWorth = (int)Tools::CalcDisabledHardpointWorth(iClientID);
        //PrintUserCmdText(iClientID, std::to_wstring(iWorth));
        //Depot::GetEquipname(iClientID);
    }

    // HkCb_AddDmgEntry
    void __stdcall HkCb_AddDmgEntry(DamageList *dmg, unsigned short p1, float damage, enum DamageEntry::SubObjFate fate) {
        returncode = DEFAULT_RETURNCODE;

        if (SpawnProtection::IsSpawnProtectionActive(iDmgTo))
        {
            //ConPrint(L"DMG WHILE PROTECTED\n");
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;
            return;
		}

        //NPCs always do damage
        if (!dmg->is_inflictor_a_player())
        {
            dmg->add_damage_entry(p1, damage, fate);
        }
        
       // ConPrint(std::to_wstring(fate) + L"\n");
         






        // Debug
        // PrintUserCmdText(iDmgTo, L"HIT");
    }

    void SendDeathMsg(const std::wstring& wscMsg, uint iSystemID, uint iClientIDVictim, uint iClientIDKiller) {
        returncode = DEFAULT_RETURNCODE;

        if (!Tools::isValidPlayer(iClientIDVictim, true))
            return;

        //ConPrint(wscMsg + L"\n");

        std::wstring victim, killer;
		Tools::eDeathTypes DeathType;
        // Extract victim and type from the death message
        if (wscMsg.find(L"was killed by an NPC") != std::wstring::npos) {
                    size_t victimStart = wscMsg.find(L"Death: ") + 7;
                    size_t victimEnd = wscMsg.find(L" was killed by an NPC");
                    victim = wscMsg.substr(victimStart, victimEnd - victimStart);
                    DeathType = Tools::PVE;

                    iClientIDKiller = 0;

        }
        else if (wscMsg.find(L"was killed by an admin") != std::wstring::npos) {
            size_t victimStart = wscMsg.find(L"Death: ") + 7;
            size_t victimEnd = wscMsg.find(L" was killed by an admin");
            victim = wscMsg.substr(victimStart, victimEnd - victimStart);
            DeathType = Tools::ADMIN;
            iClientIDKiller = 0;

        }
        else if (wscMsg.find(L" was killed by ") != std::wstring::npos) {
            size_t victimStart = wscMsg.find(L"Death: ") + 7;
            size_t victimEnd = wscMsg.find(L" was killed by ");
            victim = wscMsg.substr(victimStart, victimEnd - victimStart);

            size_t killerStart = victimEnd + 15; // length of " was killed by "
            size_t killerEnd = wscMsg.find(L" (", killerStart);
            killer = wscMsg.substr(killerStart, killerEnd - killerStart);

            size_t typeStart = killerEnd + 2; // skip the " ("
            size_t typeEnd = wscMsg.find(L")", typeStart);
            DeathType = Tools::PVP;
        }
        else if (wscMsg.find(L"killed himself") != std::wstring::npos) {
            size_t victimStart = wscMsg.find(L"Death: ") + 7;
            size_t victimEnd = wscMsg.find(L" killed himself");
            victim = wscMsg.substr(victimStart, victimEnd - victimStart);

            size_t typeStart = wscMsg.find_last_of(L"(") + 1;
            size_t typeEnd = wscMsg.find_last_of(L")");
            DeathType = Tools::KILLEDHIMSELF;

            iClientIDKiller = 0;
        }
        else if (wscMsg.find(L"committed suicide") != std::wstring::npos) {
            size_t victimStart = wscMsg.find(L"Death: ") + 7;
            size_t victimEnd = wscMsg.find(L" committed suicide");
            victim = wscMsg.substr(victimStart, victimEnd - victimStart);
            DeathType = Tools::SUICIDE;
            iClientIDKiller = 0;
        }
        else if (wscMsg.find(L" has died") != std::wstring::npos) {
            size_t victimStart = wscMsg.find(L"Death: ") + 7;
            size_t victimEnd = wscMsg.find(L" has died");
            victim = wscMsg.substr(victimStart, victimEnd - victimStart);
            DeathType = Tools::HASDIED;
            iClientIDKiller = 0;
        }


        // Remove whitespaces from victim
        victim.erase(std::remove_if(victim.begin(), victim.end(), [](unsigned char c) { return std::isspace(c); }), victim.end());
    
        // Remove whitespaces from killer
        killer.erase(std::remove_if(killer.begin(), killer.end(), [](unsigned char c) { return std::isspace(c); }), killer.end());

        // Get the victim's and killer's client IDs
        //uint victimClientID = HkGetClientIdFromCharname(victim);
       // uint killerClientID = HkGetClientIdFromCharname(killer);

		// Print the victim's and killer's client IDs for testing
		//ConPrint(L"Victim Client ID: " + std::to_wstring(victimClientID) + L"\n");
		//ConPrint(L"Killer Client ID: " + std::to_wstring(killerClientID) + L"\n");
        
        //PlayerHunt
        if (Modules::GetModuleState("PlayerHunt"))
        {
			PlayerHunt::CheckDied(iClientIDVictim, iClientIDKiller, DeathType);
		}
        
        //PVP
        if (Modules::GetModuleState("PVP"))
        {

            PVP::CheckDied(iClientIDVictim, iClientIDKiller, DeathType);

            //PVE Ranking
            if (DeathType == Tools::PVE)
            {
                PVP::UpdatePVERanking(iClientIDVictim, false);
            }
            //PVP Ranking
            else if (DeathType == Tools::PVP)
            {
                PVP::UpdatePVPRanking(iClientIDVictim, false);
                PVP::UpdatePVPRanking(iClientIDKiller, true);

            }


        }

        
    }



    
    //ShipDestroyed
    void __stdcall ShipDestroyed(DamageList* _dmg, DWORD* ecx, uint iKill) {
        returncode = DEFAULT_RETURNCODE;

        if (iKill != 1)
            return;

        DamageList dmg;
        try {
            dmg = *_dmg;
        }
        catch (...) {
            return;
        }


        CShip* cship = (CShip*)ecx[4];
        uint iClientID = cship->GetOwnerPlayer();

        if (Modules::GetModuleState("PVP"))
        {

            if (cship->is_player())
            {
                //Player is killed
                //std::wstring wscCharnameClient = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
                //ConPrint(wscCharnameClient + L" was killed\n");
            }
            else
            {
                //NPC is killed
                uint iKillerID = dmg.get_inflictor_owner_player();
                if (HkIsValidClientID(iKillerID))
                {
                    //NPC is killed by Player
                    //std::wstring wscCharnameKiller = (const wchar_t*)Players.GetActiveCharacterName(iKillerID);
                    //ConPrint(wscCharnameKiller + L" has Killed a NPC\n");
                    PVP::UpdatePVERanking(iKillerID, true);
                }
            }
        }

            if (iClientID) { // a player was killed
              

                if (iClientID != 0) {

                    // Insurance-PlayerDied

				    if (Modules::GetModuleState("InsuranceModule"))
				    {
					    //Insurance::PlayerDiedEvent(true, iClientID);
				    }
                }
			
                //EquipWhiteList
                if (Modules::GetModuleState("EquipWhiteListModule"))
                {
                    uint iShipArchIDPlayer;
                    pub::Player::GetShipID(iClientID, iShipArchIDPlayer);
                    EquipWhiteList::SendList(iShipArchIDPlayer, iClientID, false);
                }
            }
    }

    //BaseEnter_AFTER
    void __stdcall BaseEnter_AFTER(unsigned int iBaseID,unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;

        //Insurance
        if (Modules::GetModuleState("InsuranceModule"))
        {
            Insurance::UseInsurance(iClientID);
        }
		
		//AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::SpeedAC::Init(iClientID);
            AntiCheat::TimingAC::Init(iClientID);
        }	

        //PlayerHunt
        if (Modules::GetModuleState("PlayerHunt"))
        {
			PlayerHunt::CheckDock(iBaseID, iClientID);
        }

        //EquipWhiteList
        if (Modules::GetModuleState("EquipWhiteListModule"))
        {
            uint iShipArchIDPlayer;
            pub::Player::GetShipID(iClientID, iShipArchIDPlayer);
            EquipWhiteList::SendList(iShipArchIDPlayer, iClientID, false);
        }

        //PathSelection
        if (Modules::GetModuleState("PathSelection"))
        {
            PathSelection::Install_Unlawful(iClientID);
        }
        
        //Update BaseState
        ClientController::Send_ControlMsg(false, iClientID, L"_OnBaseState");
    }

	//SPObjUpdate
    void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;
        //AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::TimingAC::CheckTimeStamp(ui, iClientID);
            AntiCheat::SpeedAC::CheckSpeedCheat(ui, iClientID);
        }
    }

	//SubmitChat
    void __stdcall SubmitChat(CHAT_ID cId, unsigned long lP1, void const *rdlReader, CHAT_ID cIdTo, int iP2) {
        returncode = DEFAULT_RETURNCODE;

        ClientController::Handle_ClientControlMsg(cId, lP1, rdlReader, cIdTo, iP2);
    }

	//Dock_Call
    int __cdecl Dock_Call(unsigned int const& iShip, unsigned int const& iDockTarget, int iCancel, enum DOCK_HOST_RESPONSE response) {
        returncode = DEFAULT_RETURNCODE;
		
        uint iClientID = HkGetClientIDByShip(iShip);
        if (HkIsValidClientID(iClientID)) {
            //AC
            if (Modules::GetModuleState("ACModule"))
            {
                AntiCheat::SpeedAC::iDunno3(iShip, iDockTarget, iCancel, response);
            }

            //PathSelection
            if (Modules::GetModuleState("PathSelection"))
            {
				if (!PathSelection::Check_BlockedGate(iShip))
				{
					returncode = SKIPPLUGINS_NOFUNCTIONCALL;
				}
                
            }
        }

        return 0;
    }

    // Called when a gun hits something
    void __stdcall SPMunitionCollision(struct SSPMunitionCollisionInfo const& ci, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;

        //AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::SpeedAC::vDunno1(iClientID, 10000);
        }
    }

	//JumpInComplete
    void __stdcall JumpInComplete(unsigned int iSystemID, unsigned int iShipID) {
        returncode = DEFAULT_RETURNCODE;

        uint iClientID = HkGetClientIDByShip(iShipID);
        if (!iClientID)
            return;

        //AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::SpeedAC::Init(iClientID);
            AntiCheat::SpeedAC::vDunno2(iClientID);
            AntiCheat::SpeedAC::vDunno1(iClientID, 10000);
            AntiCheat::TimingAC::Init(iClientID);
            AntiCheat::PowerAC::Init(iClientID);
        }

        //PlayerHunt
        if (Modules::GetModuleState("PlayerHunt"))
        {
            PlayerHunt::CheckSystemReached(iClientID, iSystemID);
        }
    }
	
    void __stdcall SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;
        		
		//AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::SpeedAC::vDunno2(iClientID);
            AntiCheat::SpeedAC::vDunno1(iClientID, 20000);
        }

		//CarrierModul
		if (Modules::GetModuleState("CarrierModule"))
		{
            //ConPrint(L"SystemSwitchOutComplete\n");

            struct PlayerData* pd = 0;
            while (pd = Players.traverse_active(pd)) {
                 
				
             
                // �berpr�fe auf DockRequest
                std::list<Docking::UndockRelocate>::iterator iterRelocate = Docking::lUndockRelocate.begin();
                while (iterRelocate != Docking::lUndockRelocate.end()) {

                    if (pd->iOnlineID == iterRelocate->iClientID)
                    {
                        //ConPrint(L"PlayerData: %u\n", iterRelocate->iClientID);
                        
                        Docking::FLSR_SystemSwitchOutComplete(iterRelocate->iShip, iterRelocate->iClientID, iterRelocate->iSystem, iterRelocate->bStalkMode);

                       // PrintUserCmdText(iClientID, L"RELOCATE");
                        
						Vector pos = iterRelocate->pos;
						Matrix rot = iterRelocate->rot;
                        
                        


                        TranslateX(pos, rot, iterRelocate->fy_Undock);
                        TranslateY(pos, rot, iterRelocate->fz_Undock);
                        TranslateZ(pos, rot, iterRelocate->fx_Undock);


                        /*
                        PrintUserCmdText(iterRelocate->iClientID, L"XOffset: " + stows(std::to_string(iterRelocate->fx_Undock)));
                        PrintUserCmdText(iterRelocate->iClientID, L"YOffset: " + stows(std::to_string(iterRelocate->fy_Undock)));
                        PrintUserCmdText(iterRelocate->iClientID, L"ZOffset: " + stows(std::to_string(iterRelocate->fz_Undock)));
                        */
                        
                        HkRelocateClient(iterRelocate->iClientID, pos, rot);


                        pub::SpaceObj::SetInvincible(iterRelocate->iShip, false, false, 0);
                        
                        Docking::lUndockRelocate.erase(iterRelocate);
                    }


                    
                    iterRelocate++;
                }
                



            }
            

		}

        if (Modules::GetModuleState("SpawnProtection"))
        {
            SpawnProtection::UpdateLastSpawnTime(iClientID);
        }
        

    }

    /// Clear client info when a client connects.
    void __stdcall ClearClientInfo(unsigned int iClientID) {
		//AC
        if (Modules::GetModuleState("ACModule"))
        {
          AntiCheat::SpeedAC::Init(iClientID);
          AntiCheat::TimingAC::Init(iClientID);
          AntiCheat::PowerAC::Init(iClientID);

        }

        ClientController::Send_ControlMsg(true, iClientID, L"_INFOCARDUPDATE ClearClientInfo");
    }

    void __stdcall FireWeapon(unsigned int iClientID, struct XFireWeaponInfo const& wpn) {
        returncode = DEFAULT_RETURNCODE;
		
		//AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::PowerAC::FireWeapon(iClientID, wpn);
        }
    }

    void __stdcall PlayerLaunch_After(unsigned int iShip, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;

        //AC
        if (Modules::GetModuleState("ACModule"))
        {
            AntiCheat::SpeedAC::Init(iClientID);
            AntiCheat::TimingAC::Init(iClientID);
            AntiCheat::PowerAC::Init(iClientID);
        }
		
        //EquipWhiteList
        if (Modules::GetModuleState("EquipWhiteListModule"))
        {
            uint iShipArchIDPlayer;
            pub::Player::GetShipID(iClientID, iShipArchIDPlayer);
            EquipWhiteList::SendList(iShipArchIDPlayer, iClientID, false);        
        }

		//CarrierModule
        if (Modules::GetModuleState("CarrierModule"))
        {
            Commands::UserCMD_ENABLECARRIER(iClientID, L"");
            //ConPrint(L"Spawn\n");
        }

        //Insurance
        if (Modules::GetModuleState("InsuranceModule") && !Insurance::IsInsurancePresent(iClientID))
        {
            const bool insuranceRequested = Insurance::IsInsuranceRequested(iClientID);
            Insurance::CreateNewInsurance(iClientID, !insuranceRequested);
        }

        //SpawnProtection
        if (Modules::GetModuleState("SpawnProtection"))
        {
            SpawnProtection::UpdateLastSpawnTime(iClientID);
        }

        //Update BaseState
        ClientController::Send_ControlMsg(false, iClientID, L"_InSpaceState");
    }

    void __stdcall ReqAddItem(unsigned int goodID, char const* hardpoint, int count,float status, bool mounted, uint iClientID) {
        returncode = DEFAULT_RETURNCODE;
        
		//EquipWhiteList
        if (Modules::GetModuleState("EquipWhiteListModule"))
        {
            uint iShipArchIDPlayer;
            pub::Player::GetShipID(iClientID, iShipArchIDPlayer);
            EquipWhiteList::SendList(iShipArchIDPlayer, iClientID, false);
            if (EquipWhiteList::ReqAddItem_CheckEquipWhiteList(goodID, hardpoint, count, status, mounted, iClientID)) {
                //PrintUserCmdText(iClientID, L"Item not added");
                //pub::Audio::PlaySoundEffect(iClientID, CreateID("no_place_to_mount"));
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
            }
        }
    }

    void __stdcall ReqShipArch_AFTER(unsigned int iArchID, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;
		
        //EquipWhiteList
        if (Modules::GetModuleState("EquipWhiteListModule"))
        {
            EquipWhiteList::SendList(iArchID, iClientID, false);
        }
    }

    void __stdcall ReqEquipment(class EquipDescList const& edl, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;
		
        //EquipWhiteList
        if (Modules::GetModuleState("EquipWhiteListModule"))
        {
            uint iShipArchIDPlayer;
            pub::Player::GetShipID(iClientID, iShipArchIDPlayer);
            EquipWhiteList::SendList(iShipArchIDPlayer, iClientID, false);
        }
		
    }
	
    void __stdcall DisConnect(unsigned int iClientID, enum EFLConnection state)
    {
        returncode = DEFAULT_RETURNCODE;

        //PlayerHunt
        if (Modules::GetModuleState("PlayerHunt"))
        {
            PlayerHunt::CheckDisConnect(iClientID);
        }
        //PVP
        if (Modules::GetModuleState("PVP"))
        {
            //ConPrint (L"Disconnect: %u\n", iClientID);
            PVP::CheckDisConnect(iClientID, PVP::DisconnectReason::DISCONNECTED);
        }
    }
    
    void __stdcall CreateNewCharacter_After(struct SCreateCharacterInfo const& si, unsigned int iClientID) {
        returncode = DEFAULT_RETURNCODE;
        /*
        std::wstring wscCharname(si.wszCharname);
        ConPrint(L"CreateNewCharacter: %s", wscCharname);
        std::wstring wscFilename;
        HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename);
        ConPrint(L"NewCharfile: %s", wscFilename);
        */
        //SetLastNewChar to Account FlhookIni
    }
}