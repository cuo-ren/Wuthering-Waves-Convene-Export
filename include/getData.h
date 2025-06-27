#pragma once
#include "globals.h"
#include "config.h"
#include "utils.h"
#include "data.h"

std::map<std::string, std::string> get_params(const std::string& url);
json find_apis();
json get_gacha_data(std::string cardPoolId, std::string cardPoolType, std::string playerId, std::string recordId, std::string serverId);
void merge(const std::string uid, json new_gacha_list);
void update_data();