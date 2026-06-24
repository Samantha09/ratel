#include "JsonMapHelper.h"
#include <cassert>
#include <iostream>
#include "json.hpp"

int main() {
    using nlohmann::json;

    // --- egress: GAME_ID_SET {clientId} ---
    MapHelper idm;
    idm.put("clientId", 7);
    json j = to_event_json(ClientEventCode::CODE_GAME_ID_SET, idm);
    assert(j["event"] == "idSet");
    assert(j["data"]["clientId"] == 7);

    // --- egress: SHOW_POKERS carries pokers (someone's played cards) ---
    MapHelper sp;
    std::vector<Poker> played = { Poker(PokerType::HEART, PokerLevel::LEVEL_3),
                                  Poker(PokerType::SPADE, PokerLevel::LEVEL_3) };
    sp.put("pokers", played);
    sp.put("clientId", 7);
    sp.put("clientNickname", "san");
    sp.put("clientType", 0);
    json js = to_event_json(ClientEventCode::CODE_SHOW_POKERS, sp);
    assert(js["event"] == "showPokers");
    assert(js["data"]["pokers"].size() == 2);
    assert(js["data"]["pokers"][0]["level"] == int(PokerLevel::LEVEL_3));
    assert(js["data"]["pokers"][0]["type"] == int(PokerType::HEART));

    // --- ingress: play {pokers:[{level,type}]} -> MapHelper.options=[levels] ---
    json in = { {"event", "play"},
                {"data", { {"pokers", json::array({ { {"level", 0}, {"type", 1} } }) }} }};
    MapHelper m = from_event_json(in);
    // from_event_json keys options by level list; verify it read clientId-free input
    auto options = m.get("options", std::vector<PokerLevel>());
    assert(options.size() == 1 && options[0] == PokerLevel::LEVEL_3);

    // --- ingress: landlordElect {grab:true} -> MapHelper.is_Y == "true" ---
    json grab = { {"event", "landlordElect"}, {"data", { {"grab", true} }} };
    MapHelper mg = from_event_json(grab);
    assert(mg.get("is_Y", std::string("")) == "true");

    std::cout << "jsonmap_test OK\n";
    return 0;
}
