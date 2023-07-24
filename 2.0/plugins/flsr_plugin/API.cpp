#include "main.h"

namespace API {
	
	ushort iPort;

	bool LoadSettings()
	{
		iPort = 18080;

		return true;
	}


	void StartUp() { //API
        using json = nlohmann::json;


		ConPrint(L"API is started!\n");


		crow::SimpleApp app; //define your crow application

		//define your endpoint at the root directory
		CROW_ROUTE(app, "/")([]() {
			return "FL:SR API";
		});

        //Online players
        CROW_ROUTE(app, "/api/players")([]() {

            crow::response response;
            response.add_header("Content-Type", "application/json");

            json players;

            std::vector<json> playerList;

            {
                std::lock_guard<std::mutex> lock(m_Mutex);

                struct PlayerData* pPD = 0;

                while (pPD = Players.traverse_active(pPD)) {
                    int iRank = pPD->iRank;
                    uint iShipArch = pPD->iShipArchetype;
                    uint iClientID = HkGetClientIdFromPD(pPD);

                    std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
                    std::string scCharname = Discord::wstring_to_utf8(wscCharname);

                    HKPLAYERINFO pi;
                    HkGetPlayerInfo(wscCharname, pi, false);
                    auto ping = static_cast<int>(pi.ci.dwRoundTripLatencyMS);

                    json playerData;
                    playerData["charname"] = scCharname;
                    playerData["shipArch"] = std::to_string(iShipArch);
                    playerData["ping"] = std::to_string(ping);

                    playerList.push_back(playerData);
                }
            }

            players["players"] = playerList;

            response.write(players.dump());
            return response;
        });

        //News
        CROW_ROUTE(app, "/api/news")([]() {

            crow::response response;
            response.add_header("Content-Type", "application/json");

            json newsList;

            {
                std::lock_guard<std::mutex> lock(m_Mutex);

                for (const auto& message : Discord::lNewsList) {
                    json newsData;
                    newsData["author"] = Discord::GetDiscordUsername(message.Message.author);
                    newsData["content"] = message.Message.content;

                    newsList.push_back(newsData);
                }
            }

            json responseData;
            responseData["news"] = newsList;

            response.write(responseData.dump());
            return response;
        });

        //Events
        CROW_ROUTE(app, "/api/event")([]() {
             
            crow::response response;
            response.add_header("Content-Type", "application/json");

            json eventList;

            {
                std::lock_guard<std::mutex> lock(m_Mutex);

                for (const auto& message : Discord::lEventList) {
                    json eventdata;
                    eventdata["author"] = Discord::GetDiscordUsername(message.Message.author);
                    eventdata["content"] = message.Message.content;

                    eventList.push_back(eventdata);
                }
            }

            json responseData;
            responseData["events"] = eventList;

            response.write(responseData.dump());
            return response;
        });

        //Playerdata
        CROW_ROUTE(app, "/api/playerdata/<string>")([](const crow::request& req, std::string charname) {
            {
                std::lock_guard<std::mutex> lock(m_Mutex);

                //Get Charname
                std::string scCharname = charname;
                std::wstring wscCharname = Discord::Utf8ToWString(scCharname);

                //PlayerInfo
                HKPLAYERINFO pi;
                HK_ERROR error = HkGetPlayerInfo(wscCharname, pi, false);

                if (error != HKE_OK) {
                    return crow::response("Player not Online");
                }

                //Get Mounted Equipment
                int iRemHoldSize;
                std::list<CARGO_INFO> lstCargo;
                HkEnumCargo(wscCharname, lstCargo, iRemHoldSize);

                // Create JSON arrays for mounted equipment and ammo
                json mountedItems;
                json ammoItems;

                //Load Ressources
                HkLoadStringDLLs();

                // Iterate through the cargo list
                for (auto const& cargo : lstCargo) {
                    Archetype::Equipment const* eq = Archetype::GetEquipment(cargo.iArchID);
                    auto aType = eq->get_class_type();

                    if (aType == Archetype::MINE || aType == Archetype::MUNITION ||
                        aType == Archetype::COUNTER_MEASURE || aType == Archetype::SHIELD_BATTERY ||
                        aType == Archetype::REPAIR_KIT) {

                        json cargoItem;
                        cargoItem["iID"] = cargo.iID;
                        cargoItem["iCount"] = cargo.iCount;
                        cargoItem["iArchID"] = cargo.iArchID;
                        cargoItem["fStatus"] = cargo.fStatus;
                        cargoItem["hardpoint"] = cargo.hardpoint.value;
                        cargoItem["Name"] = wstos(HkGetWStringFromIDS(eq->iIdsName));
                        cargoItem["Type"] = Tools::typeToString(aType);                   

                        // Add the cargo item to the ammo array
                        ammoItems.push_back(cargoItem);
                    }
                    else {
                        json cargoItem;
                        cargoItem["iID"] = cargo.iID;
                        cargoItem["iCount"] = cargo.iCount;
                        cargoItem["iArchID"] = cargo.iArchID;
                        cargoItem["fStatus"] = cargo.fStatus;
                        cargoItem["hardpoint"] = cargo.hardpoint.value;
                        cargoItem["Name"] = wstos(HkGetWStringFromIDS(eq->iIdsName));
                        cargoItem["Type"] = Tools::typeToString(aType);

                        // Add the cargo item to the mounted equipment array
                        mountedItems.push_back(cargoItem);
                    }
                }

                // Create the final JSON output
                json playerData;
                playerData["charname"] = charname;
                playerData["mounted_items"] = mountedItems;
                playerData["ammo_items"] = ammoItems;

                crow::response response;
                response.add_header("Content-Type", "application/json");
                response.write(playerData.dump());
                return response;
            }
        });




        

		//set the port, set the app to run on multiple threads, and run the app
		app.port(iPort).multithreaded().run();		


	}

}