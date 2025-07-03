#pragma once
#include "globals.h"
#include "config.h"
#include "utils.h"
#include "data.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

std::map<std::string, std::string> get_params(const std::string& url);
json find_apis();
json get_gacha_data(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId);
void merge(const std::string uid, json new_gacha_list);
void update_data();
json get_gacha_data_retry(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, int max_retry = 3);