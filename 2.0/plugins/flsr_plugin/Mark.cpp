#include "Main.h"

/**
* Mark 1 -> requires Client and Target to be in same system
* Mark 0 -> requires the Client to have it on its Mark list
* Mark list on the client is cleared when leaving server/character
*/


namespace Mark
{
    const uint CLEAR_ROTATION_TIMER_INTERVAL = 500;

    // Contains a set of currently market targets (the current system's) per client id.
    static std::unordered_map<uint, std::set<uint>> currentlyMarkedObjectsPerClient;

    // Contains a set of marked targets per system per character file name.
    static std::unordered_map<std::wstring, std::unordered_map<uint, std::set<uint>>> markedObjectsPerCharacter;
    static std::unordered_map<std::wstring, std::set<std::wstring>> markedCharactersPerCharacter;

    static std::set<std::wstring> cloakedCharacterFileNames;

    const uint UI_SELECT_ID = CreateID("ui_select_add");
    const uint UI_UNSELECT_ID = CreateID("ui_select_remove");

    bool MarkObject(const uint clientId, const uint targetId)
    {
        uint shipId;
        pub::Player::GetShip(clientId, shipId);
        if (shipId && targetId && (shipId != targetId) && !currentlyMarkedObjectsPerClient[clientId].contains(targetId))
        {
            currentlyMarkedObjectsPerClient[clientId].insert(targetId);
            pub::Player::MarkObj(clientId, targetId, 1);
            return true;
        }
        return false;
    }

    bool UnmarkObject(const uint clientId, const uint targetId)
    {
        if (clientId && targetId && currentlyMarkedObjectsPerClient[clientId].contains(targetId))
        {
            currentlyMarkedObjectsPerClient[clientId].erase(targetId);
            pub::Player::MarkObj(clientId, targetId, 0);
            return true;
        }
        return false;
    }

    uint FindClientIdByCharacterFileName(const std::wstring& characterFileName)
    {
        PlayerData* playerData = 0;
        while (playerData = Players.traverse_active(playerData))
        {
            const uint otherClientId = HkGetClientIdFromPD(playerData);
            std::wstring otherCharacterFileName;
            if (HkGetCharFileName(ARG_CLIENTID(otherClientId), otherCharacterFileName) != HKE_OK)
                continue;
            if (otherCharacterFileName.compare(characterFileName) == 0)
                return otherClientId;
        }
        return 0;
    }

    void ClearNonExistingTargetIds(const std::wstring& characterFileName)
    {
        std::set<uint> deletedIds;
        std::vector<uint> emptySystemIds;
        for (auto& markedObjectIdsPerSystemId : markedObjectsPerCharacter[characterFileName])
        {
            std::vector<uint> deletedIdsInSystem;
            for (const uint targetId : markedObjectIdsPerSystemId.second)
            {
                if (pub::SpaceObj::ExistsAndAlive(targetId) != 0) // 0 -> true
                    deletedIdsInSystem.push_back(targetId);
            }

            for (const uint objectId : deletedIdsInSystem)
            {
                markedObjectIdsPerSystemId.second.erase(objectId);
                deletedIds.insert(objectId);
            }

            if (markedObjectIdsPerSystemId.second.empty())
                emptySystemIds.push_back(markedObjectIdsPerSystemId.first);
        }

        for (const uint systemId : emptySystemIds)
            markedObjectsPerCharacter[characterFileName].erase(systemId);

        if (markedObjectsPerCharacter[characterFileName].empty())
            markedObjectsPerCharacter.erase(characterFileName);

        const uint clientId = FindClientIdByCharacterFileName(characterFileName);
        if (!clientId)
            return; 

        for (const uint targetId : currentlyMarkedObjectsPerClient[clientId])
        {
            if (deletedIds.contains(targetId))
                UnmarkObject(clientId, targetId);
        }

        if (currentlyMarkedObjectsPerClient[clientId].empty())
            currentlyMarkedObjectsPerClient.erase(clientId);
    }

    static std::queue<std::wstring> characterNamesForClearingRotation;

    void RotateClearNonExistingTargetIds()
    {
        if (characterNamesForClearingRotation.empty())
        {
            for (const auto& objectsPerCharacterName : markedObjectsPerCharacter)
                characterNamesForClearingRotation.push(objectsPerCharacterName.first);
        }

        if (!characterNamesForClearingRotation.empty())
        {
            ClearNonExistingTargetIds(characterNamesForClearingRotation.front());
            characterNamesForClearingRotation.pop();
        }
    }

    bool IsMarkedObject(const uint clientId, const uint targetId)
    {
        return currentlyMarkedObjectsPerClient[clientId].contains(targetId);
    }

    void UnmarkEverywhere(const uint targetId)
    {
        for (auto& markedObjectsPerClient : currentlyMarkedObjectsPerClient)
            UnmarkObject(markedObjectsPerClient.first, targetId);

        for (auto& markedObjectsPerCharacter : markedObjectsPerCharacter)
        {
            for (auto& markedObjectsPerSystemId : markedObjectsPerCharacter.second)
                markedObjectsPerSystemId.second.erase(targetId);
        }
    }

    void DisposeCharacterEverywhere(const std::wstring& characterFileName)
    {
        for (auto& charactersPerCharacter : markedCharactersPerCharacter)
            charactersPerCharacter.second.erase(characterFileName);

        cloakedCharacterFileNames.erase(characterFileName);
    }

    void RefreshMarksForCurrentSystem(const uint clientId)
    {
        uint clientSystemId;
        pub::Player::GetSystem(clientId, clientSystemId);
        if (!clientSystemId)
            return;

        std::wstring characterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), characterFileName) != HKE_OK)
            return;

        // Mark static objects or NPCs
        if (markedObjectsPerCharacter.contains(characterFileName) && markedObjectsPerCharacter[characterFileName].contains(clientSystemId))
        {
            for (const uint targetId : markedObjectsPerCharacter[characterFileName][clientSystemId])
                MarkObject(clientId, targetId);
        }

        // Mark players within the system
        if (!markedCharactersPerCharacter.contains(characterFileName))
            return;
        for (const std::wstring targetCharacterFileName : markedCharactersPerCharacter[characterFileName])
        {
            if (cloakedCharacterFileNames.contains(targetCharacterFileName))
                continue;

            PlayerData* playerData = 0;
            while (playerData = Players.traverse_active(playerData))
            {
                const uint otherClientId = HkGetClientIdFromPD(playerData);
                std::wstring otherCharacterFileName;
                if (HkGetCharFileName(ARG_CLIENTID(otherClientId), otherCharacterFileName) != HKE_OK)
                    continue;
                if (otherCharacterFileName.compare(targetCharacterFileName) == 0)
                {
                    uint targetId;
                    pub::Player::GetShip(otherClientId, targetId);
                    MarkObject(clientId, targetId);
                }
            }
        }
    }

    void Mark(const uint clientId, const uint targetId)
    {
        uint shipId;
        pub::Player::GetShip(clientId, shipId);
        if (shipId == targetId)
            return;

        std::wstring characterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), characterFileName) != HKE_OK)
            return;

        uint targetSystemId;
        pub::SpaceObj::GetSystem(targetId, targetSystemId);
        if (!targetSystemId)
            return;

        const uint targetClientId = HkGetClientIDByShip(targetId);
        if (targetClientId)
        {
            std::wstring targetCharacterFileName;
            if (HkGetCharFileName(ARG_CLIENTID(targetClientId), targetCharacterFileName) != HKE_OK)
                return;
            markedCharactersPerCharacter[characterFileName].insert(targetCharacterFileName);
            if (cloakedCharacterFileNames.contains(targetCharacterFileName))
                return;
        }
        else
        {
            markedObjectsPerCharacter[characterFileName][targetSystemId].insert(targetId);
        }

        uint clientSystemId;
        pub::Player::GetSystem(clientId, clientSystemId);
        if (clientSystemId != targetSystemId)
            return;

        if (MarkObject(clientId, targetId))
            pub::Audio::PlaySoundEffect(clientId, UI_SELECT_ID);
    }

    void Unmark(const uint clientId, const uint targetId)
    {
        std::wstring characterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), characterFileName) != HKE_OK)
            return;

        const uint targetClientId = HkGetClientIDByShip(targetId);
        if (targetClientId)
        {
            std::wstring targetCharacterFileName;
            if (HkGetCharFileName(ARG_CLIENTID(targetClientId), targetCharacterFileName) != HKE_OK)
                return;
            markedCharactersPerCharacter[characterFileName].erase(targetCharacterFileName);
        }
        else
        {
            for (auto& markedObjectsPerSystemId : markedObjectsPerCharacter[characterFileName])
                markedObjectsPerSystemId.second.erase(targetId);
        }

        if (UnmarkObject(clientId, targetId))
            pub::Audio::PlaySoundEffect(clientId, UI_UNSELECT_ID);
    }

    void UnmarkAll(const uint clientId)
    {
        std::wstring characterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), characterFileName) != HKE_OK)
            return;

        markedCharactersPerCharacter[characterFileName].clear();
        markedObjectsPerCharacter[characterFileName].clear();

        if (currentlyMarkedObjectsPerClient[clientId].size() > 0)
        {
            pub::Audio::PlaySoundEffect(clientId, UI_UNSELECT_ID);
            for (const uint targetId : currentlyMarkedObjectsPerClient[clientId])
                pub::Player::MarkObj(clientId, targetId, 0);
        }
        currentlyMarkedObjectsPerClient.erase(clientId);
    }

    void AddClientToEveryonesCurrentlyMarkedObjects(const uint clientId)
    {
        uint targetId;
        pub::Player::GetShip(clientId, targetId);
        if (!targetId)
            return;

        uint targetSystemId;
        pub::Player::GetSystem(clientId, targetSystemId);
        if (!targetSystemId)
            return;

        std::wstring targetCharacterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), targetCharacterFileName) != HKE_OK)
            return;

        for (const auto& charactersPerCharacter : markedCharactersPerCharacter)
        {
            if (charactersPerCharacter.second.contains(targetCharacterFileName))
            {
                const uint otherClientId = FindClientIdByCharacterFileName(charactersPerCharacter.first);
                if (!otherClientId)
                    continue;
                uint otherShipId;
                pub::Player::GetShip(otherClientId, otherShipId);
                if (!otherShipId)
                    continue;

                uint otherSystemId;
                pub::Player::GetSystem(otherClientId, otherSystemId);
                if (otherSystemId != targetSystemId)
                    continue;

                MarkObject(otherClientId, targetId);
            }
        }
    }

    void RemoveTargetIdFromEveryonesCurrentlyMarkedObjects(const uint targetId)
    {
        for (const auto& markedObjectsPerClientId : currentlyMarkedObjectsPerClient)
            UnmarkObject(markedObjectsPerClientId.first, targetId);
    }

    void AddCloakedPlayer(const uint clientId)
    {
        std::wstring characterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), characterFileName) != HKE_OK)
            return;

        cloakedCharacterFileNames.insert(characterFileName);

        uint shipId;
        pub::Player::GetShip(clientId, shipId);
        if (!shipId)
            return;

        RemoveTargetIdFromEveryonesCurrentlyMarkedObjects(shipId);
    }

    void RemoveCloakedPlayer(const uint clientId)
    {
        std::wstring characterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), characterFileName) != HKE_OK)
            return;

        cloakedCharacterFileNames.erase(characterFileName);

        AddClientToEveryonesCurrentlyMarkedObjects(clientId);
    }

    std::list<GROUP_MEMBER> GetGroupMemberClientIds(const uint clientId)
    {
        std::list<GROUP_MEMBER> members;
        if (!HkIsValidClientID(clientId))
            return members;

        HkGetGroupMembers(ARG_CLIENTID(clientId), members);
        return members;
    }

    void UserCmd_Mark(const uint clientId, const std::wstring& wscParam)
    {
        uint shipId, targetId;
        pub::Player::GetShip(clientId, shipId);
        if (!shipId)
            return;
        pub::SpaceObj::GetTarget(shipId, targetId);
        if (!targetId)
            return;

        if (!IsMarkedObject(clientId, targetId))
            Mark(clientId, targetId);
        else
            Unmark(clientId, targetId);
    }

    void UserCmd_UnMark(const uint clientId, const std::wstring& wscParam)
    {
        uint shipId, targetId;
        pub::Player::GetShip(clientId, shipId);
        if (!shipId)
            return;
        pub::SpaceObj::GetTarget(shipId, targetId);
        if (!targetId)
            return;
        Unmark(clientId, targetId);
    }

    void UserCmd_GroupMark(const uint clientId, const std::wstring& wscParam)
    {
        uint shipId, targetId;
        pub::Player::GetShip(clientId, shipId);
        if (!shipId)
            return;
        pub::SpaceObj::GetTarget(shipId, targetId);
        if (!targetId)
            return;

        const bool alreadyMarkedBefore = IsMarkedObject(clientId, targetId);
        const auto groupMembers = GetGroupMemberClientIds(clientId);
        for (const auto& member : groupMembers)
            Mark(member.iClientID, targetId);
        if (alreadyMarkedBefore && groupMembers.size() > 1)
            pub::Audio::PlaySoundEffect(clientId, UI_SELECT_ID);
    }

    void UserCmd_UnGroupMark(const uint clientId, const std::wstring& wscParam)
    {
        uint shipId, targetId;
        pub::Player::GetShip(clientId, shipId);
        if (!shipId)
            return;
        pub::SpaceObj::GetTarget(shipId, targetId);
        if (!targetId)
            return;

        const bool alreadyUnmarkedBefore = !IsMarkedObject(clientId, targetId);
        const auto groupMembers = GetGroupMemberClientIds(clientId);
        for (const auto& member : groupMembers)
            Unmark(member.iClientID, targetId);
        if (alreadyUnmarkedBefore && groupMembers.size() > 1)
            pub::Audio::PlaySoundEffect(clientId, UI_UNSELECT_ID);
    }

    void UserCmd_UnMarkAll(const uint clientId, const std::wstring& wscParam)
    {
        UnmarkAll(clientId);
    }

    void __stdcall SystemSwitchOutComplete_After(unsigned int shipId, unsigned int clientId)
    {
        returncode = DEFAULT_RETURNCODE;

        RefreshMarksForCurrentSystem(clientId);
    }

    void __stdcall PlayerLaunch_After(unsigned int shipId, unsigned int clientId)
    {
        returncode = DEFAULT_RETURNCODE;

        currentlyMarkedObjectsPerClient.erase(clientId);
        RefreshMarksForCurrentSystem(clientId);
    }

    void __stdcall BaseEnter(unsigned int baseId, unsigned int clientId)
    {
        returncode = DEFAULT_RETURNCODE;

        uint shipId;
        pub::Player::GetShip(clientId, shipId);
        if (shipId)
            RemoveTargetIdFromEveryonesCurrentlyMarkedObjects(shipId);

        currentlyMarkedObjectsPerClient.erase(clientId);
    }

    void __stdcall BaseEnter_After(unsigned int baseId, unsigned int clientId)
    {
        returncode = DEFAULT_RETURNCODE;

        currentlyMarkedObjectsPerClient.erase(clientId);
    }

    void __stdcall DisConnect(unsigned int clientId, enum EFLConnection state)
    {
        returncode = DEFAULT_RETURNCODE;

        uint shipId;
        pub::Player::GetShip(clientId, shipId);
        if (shipId)
            RemoveTargetIdFromEveryonesCurrentlyMarkedObjects(shipId);

        currentlyMarkedObjectsPerClient.erase(clientId);
    }

    void __stdcall ShipDestroyed(DamageList* dmg, DWORD* ecx, uint killed)
    {
        returncode = DEFAULT_RETURNCODE;

        if (!killed)
            return;
        const CShip* ship = (CShip*)ecx[4];
        if (!ship)
            return;
        UnmarkEverywhere(ship->iID);

        const uint clientId = ship->GetOwnerPlayer();
        if (!clientId)
            return;

        std::wstring characterFileName;
        if (HkGetCharFileName(ARG_CLIENTID(clientId), characterFileName) != HKE_OK)
            return;
        DisposeCharacterEverywhere(characterFileName);
    }
}