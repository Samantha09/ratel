#ifndef LANDLORDS_COMMON_WEB_JSONMAPHELPER_H_
#define LANDLORDS_COMMON_WEB_JSONMAPHELPER_H_
#include "json.hpp"
#include "helper/SerializeHelper.h"
#include "entity/Poker.h"
#include "enums/ClientEventCode.h"
#include "enums/ServerEventCode.h"
#include <string>

using nlohmann::json;

inline static json poker_to_json(const Poker& p) {
    return { {"level", p.getLevel()}, {"type", p.getType()} };
}
inline static json pokers_to_json(const std::vector<Poker>& v) {
    json arr = json::array();
    for (const auto& p : v) arr.push_back(poker_to_json(p));
    return arr;
}

// Egress: server -> frontend, per spec §6.2
inline json to_event_json(ClientEventCode code, const MapHelper& m) {
    json data = json::object();
    // Log unmapped events for debugging
    bool mapped = true;
    switch (code) {
        case ClientEventCode::CODE_GAME_ID_SET:
            data["clientId"] = m.get("clientId", 0);
            return { {"event", "idSet"}, {"data", data} };
        case ClientEventCode::CODE_GAME_STARTING:
            data["pokers"] = pokers_to_json(m.get("pokers", std::vector<Poker>()));
            data["clientId"] = m.get("clientId", 0);
            data["nextClientId"] = m.get("nextClientId", 0);
            data["nextClientNickname"] = m.get("nextClientNickname", std::string());
            data["roomOwner"] = m.get("roomOwner", std::string());
            data["roomClientCount"] = m.get("roomClientCount", 0);
            return { {"event", "gameStarting"}, {"data", data} };
        case ClientEventCode::CODE_GAME_LANDLORD_ELECT:
            data["nextClientId"] = m.get("nextClientId", 0);
            data["nextClientNickname"] = m.get("nextClientNickname", std::string());
            data["preClientNickname"] = m.get("preClientNickname", std::string());
            return { {"event", "landlordElect"}, {"data", data} };
        case ClientEventCode::CODE_GAME_LANDLORD_CONFIRM:
            data["landlordId"] = m.get("landlordId", 0);
            data["landlordNickname"] = m.get("landlordNickname", std::string());
            data["additionalPokers"] = pokers_to_json(m.get("additionalPokers", std::vector<Poker>()));
            return { {"event", "landlordConfirm"}, {"data", data} };
        case ClientEventCode::CODE_SHOW_POKERS:
            data["pokers"] = pokers_to_json(m.get("pokers", std::vector<Poker>()));
            data["clientId"] = m.get("clientId", 0);
            data["clientNickname"] = m.get("clientNickname", std::string());
            data["clientType"] = m.get("clientType", 0);
            return { {"event", "showPokers"}, {"data", data} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_REDIRECT: {
            data["pokers"] = pokers_to_json(m.get("pokers", std::vector<Poker>()));
            data["lastSellPokers"] = pokers_to_json(m.get("lastSellPokers", std::vector<Poker>()));
            data["lastSellClientId"] = m.get("lastSellClientId", 0);
            data["sellClientId"] = m.get("sellClientId", 0);
            // tolerate C++ typo sellClinetNickname
            data["sellClientNickname"] = m.get("sellClinetNickname", std::string());
            json seats = json::array();
            for (const auto& ci : m.get("clientInfos", std::vector<ClientInfo>())) {
                seats.push_back({ {"clientId", ci.clientId},
                                  {"nickname", ci.clientNickname},
                                  {"type", int(ci.type)},
                                  {"cardsLeft", ci.surplus},
                                  {"position", ci.position} });
            }
            data["clientInfos"] = seats;
            return { {"event", "playRedirect"}, {"data", data} };
        }
        case ClientEventCode::CODE_GAME_POKER_PLAY_PASS:
            data["clientId"] = m.get("clientId", 0);
            data["clientNickname"] = m.get("clientNickname", std::string());
            data["nextClientId"] = m.get("nextClientId", 0);
            data["nextClientNickname"] = m.get("nextClientNickname", std::string());
            return { {"event", "playPass"}, {"data", data} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_MISMATCH:
            return { {"event", "playError"}, {"data", { {"code", "mismatch"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_LESS:
            return { {"event", "playError"}, {"data", { {"code", "less"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_INVALID:
            return { {"event", "playError"}, {"data", { {"code", "invalid"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_ORDER_ERROR:
            return { {"event", "playError"}, {"data", { {"code", "order"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_CANT_PASS:
            return { {"event", "playError"}, {"data", { {"code", "cantPass"} }} };
        case ClientEventCode::CODE_GAME_OVER:
            data["winnerNickname"] = m.get("winnerNickname", std::string());
            data["winnerType"] = m.get("winnerType", std::string());
            return { {"event", "gameOver"}, {"data", data} };
        case ClientEventCode::CODE_SHOW_OPTIONS:
            return { {"event", "showOptions"}, {"data", data} };
        case ClientEventCode::CODE_CLIENT_CONNECT:
            return { {"event", "clientConnect"}, {"data", data} };
        case ClientEventCode::CODE_SHOW_OPTIONS_SETTING:
            return { {"event", "showOptionsSetting"}, {"data", data} };
        case ClientEventCode::CODE_SHOW_OPTIONS_PVP:
            return { {"event", "showOptionsPvp"}, {"data", data} };
        case ClientEventCode::CODE_SHOW_OPTIONS_PVE:
            return { {"event", "showOptionsPve"}, {"data", data} };
        case ClientEventCode::CODE_ROOM_CREATE_SUCCESS:
            return { {"event", "roomCreateSuccess"}, {"data", data} };
        case ClientEventCode::CODE_GAME_LANDLORD_CYCLE:
            return { {"event", "landlordCycle"}, {"data", data} };
        case ClientEventCode::CODE_PVE_DIFFICULTY_NOT_SUPPORT:
            return { {"event", "pveDifficultyNotSupport"}, {"data", data} };
        default:
            return { {"event", "unknown"}, {"data", data} };
    }
}

// Ingress: frontend -> server, per spec §6.1. Returns MapHelper; caller resolves
// event name -> ServerEventCode. Only normalizes the well-known client events.
inline MapHelper from_event_json(const json& j) {
    MapHelper m;
    std::string ev = j.value("event", "");
    const json& d = j.value("data", json::object());
    if (ev == "setNickname") {
        m.put("nickName", d.value("nickname", std::string()));
    } else if (ev == "createRoomPve") {
        m.put("choose", std::string("1"));  // difficulty: 1 = easy (only supported level)
    } else if (ev == "landlordElect") {
        m.put("is_Y", d.value("grab", false) ? "true" : "false");
    } else if (ev == "play") {
        std::vector<PokerLevel> options;
        if (d.contains("pokers") && d["pokers"].is_array()) {
            for (const auto& c : d["pokers"]) {
                options.push_back(static_cast<PokerLevel>(c.value("level", 0)));
            }
        }
        m.put("options", options);
    } else if (ev == "pass") {
        // no fields
    }
    return m;
}

// Map a frontend event name to ServerEventCode (used by server.cc ingress).
inline ServerEventCode event_name_to_server_code(const std::string& ev) {
    if (ev == "setNickname")     return ServerEventCode::CODE_CLIENT_NICKNAME_SET;
    if (ev == "createRoomPve")   return ServerEventCode::CODE_ROOM_CREATE_PVE;
    if (ev == "landlordElect")   return ServerEventCode::CODE_GAME_LANDLORD_ELECT;
    if (ev == "play")            return ServerEventCode::CODE_GAME_POKER_PLAY;
    if (ev == "pass")            return ServerEventCode::CODE_GAME_POKER_PLAY_PASS;
    if (ev == "exit")            return ServerEventCode::CODE_CLIENT_EXIT;
    return ServerEventCode::CODE_CLIENT_EXIT;  // safe fallback
}
#endif
