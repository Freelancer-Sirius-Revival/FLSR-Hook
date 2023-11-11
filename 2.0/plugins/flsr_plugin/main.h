﻿#ifndef __MAIN_H__
#define __MAIN_H__ 1

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

//Disable Warnings
#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class

//DISCORD Includes
#include <dpp/dpp.h>

//SQL Includes
#include <SQLiteCpp/SQLiteCpp.h>

//FLHOOK Includes
#include <FLHook.h>
#include <plugin.h>

//Includes
#include <list>
#include <string>
#include <Windows.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <numeric>
#include <unordered_map>
#include <openssl/sha.h>
#include <crow.h>

//Plugin Stuff
extern PLUGIN_RETURNCODE returncode;

//Offsets
#define ADDR_CLIENT_NEWPLAYER 0x8010
#define ADDR_CRCANTICHEAT 0x6FAF0

//-Static
#define DISCORD_WEBHOOK_CHEATREPORT_FILE "C:\\Freelancer\\FLSR Public\\EXE\\flhook_plugins\\cheatreport.exe"
#define CHEATREPORT_STORE "C:\\Caddy\\files\\cheater\\"

// Mutex-Objekt deklarieren
extern std::mutex m_Mutex;

//AntiCheat
typedef void(__stdcall *_CRCAntiCheat)();
extern _CRCAntiCheat CRCAntiCheat_FLSR;

//Namespaces

namespace Globals {

    //FilePaths
    const std::string PLUGIN_CONFIG_FILE = "\\flhook_plugins\\flsr.cfg";
    const std::string DOCK_CONFIG_FILE = "\\flhook_plugins\\flsr-dock.cfg";
    const std::string PATHSELECTION_CONFIG_FILE = "\\flhook_plugins\\flsr-pathselection.cfg";
    const std::string CARRIER_CONFIG_FILE = "\\flhook_plugins\\FLSR-Carrier.cfg";
    const std::string CLOAK_CONFIG_FILE = "\\flhook_plugins\\FLSR-Cloak.cfg";
    const std::string CRAFTING_CONFIG_FILE = "\\flhook_plugins\\FLSR-Crafting.cfg";
    const std::string FLHOOKUSER_FILE = "\\flhookuser.ini";
    const std::string SENDCASHLOG_FILE = "-givecashlog.txt";
    const std::string INSURANCE_STORE = "\\flhook_plugins\\flsr-insurance\\";
    const std::string LIBRELANCER_SDK = "\\flhook_plugins\\librelancer-sdk\\";
    const std::string CMP_DUMP_FOLDER = "\\flhook_plugins\\flsr-cmpdumps\\";
    const std::string MISSION_STORE = "\\flhook_plugins\\missions\\";
    const std::string Equip_WHITELIST_FILE = "\\flhook_plugins\\FLSR-EquipWhiteList.cfg";
    const std::string AC_REPORT_TPL = "\\flhook_plugins\\flsr-cheater\\ReportTemplate.html";
    const std::string PVP_FIGHTINFO = "\\flhook_plugins\\FightInfo.cfg";
    const std::string DATADIR = "..\\DATA";

    //SQL 
    enum SQLOpenFlags {
        OPEN_READONLY = 0x00000001,
        OPEN_READWRITE = 0x00000002,
        OPEN_CREATE = 0x00000004,
        OPEN_DELETEONCLOSE = 0x00000008,
        OPEN_EXCLUSIVE = 0x00000010,
        OPEN_AUTOPROXY = 0x00000020,
        OPEN_URI = 0x00000040,
        OPEN_MEMORY = 0x00000080,
        OPEN_MAIN_DB = 0x00000100,
        OPEN_TEMP_DB = 0x00000200,
        OPEN_TRANSIENT_DB = 0x00000400,
        OPEN_MAIN_JOURNAL = 0x00000800,
        OPEN_TEMP_JOURNAL = 0x00001000,
        OPEN_SUBJOURNAL = 0x00002000,
        OPEN_SUPER_JOURNAL = 0x00004000,
        OPEN_NOMUTEX = 0x00008000,
        OPEN_FULLMUTEX = 0x00010000,
        OPEN_SHAREDCACHE = 0x00020000,
        OPEN_PRIVATECACHE = 0x00040000,
        OPEN_WAL = 0x00080000,
        OPEN_NOFOLLOW = 0x01000000,
        OPEN_EXRESCODE = 0x02000000
    };


}

namespace Modules {
    struct Module {
        std::string scModuleName;
        bool bModulestate;
    };

    void LoadModules();
    bool GetModuleState(std::string scModuleName);
    bool SetModuleState(const std::string& scModuleName, bool bModuleState);
    void SwitchModuleState(std::string scModuleName);
	
    extern std::list<Module> lModules;
}

namespace Timers {
	
    typedef void (*_TimerFunc)();

    struct TIMER {
        _TimerFunc proc;
        mstime tmIntervallMS;
        mstime tmLastCall;
    };

    int __stdcall Update();

}

namespace Commands {
    void AdminCmd_Stalk(CCmds *cmds, std::wstring Charname = L"");
    void CmdHelp_Callback(CCmds *classptr);
    bool ExecuteCommandString_Callback(CCmds *cmds, const std::wstring &wscCmd);
    void UserCMD_Clear(uint iClientID, const std::wstring &wscParam);
    void UserCMD_Contributor(uint iClientID, const std::wstring &wscParam);
    void UserCMD_DOCKACCEPT(uint iClientID, const std::wstring &wscParam);
    void UserCMD_DOCKREQUEST(uint iClientID, const std::wstring &wscParam);
    void UserCMD_ENABLECARRIER(uint iClientID, const std::wstring &wscParam);
    void UserCmd_MODREQUEST(uint iClientID, const std::wstring &wscParam);
    bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd);
    void UserCMD_SendCash$(uint iClientID, const std::wstring &wscParam);
    void UserCMD_SendCash(uint iClientID, const std::wstring &wscParam);
    void UserCmd_UV(uint iClientID, const std::wstring &wscParam);
    void UserCmd_CLOAK(uint iClientID, const std::wstring& wscParam);
    void UserCmd_UNCLOAK(uint iClientID, const std::wstring& wscParam);
    void UserCmd_HELP(uint iClientID, const std::wstring& wscParam);
    void UserCmd_Tag(uint iClientID, const std::wstring& wscParam);
    void UserCmd_PLAYERHUNT(uint iClientID, const std::wstring& wscParam);
    
    typedef void (*_UserCmdProc)(uint, const std::wstring &);
    struct USERCMD {
        const  wchar_t* wszCmd;
        _UserCmdProc proc;
    };

    #define IS_CMD(a) !wscCmd.compare(L##a)
}

namespace SendCash {
    static int set_iMinTime = 0;
    static int set_iMinTransfer = 0;

    void LogTransfer(std::wstring wscToCharname, std::wstring msg);
}

namespace Chat {
    HK_ERROR HkSendUChat(std::wstring wscCharname, std::wstring wscText);
}

namespace PopUp {
    // Contributor
    extern uint iContributor_Head;
    extern uint iContributor_Body;

    // Welcome Message - First Char on ID
    extern uint iWMsg_Head;
    extern uint iWMsg_Body;

    void WelcomeBox(uint iClientID);
    void OpenPopUp(uint iClientID);
    void HandleButtonClick(uint iClientID, uint buttonClicked);

    enum PopUpType {
		PopUpType_Help
	};

    //POPUPBox with Handle
    struct PopUpBox {
        uint iClientID;
        uint iHead;
        uint iBody;
        uint iPage;
		uint iMaxPage;
        uint iButton;
		PopUpType iType;
    };
	
    extern std::map<std::wstring, PopUpBox> mPopUpBox;
    //extern std::list<PopUpBox> lPOPUPBox;
}


namespace Tools {

    enum eDeathTypes {
        PVP,
        SUICIDE,
        PVE,
        KILLEDHIMSELF,
        ADMIN,
        HASDIED
    };

    struct HashMap {
        std::string scNickname;
        uint iResID;
    };

	struct CMPDump_Exception {
		std::string scData;
	};

    struct CMPDump_Entry {
        bool bisCollGroup;
		std::string scData;
        bool bhasParent;
		std::string scParent;
        
    };
    
    struct ParentMap {
		std::string scParent;
		std::string scFirstParent;
	};
   




    extern std::list <CMPDump_Exception> lCMPUpdateExceptions;
    void HkNewPlayerMessage(uint iClientID, struct CHARACTER_ID const &cId);
    void HkClearMissionBug(int clientID);
    extern std::wstring CS_wscCharBefore;
    extern std::string base64_chars;
    extern std::map<uint, HashMap> mNicknameHashMap;
    static inline bool is_base64(unsigned char c);
    std::string base64_encode(unsigned char const *bytes_to_encode, unsigned int in_len);
    std::string base64_decode(std::string const &encoded_string);
    std::vector<std::string> HkGetCollisionGroups(uint iClientID, bool bOnly);
    bool startsWith(std::string_view str, std::string_view prefix);
    bool endsWith(std::string_view str, std::string_view suffix);
    std::vector<std::string> GetHardpointsFromCollGroup(uint iClientID);
    float CalcDisabledHardpointWorth(uint iClientID);
    void replace_first(std::string &s, std::string const &toReplace,std::string const &replaceWith);
    std::string StringBetween(std::string str, std::string first, std::string last);
    bool GetB(std::string svalue);
    static void ReadIniNicknameFile(const std::string& filePath);
    bool ReadIniNicknames();
    Matrix Rz(float angleDeg);
    HK_ERROR FLSR_HkFLIniGet(const std::wstring& wscCharname, const std::wstring& wscKey, std::wstring& wscRet);
    HK_ERROR FLSR_HkFLIniWrite(const std::wstring & wscCharname, const std::wstring & wscKey, const std::wstring & wscValue);
    bool IsPlayerInRange(uint iClientID, uint iClientID2, float fDistance);
    void get_cmpfiles(const std::filesystem::path& path);
    void get_cmpExceptions();
    std::vector<std::string> getHardpoints(std::string scParent, std::list<CMPDump_Entry> CMPList);

    bool isValidPlayer(uint iClientID, bool bCharfile);
    void CharSelectMenu();

    //Reputation Stuff
    //Reputation callback struct
    struct RepCB
    {
        uint iGroup;
        uint iNameLen;
        char szName[16];
    };
    typedef bool(__stdcall* _RepCallback)(RepCB* rep);

    extern std::list<RepCB> lstTagFactions;

    bool __stdcall RepCallback(RepCB* rep);
    std::list<RepCB> HkGetFactions();
    bool __stdcall RepEnumCallback(RepCB* rep);
    void HkEnumFactions(_RepCallback callback);
    uint GetiGroupOfFaction(std::wstring wscParam);


    //Shortest Path

    // Definition for a graph node
    struct Node {
        std::string system;
        int distance;
        std::string previous;

        bool operator>(const Node& other) const {
            return distance > other.distance;
        }
    };

    // Definition for a graph edge
    struct Edge {
        std::string start;
        std::string end;
        int distance;
    };
    
    std::vector<std::string> FindShortestPath(const std::vector<Edge>& edges, const std::string& start, const std::string& end);
    void ParsePathsFromFile(std::vector<Edge>& edges);
    std::vector<std::string> GetShortestPath(std::string start, std::string end);
    int CountShortestPath(std::string start, std::string end);

    void FLSRIniDelete(const std::string& scFile, const std::string& scApp, const std::string& scKey);
    std::wstring ToUpper(const std::wstring& str);
    HK_ERROR FLSRHkAddEquip(const std::wstring& wscCharname, uint iGoodID,const std::string& scHardpoint, bool bMounted);
    HK_ERROR FLSRHkAddCargo(const std::wstring& wscCharname, uint iGoodID, int iCount, bool bMission);
    float GetAveragePingOfAllPlayers();
    std::string sha1(const std::string& input);
    std::string typeToString(Archetype::AClassType aType);
 
}

namespace Docking {
   
    

    struct UndockRelocate {

		bool bStalkMode;
        float fx_Undock;
        float fy_Undock;
        float fz_Undock;
        Vector pos;
        Matrix rot;
        uint iClientID;
		uint iShip;
		uint iSystem;
        
    };
 


    struct CarrierList {

        uint iCarrierID;
        uint iDockedPlayers;
        uint iBaseDocked;
        uint iSlots;
        std::string Interior;
        float fx_Undock;
        float fy_Undock;
        float fz_Undock;

        CarrierList() {
            iCarrierID = 0;
            iDockedPlayers = 0;
            iBaseDocked = 0;
            iSlots = 0;
            Interior = "";
            fx_Undock = 0.0f;
            fy_Undock = 0.0f;
            fz_Undock = 0.0f;
        }

		CarrierList(uint iCarrierID, uint iDockedPlayers, uint iBaseDocked, uint iSlots, std::string Interior, float fx_Undock, float fy_Undock, float fz_Undock) {
            this->iCarrierID = iCarrierID;
            this->iDockedPlayers = iDockedPlayers;
            this->iBaseDocked = iBaseDocked;
            this->iSlots = iSlots;
            this->Interior = Interior;
            this->fx_Undock = fx_Undock;
            this->fy_Undock = fy_Undock;
            this->fz_Undock = fz_Undock;
        }
    };

    struct CarrierDockedPlayers {

        uint iCarrierID;
        uint iPlayerID;
        float fx_Undock;
        float fy_Undock;
        float fz_Undock;

        CarrierDockedPlayers() {
            iCarrierID = 0;
            iPlayerID = 0;
            fx_Undock = 0.0f;
            fy_Undock = 0.0f;
            fz_Undock = 0.0f;
        }

		CarrierDockedPlayers(uint iCarrierID, uint iPlayerID, float fx_Undock, float fy_Undock, float fz_Undock) {
            this->iCarrierID = iCarrierID;
            this->iPlayerID = iPlayerID;
            this->fx_Undock = fx_Undock;
            this->fy_Undock = fy_Undock;
            this->fz_Undock = fz_Undock;
        }
    };

    struct CarrierDockRequest {

        uint iCarrierID;
        uint iPlayerID;
        mstime tmRequestTime;
        uint iBaseCarrierDocked;
        std::string sInterior;
        bool bSend;
        float fx_Undock;
        float fy_Undock;
        float fz_Undock;

        CarrierDockRequest() {
            iCarrierID = 0;
            iPlayerID = 0;
            tmRequestTime = 0;
            iBaseCarrierDocked = 0;
            sInterior = "test";
            bSend = false;
            fx_Undock = 0.0f;
            fy_Undock = 0.0f;
            fz_Undock = 0.0f;
        }

        CarrierDockRequest(uint iCarrierID, uint iPlayerID, mstime tmRequestTime, bool bSend, uint iBaseCarrierDocked, std::string sInterior, float fx_Undock, float fy_Undock, float fz_Undock) {
            this->iCarrierID = iCarrierID;
            this->iPlayerID = iPlayerID;
            this->tmRequestTime = tmRequestTime;
            this->iBaseCarrierDocked = iBaseCarrierDocked;
            this->sInterior = sInterior;
            this->bSend = bSend;
            this->fx_Undock = fx_Undock;
            this->fy_Undock = fy_Undock;
            this->fz_Undock = fz_Undock;
        }
    };

    struct CarrierConfig {

        uint iShipArch;
        uint iSlots;
        std::string sInterior;
        float fx_Undock;
        float fy_Undock;
        float fz_Undock;


        CarrierConfig() {
            iShipArch = 0;
            iSlots = 0;
            sInterior = "";
            fx_Undock = 0.0f;
            fy_Undock = 0.0f;
            fz_Undock = 0.0f;
        }

		CarrierConfig(uint iShipArch, uint iSlots, std::string sInterior, float fx_Undock, float fy_Undock, float fz_Undock) {
            this->iShipArch = iShipArch;
            this->iSlots = iSlots;
            this->sInterior = sInterior;
			this->fx_Undock = fx_Undock;
			this->fy_Undock = fy_Undock;
			this->fz_Undock = fz_Undock;
        }



    };

    // Carrier - Lists
    extern std::list<CarrierList> lCarrierList;
    extern std::list<CarrierDockedPlayers> lCarrierDockedPlayers;
    extern std::list<CarrierDockRequest> lCarrierDockRequest;
    extern std::list<CarrierConfig> lCarrierConfig;
    extern std::list<UndockRelocate> lUndockRelocate;


    // Carrier - Timeout
    extern mstime msRequestTimeout;

    // Carrier - Dockrange
    extern float fDockRange;


    void ClearCarrier(uint iClientID);
    void HandleUndocking(uint iClientID);
    void DockRequest3000ms();
    bool FLSR_SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID, unsigned int iSystem, bool bstalkmode = false);
    void DockOnProxyCarrierBase(std::string scBasename, uint iClientID, std::string scCarrierBase, uint iCarrierID);
    void UndockProxyBase(uint iCarrierId, uint iClientID, float fx_Undock, float fy_Undock, float fz_Undock, bool bstalkmode = false);
    void DOCKACCEPT_ALL(uint iClientID);
}

namespace Hooks {
    void __stdcall PopUpDialog(unsigned int iClientID, unsigned int buttonClicked);
    void __stdcall CharacterSelect(struct CHARACTER_ID const &cId, unsigned int iClientID);
    void __stdcall LaunchComplete(unsigned int iBaseID, unsigned int iShip);
    void __stdcall HkCb_AddDmgEntry(DamageList *dmg, unsigned short p1, float damage, enum DamageEntry::SubObjFate fate);
    void __stdcall ShipDestroyed(DamageList *_dmg, DWORD *ecx, uint iKill);
    void __stdcall BaseEnter_AFTER(unsigned int iBaseID, unsigned int iClientID);
    void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui, unsigned int iClientID);
    void __stdcall SubmitChat(CHAT_ID cId, unsigned long lP1, void const *rdlReader, CHAT_ID cIdTo, int iP2);
    int __cdecl Dock_Call(unsigned int const& iShip, unsigned int const& iDockTarget, int iCancel, enum DOCK_HOST_RESPONSE response);
	void __stdcall SPMunitionCollision(struct SSPMunitionCollisionInfo const& ci, unsigned int iClientID);
    void __stdcall JumpInComplete(unsigned int iSystemID, unsigned int iShipID);
	void __stdcall SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID);
    void __stdcall ClearClientInfo(unsigned int iClientID);
    void __stdcall FireWeapon(unsigned int iClientID, struct XFireWeaponInfo const& wpn);
    void __stdcall PlayerLaunch_After(unsigned int iShip, unsigned int iClientID);
    void __stdcall ReqAddItem(unsigned int goodID, char const* hardpoint, int count, float status, bool mounted, uint iClientID);
    void __stdcall ReqShipArch_AFTER(unsigned int iArchID, unsigned int iClientID);
	void __stdcall ReqEquipment(class EquipDescList const& edl, unsigned int iClientID);
	void __stdcall GoTradelane(unsigned int iClientID, struct XGoTradelane const& gtl);
    void __stdcall DisConnect(unsigned int iClientID, enum EFLConnection state);
    void __stdcall CreateNewCharacter_After(struct SCreateCharacterInfo const& si, unsigned int iClientID);
    void SendDeathMsg(const std::wstring& wscMsg, uint iSystemID, uint iClientIDVictim, uint iClientIDKiller);
    }

namespace ClientController {
    void Send_ControlMsg(bool sHook, uint iClientID, std::wstring wscText, ...);
    void Handle_ClientControlMsg(CHAT_ID cId, unsigned long lP1, void const* rdlReader, CHAT_ID cIdTo, int iP2);
}

namespace Insurance
{
    template<typename T>
    constexpr inline T operator~ (T a) { return static_cast<T>(~static_cast<typename std::underlying_type<T>::type>(a)); }

    template<typename T>
    constexpr inline T operator| (T a, T b) { return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(a) | static_cast<typename std::underlying_type<T>::type>(b)); }

    template<typename T>
    constexpr inline T operator& (T a, T b) { return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(a) & static_cast<typename std::underlying_type<T>::type>(b)); }

    template<typename T>
    constexpr inline T operator^ (T a, T b) { return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(a) ^ static_cast<typename std::underlying_type<T>::type>(b)); }

    template<typename T>
    constexpr inline T& operator|= (T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<typename std::underlying_type<T>::type&>(a) |= static_cast<typename std::underlying_type<T>::type>(b)); }

    template<typename T>
    constexpr inline T& operator&= (T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<typename std::underlying_type<T>::type&>(a) &= static_cast<typename std::underlying_type<T>::type>(b)); }

    template<typename T>
    constexpr inline T& operator^= (T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<typename std::underlying_type<T>::type&>(a) ^= static_cast<typename std::underlying_type<T>::type>(b)); }

    extern float insuranceEquipmentCostFactor;

    void CreateNewInsurance(uint clientId, bool onlyFreeItems);
    void UseInsurance(uint clientId);
    bool IsInsuranceRequested(uint clientId);
    bool IsInsurancePresent(uint clientId);
    void UserCMD_INSURANCE(uint clientId, const std::wstring& argumentsWS);
}

namespace AntiCheat {

    namespace TimingAC {    
        void Init(unsigned int iClientID);
        void CheckTimeStamp(struct SSPObjUpdateInfo const &pObjInfo, unsigned int iClientID);       
    }
        
    namespace SpeedAC {
		void Init(uint iClientID);
        void UpdateShipSpeed(uint iClientID);
        float GetPlayerAllowedSpeed(uint iClientID, enum ENGINE_STATE state);
        bool CheckClientSpeed(uint iClientID, std::vector<float>& vecTimes, std::vector<float>& vecDistances, enum ENGINE_STATE engineState);
        void vDunno1(uint iClientID, mstime delay);
        void vDunno2(uint iClientID);
        int iDunno3(unsigned int const& iShip, unsigned int const& iDockTarget, int iCancel, enum DOCK_HOST_RESPONSE response);
        void CheckSpeedCheat(struct SSPObjUpdateInfo const& pObjInfo, unsigned int iClientID);
    }

    namespace PowerAC {
        void Init(unsigned int iClientID);
        void Setup(unsigned int iClientID);
        void FireWeapon(unsigned int iClientID, struct XFireWeaponInfo const wpn);
    }
 
    namespace Reporting {
        void ReportCheater(uint iClientID, std::string scType, std::string sData);
        std::string CreateReport(uint iClientID, std::wstring wscType,std::wstring wscTime, std::wstring wscDETAILS);
    }
    
    namespace DataGrab {
        void CharnameToFLHOOKUSER_FILE(uint iClientID);
    }

    struct AC_INFO {
        //Timing Detecion
        float dClientTimestamp;
        float dServerTimestamp;
        int iRateDetectCountTimingSpeed;
        bool bTimingSpeedCheater = false;
        //Speed Detection
        float fAllowedCruiseSpeed;
        float fAllowedThrusterSpeed; 
        float fAllowedEngineSpeed;
        float fAllowedTradelaneSpeed;
        float fAllowedStrafeSpeed;
        int iRateDetectSpeed;
        std::map<enum ENGINE_STATE, std::vector<float>> vecDistances;
        std::map<enum ENGINE_STATE, std::vector<float>> vecTimes;
        int iSpeedDetections;
        enum ENGINE_STATE engineState;
        mstime tmSpeedExceptionTimeout;
        double fLastSpeedTimestamp;
        Vector vLastPos;
        mstime tmCheckTime;
		bool bSpeedCheater = false;
		//Power Detection
        bool bSetupPowerCheatDet;
        float fMaxCapacity;
        float fChargeRate;
        float fCurEstCapacity;
        mstime tmLastPowerUpdate;
        float fMinCapacity;
        bool bPowerCheater = false;

    };

    extern IMPORT AC_INFO AC_Info[MAX_CLIENT_ID + 1];
}


namespace CustomMissions {
    struct PlayerWaypoint {
        std::string X;                              // Coord X (Client convert it to float)
        std::string Y;                              // Coord Y (Client convert it to float)
        std::string Z;                              // Coord Z (Client convert it to float)
        uint iSystemID;                             // ID of the WP-System
        uint iSolarObjectID;                        // ID of the selected Object | needed for BestPath, SystemSwitch
    };

    struct CustomMission {
		// Mission Data
        int iMissionID;                             // Mission ID
        std::string scMissionFilepath;              // Mission Filepath
        std::string scMissionName;                  // Mission Name
        std::string scMissionDesc;                  // Mission Description
        std::string scMissionType;                  // Mission Type
        bool bSPMission;                            // Single Player Mission
        bool bMPMission;                            // Multi Player Mission

		// Mission Reward
        bool bRewardCredits;                        // Reward Credits
        int iRewardCredits;                         // Reward Credits Amount
        bool bRewardReputation;                     // Reward Reputation
        int iRewardReputation;                      // Reward Reputation Amount
        std::string scRewardReputationNickname;     // Reward Reputation Nickname
        bool bRewardShip;                           // Reward Ship
        std::string scRewardShipNickname;           // Reward Ship Nickname
        bool bRewardEquip;                           // Reward Equipment
        std::string scRewardEquipNickname;           // Reward Equipment Nickname
        int iRewardEquip;                            // Reward Equipment Amount

		// Trade Mission
        std::string scTargetBaseNickname;           // Target Base Nickname
        std::string scGoodToTradeNickname;          // Good To Trade Nickname
        std::string iAmountToTrade;                 // Amount To Trade

		// Kill NPC Mission
        std::string scNPCType;                      // NPC Type
        int iAmountofNPCs;                          // Amount of NPCs
        int iAmountofWaves;                         // Amount of Waves
        bool bRewardKill;                           // Reward Kill
        bool bRewardGroup;                          // Reward Group
        bool bKillNamedNPC;                         // Kill Named NPC
        std::string scNamedNPCName;                 // Named NPC Name

		// Player Hunt Mission
        std::wstring wscPlayerCharname;             // Player Charname
        int Bounty;                                 // Bounty
        
        // Mining Mision
        std::string scGoodToMineNickname;           // Good To Mine Nickname
        int iAmountToMine;                          // Amount To Mine	

		// MissionWaypoints
        bool bSendMissionWaypoints;                 // Send MissionWaypoints to Player
        bool bSendMissionWaypointsToGroup;          // Send MissionWaypoints to Group
        std::list<PlayerWaypoint> lPlayerWaypoints; // List of Waypoints

        //POPUPs
        // MissionStart POPUP
        bool bMissionStartPopup;                    //Show a Popup on Mission start
		int iMissionStartPopupHead;                 //The ResID of the Popup Head
		int iMissionStartPopupBody;                 //The ResID of the Popup Body
        //MissionEnd POPUP
		bool bMissionEndPopup;                      //Show a Popup on Mission end
		int iMissionEndPopupHead;                   //The ResID of the Popup Head
		int iMissionEndPopupBody;                   //The ResID of the Popup Body

        //MISSIONTEXTs
        //MissonStart Text
		bool bMissionStartText;                     //Show a Text on Mission start
		int iMissionStartText;                      //The ResID of the Text
        //MissionEnd Text
		bool bMissionEndText;                       //Show a Text on Mission end
		int iMissionEndText;                        //The ResID of the Text



    };
	
    extern std::list<CustomMission> lCustomMission;

    void LoadMissions();
    void Send_WPs(uint iClientID, std::list <CustomMissions::PlayerWaypoint> lWP, bool bBestPath);

}

namespace Cloak
{
    void ClearClientData(uint clientId, bool storeCloakCapacity);
    void InstallCloak(uint clientId);
    void LoadCloakSettings();
    void InitializeWithGameData();
    bool CheckDockCall(uint ship, uint dockTargetId, uint dockPortIndex, enum DOCK_HOST_RESPONSE response);
    bool IsFullyUncloaked(uint clientId);
    void AttemptInitialUncloak(uint clientId);
    void QueueUncloak(uint clientId);
    void SynchronizeCloakedClients();
    void UserCmd_CLOAK(uint clientId, const std::wstring& wscParam);
    void UserCmd_UNCLOAK(uint clientId, const std::wstring& wscParam);
    void UserCmd_CLOAK_TIME(uint clientId, const std::wstring& wscParam);

    struct CloakDeviceInfo
    {
        uint archetypeId;
        float capacity;
		float powerUsage;
		float powerRecharge;
        float minRequiredCapacityToCloak;
        bool shieldDown;
        int cloakEffectDuration;
        int uncloakEffectDuration;
    };

    enum CloakState
    {
        Uncloaked,
        Cloaking,
        Cloaked,
        Uncloaking
    };

    struct ClientCloakingDevice
    {
        int cargoId = 0;
        CloakState cloakState = CloakState::Cloaked;
        bool initialUncloakRequired = true;
        mstime cloakTimeStamp = 0;
		mstime uncloakTimeStamp = 0;
        mstime lastTimingInfoTimeStamp = 0;
        float capacity = 0.0f;
        bool insideNoCloakZone = false;
        CloakDeviceInfo cloakData;
    };
}

namespace Crafting
{
    void LoadSettings();
    bool UserCmd_Craft(uint clientId, const std::wstring& argumentsWS);
}

namespace EquipWhiteList {

	struct EquipWhiteListEntry {
		std::string scEquipNickname;
		uint iEquipID;
		std::vector<std::pair<uint, std::string>> vShip; // ShipID, ShipNickname
	};

    extern std::list<EquipWhiteListEntry> lEquipWhiteList;

    void LoadEquipWhiteList();
    bool ReqAddItem_CheckEquipWhiteList(unsigned int goodID, char const* hardpoint, int count, float status, bool mounted, uint iClientID);
    void SendList(uint iShipArch, uint iClientID, bool oldShip);
}

namespace SQL {
    

    extern std::string scDbName;
    void InitializeDB();
    void CheckAndCreateDuelRankingTable();
    void CheckAndCreateFFARankingTable();
    void CheckAndCreatePVPRankingTable();
    void CheckAndCreatePVERankingTable();

}

namespace Depot {

    struct PlayerDepot {
        uint iDepotID;
        uint iBaseID;
        std::string scAccountName;
        uint iCapacity;		
    };

	struct PlayerDepotItem {
		uint iDepotID;
		uint iGoodID;
		uint iAmount;
        uint iIDSName;
	};

    struct PlayerCargoItem {
        uint iGoodID;
        uint iAmount;
        uint iIDSName;
    };

    bool LoadDepotData();
    std::string GetEquipNicknameFromID(uint goodID);
    std::list<PlayerDepotItem> GetEquipFromBaseDepot(uint iClientID, bool bPrint);
    void PlayerDepotOpen(uint iClientID);
    void GetPlayerEquip(uint iClientID);

    extern std::list<PlayerDepot> lPlayerDepot;	
}


namespace PathSelection {
   
    struct Reputation{
		std::string scFactionName;
		float fReputation;
    };
    
    struct BlockedGate {
       uint iGateID;
    };

    struct UnlawfulPlayer {
        bool bisUnlawful;
        bool bisCharModified;
    };
    
	struct OpenUnlawfulMod {
		uint iClientID;
        std::string scCharname;
        std::wstring scAccountID;
        std::wstring wscAccDir;
    };

    extern std::string scStart_Base;
    extern std::string scSystem;
    extern uint iCash;
    
    extern std::list<PathSelection::OpenUnlawfulMod> lOpenUnlawfulMods;
    extern std::list<PathSelection::Reputation> lReputations;
    extern std::list<PathSelection::BlockedGate> lBlockedGates;
    extern IMPORT UnlawfulPlayer UnlawfulPlayerData[MAX_CLIENT_ID + 1];
    //extern std::map<std::wstring, UnlawfulPlayer> mUnlawfulPlayer;


    void LoadPathSelectionSettings();
    bool Check_BlockedGate(uint iShip);
    void Install_Unlawful(uint iClientID);
    void SetUnlawful(uint iClientID, std::string scCharname, std::string scState);
    void ModUnlawfulChar500ms();

}

namespace FuseControl {

    bool ReadIniFuseConfig();
    static void ReadIniFuseConfigFile(const std::string& filePath, bool bFuseIni);
    
    enum FuseType
    {
        FUSE_UNKNOWN,
        FUSE_SHIP,
        FUSE_SOLAR,
        FUSE_COLLGRP

    };
    
    struct Fuse {

        FuseType eType;
        std::string scShipNickname;
        std::string scNickname;
        float fLifeTime;
        float fHitpoint;
    };
    
    struct ClientFuse {
        Fuse oldFuse;
		Fuse newFuse;       
    };
    
    extern IMPORT ClientFuse FuseControl[MAX_CLIENT_ID + 1];
    extern std::map<uint, Fuse> mFuseMap;
}


namespace PlayerHunt {

    extern std::vector<std::string> SystemWhitelist;

	enum HuntState {
		HUNT_STATE_NONE,
        HUNT_STATE_LOST,
		HUNT_STATE_HUNTING
	};

	struct ServerHuntInfo {
		std::wstring wscCharname;
		uint iTargetBase;
        uint iTargetSystem;
		std::wstring wscTargetSystem;
		std::wstring wscTargetBase;
		HuntState eState;
        uint iReward;
        uint iHuntCredits;
        std::list<std::string> lSystems;
	};

	struct LastPlayerHuntWinners {
		std::wstring wscCharname;
		uint iBaseID;
        uint Credits;
	};
    
	struct BaseData {
		uint iBaseID;
		uint iSystemID;
        std::string scBaseNickname;

	};
    
    extern float set_fRewardMultiplicator;
	extern int set_iMinTargetSystemDistance;
    extern int set_iMinCredits;
    extern int set_iMaxCredits;
    extern int set_iMinPlayer;
    
	extern std::list <LastPlayerHuntWinners> lLastPlayerHuntWinners;
	extern ServerHuntInfo ServerHuntData;
    
    uint getRandomSysteminRange(uint iClientID);
    BaseData getRandomBaseInSystem(uint iSystemID, uint iClientID);
    BaseData getTargetBase(uint iClientID);
    void Start_PlayerHunt(uint iClientID, const std::wstring& wscParam);
    void PlayerHuntMulti(uint iClientID, uint iPlayerSystemID);
    void CalcReward();
    void CheckSystemReached(uint iClientID, uint iPlayerSystemID);
    void CheckDock(uint iBaseID, uint iClientID);
    void CheckDisConnect(uint iClientID);
    bool IsInSameFight(uint iClientID, uint iClientID2);
    void CheckDied(uint iClientID, uint iClientKillerID, Tools::eDeathTypes DeathType);
    void LoadPlayerHuntSettings();
}


namespace PVP {



    enum PVPType {
        PVPTYPE_NONE,
        PVPTYPE_DUEL,
        PVPTYPE_FFA,
        PVPTYPE_RANKED

    };

    enum DisconnectReason {
        CHARSWITCH,
        DISCONNECTED
    };

    struct Member {
        uint iClientID;
        std::wstring wscCharname;
        std::wstring wscCharFilename;
        uint iKills;
        bool bIsInFight = false;

    };

    static uint nextFightID = 1;

    struct Fights {
        uint iFightID;
        std::list<Member> lMembers;
        PVPType ePVPType;
        uint iFights;
        uint iFightsRemaining;

        Fights() {
            iFightID = nextFightID;
            ++nextFightID;
        }
    };

    extern std::list<Fights> ServerFightData;

    void LoadPVP();
    std::wstring GetWStringFromPVPTYPE(PVPType pvpType);
    uint IsInFight(uint iClientID, bool bSkipFightCheck);
    uint GetKills(uint iClientID);
    PVPType GetPVPType(uint iClientID);
    uint GetFightIDByCharname(const std::wstring& wscCharname);
    PVPType GetActiveFightPVPType(uint iFightID);
    std::list<Member> GetPVPMember(uint iClientID);
    void HandleKill(uint iClientKillerID, PVPType ePVPType);
    void CmdAcceptPVP(uint iClientID, const std::wstring& wscParam);
    void CmdFight(uint iClientID, const std::wstring& wscParam, PVP::PVPType ePVPType);
    void AcceptFight(PVP::Fights& fight, uint iClientID);
    void CheckDisConnect(uint iClientID, DisconnectReason reason);
    void CheckDied(uint iClientID, uint iClientKillerID, Tools::eDeathTypes DeathType);
    void UpdateDuelRanking(uint iClientID, bool bKills);
    void UpdateFFARanking(uint iClientID, bool bKills);
    void CalcRanking(const std::string& tableName);
    void CheckLastRound();
    void WriteFightInfoToCFG(uint iFightID, const std::string& scCharFilename);
    bool IsCharInFightInfo(const std::string& scCharFilename);
    void ClearFightInfo();
    void RemoveCharFromFightInfo(const std::string& scCharFilename);
    void CmdClearPVP(uint iClientID, const std::wstring& wscParam);
    void AddPlayerToFFAFight(uint iFightID, const std::wstring& wscCharname, const std::wstring& wscCharFilename);
    void InvitePlayerToFFAFight(uint iClientID, uint iTargetClientID);
    void RemovePlayerFromFight(uint iFightID, const std::wstring& wscCharname);
    void UpdatePVERanking(uint iClientID, bool bKills);
    void UpdatePVPRanking(uint iClientID, bool bKills);
    void CmdStats(uint iClientID, const std::wstring& wscParam);

}

namespace Discord {
    struct ChatMessage {
        std::wstring wscCharname;
        std::wstring wscChatMessage;

    };

    struct DMMessage {
        std::string DiscordUserID;
        dpp::message DiscordMessage;

    };

    struct LastSelectClick {
        dpp::user User;
        dpp::select_click_t event;

        LastSelectClick(const dpp::user& user, const dpp::select_click_t& clickEvent)
            : User(user), event(clickEvent)
        {
        }
    };

    struct MessageListEntry {
        std::string Nickname;
        dpp::message Message;

    };


    struct DiscordUser {
        std::string scServerUsername;
        std::string scDiscordUsername;
        std::string scDiscordDisplayName;
        std::string scDiscordID;
    };


    //extern
    extern std::list<ChatMessage> lChatMessages;
    extern std::list<ChatMessage> lModMessages;
    extern std::list<DMMessage> lDMMessages;
    extern std::list <LastSelectClick> lLastSelectClick;
    extern std::list<MessageListEntry> lNewsList;
    extern std::list<MessageListEntry> lEventList;
    extern std::map<std::string, DiscordUser> userDataMap;
    extern int iOnlinePlayers;

    //Konfig
    extern std::string scDiscordBotToken;
	extern std::string scDiscordServerID;
    extern std::string scUVChatChannelID;
    extern std::string scModRequestChannelID;
    extern std::string scModGroupID;
    extern std::string scNewsChannelID;
    extern std::string scEventChannelID;


    extern int iRenameCost;

    bool LoadSettings();
    void StartUp();
    void CommandUptime(const dpp::slashcommand_t& event);
    std::string wstring_to_utf8(const std::wstring& wstr);
    std::wstring Utf8ToWString(const std::string& utf8Str);
    std::string GeneratePassword();

    //Pages
    void CharManagerPageMenu(dpp::cluster& DiscordBot, const dpp::button_click_t& event);

    //Embeds
    template<typename T>
    void BankEmbed(const T& event);

    //Modals
    template<typename T>
    void LinkModal(const T& event);
    template<typename T>
    void CharRenameModal(const T& event);
    template<typename T>
    void BankTransferModal(const T& event, const std::string modal_id);
    template<typename T>
    void GetServerstatus(const T& event);


    //Helper
    std::string GetFormComponentValue(const dpp::form_submit_t& event, const std::string& customId);

    //Charmanager Stuff
    std::string CharManager_Insert(const std::string& charfile, const std::string& discordID, const std::string& password);
    void CharManager_DeleteInvalidEntries();
    std::string GetValidationForChar(const std::string& charfile);
    void UpdateValidationForChar(const std::string& charfile);
    std::string GetDiscordIDForChar(const std::string& charfile);
    bool IsCharacterLinkedWithDiscordID(const std::string& discordID, const std::string& charfile);
    bool CharManager_Rename(const std::string& oldcharfile, const std::string& newcharfile);
    bool CharManager_UpdateCharname(const std::string& charfile, const std::string& charname);
    std::string GetCreditsForDiscordAccount(const std::string& discordAccount);
    bool UpdateCreditsForDiscordAccount(const std::string& discordAccount, const std::string& credits, bool bAdd);
    bool DoesDiscordAccountHaveValidChars(const std::string& discordAccount);
    std::string GetUserIDByDiscordName(const std::string& discordName);
    void Update_NewsList(dpp::cluster &DiscordBot);
    void Update_EventList(dpp::cluster &DiscordBot);

    std::string GetDiscordUsername(const dpp::user& dppUser);
    bool containsWhitespace(const std::string& str);

} // namespace DiscordBot


namespace SpawnProtection {

    extern std::map<uint, mstime> g_lastSpawnTimes;
    extern uint g_spawnProtectionDuration;
    bool LoadSettings();
    void SetLastSpawnTime(uint iClientID, mstime spawnTimestamp);
    mstime GetLastSpawnTime(uint iClientID);
    void UpdateLastSpawnTime(uint iClientID);;
    bool IsSpawnProtectionActive(uint iClientID);



} // namespace SpawnProtection

namespace GroupReputation
{
    void InitializeWithGameData();
    void BaseDestroyed(uint objectId, uint clientId);
    void __stdcall ShipDestroyed(DamageList* dmg, DWORD* ecx, uint killed);
    void CleanDestroyedShipRegistry();
}

namespace API {

    extern ushort;

    bool LoadSettings();
    void StartUp();

} // namespace API

//namespace SrvCtrlObj {
namespace SrvCtrlObj {

    struct SrvObj {
        //Name
        std::string scSrvObjNickname;

        //Type
        std::string scType;

        //Coords
        float x;
        float y;
        float z;

        //Orientation
        Matrix m;

        //System
        std::string scSystem;

        //ShipArch
        std::string scShipArch;

        //Loadout
        std::string scLoadOut;

        //AI
        pub::AI::SetPersonalityParams p;
    };

   // extern std::list<SrvObj> lstSrvObjs;


}// namespace SrvCtrlObj

#endif