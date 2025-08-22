#include "utils.h"
#include "ErrorNotifier.h"

std::string RunAndGetOutput(const std::string& exePath) {
	HANDLE hRead, hWrite;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	// 创建匿名管道
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		std::cerr << "CreatePipe failed.\n";
		return "";
	}

	// 设置写端继承属性为 false
	SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

	// 设置启动信息以重定向输出
	STARTUPINFOA si = { sizeof(si) };
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWrite;
	si.hStdError = hWrite;
	si.hStdInput = NULL;

	PROCESS_INFORMATION pi;

	// 创建进程
	if (!CreateProcessA(
		NULL,
		const_cast<LPSTR>(exePath.c_str()),  // 可修改的字符串
		NULL, NULL, TRUE,
		CREATE_NO_WINDOW,//可改为0
		NULL, NULL,
		&si, &pi))
	{
		std::cerr << "CreateProcess failed.\n";
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return "";
	}

	// 关闭写端以防死锁
	CloseHandle(hWrite);

	// 读取输出
	char buffer[4096];
	DWORD bytesRead;
	std::string output;

	while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead != 0) {
		buffer[bytesRead] = '\0';
		output += buffer;
	}

	// 等待子进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 清理句柄
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hRead);

	return output;
}

std::int64_t get_timestamp() {
	using namespace std::chrono;
	return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

std::string sha256_file_streaming(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::binary);
	if (!file) throw std::runtime_error("Cannot open file: " + filepath);

	picosha2::hash256_one_by_one hasher;
	hasher.init();

	std::vector<unsigned char> buffer(8192);  // 8KB缓存
	while (file) {
		file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
		std::streamsize read_bytes = file.gcount();
		if (read_bytes > 0) {
			hasher.process(buffer.begin(), buffer.begin() + read_bytes);
		}
	}

	hasher.finish();
	std::vector<unsigned char> hash(picosha2::k_digest_size);
	hasher.get_hash_bytes(hash.begin(), hash.end());

	return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

std::string local_to_utf8(const std::string& gbk) {
	UINT acp = GetConsoleOutputCP();
	if (acp == CP_UTF8) {
		return gbk;
	}
	int wide_len = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, nullptr, 0);
	std::wstring wide_str(wide_len, 0);
	MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, &wide_str[0], wide_len);

	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8_str(utf8_len, 0);
	WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &utf8_str[0], utf8_len, nullptr, nullptr);
	utf8_str.pop_back();
	return utf8_str;
}

std::string utf8_to_local(const std::string& utf8) {
	UINT acp = GetConsoleOutputCP();
	if (acp == CP_UTF8) {
		// 当前系统 ACP 是 UTF-8，说明 utf8 本身就是目标编码
		return utf8;  // 无需转换
	}
	// 第一步：UTF-8 转宽字符（UTF-16）
	int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
	if (wide_len <= 0) return "";

	std::wstring wide_str(wide_len, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide_str[0], wide_len);

	// 第二步：宽字符转 CP_ACP（如 GBK）
	int gbk_len = WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (gbk_len <= 0) return "";

	std::string gbk_str(gbk_len, 0);
	WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, &gbk_str[0], gbk_len, nullptr, nullptr);
	gbk_str.pop_back();
	return gbk_str;
}

std::string gbk_to_local(const std::string& gbk) {
	UINT acp = GetConsoleOutputCP();
	if (acp == 936) {
		return gbk;
	}
	int wide_len = MultiByteToWideChar(936 /*CP_GBK*/, 0, gbk.c_str(), -1, nullptr, 0);
	if (wide_len <= 0) return "";

	std::wstring wide_str(wide_len, 0);
	MultiByteToWideChar(936, 0, gbk.c_str(), -1, &wide_str[0], wide_len);

	int local_len = WideCharToMultiByte(acp, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (local_len <= 0) return "";

	std::string local_str(local_len, 0);
	WideCharToMultiByte(acp, 0, wide_str.c_str(), -1, &local_str[0], local_len, nullptr, nullptr);
	local_str.pop_back();
	return local_str;
}

// 当前 ACP -> GBK
std::string local_to_gbk(const std::string& local) {
	UINT acp = GetConsoleOutputCP();
	if (acp == 936) {
		return local;
	}
	
	int wide_len = MultiByteToWideChar(acp, 0, local.c_str(), -1, nullptr, 0);
	if (wide_len <= 0) return "";

	std::wstring wide_str(wide_len, 0);
	MultiByteToWideChar(acp, 0, local.c_str(), -1, &wide_str[0], wide_len);

	int gbk_len = WideCharToMultiByte(936 /*CP_GBK*/, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (gbk_len <= 0) return "";

	std::string gbk_str(gbk_len, 0);
	WideCharToMultiByte(936, 0, wide_str.c_str(), -1, &gbk_str[0], gbk_len, nullptr, nullptr);
	gbk_str.pop_back();
	return gbk_str;
}

json ReadJsonFile(const std::string& path) {
	std::ifstream f(path);
	if (!f) {
		qCritical() << "文件打开失败! " << "path:" << QString::fromUtf8(path);
		throw std::runtime_error("无法打开文件: " + path);
	}
	json data;
	try {
		f >> data;
	}
	catch (const json::parse_error& e) {
		qWarning() << "json解析失败: " << e.what();
		throw;
	}
	catch (...) {
		qCritical() << "文件写入发生未知错误 " << "path:" << QString::fromUtf8(path);
		throw;
	}
	return data;
}

void WriteJsonFile(const std::string& path, const json& data) {
	std::ofstream f(path);
	f.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	if (!f) {
		qCritical() << "文件打开失败! " << "path:" << QString::fromUtf8(path);
		ErrorNotifier::instance().notifyError("文件打开失败! ");
	}
	try {
		f << data.dump(2);
	}
	catch (const json::type_error& e) {
		qWarning() << "json解析失败: " << e.what();
		ErrorNotifier::instance().notifyError("json解析失败 ");
	}
	catch (const std::ios_base::failure& e) {
		qCritical() << "文件写入失败! " << "path:" << QString::fromUtf8(path);
		ErrorNotifier::instance().notifyError("文件写入失败!");
	}
	catch (...) {
		qCritical() << "文件写入发生未知错误 " << "path:" << QString::fromUtf8(path);
		ErrorNotifier::instance().notifyError("文件写入发生未知错误");
	}
}

void makedirs(const std::string& path) {
	std::error_code ec; // 防止抛异常
	if (!std::filesystem::exists(path)) {
		if (!std::filesystem::create_directories(path, ec)) {
			if (ec) {
				std::cerr << "Failed to create directory '" << path << "': " << ec.message() << '\n';
			}
		}
	}
}

std::string current_time_str() {/*
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	char buf[20];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
	return std::string(buf);*/
	return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
}

bool compareByTime(const json& a, const json& b) {
	return a["time"].get<std::string>() < b["time"].get<std::string>();
}

bool is_digit(const std::string& s) {
	if (s.empty()) {
		return true;
	}
	for (char i : s) {
		if (i < '0' or i>'9') {
			return false;
		}
	}
	return true;
}

std::string timestamp_to_str(int timestamp) {/*
	std::time_t t = static_cast<std::time_t>(timestamp);  // 将 int 转为 time_t
	std::tm* tm_ptr = std::localtime(&t);                 // 转为本地时间
	std::ostringstream oss;
	oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");    // 格式化输出
	return oss.str();*/
	QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);
	return dt.toString("yyyy-MM-dd HH:mm:ss").toStdString();
}