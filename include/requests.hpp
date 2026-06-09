#pragma once
#include <string>
#include <filesystem>
#include "json.hpp"
#include <thread>
#include <chrono>
#include <unordered_map>
#include <set>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#define QT_DEBUG_URL
#define QT_DEBUG_PARAMS
#define QT_DEBUG_CONTENT

using json = nlohmann::json;

struct ParsedUrl
{
	std::string protocol;
	std::string host;
	std::string path;
	std::unordered_multimap<std::string, std::string> query;

	friend std::ostream& operator<<(std::ostream& os, const ParsedUrl& pu) {
		os << "{" << "protocol:" << pu.protocol << ", host:" << pu.host << ", path:" << pu.path << ", query:{";
		for (const auto& item : pu.query) {
			os << item.first << ":" << item.second << ",";
		}
		os << "})";
		return os;
	}
};

enum class SameSite {
	None,
	Lax,
	Strict,
	Unset  // 未声明
};

struct Cookie {
	std::string name = {};
	std::string value = {};
	std::string domain = {};
	std::string path = "/";
	bool hostOnly = false;
	bool session = false;
	bool secure = false;
	bool httpOnly = false;
	time_t expires = 0;
	time_t creationTime = 0;
	SameSite sameSite = SameSite::Unset;

	friend std::ostream& operator<<(std::ostream& os, const Cookie& cookie) {
		os << cookie.name << " = " << cookie.value << "; domain = " << cookie.domain << "; path = " << cookie.path << "; Expires = " << cookie.expires << "; creationTine = " << cookie.creationTime;
		if (cookie.hostOnly) {
			os << "; HostOnly";
		}
		if (cookie.session) {
			os << "; Session";
		}
		if (cookie.secure) {
			os << "; Secure";
		}
		if (cookie.httpOnly) {
			os << "; HttpOnly";
		}
		if (cookie.sameSite == SameSite::Unset) {
			os << "; SameSite: Unset";
		}
		if (cookie.sameSite == SameSite::Lax) {
			os << "; SameSite: Lax";
		}
		if (cookie.sameSite == SameSite::Strict) {
			os << "; SameSite: Strict";
		}
		if (cookie.sameSite == SameSite::None) {
			os << "; SameSite: None";
		}
		return os;
	}
};

struct SetCookieOptions {
	std::string path = "";
	time_t expires = 0;
	time_t creationTime = 0;
	SameSite sameSite = SameSite::Unset;
	bool hostOnly = false;
	bool session = true;
	bool secure = false;
	bool httpOnly = false;
};

struct GetCallBackOptions {
	std::function<bool(const httplib::Response&)> ResponseHandler = nullptr;
	std::function<bool(const char*, size_t)> ContentReceiver = nullptr;
	std::function<bool(uint64_t, uint64_t)> DownloadProgress = nullptr;
};

struct PostCallBackOptions {
	std::function<bool(size_t current, size_t total)> UploadProgress = nullptr;
	std::function<bool(const char*, size_t)> ContentReceiver = nullptr;
	std::function<bool(uint64_t, uint64_t)> DownloadProgress = nullptr;
};

class CookiesJar {
public:
	CookiesJar() = default;
	CookiesJar(const CookiesJar& other)
		: cookies(other.cookies) {}
	CookiesJar(CookiesJar&& other) noexcept
		: cookies(std::move(other.cookies)) {}
	CookiesJar& operator=(const CookiesJar& other) = default;
	CookiesJar& operator=(CookiesJar&& other) noexcept = default;

	~CookiesJar() = default;
	class iterator {
	public:
		using OuterIter = std::unordered_map<std::string,
			std::unordered_map<std::string,
			std::unordered_map<std::string, Cookie>>>::iterator;
		using MiddleIter = std::unordered_map<std::string,
			std::unordered_map<std::string, Cookie>>::iterator;
		using InnerIter = std::unordered_map<std::string, Cookie>::iterator;

		iterator() = default;
		iterator(OuterIter outer, OuterIter outerEnd)
			: outerIt(outer), outerEnd(outerEnd) {
			if (outerIt != outerEnd) {
				middleIt = outerIt->second.begin();
				advance_to_valid_middle();
			}
		}

		Cookie& operator*() { return innerIt->second; }
		Cookie* operator->() { return &innerIt->second; }

		iterator& operator++() {
			++innerIt;
			if (innerIt == middleIt->second.end()) {
				++middleIt;
				advance_to_valid_middle();
			}
			return *this;
		}

		bool operator==(const iterator& other) const {
			return outerIt == other.outerIt &&
				(outerIt == outerEnd ||
					(middleIt == other.middleIt && innerIt == other.innerIt));
		}

		bool operator!=(const iterator& other) const {
			return !(*this == other);
		}

	private:
		OuterIter outerIt, outerEnd;
		MiddleIter middleIt;
		InnerIter innerIt;

		void advance_to_valid_middle() {
			while (outerIt != outerEnd) {
				if (middleIt == outerIt->second.end()) {
					++outerIt;
					if (outerIt != outerEnd)
						middleIt = outerIt->second.begin();
				}
				else {
					innerIt = middleIt->second.begin();
					if (innerIt == middleIt->second.end()) {
						++middleIt;
						continue;
					}
					return;
				}
			}
		}
	};

	class const_iterator {
	public:
		using OuterIter = std::unordered_map<std::string,
			std::unordered_map<std::string,
			std::unordered_map<std::string, Cookie>>>::const_iterator;
		using MiddleIter = std::unordered_map<std::string,
			std::unordered_map<std::string, Cookie>>::const_iterator;
		using InnerIter = std::unordered_map<std::string, Cookie>::const_iterator;

		const_iterator() = default;
		const_iterator(OuterIter outer, OuterIter outerEnd)
			: outerIt(outer), outerEnd(outerEnd) {
			if (outerIt != outerEnd) {
				middleIt = outerIt->second.begin();
				advance_to_valid_middle();
			}
		}

		const Cookie& operator*() const { return innerIt->second; }
		const Cookie* operator->() const { return &innerIt->second; }

		const_iterator& operator++() {
			++innerIt;
			if (innerIt == middleIt->second.end()) {
				++middleIt;
				advance_to_valid_middle();
			}
			return *this;
		}

		bool operator==(const const_iterator& other) const {
			return outerIt == other.outerIt &&
				(outerIt == outerEnd ||
					(middleIt == other.middleIt && innerIt == other.innerIt));
		}

		bool operator!=(const const_iterator& other) const {
			return !(*this == other);
		}

	private:
		OuterIter outerIt, outerEnd;
		MiddleIter middleIt;
		InnerIter innerIt;

		void advance_to_valid_middle() {
			while (outerIt != outerEnd) {
				if (middleIt == outerIt->second.end()) {
					++outerIt;
					if (outerIt != outerEnd)
						middleIt = outerIt->second.begin();
				}
				else {
					innerIt = middleIt->second.begin();
					if (innerIt == middleIt->second.end()) {
						++middleIt;
						continue;
					}
					return;
				}
			}
		}
	};

	iterator begin() { return iterator(cookies.begin(), cookies.end()); }
	iterator end() { return iterator(cookies.end(), cookies.end()); }

	const_iterator begin() const { return const_iterator(cookies.begin(), cookies.end()); }
	const_iterator end() const { return const_iterator(cookies.end(), cookies.end()); }

	std::string to_header(const std::string& url);

	void clear() {
		cookies.clear();
	}
	void clear_expired_cookies();//删除已过期 cookie。
	void clear_session_cookies();//删除session cookie
	//void clear_invalid_domain_cookies();//清除不合法的domain，暂时不限制顶级域名

	void extract_cookies_from_setCookie(const std::string& url, const std::string& setCookie);//从setCookie中提取cookie
	void extract_cookies_from_header(const std::string& url, const json& setCookie);//json::array
	void merge(const CookiesJar& jar, bool priority = false);

	std::string get(const std::string& key, std::string domain = {}, std::string path = {});//根据key,domain,path查找value
	Cookie get_cookie(const std::string& key, std::string domain = {}, std::string path = {}); //根据key,domain,path返回cookie
	json get_dict(std::string domain = {}, std::string path = {});//获取指定domain path下的键值对，若不指定，可能存在覆盖

	std::vector<std::string> list_domains();//返回所有domain
	std::vector<std::string> list_paths(std::string domain = {});//返回所有path

	bool multiple_domains();//判断是否含有多个域，0个算单个

	Cookie pop(const std::string& key, std::string domain = {}, std::string path = {});//删除并返回cookie,若不指定domain path删除第一个匹配的，不可控！

	void set(const std::string& key, const std::string& value, const std::string& domain, SetCookieOptions op = {});
	void setCookie(Cookie c);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, Cookie>>> cookies;

	Cookie parseSetCookie(const std::string& url, std::string set_cookie_string);
	time_t parseExpires(std::string& expires);
	bool is_matcheddomain(const std::string& host, const  std::string& domain);
	bool is_matchedpath(const std::string& url_path, const  std::string& path);
	bool is_ip(std::string& host) {
		return false;//懒得实现
	}
};

class Response
{
public:
	Response() : status_code(0), reason("No Response"), headers(json::object()), text(""), content(""), encoding(""), cookies(CookiesJar()), elapsed(0), url("") {};
	Response(const std::string url_, const httplib::Result& result) {
		status_code = 0;
		reason = "No Response";
		headers = json::object();
		text = "";
		content = "";
		encoding = "";
		cookies = CookiesJar();
		elapsed = 0;
		url = url_;

		if (result) {
			status_code = result->status;
			reason = result->reason;
			for (const auto& it : result->headers) {
				std::string key = to_lower(it.first);
				if (!headers.contains(key)) {
					headers[key] = json::array();
				}
				headers[key].push_back(it.second);
			}
			text = result->body;
			content = result->body;

			cookies.extract_cookies_from_header(url, headers);
		}
		else {
			status_code = 0;
			reason = "No Response";
		}
	}
	~Response() = default;
	explicit operator bool() const noexcept {
		return status_code != 0;
	}

	bool ok() const {
		return status_code >= 200 and status_code < 400;
	}
	json to_json() {
		return json::parse(text);
	}
	void raise_for_status() const {
		if (status_code >= 400) {
			throw std::runtime_error("HTTP error: " + std::to_string(status_code));
		}
	}

	size_t status_code;
	std::string reason;
	json headers;
	std::string url;
	std::string text;
	std::string content;
	std::string encoding;

	CookiesJar cookies;
	size_t elapsed;

	std::vector<Response> history;

private:
	std::string body;
	static inline std::string trim(const std::string& s) {
		size_t a = 0, b = s.size();
		while (a < b && std::isspace((unsigned char)s[a])) ++a;
		while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
		return s.substr(a, b - a);
	}
	static inline std::string to_lower(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
		return s;
	}
};

struct GetOptions {
	json headers = json::object();
	json params = json::object();
	Response response = Response();
	bool allowProxies = true;
	bool allow_redirects = true;
	bool no_exception = true;
	int readTimeout = 10;
	int connectionTimeout = 10;
	int maxRetries = 1;
	int retryDelay = 1;
};

struct PostOptioms {
	json headers = json::object();
	std::string text = {};
	json Json = json::object();
	std::string xml = {};
	std::string raw = {};
	std::string urlencode = {};
	std::string binary = {};
	json mulitipart = json::object();
	std::filesystem::path file = {};
	bool allow_redirects = true;
	bool allowProxies = true;
	bool no_exception = true;
	int readTimeout = 10;
	int connectionTimeout = 10;
	int maxRetries = 1;
	int retryDelay = 1;
};

class Requests
{
public:
	Requests() = default;
	~Requests() = default;
	static Response get(std::string url, GetOptions op = {}, GetCallBackOptions cop = {});
	static Response post(std::string url, PostOptioms op = {}, PostCallBackOptions cop = {});
private:
	friend class CookiesJar;
	friend class Response;
	static Response getOnce(std::string url, Response& res, json headers, json params, bool allow_redirects, bool allowProxies, int readTimeout, int connectionTimeout, std::function<bool(const httplib::Response&)> ResponseHandler, std::function<bool(const char*, size_t)> ContentReceiver, std::function<bool(uint64_t, uint64_t)> DownloadProgress);
	static Response postOnce(std::string url, Response& res, std::string& contentType, json headers, std::string body, std::filesystem::path file, bool allow_redirects, bool allowProxies, int readTimeout, int connectionTimeout);

	static std::string get_system_proxy();
	static std::optional<std::pair<std::string, int>> parse_proxy(const std::string& proxy_raw);
	static inline std::string trim(const std::string& s) {
		size_t a = 0, b = s.size();
		while (a < b && std::isspace((unsigned char)s[a])) ++a;
		while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
		return s.substr(a, b - a);
	}
	static inline std::string to_lower(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
		return s;
	}
	static ParsedUrl parse_url(const std::string& url);
	static json parse_headers(const json& headers);
	static std::string get_redirects_url(const std::string& url, const json& headers);
};

inline std::vector<std::string> CookiesJar::list_domains() {
	std::vector<std::string> domains;
	for (const auto& item : cookies) {
		domains.push_back(item.first);
	}
	return domains;
}

inline std::vector<std::string> CookiesJar::list_paths(std::string domain) {
	std::set<std::string> paths;
	if (domain.empty()) {
		for (const auto& item1 : cookies) {
			for (const auto& item2 : item1.second) {
				paths.insert(item2.first);
			}
		}
	}
	else if (cookies.find(domain) != cookies.end()) {
		for (const auto& item : cookies[domain]) {
			paths.insert(item.first);
		}
	}
	return std::vector<std::string>(paths.begin(), paths.end());
}

inline bool CookiesJar::multiple_domains() {
	return cookies.size() > 1;
}

inline std::string CookiesJar::get(const std::string& key, std::string domain, std::string path) {
	for (const auto& [dm, paths] : cookies) {
		if (!domain.empty() && dm != domain) continue;

		for (const auto& [pt, items] : paths) {
			if (!path.empty() && pt != path) continue;
			if (items.find(key) != items.end()) {
				return items.at(key).value;
			}
		}
	}
	return {};
}

inline Cookie CookiesJar::get_cookie(const std::string& key, std::string domain, std::string path) {
	for (const auto& [dm, paths] : cookies) {
		if (!domain.empty() && dm != domain) continue;

		for (const auto& [pt, items] : paths) {
			if (!path.empty() && pt != path) continue;
			if (items.find(key) != items.end()) {
				return items.at(key);
			}
		}
	}
	return {};
}

inline json CookiesJar::get_dict(std::string domain, std::string path) {
	json result = json::object();

	for (const auto& [dm, paths] : cookies) {
		if (!domain.empty() && dm != domain) continue;

		for (const auto& [pt, items] : paths) {
			if (!path.empty() && pt != path) continue;

			for (const auto& [key, cookie] : items) {
				result[key] = cookie.value;
			}
		}
	}

	return result;
}

inline void CookiesJar::set(const std::string& key, const std::string& value, const std::string& domain, SetCookieOptions op) {
	Cookie cookie;
	if (op.path.empty()) {
		op.path = "/";
	}
	if (op.creationTime == 0) {
		//取当前时间
		auto now = std::chrono::system_clock::now();
		op.creationTime = std::chrono::duration_cast<std::chrono::seconds>(
			now.time_since_epoch()
		).count();
	}
	if (op.expires == 0 and !op.session) {
		pop(key, domain, op.path);
		return;
	}
	if (!op.session) {
		op.expires = 0;
	}
	if (op.expires == 0) {
		op.session = true;
	}

	cookie.name = key; cookie.value = value; cookie.domain = domain; cookie.path = op.path; cookie.expires = op.expires; cookie.creationTime = op.creationTime; cookie.sameSite = op.sameSite; cookie.hostOnly = op.hostOnly; cookie.session = op.session; cookie.secure = op.secure; cookie.httpOnly = op.httpOnly;
	cookies[domain][op.path][key] = cookie;
}

inline void CookiesJar::setCookie(Cookie c) {
	if (c.path.empty()) {
		c.path = "/";
	}
	cookies[c.domain][c.path][c.name] = c;
}

inline Cookie CookiesJar::pop(const std::string& key, std::string domain, std::string path) {
	Cookie c;
	std::string del_domain;
	std::string del_path;
	bool find = false;
	for (const auto& [dm, paths] : cookies) {
		bool flag = false;
		if (!domain.empty() && dm != domain) continue;
		for (const auto& [pt, items] : paths) {
			if (!path.empty() && pt != path) continue;
			if (items.find(key) != items.end()) {
				c = items.at(key);
				del_domain = dm;
				del_path = pt;
				flag = true;
				find = true;
				break;
			}
		}
		if (flag)break;
	}
	if (find) {
		cookies.at(del_domain).at(del_path).erase(key);
		if (cookies.at(del_domain).at(del_path).empty())cookies.at(del_domain).erase(del_path);
		if (cookies.at(del_domain).empty())cookies.erase(del_domain);
		return c;
	}
	return {};
}

inline std::string CookiesJar::to_header(const std::string& url) {
	std::string cookie = {};
	ParsedUrl u = Requests::parse_url(url);

	auto now = std::chrono::system_clock::now();
	// 转换为自 1970-01-01 00:00:00 UTC 起的秒数
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
		now.time_since_epoch()
	).count();

	std::vector<Cookie> matched_cookies;

	for (const auto& [domain, paths] : cookies) {
		if (is_matcheddomain(u.host, domain)) {
			for (const auto& [path, dict] : paths) {
				if (is_matchedpath(u.path, path)) {
					for (const auto& [key, value] : dict) {
						if ((value.session or value.expires > timestamp) and (value.hostOnly and domain == u.host or !value.hostOnly)) {
							//是session cookie (expires为0) 或未过期 以及 hostonly cookie需要完全匹配
							matched_cookies.push_back(value);
						}
					}
				}
			}
		}
	}

	std::sort(matched_cookies.begin(), matched_cookies.end(), [](const Cookie& a, const Cookie& b) {
		if (a.path.length() == b.path.length())
			return a.creationTime < b.creationTime;
		return a.path.length() > b.path.length(); // 长 path 优先
		});

	for (const auto& i : matched_cookies) {
		cookie += i.name + "=" + i.value + "; ";
	}

	if (!cookie.empty()) {
		cookie = cookie.substr(0, cookie.length() - 2);
	}
	return cookie;
}

inline bool CookiesJar::is_matcheddomain(const std::string& host, const std::string& domain) {
	std::string h = Requests::trim(host);
	h = Requests::to_lower(h);
	std::string d = Requests::trim(domain);
	d = Requests::to_lower(d);

	size_t index = 0;
	while (index < d.length() and d[index] == '.')index++;

	d = d.substr(index, d.length() - index);

	//判断host是否为ip
	if (is_ip(h)) {
		return false;
	}
	//完全相同
	if (h == d) {
		return true;
	}
	if (h.length() <= d.length()) {
		return false;
	}
	//后缀不匹配
	if (h.substr(h.length() - d.length(), d.length()) != d) {
		return false;
	}
	//host匹配前一个字符不是.
	if (h[h.length() - d.length() - 1] != '.') {
		return false;
	}
	return true;
}

inline bool CookiesJar::is_matchedpath(const std::string& url_path, const std::string& path) {
	std::string up = Requests::trim(url_path);
	std::string p = Requests::trim(path);
	if (up.length() == 0)up = "/";
	if (p.length() == 0)p = "/";

	//完全相同
	if (up == p) {
		return true;
	}
	//up小、等则前缀肯定不匹配
	if (up.length() <= p.length()) {
		return false;
	}
	//前缀不匹配
	if (p != up.substr(0, p.length())) {
		return false;
	}
	//前缀匹配且cookie最后为/
	if (p.at(p.length() - 1) == '/') {
		return true;
	}
	//前缀匹配，cookie不为/但path后为/
	if (up.at(p.length()) == '/') {
		return true;
	}
	return false;
}

inline void CookiesJar::clear_expired_cookies() {
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
		now.time_since_epoch()
	).count();
	for (auto it1 = cookies.begin(); it1 != cookies.end();) {
		for (auto it2 = it1->second.begin(); it2 != it1->second.end();) {
			for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ) {
				const auto& value = it3->second;
				if (!value.session && value.expires <= timestamp) {
					it3 = it2->second.erase(it3); // 删除过期
				}
				else {
					++it3;
				}
			}
			if (it2->second.empty()) {
				it2 = it1->second.erase(it2);
			}
			else {
				++it2;
			}
		}
		if (it1->second.empty()) {
			it1 = cookies.erase(it1);
		}
		else {
			++it1;
		}
	}
}

inline void CookiesJar::clear_session_cookies() {
	for (auto it1 = cookies.begin(); it1 != cookies.end();) {
		for (auto it2 = it1->second.begin(); it2 != it1->second.end();) {
			for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ) {
				const auto& value = it3->second;
				if (value.session) {
					it3 = it2->second.erase(it3); // 删除session cookie
				}
				else {
					++it3;
				}
			}
			if (it2->second.empty()) {
				it2 = it1->second.erase(it2);
			}
			else {
				++it2;
			}
		}
		if (it1->second.empty()) {
			it1 = cookies.erase(it1);
		}
		else {
			++it1;
		}
	}
}

inline void CookiesJar::extract_cookies_from_setCookie(const std::string& url, const std::string& setCookie) {
	Cookie cookie = parseSetCookie(url, setCookie);
	if (!cookie.name.empty()) {
		//有效cookie
		if (!(!cookie.session and cookie.expires == 0)) {
			//非删除
			cookies[cookie.domain][cookie.path][cookie.name] = cookie;
		}
		else {
			this->pop(cookie.name, cookie.domain, cookie.path);
		}
	}
}

inline void CookiesJar::extract_cookies_from_header(const std::string& url, const json& header) {
	for (const auto& [key, value] : header.items()) {
		if (Requests::to_lower(key) == "set-cookie") {
			for (const auto& i : value) {
				extract_cookies_from_setCookie(url, i.get<std::string>());
			}
		}
	}
}

inline void CookiesJar::merge(const CookiesJar& jar, bool priority) {
	for (const auto& cookie : jar) {
		if (cookies.contains(cookie.domain) and cookies.at(cookie.domain).contains(cookie.path) and cookies.at(cookie.domain).at(cookie.path).contains(cookie.name)) {
			//存在此cookie
			if (!(!cookie.session and cookie.expires == 0)) {
				//非删除
				if (!priority) {
					cookies[cookie.domain][cookie.path][cookie.name] = cookie;
				}
			}
			else {
				if (!priority) {
					this->pop(cookie.name, cookie.domain, cookie.path);
				}
			}
		}
		else {
			cookies[cookie.domain][cookie.path][cookie.name] = cookie;
		}
	}
}

inline Cookie CookiesJar::parseSetCookie(const std::string& url, std::string set_cookie_string) {
	Cookie cookie;
	set_cookie_string = Requests::trim(set_cookie_string);
	if (set_cookie_string.find(";") != std::string::npos) {
		std::string key_value = set_cookie_string.substr(0, set_cookie_string.find(";"));
		std::string querys = set_cookie_string.substr(set_cookie_string.find(";") + 1);
		if (key_value.find("=") == std::string::npos) {
			//忽略此cookie 返回name为空的cookie表示无效
			return cookie;
		}
		ParsedUrl up = Requests::parse_url(url);
		cookie.name = Requests::trim(key_value.substr(0, key_value.find("=")));
		cookie.value = Requests::trim(key_value.substr(key_value.find("=") + 1));
		cookie.path = "";//默认构造为/为了避免影响后续判断，先置空

		time_t exp = 0;
		long long maxage = 0;
		bool found_exp = false, found_maxage = false;
		auto now = std::chrono::system_clock::now();
		auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
			now.time_since_epoch()
		).count();

		querys += ";";
		size_t start = 0;
		while (start < querys.size()) {
			size_t end = querys.find(";", start);
			if (end == std::string::npos) {
				break;
			}
			std::string t = querys.substr(start, end - start);
			t = Requests::to_lower(Requests::trim(t));

			size_t mid = querys.substr(start, end - start).find("=");

			if (mid == std::string::npos) {
				if (t == "httponly") {
					cookie.httpOnly = true;
				}
				else if (t == "session") {
					cookie.session = true;
				}
				else if (t == "hostonly") {
					cookie.hostOnly = true;
				}
				else if (t == "secure") {
					cookie.secure = true;
				}
				start = end + 1;
				continue;
			}
			mid += start;

			std::string key = querys.substr(start, mid - start);
			key = Requests::to_lower(Requests::trim(key));
			std::string value = querys.substr(mid + 1, end - mid - 1);
			value = Requests::trim(value);

			if (key == "expires") {
				exp = parseExpires(value);
				if (exp != -1) {
					found_exp = true;
				}
			}
			else if (key == "max-age") {
				try {
					maxage = std::stoi(value);
				}
				catch (...) {
					;
				}
				found_maxage = true;
			}
			else if (key == "domain") {
				if (!value.empty())
					cookie.domain = Requests::to_lower(value);
			}
			else if (key == "path") {
				if (!value.empty()) {
					cookie.path = value;
				}
				else {
					cookie.path = "/";
				}
			}
			else if (key == "samesite") {
				if (Requests::to_lower(value) == "none") {
					cookie.sameSite = SameSite::None;
				}
				else if (Requests::to_lower(value) == "lax") {
					cookie.sameSite = SameSite::Lax;
				}
				else if (Requests::to_lower(value) == "strict") {
					cookie.sameSite = SameSite::Strict;
				}
				else {
					cookie.sameSite = SameSite::Unset;
				}
			}

			start = end + 1;
		}

		if (!found_exp and !found_maxage) {
			cookie.session = true;
			cookie.expires = 0;
		}
		if (found_maxage) {
			if (maxage <= 0) {
				cookie.session = false;
				cookie.expires = 0;
			}
			else {

				cookie.expires = timestamp + maxage;
			}
		}
		if (found_exp and !found_maxage) {
			cookie.expires = exp;
		}
		if (cookie.domain.empty()) {
			cookie.domain = up.host;
		}
		if (cookie.path.empty()) {
			cookie.path = up.path;
			size_t first = up.path.find("/");
			size_t last = up.path.rfind("/");

			if (first == last) {
				cookie.path = "/";
			}
			else {
				cookie.path = up.path.substr(0, last);
			}
		}
		cookie.creationTime = timestamp;
		return cookie;

	}
	else {
		if (set_cookie_string.find("=") == std::string::npos) {
			//忽略此cookie 返回name为空的cookie表示无效
			return cookie;
		}
		ParsedUrl up = Requests::parse_url(url);
		cookie.domain = up.host;
		cookie.path = up.path;
		cookie.hostOnly = true;
		cookie.session = true;
		auto now = std::chrono::system_clock::now();
		auto creationTime = std::chrono::duration_cast<std::chrono::seconds>(
			now.time_since_epoch()
		).count();
		cookie.creationTime = creationTime;
		cookie.name = Requests::trim(set_cookie_string.substr(0, set_cookie_string.find("=")));
		cookie.value = Requests::trim(set_cookie_string.substr(set_cookie_string.find("=") + 1));
		return cookie;
	}
}

inline time_t CookiesJar::parseExpires(std::string& expires) {
	//解析失败返回-1
	std::vector<char> delimiters = { '\t',' ','!','\"','#','$','%','&','\'','(',')','*','+',',','-','.','/',';','<','=','>','?','@','[','\\',']','^','_','`','{','|','}','~' };
	std::string date = "";
	std::vector<std::string> dates;
	for (const auto& c : expires) {
		if (std::find(delimiters.begin(), delimiters.end(), c) != delimiters.end()) {
			if (!date.empty()) {
				dates.push_back(date);
				date = "";
			}
			continue;
		}
		else {
			date += c;
		}
	}
	if (!date.empty()) {
		dates.push_back(date);
		date = "";
	}
	bool found_time = false, found_day = false, found_month = false, found_year = false;
	int hour, min, sec, day, month, year;
	static const std::array<std::string, 12> months = {
	"jan","feb","mar","apr","may","jun",
	"jul","aug","sep","oct","nov","dec"
	};

	for (const auto& item : dates) {
		if (!found_time) {
			if (item.find(':') != std::string::npos) {
				std::string hour_string = item.substr(0, item.find(':'));
				std::string min_sec_string = item.substr(item.find(':') + 1);
				if (min_sec_string.find(':') != std::string::npos) {
					std::string min_string = min_sec_string.substr(0, min_sec_string.find(':'));
					std::string sec_string = min_sec_string.substr(min_sec_string.find(':') + 1);
					try {
						hour = std::stoi(hour_string);
						min = std::stoi(min_string);
						sec = std::stoi(sec_string);
						found_time = true;
						continue;
					}
					catch (...) {
						;
					}
				}
			}
		}
		if (!found_day) {
			int temp = 0;
			for (const char& c : item) {
				if (c >= '0' and c <= '9') {
					temp *= 10;
					temp += c - '0';
				}
				else {
					break;
				}
			}
			if (temp > 0 and temp <= 31) {
				day = temp;
				found_day = true;
				continue;
			}
		}
		if (!found_month) {
			if (std::find(months.begin(), months.end(), Requests::to_lower(item.substr(0, 3))) != months.end()) {
				month = std::find(months.begin(), months.end(), Requests::to_lower(item.substr(0, 3))) - months.begin() + 1;
				found_month = true;
				continue;
			}
		}
		if (!found_year) {
			int temp = 0;
			for (const char& c : item) {
				if (c >= '0' and c <= '9') {
					temp *= 10;
					temp += c - '0';
				}
				else {
					break;
				}
			}
			if (temp >= 70 and temp <= 99) {
				temp += 1900;
			}
			else if (temp >= 0 and temp <= 69) {
				temp += 2000;
			}
			if (temp >= 1000) {
				year = temp;
				found_year = true;
				continue;
			}
		}
	}

	if ((!found_day or !found_time or !found_month or !found_year) or (day < 1 or day>31) or (year < 1601) or (hour > 23) or (min > 59) or (sec > 59)) {
		return -1;
	}
	if ((month == 4 or month == 6 or month == 9 or month == 11) and day == 31) {
		return -1;
	}
	if (month == 2 and day > 29) {
		return -1;
	}
	if (!(year % 400 == 0 or (year % 4 == 0 and year % 100 != 0)) and day == 29) {
		return -1;
	}

	auto div_count = [](int n, int d) {
		return n / d;
		};
	int by4 = (year - 1) / 4 - (1970 - 1) / 4;
	int by100 = (year - 1) / 100 - (1970 - 1) / 100;
	int by400 = (year - 1) / 400 - (1970 - 1) / 400;

	time_t ts = 0;
	ts += (by4 - by100 + by400) * 366 * 24 * 60 * 60 + (year - 1970 - by4 + by100 - by400) * 365 * 24 * 60 * 60;
	std::vector<int> md1 = { 0,31,59,90,120,151,181,212,243,273,304,334,365 };
	std::vector<int> md2 = { 0,31,60,91,121,152,182,213,244,274,305,335,366 };
	if (year % 400 == 0 or (year % 4 == 0 and year % 100 != 0)) {
		ts += md2[month - 1] * 24 * 60 * 60;
	}
	else {
		ts += md1[month - 1] * 24 * 60 * 60;
	}
	ts += (day - 1) * 24 * 60 * 60 + hour * 60 * 60 + min * 60 + sec;
	return ts;
}

inline std::optional<std::pair<std::string, int>> Requests::parse_proxy(const std::string& proxy_raw) {
	/*
	parse_proxy:
	- 支持这些形式：
	  "127.0.0.1:1234"
	  "http://127.0.0.1:1234"
	  "http=127.0.0.1:1234;https=..."
	  "[::1]:8080"
	  "socks5://127.0.0.1:1080"
	- 返回 std::optional<pair<host,port>>
*/
	if (proxy_raw.empty()) return {};

	// split by ';' (Windows often uses per-protocol list separated by ';')
	std::vector<std::string> parts;
	size_t cur = 0;
	while (cur < proxy_raw.size()) {
		auto pos = proxy_raw.find(';', cur);
		if (pos == std::string::npos) {
			parts.push_back(proxy_raw.substr(cur));
			break;
		}
		else {
			parts.push_back(proxy_raw.substr(cur, pos - cur));
			cur = pos + 1;
		}
	}

	// pick preferred token: http=... or http://... first, otherwise first token
	std::string chosen;
	for (auto& p : parts) {
		std::string t = trim(p);
		std::string tl = to_lower(t);
		if (tl.rfind("http=", 0) == 0 || tl.rfind("http://", 0) == 0) {
			chosen = t;
			break;
		}
	}
	if (chosen.empty()) chosen = trim(parts[0]);

	// if it's like "http=127.0.0.1:1234", cut off "http="
	auto eqpos = chosen.find('=');
	if (eqpos != std::string::npos) chosen = trim(chosen.substr(eqpos + 1));

	// remove scheme like "http://", "socks5://"
	auto schpos = chosen.find("://");
	if (schpos != std::string::npos) chosen = trim(chosen.substr(schpos + 3));

	// now chosen should be like "127.0.0.1:1234" or "[::1]:8080"
	std::string host, portstr;
	if (!chosen.empty() && chosen.front() == '[') {
		// IPv6 like [::1]:8080
		auto rb = chosen.find(']');
		if (rb == std::string::npos) return {};
		host = chosen.substr(1, rb - 1); // remove brackets
		if (rb + 1 < chosen.size() && chosen[rb + 1] == ':') {
			portstr = chosen.substr(rb + 2);
		}
		else return {};
	}
	else {
		// use last ':' to split host:port (rfind to survive IPv6 without brackets)
		auto pos = chosen.rfind(':');
		if (pos == std::string::npos) return {};
		host = chosen.substr(0, pos);
		portstr = chosen.substr(pos + 1);
	}
	host = trim(host);
	portstr = trim(portstr);
	if (host.empty() || portstr.empty()) return {};

	// strip possible surrounding brackets for host just in case
	if (host.size() >= 2 && host.front() == '[' && host.back() == ']') {
		host = host.substr(1, host.size() - 2);
	}

	try {
		int port = std::stoi(portstr);
		return std::make_pair(host, port);
	}
	catch (...) {
		return {};
	}
}

inline std::string Requests::get_system_proxy() {
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
		0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD proxyEnable = 0;
		DWORD size = sizeof(proxyEnable);
		RegQueryValueExA(hKey, "ProxyEnable", nullptr, nullptr, (LPBYTE)&proxyEnable, &size);

		if (proxyEnable) {
			char proxyServer[256];
			DWORD bufSize = sizeof(proxyServer);
			if (RegQueryValueExA(hKey, "ProxyServer", nullptr, nullptr, (LPBYTE)proxyServer, &bufSize) == ERROR_SUCCESS) {
				RegCloseKey(hKey);
				return std::string(proxyServer);
			}
		}
		RegCloseKey(hKey);
	}
	return {};
}

inline ParsedUrl Requests::parse_url(const std::string& url) {
	std::string url_c = trim(url);
	if (url_c.find("://") == std::string::npos) {
		throw std::invalid_argument("URL 缺少协议部分");
	}

	ParsedUrl result;
	result.protocol = url_c.substr(0, url_c.find("://"));
	std::string rest = url_c.substr(url_c.find("://") + 3);

	size_t slash_pos = rest.find("/");
	std::string query_str;
	if (slash_pos == std::string::npos) {
		result.path = "/";
		if (rest.find("?") == std::string::npos) {
			result.host = rest;
			query_str = "";
		}
		else {
			result.host = rest.substr(0, rest.find("?"));
			query_str = rest.substr(rest.find("?") + 1);
		}
	}
	else {
		result.host = rest.substr(0, slash_pos);
		std::string full_path = rest.substr(slash_pos);

		size_t qpos = full_path.find("?");
		if (qpos != std::string::npos) {
			result.path = full_path.substr(0, qpos);
			query_str = full_path.substr(qpos + 1);
		}
		else {
			result.path = full_path;
			query_str = "";
		}
	}

	size_t start = 0;
	query_str = query_str + "&";
	while (start < query_str.size()) {
		size_t end = query_str.find("&", start);
		if (end == std::string::npos) {
			break;
		}

		size_t mid = query_str.substr(start, end - start).find("=");

		if (mid == std::string::npos) {
			start = end + 1;
			continue;
		}

		mid += +start;

		std::string key = query_str.substr(start, mid - start);
		std::string value = query_str.substr(mid + 1, end - mid - 1);
		result.query.insert(std::make_pair(key, value));
		start = end + 1;
	}
	return result;
}

inline Response Requests::get(std::string url, GetOptions op, GetCallBackOptions cop) {
	Response r = op.response;
	size_t retryCount = 0, redirectCount = 0;

	json headers = parse_headers(op.headers);
	json params = parse_headers(op.params);
#ifdef QT_DEBUG_URL
	qDebug() << "url: " << QString::fromStdString(url);
#endif
#ifdef DEBUG_URL
	Debug() << "url: " << url;
#endif
#ifdef QT_DEBUG_HEADERS
	qDebug() << "headers: " << QString::fromStdString(headers.dump(2));
#endif
#ifdef DEBUG_HEADERS
	Debug() << "headers: " << headers.dump(2);
#endif
#ifdef QT_DEBUG_PARAMS
	qDebug() << "params: " << QString::fromStdString(params.dump(2));
#endif
#ifdef DEBUG_PARAMS
	Debug() << "params: " << params.dump(2);
#endif

	std::string last_url = url;

	while (retryCount < op.maxRetries and redirectCount < 30) {
		r = getOnce(last_url, r, headers, params, op.allow_redirects, op.allowProxies, op.readTimeout, op.connectionTimeout, cop.ResponseHandler, cop.ContentReceiver, cop.DownloadProgress);
		if (r) {
			if (op.allow_redirects and r.status_code >= 300 and r.status_code < 400) {
				redirectCount++;
				last_url = get_redirects_url(last_url, r.headers);
				continue;
			}
			else {
#ifdef QT_DEBUG_CONTENT
				qDebug() << "content: " << QString::fromStdString(r.content);
#endif
#ifdef DEBUG_CONTENT
				Debug() << "content: " << r.content;
#endif
				return r;
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::seconds(op.retryDelay));
			retryCount++;
		}
	}
	if (redirectCount == 30 and !op.no_exception) {
		throw std::exception("重定向达到上限");
	}
	if (!op.no_exception) {
		throw std::exception("连接失败");
	}
	return r;
}

inline Response Requests::getOnce(std::string url, Response& res, json headers, json params, bool allow_redirects, bool allowProxies, int readTimeout, int connectionTimeout, std::function<bool(const httplib::Response&)> ResponseHandler, std::function<bool(const char*, size_t)> ContentReceiver, std::function<bool(uint64_t, uint64_t)> DownloadProgress) {
	//解析url
	std::string protocol;
	std::string host;
	std::string path;

	auto result = parse_url(url);
	protocol = result.protocol;
	host = result.host;
	path = result.path;

	httplib::Client cli(protocol + "://" + host);

	cli.set_read_timeout(readTimeout, 0);
	cli.set_connection_timeout(connectionTimeout, 0);
	cli.set_follow_location(false);
	if (headers.contains("user-agent")) {
		httplib::user_agent_override = headers["user-agent"][0];
	}
	else {
		httplib::user_agent_override = "cpp-httplib/0.26.0";
	}
	//将json headers转化成httplib headers
	httplib::Headers header;
	if (res) {
		header.emplace("cookie", res.cookies.to_header(url));
	}
	for (const auto& [key, value] : headers.items()) {
		for (const auto& i : value) {
			header.insert(std::make_pair(key, i.get<std::string>()));
		}
	}

	//将params拼接到path里
	if (!res) {
		for (const auto& [key, value] : params.items()) {
			for (const auto& item : value) {
				result.query.insert(std::make_pair(key, item.get<std::string>()));
			}
		}
	}
	if (result.query.size() != 0) {
		path += '?';
		for (const auto& item : result.query) {
			path += item.first + "=" + item.second + "&";
		}
		path = path.substr(0, path.length() - 1);
	}

	if (allowProxies) {
		//处理代理
		std::string proxy = get_system_proxy();
		if (!proxy.empty()) {
			auto r = parse_proxy(proxy);
			cli.set_proxy(r->first, r->second);
		}
	}
	httplib::Result r;

	if (ContentReceiver == nullptr and DownloadProgress == nullptr and ResponseHandler == nullptr) {
		r = cli.Get(path, header);
	}
	else {
		if (DownloadProgress == nullptr) {
			DownloadProgress = [&](uint64_t current, uint64_t total) {return true; };
		}
		if (ResponseHandler == nullptr) {
			ResponseHandler = [&](const httplib::Response& res) {return true; };
		}
		if (ContentReceiver != nullptr) {
			r = cli.Get(
				path,
				header,
				[&](const httplib::Response& res) {
					if (res.status >= 300 and res.status < 400) {
						//return !allow_redirects;
						return true;
					}
					return ResponseHandler(res);
				},
				[&](const char* data, size_t len) {
					return ContentReceiver(data, len);
				},
				[&](uint64_t current, uint64_t total) {
					return DownloadProgress(current, total);
				}
			);
		}
		else {
			std::string buf;
			r = cli.Get(
				path,
				header,
				[&](const httplib::Response& res) {
					return ResponseHandler(res);
				},
				[&](const char* data, size_t len) {
					buf.append(data, len);
					return true;
				},
				[&](uint64_t current, uint64_t total) {
					return DownloadProgress(current, total);
				}
			);
			if (r) r->body = buf;
		}
	}

	Response new_res(url, r);
	new_res.cookies.merge(res.cookies);
	new_res.history = res.history;
	new_res.history.push_back(new_res);
	new_res.history.at(new_res.history.size() - 1).history = {};

	return new_res;
}

inline json Requests::parse_headers(const json& headers) {
	json header = json::object();
	if (!headers.is_object()) {
		return header;
	}
	for (const auto& [key, value] : headers.items()) {
		std::string lower_key = to_lower(key);
		header[lower_key] = json::array();
		if (value.is_array()) {
			for (const auto& item : value) {
				if (item.is_string()) {
					header[lower_key].push_back(item);
				}
			}
		}
		else if (value.is_string()) {
			header[lower_key].push_back(value);
		}
		else {
			continue;
		}
	}
	return header;
}

inline std::string Requests::get_redirects_url(const std::string& url, const json& headers) {
	std::string location_url = headers.at("location").at(0);
	ParsedUrl up = parse_url(url);

	if (location_url.find("://") == std::string::npos) {
		//相对路径
		location_url = trim(location_url);
		if (location_url.empty()) {
			return url;
		}
		//协议基准
		if (location_url.length() >= 2 and location_url.at(0) == '/' and location_url.at(1) == '/') {
			if (location_url.length() == 2) return url;
			return up.protocol + "://" + location_url.substr(2);
		}
		//host基准
		else if (location_url.at(0) == '/') {
			return up.protocol + "://" + up.host + location_url;
		}
		//相对路径
		else if (location_url.at(0) == '.') {
			size_t t = 0;
			for (const char& c : location_url) {
				if (c == '.') {
					t++;
				}
				else {
					location_url = location_url.substr(t);
					break;
				}
			}
			std::string path = up.path;
			while (t--) {
				if (path.rfind("/") != std::string::npos) {
					if (path.length() == 1) path = "";
					else path = path.substr(0, path.rfind("/"));
				}
				else {
					//已经退回根路径
					return up.protocol + "://" + up.host + location_url;
				}
			}
			return up.protocol + "://" + up.host + path + location_url;
		}
		//相同路径
		else {
			std::string path = up.path;
			if (path.rfind("/") != std::string::npos) {
				if (path.length() == 1) path = "";
				else path = path.substr(0, path.rfind("/"));
			}
			else {
				//已经退回根路径
				return up.protocol + "://" + up.host + location_url;
			}
			return up.protocol + "://" + up.host + path + location_url;
		}
	}
	else {
		return location_url;
	}
}

inline Response Requests::post(std::string url, PostOptioms op, PostCallBackOptions cop) {
	//优先级 multipart > raw > file > json > xml > urlencode > text
	//先只实现json后的，get重定向先不处理
	Response r = {};
	size_t retryCount = 0, redirectCount = 0;

	json headers = parse_headers(op.headers);
#ifdef QT_DEBUG_URL
	qDebug() << "url: " << QString::fromStdString(url);
#endif
#ifdef DEBUG_URL
	Debug() << "url: " << url;
#endif
#ifdef QT_DEBUG_HEADERS
	qDebug() << "headers: " << QString::fromStdString(headers.dump(2));
#endif
#ifdef DEBUG_HEADERS
	Debug() << "headers: " << headers.dump(2);
#endif
	std::string postBody = {};
	std::string contentType = {};

	if (!op.Json.empty()) {
		postBody = op.Json.dump();
		contentType = "application/json";
	}
	else if (!op.xml.empty()) {
		postBody = op.xml;
		contentType = "application/xml";
	}
	else if (!op.urlencode.empty()) {
		postBody = op.urlencode;
		contentType = "application/x-www-form-urlencoded";
	}
	else if (!op.text.empty()) {
		postBody = op.text;
		contentType = "text/plain";
	}
	else {
		postBody = "";
		contentType = "";
	}

#ifdef QT_DEBUG_PARAMS
	qDebug() << "params: " << QString::fromStdString(postBody);
#endif
#ifdef DEBUG_PARAMS
	Debug() << "params: " << postBody;
#endif
	bool isGet = false;

	std::string last_url = url;
	while (retryCount < op.maxRetries and redirectCount < 30) {
		if (isGet) {
			r = getOnce(last_url, r, headers, json::object(), false, op.allowProxies, op.readTimeout, op.connectionTimeout, nullptr, nullptr, nullptr);
		}
		else {
			r = postOnce(last_url, r, contentType, headers, postBody, "", op.allow_redirects, op.allowProxies, op.readTimeout, op.connectionTimeout);
		}
		if (r) {
			if (op.allow_redirects and r.status_code >= 300 and r.status_code < 400) {
				redirectCount++;
				last_url = get_redirects_url(last_url, r.headers);
				if (r.status_code == 303) {
					isGet = true;
					//重定向到get
				}
				continue;
			}
			else {
#ifdef QT_DEBUG_CONTENT
				qDebug() << "content: " << QString::fromStdString(r.content);
#endif
#ifdef DEBUG_CONTENT
				Debug() << "content: " << r.content;
#endif
				return r;
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::seconds(op.retryDelay));
			retryCount++;
		}
	}
	if (redirectCount == 30 and !op.no_exception) {
		throw std::exception("重定向达到上限");
	}
	if (!op.no_exception) {
		throw std::exception("连接失败");
	}
	return r;
}

inline Response Requests::postOnce(std::string url, Response& res, std::string& contentType, json headers, std::string body, std::filesystem::path file, bool allow_redirects, bool allowProxies, int readTimeout, int connectionTimeout) {
	//解析url
	std::string protocol;
	std::string host;
	std::string path;

	auto result = parse_url(url);
	protocol = result.protocol;
	host = result.host;
	path = result.path;

	httplib::Client cli(protocol + "://" + host);

	cli.set_read_timeout(readTimeout, 0);
	cli.set_connection_timeout(connectionTimeout, 0);
	cli.set_follow_location(false);

	if (headers.contains("user-agent")) {
		httplib::user_agent_override = headers["user-agent"][0];
	}
	else {
		httplib::user_agent_override = "cpp-httplib/0.26.0";
	}
	//将json headers转化成httplib headers
	httplib::Headers header;
	if (res) {
		header.emplace("cookie", res.cookies.to_header(url));
	}

	for (const auto& [key, value] : headers.items()) {
		for (const auto& i : value) {
			if (key != "content-type") {
				header.insert(std::make_pair(key, i.get<std::string>()));
			}
		}
	}

	if (result.query.size() != 0) {
		path += '?';
		for (const auto& item : result.query) {
			path += item.first + "=" + item.second + "&";
		}
		path = path.substr(0, path.length() - 1);
	}

	if (allowProxies) {
		//处理代理
		std::string proxy = get_system_proxy();
		if (!proxy.empty()) {
			auto r = parse_proxy(proxy);
			cli.set_proxy(r->first, r->second);
		}
	}

	httplib::Result r;
	r = cli.Post(path, header, body, contentType);

	Response new_res(url, r);
	new_res.history = res.history;
	new_res.history.push_back(new_res);
	new_res.history.at(new_res.history.size() - 1).history = {};

	return new_res;
}