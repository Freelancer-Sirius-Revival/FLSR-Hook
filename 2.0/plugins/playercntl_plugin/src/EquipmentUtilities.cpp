﻿// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"
#include <FLHook.h>
#include <plugin.h>

/// A map of hashcode and their associated nicknames.
static std::map<uint, std::string> mapHashToNickname;

/// Read an ini file for nicknames and save the associated hashcode
static void ReadIniNicknameFile(const std::string &filePath) {
    INI_Reader ini;
    if (ini.open(filePath.c_str(), false)) {
        while (ini.read_header()) {
            while (ini.read_value()) {
                if (ini.is_value("nickname")) {
                    uint hash =
                        CreateID(Trim(ToLower(ini.get_value_string())).c_str());
                    mapHashToNickname[hash] = ini.get_value_string();
                }
            }
        }
        ini.close();
    }
}

const char *EquipmentUtilities::FindNickname(uint hash) {
    std::map<uint, std::string>::iterator i = mapHashToNickname.find(hash);
    if (i == mapHashToNickname.end())
        return "";
    return i->second.c_str();
}

/** Read freelancer data to determine mod settings. */
void EquipmentUtilities::ReadIniNicknames() {
    std::string Globals::DATADIRPath = "..\\data";

    INI_Reader ini;
    if (ini.open("freelancer.ini", false)) {
        while (ini.read_header()) {
            if (ini.is_header("Freelancer")) {
                while (ini.read_value()) {
                    if (ini.is_value("data path")) {
                        Globals::DATADIRPath = ini.get_value_string();
                        break;
                    }
                }
            }
        }
        ini.close();
    }

    mapHashToNickname.clear();
    if (ini.open("freelancer.ini", false)) {
        while (ini.read_header()) {
            if (ini.is_header("Data")) {
                while (ini.read_value()) {
                    if (ini.is_value("equipment")) {
                        ReadIniNicknameFile(Globals::DATADIRPath + std::string("\\") + ini.get_value_string());
                    } else if (ini.is_value("ships")) {
                        ReadIniNicknameFile(Globals::DATADIRPath + std::string("\\") + ini.get_value_string());
                    } else if (ini.is_value("goods")) {
                        ReadIniNicknameFile(Globals::DATADIRPath + std::string("\\") + ini.get_value_string());
                    } else if (ini.is_value("loadouts")) {
                        ReadIniNicknameFile(Globals::DATADIRPath + std::string("\\") + ini.get_value_string());
                    }
                }
            }
        }
        ini.close();
    }
}
