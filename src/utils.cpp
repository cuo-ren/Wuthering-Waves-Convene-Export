#include "utils.h"
#include "Notifier.h"

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
	std::filesystem::path fsPath = std::filesystem::path(std::u8string(filepath.data(), filepath.data() + filepath.size()));

	std::ifstream file(fsPath, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open file: " + filepath);
	}

	picosha2::hash256_one_by_one hasher;
	hasher.init();

	std::vector<unsigned char> buffer(8192);  // 8KB 缓冲区
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

std::string sha256_file_streaming(const std::u8string& filepath) {
	std::filesystem::path fsPath = std::filesystem::path(filepath);

	std::ifstream file(fsPath, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open file: " + std::string(filepath.data(), filepath.data() + filepath.size()));
	}

	picosha2::hash256_one_by_one hasher;
	hasher.init();

	std::vector<unsigned char> buffer(8192);  // 8KB 缓冲区
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
	UINT acp = GetACP();
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
	UINT acp = GetACP();
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
	UINT acp = GetACP();
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
	UINT acp = GetACP();
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
	std::filesystem::path fsPath = std::filesystem::path(std::u8string(path.data(), path.data() + path.size()));

	std::ifstream file(fsPath);  // 这里会调用 _wfopen 支持 UTF-16 路径
	if (!file.is_open()) {
		qCritical().noquote() << "文件打开失败! " << "path:" << QString::fromUtf8(path);
		throw std::runtime_error("无法打开文件: " + path);
	}
	json data;
	try {
		file >> data;
	}
	catch (const json::parse_error& e) {
		qWarning().noquote() << "json解析失败: " << QString::fromLocal8Bit(e.what());
		throw;
	}
	catch (...) {
		qCritical().noquote() << "文件读取发生未知错误 " << "path:" << QString::fromUtf8(path);
		throw;
	}
	return data;
}

json ReadJsonFile(const std::filesystem::path& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		qCritical().noquote() << "文件打开失败! " << "path:" << QString::fromStdString(path.string());
		throw std::runtime_error("无法打开文件: " + path.string());
	}
	json data;
	try {
		file >> data;
	}
	catch (const json::parse_error& e) {
		qWarning().noquote() << "json解析失败: " << QString::fromLocal8Bit(e.what());
		throw e;
	}
	catch (...) {
		qCritical().noquote() << "文件读取发生未知错误 " << "path:" << QString::fromStdString(path.string());
		throw;
	}
	return data;
}

void WriteJsonFile(const std::string& path, const json& data) {
	std::filesystem::path fsPath = std::filesystem::path(std::u8string(path.data(), path.data() + path.size()));

	std::ofstream f(fsPath, std::ios::binary);  // 用 fs::path 保证 UTF-16 路径
	f.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	if (!f.is_open()) {
		qCritical().noquote() << "文件打开失败! " << "path:" << QString::fromUtf8(path);
		Notifier::instance().notify(3, "文件打开失败! ");
		return;
	}

	try {
		f << data.dump(2);
	}
	catch (const json::type_error& e) {
		qWarning().noquote() << "json解析失败: " << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(3, "json解析失败 ");
	}
	catch (const std::ios_base::failure& e) {
		qCritical().noquote() << "文件写入失败!" << "path:" << QString::fromUtf8(path) << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(3, "文件写入失败!");
	}
	catch (...) {
		qCritical().noquote() << "文件写入发生未知错误" << "path:" << QString::fromUtf8(path);
		Notifier::instance().notify(3, "文件写入发生未知错误");
	}
}

void WriteJsonFile(const std::filesystem::path& path, const json& data) {
	std::ofstream f(path, std::ios::binary);
	f.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	if (!f.is_open()) {
		qCritical().noquote() << "文件打开失败! " << "path:" << QString::fromStdString(path.string());
		Notifier::instance().notify(3, "文件打开失败! ");
		return;
	}

	try {
		f << data.dump(2);
	}
	catch (const json::type_error& e) {
		qWarning().noquote() << "json解析失败: " << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(3, "json解析失败 ");
	}
	catch (const std::ios_base::failure& e) {
		qCritical().noquote() << "文件写入失败!" << "path:" << QString::fromStdString(path.string()) << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(3, "文件写入失败!");
	}
	catch (...) {
		qCritical().noquote() << "文件写入发生未知错误" << "path:" << QString::fromStdString(path.string());
		Notifier::instance().notify(3, "文件写入发生未知错误");
	}
}

bool makedirs(const std::string& path) {
	std::filesystem::path fsPath = std::filesystem::path(std::u8string(path.data(), path.data() + path.size()));

	std::error_code ec;

	std::filesystem::path current;
	//逐层检查是否是文件夹
	for (const auto& part : fsPath) {
		current /= part;
		//避免出现符号链接
		if (std::filesystem::is_symlink(current)) {
			qFatal("路径包含符号链接，可能存在风险: %s", current.string().c_str());
			return false;
		}
		if (std::filesystem::exists(current)) {
			//存在文件夹同名文件
			if (std::filesystem::is_regular_file(current)) {
				qWarning() << "存在文件夹同名文件" << QString::fromStdString(current.string()) << "尝试删除";
				std::filesystem::remove(current, ec);
				if (ec) {
					qCritical().noquote() << "删除文件失败: " << QString::fromStdString(current.string()) << " " << ec.message();
					return false;
				}
			}
		}
		else {
			break;
		}
	}
	if (!std::filesystem::exists(fsPath)) {
		std::filesystem::create_directories(fsPath, ec);
		if (ec) {
			qCritical().noquote() << "创建目录失败: " << QString::fromStdString(fsPath.string()) << " " << ec.message();
			return false;
		}
	}
	return true;
}

bool makedirs(const std::u8string& path) {
	std::filesystem::path fsPath = std::filesystem::path(path);
	std::error_code ec;
	std::filesystem::path current;
	//逐层检查是否是文件夹
	for (const auto& part : fsPath) {
		current /= part;
		//避免出现符号链接
		if (std::filesystem::is_symlink(current)) {
			qFatal("路径包含符号链接，可能存在风险: %s", current.string().c_str());
			return false;
		}
		if (std::filesystem::exists(current)) {
			//存在文件夹同名文件
			if (std::filesystem::is_regular_file(current)) {
				qWarning() << "存在文件夹同名文件" << QString::fromStdString(current.string()) << "尝试删除";
				std::filesystem::remove(current, ec);
				if (ec) {
					qCritical().noquote() << "删除文件失败: " << QString::fromStdString(current.string()) << " " << ec.message();
					return false;
				}
			}
		}
		else {
			break;
		}
	}
	if (!std::filesystem::exists(fsPath)) {
		std::filesystem::create_directories(fsPath, ec);
		if (ec) {
			qCritical().noquote() << "创建目录失败: " << QString::fromStdString(fsPath.string()) << " " << ec.message();
			return false;
		}
	}
	return true;
}

std::string current_time_str() {
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

std::string timestamp_to_str(int timestamp) {
	QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);
	return dt.toString("yyyy-MM-dd HH:mm:ss").toStdString();
}

void reset_folder(const std::string& path) {
	std::filesystem::path folder_path = std::filesystem::path(std::u8string(path.data(), path.data() + path.size()));

	// 如果文件夹存在，删除整个文件夹（包括内容）
	if (std::filesystem::exists(folder_path)) {
		std::error_code ec;
		std::filesystem::remove_all(folder_path, ec);
		if (ec) {
			qCritical() << "删除文件夹失败" << QString::fromStdString(ec.message());
			throw std::runtime_error("删除文件夹失败" + ec.message());
		}
	}

	// 创建空文件夹
	std::error_code ec;
	std::filesystem::create_directory(folder_path, ec);
	if (ec) {
		qCritical() << "创建文件夹失败" << QString::fromStdString(ec.message());
		throw std::runtime_error("创建文件夹失败" + ec.message());
	}
}

void reset_folder(const std::filesystem::path& path) {
	// 如果文件夹存在，删除整个文件夹（包括内容）
	if (std::filesystem::exists(path)) {
		std::error_code ec;
		std::filesystem::remove_all(path, ec);
		if (ec) {
			qCritical() << "删除文件夹失败" << QString::fromStdString(ec.message());
			throw std::runtime_error("删除文件夹失败" + ec.message());
		}
	}

	// 创建空文件夹
	std::error_code ec;
	std::filesystem::create_directory(path, ec);
	if (ec) {
		qCritical() << "创建文件夹失败" << QString::fromStdString(ec.message());
		throw std::runtime_error("创建文件夹失败" + ec.message());
	}
}