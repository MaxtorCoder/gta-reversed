#include "StdInc.h"

void CEventEditableResponse::InjectHooks() {
    HookInstall(0x4AC450, &CEventEditableResponse::Constructor);
    HookInstall(0x420ED0, &CEventEditableResponse::Clone_Reversed);
    HookInstall(0x420EF0, &CEventEditableResponse::HasEditableResponse_Reversed);
    HookInstall(0x4AC490, &CEventEditableResponse::WillRespond);
    HookInstall(0x4AC4A0, &CEventEditableResponse::InformVehicleOccupants);
    HookInstall(0x4B2B00, &CEventEditableResponse::InformRespectedFriends);
    HookInstall(0x4B7DF0, &CEventEditableResponse::InformGroup);
    HookInstall(0x4AC5A0, &CEventEditableResponse::TriggerLookAt);
    HookInstall(0x4B56C0, (void (CEventEditableResponse::*)(CPed * ped, bool))&CEventEditableResponse::ComputeResponseTaskType);
    HookInstall(0x4B57A0, (void (CEventEditableResponse::*)(CPedGroup*)) & CEventEditableResponse::ComputeResponseTaskType);
    HookInstall(0x4B5730, &CEventEditableResponse::ComputeResponseTaskOfType);
}

void CEventSpecial::InjectHooks()
{
    HookInstall(0x4B1AE0, &CEventSpecial::Constructor);
}

void CEventFireNearby::InjectHooks()
{
    HookInstall(0x4B1F10, &CEventFireNearby::Constructor);
    HookInstall(0x4B1F90, &CEventFireNearby::AffectsPed_Reversed);
}

void CEventDanger::InjectHooks()
{
    HookInstall(0x4B2600, &CEventDanger::Constructor);
    HookInstall(0x4B5470, &CEventDanger::AffectsPed_Reversed);
    HookInstall(0x4B54E0, &CEventDanger::AffectsPedGroup_Reversed);
    HookInstall(0x4B2700, &CEventDanger::GetSourceEntity_Reversed);
}

void CEventSeenPanickedPed::InjectHooks()
{
    HookInstall(0x4B2080, &CEventSeenPanickedPed::Constructor);
    HookInstall(0x4B53C0, &CEventSeenPanickedPed::AffectsPed_Reversed);
}

CEventEditableResponse::CEventEditableResponse() {
    m_bAddToEventGroup = true;
    m_taskId = TASK_NONE;
    field_10 = -1;
}

CEventEditableResponse::~CEventEditableResponse() {
    // nothing here
}

CEventEditableResponse* CEventEditableResponse::Constructor() {
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<CEventEditableResponse*, 0x4AC450>(this);
#else
    this->CEventEditableResponse::CEventEditableResponse();
    return this;
#endif
}

CEvent* CEventEditableResponse::Clone() {
#ifdef USE_DEFAULT_FUNCTIONS
    return ((CEvent * (__thiscall*)(CEvent*))0x420ED0)(this);
#else
    return CEventEditableResponse::Clone_Reversed();
#endif
}

bool CEventEditableResponse::HasEditableResponse() {
#ifdef USE_DEFAULT_FUNCTIONS
    return ((bool(__thiscall*)(CEvent*))0x420EF0)(this);
#else
    return CEventEditableResponse::HasEditableResponse_Reversed();
#endif
}

CEvent* CEventEditableResponse::Clone_Reversed() {
    CEventEditableResponse* pClonedEvent = CloneEditable();
    pClonedEvent->m_taskId = m_taskId;
    pClonedEvent->field_10 = field_10;
    pClonedEvent->m_bAddToEventGroup = m_bAddToEventGroup;
    return pClonedEvent;
}


bool CEventEditableResponse::WillRespond() {
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<bool, 0x4AC490, CEventEditableResponse*>(this);
#else
    return m_taskId != TASK_NONE;
#endif
}

void CEventEditableResponse::InformVehicleOccupants(CPed* ped) {
#ifdef USE_DEFAULT_FUNCTIONS
   plugin::CallMethod<0x4AC4A0, CEventEditableResponse*, CPed*>(this, ped);
#else
    CVehicle* pVehicle = ped->m_pVehicle;
    if (ped->bInVehicle && DoInformVehicleOccupants(ped) && m_bAddToEventGroup) {
        CPed* pDriver = pVehicle->m_pDriver;
        if (pDriver && pDriver != ped) {
            auto pClonedEvent = static_cast<CEventEditableResponse*>(Clone());
            pClonedEvent->m_taskId = TASK_NONE;
            pClonedEvent->m_bAddToEventGroup = false;
            pDriver->m_pIntelligence->m_eventGroup.Add(pClonedEvent, false);
            delete pClonedEvent;
        }
        for (unsigned char i = 0; i < pVehicle->m_nMaxPassengers; i++) {
            CPed* pPassenger = pVehicle->m_apPassengers[i];
            if (pPassenger && pPassenger != ped) {
                auto pClonedEvent = static_cast<CEventEditableResponse*>(Clone());
                pClonedEvent->m_taskId = TASK_NONE;
                pClonedEvent->m_bAddToEventGroup = false;
                pPassenger->m_pIntelligence->m_eventGroup.Add(pClonedEvent, false);
                delete pClonedEvent;
            }
        }
    }
#endif
}

void CEventEditableResponse::InformRespectedFriends(CPed* ped) {
#ifdef USE_DEFAULT_FUNCTIONS
    plugin::CallMethod<0x4B2B00, CEventEditableResponse*, CPed*>(this, ped);
#else
    if (!m_bAddToEventGroup)
        return;
    unsigned int numPedsToScan = ped->m_pIntelligence->m_nDmNumPedsToScan;
    if (!numPedsToScan)
        return;
    CEntity** pEntities = ped->m_pIntelligence->m_entityScanner.m_apEntities;
    for (size_t entityIndex = 0; entityIndex < numPedsToScan; entityIndex++) {
        CEntity* pEntity = pEntities[entityIndex];
        if (!pEntity)
            continue;
        CPed* pEntityPed = static_cast<CPed*>(pEntity);
        if (pEntityPed->m_nPedType == PED_TYPE_COP) {
            if (pEntityPed->m_nPedType != PED_TYPE_COP)
                continue;
            CPlayerData* pPlayerData = FindPlayerPed(0)->m_pPlayerData;
            if (pPlayerData->m_pWanted && pPlayerData->m_pWanted->m_nWantedLevel)
                continue;
        }
        else {
            unsigned int pedAcquaintances = ped->m_acquaintance.GetAcquaintances(0);
            bool bFlagSet = pedAcquaintances & CPedType::GetPedFlag(pEntityPed->m_nPedType);
            if (pEntityPed->IsPlayer()) {
                unsigned int entityAcquaintances = pEntityPed->m_acquaintance.GetAcquaintances(0);
                bFlagSet = entityAcquaintances & CPedType::GetPedFlag(ped->m_nPedType);
            }
            if (!bFlagSet)
                continue;
        }
        float fDmRadius = ped->m_pIntelligence->m_fDmRadius;
        CVector direction = ped->GetPosition() - pEntity->GetPosition();
        if (fDmRadius * fDmRadius > direction.SquaredMagnitude()) {
            auto pClonedEvent = static_cast<CEventEditableResponse*>(Clone());
            pClonedEvent->m_taskId = TASK_NONE;
            pClonedEvent->m_bAddToEventGroup = false;
            if (!CInformFriendsEventQueue::Add(pEntity, pClonedEvent))
                delete pClonedEvent;
        }
    }
#endif
}

void CEventEditableResponse::InformGroup(CPed* ped) {
#ifdef USE_DEFAULT_FUNCTIONS
    plugin::CallMethod<0x4B7DF0, CEventEditableResponse*, CPed*>(this, ped);
#else
    CPedGroup* pPedGroup = CPedGroups::GetPedsGroup(ped);
    if (!pPedGroup)
        return;
    auto pClonedEvent = static_cast<CEventEditableResponse*>(Clone());
    pClonedEvent->m_taskId = TASK_NONE;
    pClonedEvent->m_bAddToEventGroup = false;
    if (!CInformGroupEventQueue::Add(ped, pPedGroup, pClonedEvent))
        delete pClonedEvent;
#endif
}

void CEventEditableResponse::TriggerLookAt(CPed* ped) {
#ifdef USE_DEFAULT_FUNCTIONS
    plugin::CallMethod<0x4AC5A0, CEventEditableResponse*, CPed*>(this, ped);
#else
    CEntity* pSourceEntity = GetSourceEntity();
    if (pSourceEntity) {
        if (pSourceEntity->m_nType == ENTITY_TYPE_PED) {
            CPed* pTargetPed = static_cast<CPed*>(pSourceEntity);
            g_ikChainMan->LookAt("CEventEditableResponse", ped, pTargetPed, 2000, BONE_HEAD, nullptr, true, 0.25f, 500, 3, false);
            return;
        }
        g_ikChainMan->LookAt("CEventEditableResponse", ped, pSourceEntity, 2000, BONE_UNKNOWN, nullptr, true, 0.25f, 500, 3, false);
    }
#endif
}

void CEventEditableResponse::ComputeResponseTaskType(CPed* ped, bool bDecisionMakerTypeInGroup) {
#ifdef USE_DEFAULT_FUNCTIONS
    plugin::CallMethod<0x4B56C0, CEventEditableResponse*, CPed*, bool>(this, ped, bDecisionMakerTypeInGroup);
#else
    if (m_taskId == TASK_NONE) {
        int eventSourceType = CEventSource::ComputeEventSourceType(this, ped);
        CDecisionMakerTypes::GetInstance()->MakeDecision(ped, GetEventType(), eventSourceType, ped->bInVehicle,
            TASK_SIMPLE_INFORM_RESPECTED_FRIENDS, 
            TASK_SIMPLE_INFORM_GROUP, 
            TASK_SIMPLE_LOOK_AT_ENTITY_OR_COORD, 
            -1,
            bDecisionMakerTypeInGroup, &m_taskId, &field_10);
    }
#endif
}

void CEventEditableResponse::ComputeResponseTaskType(CPedGroup* pedGroup) {
#ifdef USE_DEFAULT_FUNCTIONS
    plugin::CallMethod<0x4B57A0, CEventEditableResponse*, CPedGroup*>(this, pedGroup);
#else
    if (m_taskId == TASK_NONE) {
        CPed* pGroupLeader = pedGroup->m_groupMembership.GetLeader();
        CPed* pMember = pGroupLeader;
        if (pGroupLeader && pGroupLeader->IsPlayer())
            pMember = nullptr;
        if (!pMember){
            for (size_t memberId = 0; memberId < TOTAL_PED_GROUP_FOLLOWERS; memberId++) {
                pMember = pedGroup->m_groupMembership.GetMember(memberId);
                if (pMember)
                    break;
            }
        }
        if (pMember) {
            int eventSourceType = CEventSource::ComputeEventSourceType(this, pMember);
            m_taskId = CDecisionMakerTypes::GetInstance()->MakeDecision(pedGroup, GetEventType(), eventSourceType, pMember->bInVehicle,
                TASK_SIMPLE_INFORM_GROUP,
                TASK_SIMPLE_INFORM_RESPECTED_FRIENDS,
                TASK_SIMPLE_LOOK_AT_ENTITY_OR_COORD,
                -1);
        }
        else {
            m_taskId = TASK_NONE;
        }
        pGroupLeader = pedGroup->m_groupMembership.GetLeader();
        if (m_taskId == TASK_NONE && pGroupLeader) {
            if (pGroupLeader->IsPlayer()) {
                int eventSourceType = CEventSource::ComputeEventSourceType(this, pGroupLeader);
                m_taskId = CDecisionMakerTypes::GetInstance()->MakeDecision(pedGroup, GetEventType(), eventSourceType, pGroupLeader->bInVehicle,
                    TASK_SIMPLE_INFORM_GROUP,
                    TASK_SIMPLE_INFORM_RESPECTED_FRIENDS,
                    TASK_SIMPLE_LOOK_AT_ENTITY_OR_COORD,
                    -1);
            }
        }
    }
#endif
}

bool CEventEditableResponse::ComputeResponseTaskOfType(CPed* ped, int taskId) {
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<bool, 0x4B5730, CEventEditableResponse*, CPed*, int>(this, ped, taskId);
#else
    short outTaskId = -1;
    short unknownId = -1;
    int eventSourceType = CEventSource::ComputeEventSourceType(this, ped);
    CDecisionMakerTypes::GetInstance()->MakeDecision(ped, GetEventType(), eventSourceType, ped->bInVehicle,
        -1, -1, -1, taskId, false, &outTaskId, &unknownId);
    return taskId == outTaskId;
#endif
}

CEventSpecial* CEventSpecial::Constructor()
{
    this->CEventSpecial::CEventSpecial();
    return this;
}

CEventFireNearby::CEventFireNearby(CVector const& position)
{
    m_position = position;
}

CEventFireNearby* CEventFireNearby::Constructor(CVector const& position)
{
    this->CEventFireNearby::CEventFireNearby(position);
    return this;
}

bool CEventFireNearby::AffectsPed(CPed* ped)
{
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<bool, 0x4B1F90, CEventFireNearby*, CPed*>(this, ped);
#else
    return CEventFireNearby::AffectsPed_Reversed(ped);
#endif
}

bool CEventFireNearby::AffectsPed_Reversed(CPed* ped)
{
    if (ped->GetTaskManager().FindActiveTaskByType(TASK_COMPLEX_EXTINGUISH_FIRES))
        return false;
    return ped->IsAlive();
}


CEventDanger::CEventDanger(CEntity* dangerFrom, float dangerRadius)
{
    m_dangerFrom = dangerFrom;
    if (dangerFrom)
        dangerFrom->RegisterReference(reinterpret_cast<CEntity**>(&m_dangerFrom));
    m_dangerRadius = dangerRadius;
}

CEventDanger::~CEventDanger()
{
    if (m_dangerFrom)
        m_dangerFrom->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_dangerFrom));
}

CEventDanger* CEventDanger::Constructor(CEntity* dangerFrom, float dangerRadius)
{
    this->CEventDanger::CEventDanger(dangerFrom, dangerRadius);
    return this;
}

bool CEventDanger::AffectsPed(CPed* ped)
{
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<bool, 0x4B5470, CEventDanger*, CPed*>(this, ped);
#else
    return CEventDanger::AffectsPed_Reversed(ped);
#endif
}

bool CEventDanger::AffectsPedGroup(CPedGroup* pedGroup)
{
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<bool, 0x4B54E0, CEventDanger*, CPedGroup*>(this, pedGroup);
#else
    return CEventDanger::AffectsPedGroup_Reversed(pedGroup);
#endif
}

CEntity* CEventDanger::GetSourceEntity()
{
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<CEntity*, 0x4B2700, CEventDanger*>(this);
#else
    return CEventDanger::GetSourceEntity_Reversed();
#endif
}

bool CEventDanger::AffectsPed_Reversed(CPed* ped)
{
    CVehicle* dangerFrom = static_cast<CVehicle*>(m_dangerFrom);
    if (dangerFrom && dangerFrom != ped->m_pVehicle) {
        CVector2D distance = ped->GetPosition() - dangerFrom->GetPosition();
        if (m_dangerRadius * m_dangerRadius >= distance.SquaredMagnitude())
            return ped->IsAlive();
    }
    return false;
}

bool CEventDanger::AffectsPedGroup_Reversed(CPedGroup* pedGroup)
{
    if (GetSourceEntity() && GetSourceEntity()->m_nType == ENTITY_TYPE_PED) {
        CPed* leader = pedGroup->GetMembership().GetLeader();
        if (leader) {
            CVector2D distance = leader->GetPosition() - m_dangerFrom->GetPosition();
            return distance.SquaredMagnitude() <= m_dangerRadius * m_dangerRadius;
        }
    }
    return false;
}

CEntity* CEventDanger::GetSourceEntity_Reversed()
{
    if (m_dangerFrom && m_dangerFrom->m_nType != ENTITY_TYPE_PED && m_dangerFrom->m_nType == ENTITY_TYPE_VEHICLE) {
        CVehicle* vehicle = static_cast<CVehicle*>(m_dangerFrom);
        if (vehicle->m_pDriver)
            return vehicle->m_pDriver;
    }
    return m_dangerFrom;
}

CEventSeenPanickedPed::CEventSeenPanickedPed(CPed* ped)
{
    m_ped = ped;
    if (ped)
        ped->RegisterReference(reinterpret_cast<CEntity**>(&m_ped));
}

CEventSeenPanickedPed::~CEventSeenPanickedPed()
{
    if (m_ped)
        m_ped->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_ped));
}

CEventSeenPanickedPed* CEventSeenPanickedPed::Constructor(CPed* ped)
{
    this->CEventSeenPanickedPed::CEventSeenPanickedPed(ped);
    return this;
}

bool CEventSeenPanickedPed::AffectsPed(CPed* ped)
{
#ifdef USE_DEFAULT_FUNCTIONS
    return plugin::CallMethodAndReturn<bool, 0x4B53C0, CEventSeenPanickedPed*, CPed*>(this, ped);
#else
    return CEventSeenPanickedPed::AffectsPed_Reversed(ped);
#endif
}

bool CEventSeenPanickedPed::AffectsPed_Reversed(CPed* ped)
{
    if (!ped->IsPlayer() && m_ped && m_ped != ped) {
        CEvent* currentEvent = m_ped->GetEventHandlerHistory().GetCurrentEvent();
        if (currentEvent && currentEvent->GetSourceEntity()) {
            CVector distance = ped->GetPosition() - m_ped->GetPosition();
            return distance.SquaredMagnitude() < 100.0f;
        }
    }
    return false;
}

