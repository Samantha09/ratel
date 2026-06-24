#include "json.hpp"
int main() {
    nlohmann::json j;
    j["event"] = "idSet";
    j["data"]["clientId"] = 1;
    return j["data"]["clientId"].get<int>() == 1 ? 0 : 1;
}
