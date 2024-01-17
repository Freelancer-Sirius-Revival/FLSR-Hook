#include "Main.h"

namespace Commands {
    //Test Commands
    /*
    void UserCmd_TESTDEPOT(uint iClientID, const std::wstring& wscParam) {
        Depot::PlayerDepotOpen(iClientID);
        
    }

    void UserCmd_TESTAC(uint iClientID, const std::wstring &wscParam) {
        std::wstring wscType = wscParam;
        std::string scType = wstos(wscType);
        AntiCheat::Reporting::ReportCheater(iClientID, scType, "");
    }

    void UserCmd_TESTCC(uint iClientID, const std::wstring& wscParam) {
        std::wstring wscMsg = wscParam;
        ClientController::Send_ControlMsg(false, iClientID, wscMsg);
    }



    void UserCmd_TESTINSURANCE(uint iClientID, const std::wstring& wscParam) {
        Tools::GetHardpointsFromCollGroup(iClientID);
    }

    void UserCmd_TESTWP(uint iClientID, const std::wstring& wscParam) {
        std::wstring wscMsg = wscParam;
        std::list <CustomMissions::PlayerWaypoint> lWP;
        uint iSysID;
        pub::Player::GetSystem(iClientID, iSysID);
        CustomMissions::PlayerWaypoint TestWP;
		TestWP.iSystemID = iSysID;
        TestWP.iSolarObjectID = 0;
        TestWP.X = "-33356";
        TestWP.Y = "0";
        TestWP.Z = "-500";
        CustomMissions::PlayerWaypoint TestWP2;
        TestWP2.iSystemID = iSysID;
		TestWP2.iSolarObjectID = 0;
        TestWP2.X = "-12345";
        TestWP2.Y = "0";
        TestWP2.Z = "-500";
        CustomMissions::PlayerWaypoint TestWP3;
        TestWP3.iSystemID = iSysID;
		TestWP3.iSolarObjectID = 0;
        TestWP3.X = "-23333";
        TestWP3.Y = "0";
        TestWP3.Z = "-500";
        lWP.push_back(TestWP);
        lWP.push_back(TestWP2);
        lWP.push_back(TestWP3);
        CustomMissions::Send_WPs(iClientID, lWP, true);
    }
    
      void UserCmd_testcloak(uint iClientID, const std::wstring& wscParam) {
        ClientController::Send_ControlMsg(true, iClientID, L"_cloaktoggle");

    }
        */

    // Discord
    void UserCMD_BANK(uint iClientID, const std::wstring& wscParam) {
        if (Modules::GetModuleState("DiscordBot"))
        {
			std::wstring wscCharFileName;
			HkGetCharFileName(ARG_CLIENTID(iClientID), wscCharFileName);
			std::string scCharfile = wstos(wscCharFileName);
			std::string scDiscordID = Discord::GetDiscordIDForChar(scCharfile);
            if (scDiscordID == "") // No Link Request
            {
				PrintUserCmdText(iClientID, L"Char not linked!");
				return;
			}
            else
            {

                //Show Balance without Parameter
                if (wscParam == L"")
                {
                    //Get Current Balance
                    std::string scCredits = Discord::GetCreditsForDiscordAccount(scDiscordID);
                    PrintUserCmdText(iClientID, L"Current Balance: " + stows(scCredits));
                    PrintUserCmdText(iClientID, L"Use /bank <amount> to deposit credits");
                }
                else
                {
                    std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);


                    // Parameter �bergeben - Betrag auf Discord-Account �berweisen

                    // Ist der Char alt genug (Abuse-Prevention - 1h)?
                    int secs = 0;
                    HkGetOnlineTime(wscCharname, secs);
                    if (secs < 600) {
                        PrintUserCmdText(iClientID, L"ERR insufficient time online");
                        return;
                    }

                    int amount = -1;

                    // Betrag aus dem Parameter extrahieren und in einen int umwandeln
                    try
                    {
                        amount = std::stoi(wstos(wscParam));
                    }
                    catch (const std::exception&)
                    {
                        PrintUserCmdText(iClientID, L"Invalid amount specified.");
                        return;
                    }

                    // Betrag validieren (muss positiv sein)
                    if (amount <= 0)
                    {
                        PrintUserCmdText(iClientID, L"Invalid amount specified.");
                        return;
                    }

                    // Ist gen�gend Geld auf dem Charakter?
                    int iCash;
                    HkGetCash(wscCharname, iCash);
                    if (iCash < amount)
                    {
                        //Nicht gen�geng Geld
                        PrintUserCmdText(iClientID, L"You don't have enough credits.");
                        return;
                    }

                    
                    // The last error.
                    HK_ERROR err;

                    if ((err = HkAddCash(wscCharname, -amount)) != HKE_OK) {

                        PrintUserCmdText(iClientID, L"Error while updateing ingame balance");
                        return;
                    }

                    // Betrag auf den Discord-Account �berweisen
                    if (!Discord::UpdateCreditsForDiscordAccount(scDiscordID, std::to_string(amount), true))
                    {
						PrintUserCmdText(iClientID, L"Error while updating discord balance");
						return;
					}

                    // Erfolgmeldung anzeigen
                    PrintUserCmdText(iClientID, L"Successfully deposited " + wscParam + L" credits to your account.");
                }




			}
		}   


    }

    void UserCMD_LINK(uint iClientID, const std::wstring& wscParam) {

        if (Modules::GetModuleState("DiscordBot"))
        {

            std::wstring wscCharFileName;
            HkGetCharFileName(ARG_CLIENTID(iClientID), wscCharFileName);
            std::string scCharfile = wstos(wscCharFileName);
            std::string sha1PW = Tools::sha1(wstos(wscParam));
 

            //Get 2FA
            std::string scValid = Discord::GetValidationForChar(scCharfile);
            if (scValid == "") // No Link Request
            {
                PrintUserCmdText(iClientID, L"No link request!");
                return;
            }
            else if (scValid == "TRUE") // Already Linked
            {
				PrintUserCmdText(iClientID, L"Char already linked!");
				return;
			}


            if (scValid == sha1PW)
            {
                Discord::UpdateValidationForChar(scCharfile);
                PrintUserCmdText(iClientID, L"Char Linked!");
                std::string sscCharname = wstos((wchar_t*)Players.GetActiveCharacterName(iClientID));

                std::string scDiscordID = Discord::GetDiscordIDForChar(scCharfile);
                std::string scMsg = "Char " + sscCharname + " linked!";
                dpp::message dm(scMsg);
                Discord::DMMessage NewMessage;
                NewMessage.DiscordUserID = scDiscordID;
                NewMessage.DiscordMessage = dm;

                {
                    // Mutex sperren
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    Discord::lDMMessages.push_back(NewMessage);

                    Discord::CharManager_UpdateCharname(scCharfile, sscCharname);
                } // Mutex wird hier automatisch freigegeben
            }
            else
            {
				PrintUserCmdText(iClientID, L"Wrong Pass!");
			}

        }
    }

    ///// PVP


    void UserCmd_pvpduel(uint iClientID, const std::wstring& wscParam) {

        if (Modules::GetModuleState("PVP"))
        {
            PVP::CmdFight(iClientID, wscParam, PVP::PVPTYPE_DUEL);
        }
    }

    void UserCmd_pvpffa(uint iClientID, const std::wstring& wscParam) {

        if (Modules::GetModuleState("PVP"))
        {
            PVP::CmdFight(iClientID, wscParam, PVP::PVPTYPE_FFA);
        }
    }

    void UserCmd_pvpaccept(uint iClientID, const std::wstring& wscParam) {
        if (Modules::GetModuleState("PVP"))
        {
            PVP::CmdAcceptPVP(iClientID, wscParam);
        }

    }
    
    void UserCmd_pvpclear(uint iClientID, const std::wstring& wscParam) {
        if (Modules::GetModuleState("PVP"))
        {
            PVP::CmdClearPVP(iClientID, wscParam);
        }

    }

    void UserCmd_stats(uint iClientID, const std::wstring& wscParam) {
        if (Modules::GetModuleState("PVP"))
        {
            PVP::CmdStats(iClientID, wscParam);
        }

    }

    void UserCmd_pvpinvite(uint iClientID, const std::wstring& wscParam) {
        if (Modules::GetModuleState("PVP"))
        {
            // �berpr�fe ob Spieler im Space ist
            uint iShip;
            pub::Player::GetShip(iClientID, iShip);
            if (!iShip)
            {
                PrintUserCmdText(iClientID, L"Please undock!");
                return;
            }

            // �berpr�fe auf Target
            uint iTargetShip;
            pub::SpaceObj::GetTarget(iShip, iTargetShip);
            if (!iTargetShip)
            {
                PrintUserCmdText(iClientID, L"Please select a target!");
                return;
            }

            // �berpr�fe ob Target ein Spieler ist
            uint iTargetClientID = HkGetClientIDByShip(iTargetShip);
            if (!iTargetClientID)
            {
                PrintUserCmdText(iClientID, L"Please select a player!");
                return;
            }

            PVP::InvitePlayerToFFAFight(iClientID, iTargetClientID);

        }

    }
	

    void UserCmd_PLAYERHUNT(uint iClientID, const std::wstring& wscParam) {

        if (Modules::GetModuleState("PlayerHunt"))
        {
            PlayerHunt::Start_PlayerHunt(iClientID, wscParam);
        }
    }

    void UserCmd_HELP(uint iClientID, const std::wstring& wscParam) {

        
        //Create Popup struct
        PopUp::PopUpBox NewPopUpBox;
        NewPopUpBox.iClientID = iClientID;
        NewPopUpBox.iHead = 520006;
        NewPopUpBox.iBody = 520007;
        NewPopUpBox.iPage = 1;
        NewPopUpBox.iMaxPage = 1;
        NewPopUpBox.iButton = POPUPDIALOG_BUTTONS_CENTER_OK;
        
        std::wstring wscCharFileName;
        HkGetCharFileName(ARG_CLIENTID(iClientID), wscCharFileName);

        //Setup New Popup
		PopUp::mPopUpBox[wscCharFileName] = NewPopUpBox;
        
        //OpenPopup
		PopUp::OpenPopUp(iClientID);
        
    }

    void UserCmd_Tag(uint iClientID, const std::wstring& wscParam)
    {
        std::wstring wscError[] =
        {
            L"Error: Invalid parameters",
            L"Usage: /tag <faction name>"
        };     

		std::wstring wscParameters = wscParam;
        wscParameters = Trim(wscParameters);

        if (wscParameters.length())
        {
            wscParameters = ToLower(wscParameters);
            uint iGroup = Tools::GetiGroupOfFaction(wscParameters);
      
            
            
            if (iGroup)
            {
                int iRep;
                pub::Player::GetRep(iClientID, iRep);
                uint iIDS = Reputation::get_name(iGroup);
                std::wstring wscFaction = HkGetWStringFromIDS(iIDS);
                float fRep;
                pub::Reputation::GetGroupFeelingsTowards(iRep, iGroup, fRep);
                if (fRep < 0.6f)
                {
                    PrintUserCmdText(iClientID, L"Error: your reputation of %g to " + wscFaction + L" is less than the required %g.", fRep, 0.6f);
                }
                else
                {
                    pub::Reputation::SetAffiliation(iRep, iGroup);
                    PrintUserCmdText(iClientID, L"Affiliation changed to " + wscFaction);
                }
            }
            else
            {
                PrintUserCmdText(iClientID, L"Error: could not find faction");
            }
        }
        else
        {
            PrintUserCmdText(iClientID, L"Error: could not find faction");
        }
    }

    void UserCmd_UV(uint iClientID, const std::wstring &wscParam) {
        std::wstring Chat = wscParam;
        std::wstring Charname = (wchar_t *)Players.GetActiveCharacterName(iClientID);
        Chat::HkSendUChat(Charname, Chat);

        //Discord
        Discord::ChatMessage ChatMsg;
        ChatMsg.wscCharname = Charname;
        ChatMsg.wscChatMessage = Chat;


        {
            // Mutex sperren
            std::lock_guard<std::mutex> lock(m_Mutex);

            // Chat-Nachricht zur Liste hinzuf�gen
            Discord::lChatMessages.push_back(ChatMsg);
        } // Mutex wird hier automatisch freigegeben


    }

    void UserCmd_MODREQUEST(uint iClientID, const std::wstring &wscParam) {
        std::wstring Chat = wscParam;
        std::wstring Charname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
        PrintUserCmdText (iClientID, L"Your request has been sent to the moderators.");
        //Discord
        Discord::ChatMessage ChatMsg;
        ChatMsg.wscCharname = Charname;
        ChatMsg.wscChatMessage = Chat;


        {
            // Mutex sperren
            std::lock_guard<std::mutex> lock(m_Mutex);

            // Chat-Nachricht zur Liste hinzuf�gen
            Discord::lModMessages.push_back(ChatMsg);
        } // Mutex wird hier automatisch freigegeben
    }

    /** Process a give cash command */
    void UserCMD_SendCash(uint iClientID, const std::wstring &wscParam) {
        // The last error.
        HK_ERROR err;

        // Get the current character name
        std::wstring wscCharname =
            (const wchar_t *)Players.GetActiveCharacterName(iClientID);

        // Get the parameters from the user command.
        std::wstring wscTargetCharname = GetParam(wscParam, L' ', 0);
        std::wstring wscCash = GetParam(wscParam, L' ', 1);
        std::wstring wscAnon = GetParam(wscParam, L' ', 2);
        wscCash = ReplaceStr(wscCash, L".", L"");
        wscCash = ReplaceStr(wscCash, L",", L"");
        wscCash = ReplaceStr(wscCash, L"$", L"");
        int cash = ToInt(wscCash);
        if ((!wscTargetCharname.length() || cash <= 0) ||
            (wscAnon.size() && wscAnon != L"anon")) {
            PrintUserCmdText(iClientID, L"ERR Invalid parameters");
            PrintUserCmdText(iClientID, L"/sendcash <charname> <cash> [anon]");
            return;
        }

        bool bAnon = false;
        if (wscAnon == L"anon")
            bAnon = true;

        if (HkGetAccountByCharname(wscTargetCharname) == 0) {
            PrintUserCmdText(iClientID, L"ERR char does not exist");
            return;
        }

        int secs = 0;
        HkGetOnlineTime(wscCharname, secs);
        if (secs < SendCash::set_iMinTime) {
            PrintUserCmdText(iClientID, L"ERR insufficient time online");
            return;
        }

        // Read the current number of credits for the player
        // and check that the character has enough cash.
        int iCash = 0;
        if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
            return;
        }
        if (cash < SendCash::set_iMinTransfer || cash < 0) {
            PrintUserCmdText(iClientID,
                             L"ERR Transfer too small, minimum transfer " +
                                 ToMoneyStr(SendCash::set_iMinTransfer) +
                                 L" credits");
            return;
        }
        if (iCash < cash) {
            PrintUserCmdText(iClientID, L"ERR Insufficient credits");
            return;
        }

        // Prevent target ship from becoming corrupt.
        float fTargetValue = 0.0f;
        if ((err = HKGetShipValue(wscTargetCharname, fTargetValue)) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
            return;
        }
        if ((fTargetValue + cash) > 2000000000.0f) {
            PrintUserCmdText(iClientID, L"ERR Transfer will exceed credit limit");
            return;
        }

        // Calculate the new cash
        int iExpectedCash = 0;
        if ((err = HkGetCash(wscTargetCharname, iExpectedCash)) != HKE_OK) {
            PrintUserCmdText(iClientID,
                             L"ERR Get cash failed err=" + HkErrGetText(err));
            return;
        }
        iExpectedCash += cash;

        // Do an anticheat check on the receiving character first.
        uint targetClientId = HkGetClientIdFromCharname(wscTargetCharname);
        if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId)) {
            if (HkAntiCheat(targetClientId) != HKE_OK) {
                PrintUserCmdText(iClientID, L"ERR Transfer failed");
                AddLog(
                    "NOTICE: Possible cheating when sending %s credits from %s "
                    "(%s) "
                    "to %s (%s)",
                    wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname)))
                        .c_str(),
                    wstos(wscTargetCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                        .c_str());
                return;
            }
            HkSaveChar(targetClientId);
        }

        if (targetClientId != -1) {
            if (HkIsValidClientID(ClientInfo[iClientID].iTradePartner) ||
                HkIsValidClientID(ClientInfo[targetClientId].iTradePartner)) {
                PrintUserCmdText(iClientID, L"ERR Trade window open");
                AddLog(
                    "NOTICE: Trade window open when sending %s credits from %s "
                    "(%s) "
                    "to %s (%s) %u %u",
                    wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname)))
                        .c_str(),
                    wstos(wscTargetCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                        .c_str(),
                    iClientID, targetClientId);
                return;
            }
        }

        // Remove cash from current character and save it checking that the
        // save completes before allowing the cash to be added to the target ship.
        if ((err = HkAddCash(wscCharname, 0 - cash)) != HKE_OK) {
            PrintUserCmdText(iClientID,
                             L"ERR Remove cash failed err=" + HkErrGetText(err));
            return;
        }

        if (HkAntiCheat(iClientID) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR Transfer failed");
            AddLog(
                "NOTICE: Possible cheating when sending %s credits from %s (%s) to "
                "%s (%s)",
                wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname))).c_str(),
                wstos(wscTargetCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                    .c_str());
            return;
        }
        HkSaveChar(iClientID);

        // Add cash to target character
        if ((err = HkAddCash(wscTargetCharname, cash)) != HKE_OK) {
            PrintUserCmdText(iClientID,
                             L"ERR Add cash failed err=" + HkErrGetText(err));
            return;
        }

        targetClientId = HkGetClientIdFromCharname(wscTargetCharname);
        if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId)) {
            if (HkAntiCheat(targetClientId) != HKE_OK) {
                PrintUserCmdText(iClientID, L"ERR Transfer failed");
                AddLog(
                    "NOTICE: Possible cheating when sending %s credits from %s "
                    "(%s) "
                    "to %s (%s)",
                    wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname)))
                        .c_str(),
                    wstos(wscTargetCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                        .c_str());
                return;
            }
            HkSaveChar(targetClientId);
        }

        // Check that receiving character has the correct ammount of cash.
        int iCurrCash;
        if ((err = HkGetCash(wscTargetCharname, iCurrCash)) != HKE_OK ||
            iCurrCash != iExpectedCash) {
            AddLog(
                "ERROR: Cash transfer error when sending %s credits from %s (%s) "
                "to "
                "%s (%s) current %s credits expected %s credits ",
                wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname))).c_str(),
                wstos(wscTargetCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                    .c_str(),
                wstos(ToMoneyStr(iCurrCash)).c_str(),
                wstos(ToMoneyStr(iExpectedCash)).c_str());
            PrintUserCmdText(iClientID, L"ERR Transfer failed");
            return;
        }

        // If the target player is online then send them a message saying
        // telling them that they've received the cash.
        std::wstring msg = L"You have received " + ToMoneyStr(cash) +
                           L" credits from " +
                           ((bAnon) ? L"anonymous" : wscCharname);
        if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId)) {
            PrintUserCmdText(targetClientId, L"%s", msg.c_str());
        }
        // Otherwise we assume that the character is offline so we record an entry
        // in the character's givecash.ini. When they come online we inform them
        // of the transfer. The ini is cleared when ever the character logs in.
        else {
            std::wstring msg = L"You have received " + ToMoneyStr(cash) +
                               L" credits from " +
                               ((bAnon) ? L"anonymous" : wscCharname);
            SendCash::LogTransfer(wscTargetCharname, msg);
        }

        AddLog("Send %s credits from %s (%s) to %s (%s)",
               wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
               wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname))).c_str(),
               wstos(wscTargetCharname).c_str(),
               wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                   .c_str());

        // A friendly message explaining the transfer.
        msg = L"You have sent " + ToMoneyStr(cash) + L" credits to " +
              wscTargetCharname;
        if (bAnon)
            msg += L" anonymously";
        PrintUserCmdText(iClientID, L"%s", msg.c_str());
        return;
    }

    void UserCMD_SendCash$(uint iClientID, const std::wstring &wscParam) {
        // The last error.
        HK_ERROR err;

        // Get the current character name
        std::wstring wscCharname =
            (const wchar_t *)Players.GetActiveCharacterName(iClientID);

        // Get the parameters from the user command.

        std::wstring wscClientID = GetParam(wscParam, L' ', 0);
        uint iClientIDTarget = ToInt(wscClientID);
        if (!HkIsValidClientID(iClientIDTarget) ||
            HkIsInCharSelectMenu(iClientIDTarget)) {
            PrintUserCmdText(iClientID, L"Error: Invalid client-id");
            return;
        }

        std::wstring wscTargetCharname =
            (const wchar_t *)Players.GetActiveCharacterName(iClientIDTarget);
        std::wstring wscCash = GetParam(wscParam, L' ', 1);
        std::wstring wscAnon = GetParam(wscParam, L' ', 2);
        wscCash = ReplaceStr(wscCash, L".", L"");
        wscCash = ReplaceStr(wscCash, L",", L"");
        wscCash = ReplaceStr(wscCash, L"$", L"");
        int cash = ToInt(wscCash);
        if ((!wscTargetCharname.length() || cash <= 0) ||
            (wscAnon.size() && wscAnon != L"anon")) {
            PrintUserCmdText(iClientID, L"ERR Invalid parameters");
            PrintUserCmdText(iClientID, L"/sendcash$ <id> <cash> [anon]");
            return;
        }

        bool bAnon = false;
        if (wscAnon == L"anon")
            bAnon = true;

        if (HkGetAccountByCharname(wscTargetCharname) == 0) {
            PrintUserCmdText(iClientID, L"ERR char does not exist");
            return;
        }

        int secs = 0;
        HkGetOnlineTime(wscCharname, secs);
        if (secs < SendCash::set_iMinTime) {
            PrintUserCmdText(iClientID, L"ERR insufficient time online");
            return;
        }

        // Read the current number of credits for the player
        // and check that the character has enough cash.
        int iCash = 0;
        if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
            return;
        }
        if (cash < SendCash::set_iMinTransfer || cash < 0) {
            PrintUserCmdText(iClientID,
                             L"ERR Transfer too small, minimum transfer " +
                                 ToMoneyStr(SendCash::set_iMinTransfer) +
                                 L" credits");
            return;
        }
        if (iCash < cash) {
            PrintUserCmdText(iClientID, L"ERR Insufficient credits");
            return;
        }

        // Prevent target ship from becoming corrupt.
        float fTargetValue = 0.0f;
        if ((err = HKGetShipValue(wscTargetCharname, fTargetValue)) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
            return;
        }
        if ((fTargetValue + cash) > 2000000000.0f) {
            PrintUserCmdText(iClientID, L"ERR Transfer will exceed credit limit");
            return;
        }

        // Calculate the new cash
        int iExpectedCash = 0;
        if ((err = HkGetCash(wscTargetCharname, iExpectedCash)) != HKE_OK) {
            PrintUserCmdText(iClientID,
                             L"ERR Get cash failed err=" + HkErrGetText(err));
            return;
        }
        iExpectedCash += cash;

        // Do an anticheat check on the receiving character first.
        uint targetClientId = HkGetClientIdFromCharname(wscTargetCharname);
        if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId)) {
            if (HkAntiCheat(targetClientId) != HKE_OK) {
                PrintUserCmdText(iClientID, L"ERR Transfer failed");
                AddLog(
                    "NOTICE: Possible cheating when sending %s credits from %s "
                    "(%s) "
                    "to %s (%s)",
                    wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname)))
                        .c_str(),
                    wstos(wscTargetCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                        .c_str());
                return;
            }
            HkSaveChar(targetClientId);
        }

        if (targetClientId != -1) {
            if (HkIsValidClientID(ClientInfo[iClientID].iTradePartner) ||
                HkIsValidClientID(ClientInfo[targetClientId].iTradePartner)) {
                PrintUserCmdText(iClientID, L"ERR Trade window open");
                AddLog(
                    "NOTICE: Trade window open when sending %s credits from %s "
                    "(%s) "
                    "to %s (%s) %u %u",
                    wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname)))
                        .c_str(),
                    wstos(wscTargetCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                        .c_str(),
                    iClientID, targetClientId);
                return;
            }
        }

        // Remove cash from current character and save it checking that the
        // save completes before allowing the cash to be added to the target ship.
        if ((err = HkAddCash(wscCharname, 0 - cash)) != HKE_OK) {
            PrintUserCmdText(iClientID,
                             L"ERR Remove cash failed err=" + HkErrGetText(err));
            return;
        }

        if (HkAntiCheat(iClientID) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR Transfer failed");
            AddLog(
                "NOTICE: Possible cheating when sending %s credits from %s (%s) to "
                "%s (%s)",
                wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname))).c_str(),
                wstos(wscTargetCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                    .c_str());
            return;
        }
        HkSaveChar(iClientID);

        // Add cash to target character
        if ((err = HkAddCash(wscTargetCharname, cash)) != HKE_OK) {
            PrintUserCmdText(iClientID,
                             L"ERR Add cash failed err=" + HkErrGetText(err));
            return;
        }

        targetClientId = HkGetClientIdFromCharname(wscTargetCharname);
        if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId)) {
            if (HkAntiCheat(targetClientId) != HKE_OK) {
                PrintUserCmdText(iClientID, L"ERR Transfer failed");
                AddLog(
                    "NOTICE: Possible cheating when sending %s credits from %s "
                    "(%s) "
                    "to %s (%s)",
                    wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname)))
                        .c_str(),
                    wstos(wscTargetCharname).c_str(),
                    wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                        .c_str());
                return;
            }
            HkSaveChar(targetClientId);
        }

        // Check that receiving character has the correct ammount of cash.
        int iCurrCash;
        if ((err = HkGetCash(wscTargetCharname, iCurrCash)) != HKE_OK ||
            iCurrCash != iExpectedCash) {
            AddLog(
                "ERROR: Cash transfer error when sending %s credits from %s (%s) "
                "to "
                "%s (%s) current %s credits expected %s credits ",
                wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname))).c_str(),
                wstos(wscTargetCharname).c_str(),
                wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                    .c_str(),
                wstos(ToMoneyStr(iCurrCash)).c_str(),
                wstos(ToMoneyStr(iExpectedCash)).c_str());
            PrintUserCmdText(iClientID, L"ERR Transfer failed");
            return;
        }

        // If the target player is online then send them a message saying
        // telling them that they've received the cash.
        std::wstring msg = L"You have received " + ToMoneyStr(cash) +
                           L" credits from " +
                           ((bAnon) ? L"anonymous" : wscCharname);
        if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId)) {
            PrintUserCmdText(targetClientId, L"%s", msg.c_str());
        }
        // Otherwise we assume that the character is offline so we record an entry
        // in the character's givecash.ini. When they come online we inform them
        // of the transfer. The ini is cleared when ever the character logs in.
        else {
            std::wstring msg = L"You have received " + ToMoneyStr(cash) +
                               L" credits from " +
                               ((bAnon) ? L"anonymous" : wscCharname);
            SendCash::LogTransfer(wscTargetCharname, msg);
        }

        AddLog("Send %s credits from %s (%s) to %s (%s)",
               wstos(ToMoneyStr(cash)).c_str(), wstos(wscCharname).c_str(),
               wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname))).c_str(),
               wstos(wscTargetCharname).c_str(),
               wstos(HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)))
                   .c_str());

        // A friendly message explaining the transfer.
        msg = L"You have sent " + ToMoneyStr(cash) + L" credits to " +
              wscTargetCharname;
        if (bAnon)
            msg += L" anonymously";
        PrintUserCmdText(iClientID, L"%s", msg.c_str());
        return;
    }

    // Contributor TextBox
    void UserCMD_Contributor(uint iClientID, const std::wstring &wscParam) {
        FmtStr caption(0, 0);
        caption.begin_mad_lib(PopUp::iContributor_Head);
        caption.end_mad_lib();

        FmtStr message(0, 0);
        message.begin_mad_lib(PopUp::iContributor_Body);
        message.end_mad_lib();

        pub::Player::PopUpDialog(iClientID, caption, message, POPUPDIALOG_BUTTONS_CENTER_OK);
        return;
    }

    //Not used commands

    /*
    
        // MissionGroup Bug
    void UserCMD_Clear(uint iClientID, const std::wstring &wscParam) {

        Tools::HkClearMissionBug(iClientID);
        return;
    }

    */
   
	
	//ChangeModuleState
    void AdminCmd_SwitchModuleState(CCmds* cmds, std::wstring wscModulename) {
		
		// Rechte Check
		if (!(cmds->rights & RIGHT_SUPERADMIN)) {
			cmds->Print(L"ERR No permission\n");
			return;
		}
		
		//Switch Module State
        Modules::SwitchModuleState(wstos(wscModulename));
        cmds->Print(L"OK\n");
    }

    USERCMD UserCmds[] = {
        {L"/uv", UserCmd_UV},
        {L"/modrequest", UserCmd_MODREQUEST},
        {L"/sendcash", UserCMD_SendCash},
        {L"/sendcash$", UserCMD_SendCash$},
        {L"/contributor", UserCMD_Contributor},
        {L"/autoinsurance", Insurance::UserCMD_INSURANCE},
        {L"/help", UserCmd_HELP},
        {L"/tag", UserCmd_Tag},
        {L"/playerhunt", UserCmd_PLAYERHUNT},
        {L"/pvpduel", UserCmd_pvpduel},
        {L"/pvpffa", UserCmd_pvpffa},
        {L"/pvpaccept", UserCmd_pvpaccept},
        {L"/pvpclear", UserCmd_pvpclear},
        {L"/pvpinvite", UserCmd_pvpinvite},
        {L"/stats", UserCmd_stats},
        {L"/link", UserCMD_LINK},
        {L"/bank", UserCMD_BANK},
        {L"/cloak", Cloak::UserCmd_CLOAK},
        {L"/c", Cloak::UserCmd_CLOAK},
        {L"/uncloak", Cloak::UserCmd_UNCLOAK},
        {L"/uc", Cloak::UserCmd_UNCLOAK},
        {L"/dock", Docking::UserCmd_Dock},
        {L"/mark", Mark::UserCmd_Mark},
        {L"/m", Mark::UserCmd_Mark},
        {L"/groupmark", Mark::UserCmd_GroupMark},
        {L"/gm", Mark::UserCmd_GroupMark},
        {L"/unmark", Mark::UserCmd_UnMark},
        {L"/um", Mark::UserCmd_UnMark},
        {L"/groupunmark", Mark::UserCmd_UnGroupMark},
        {L"/gum", Mark::UserCmd_UnGroupMark},
        {L"/unmarkall", Mark::UserCmd_UnMarkAll},
        {L"/uma", Mark::UserCmd_UnMarkAll}



        //Not Used Commands
        /*
        
         {L"/clear", UserCMD_Clear},

        */

		//Test Commands
        /*{L"/testcloak", UserCmd_testcloak},
        {L"/testac", UserCmd_TESTAC},
        {L"/testcc", UserCmd_TESTCC},
        {L"/testdepot", UserCmd_TESTDEPOT},
        {L"/testins", UserCmd_TESTINSURANCE},
        */

    };

    // User command processing
    bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
        std::wstring wscCmdLower = ToLower(wscCmd);
        for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++) {
            if (wscCmdLower.find(ToLower(UserCmds[i].wszCmd)) == 0) {
                std::wstring wscParam = L"";
                if (wscCmd.length() > wcslen(UserCmds[i].wszCmd)) {
                    if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
                        continue;
                    wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
                }
                UserCmds[i].proc(iClientID, wscParam);
                returncode = SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command,
                                                         // return immediatly
                return true;
            }
        }
        returncode = DEFAULT_RETURNCODE; // we did not handle the command, so let
                                         // other plugins or FLHook kick in
        return false;
    }

    // Admin command processing
    bool ExecuteCommandString_Callback(CCmds *cmds, const std::wstring &wscCmd) {
        returncode = DEFAULT_RETURNCODE;
        if (IS_CMD("switchModulestate")) {
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;
            AdminCmd_SwitchModuleState(cmds, cmds->ArgStr(1));
            return true;
        }
        return false;
    }
}