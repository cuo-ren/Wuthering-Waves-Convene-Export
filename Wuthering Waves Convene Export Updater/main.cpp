#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#include "logger.h"
#include "file_log_handler.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <windows.h>
#include <tlhelp32.h>
#include <unordered_set>
#include "json.hpp"


namespace fs = std::filesystem;
using json = nlohmann::json;

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

bool makedirs(const std::string& path) {
	std::filesystem::path fsPath = std::filesystem::u8path(path);
	std::error_code ec;

	std::filesystem::path current;
	//逐层检查是否是文件夹
	for (const auto& part : fsPath) {
		current /= part;
		//避免出现符号链接
		if (std::filesystem::is_symlink(current)) {
            std::cerr << "路径包含符号链接，可能存在风险 " << current;
            abort();
			return false;
		}
		if (std::filesystem::exists(current)) {
			//存在文件夹同名文件
			if (std::filesystem::is_regular_file(current)) {
                std::cerr << "存在文件夹同名文件" << current.string() << "尝试删除";
				std::filesystem::remove(current, ec);
				if (ec) {
                    std::cerr << "删除文件失败: " << current.string() << " " << ec.message();
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
            std::cerr << "创建目录失败: " << fsPath.string() << " " << ec.message();
			return false;
		}
	}
	return true;
}

bool isSubPath(const std::filesystem::path& base, const std::filesystem::path& target) {
    auto baseCanonical = std::filesystem::weakly_canonical(base);
    auto targetCanonical = std::filesystem::weakly_canonical(target);

    auto baseIt = baseCanonical.begin();
    auto targetIt = targetCanonical.begin();

    for (; baseIt != baseCanonical.end() && targetIt != targetCanonical.end(); ++baseIt, ++targetIt) {
        if (*baseIt != *targetIt)
            return false;
    }

    return baseIt == baseCanonical.end(); // base 完全匹配 target 的前缀
}

json ReadJsonFile(const std::string& path) {
    fs::path fsPath = std::filesystem::u8path(path);
    std::ifstream file(fsPath);  // 这里会调用 _wfopen 支持 UTF-16 路径
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + path);
    }
    json data;
    try {
        file >> data;
    }
    catch (const json::parse_error& e) {
        throw;
    }
    catch (...) {
        throw;
    }
    return data;
}

// 将 UTF-8 转换为本地编码（ACP，如 GBK）
std::string utf8_to_local(const std::string& utf8) {
    UINT acp = GetACP();
    if (acp == CP_UTF8) {
        return utf8;
    }

    int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (wide_len <= 0) return "";

    std::wstring wide_str(wide_len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide_str[0], wide_len);

    int gbk_len = WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (gbk_len <= 0) return "";

    std::string gbk_str(gbk_len, 0);
    WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, &gbk_str[0], gbk_len, nullptr, nullptr);
    gbk_str.pop_back(); // 去掉多余的 '\0'
    return gbk_str;
}

// 弹窗提示（自动转成本地编码）
void show_message(const std::string& msg, UINT type = MB_ICONERROR) {
    std::string local = utf8_to_local(msg);
    std::string title = utf8_to_local("updater");
    MessageBoxA(nullptr, local.c_str(), title.c_str(), MB_OK | type);
}

// 判断进程是否存在
bool process_exists(DWORD pid) {
    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (hProcess) {
        DWORD ret = WaitForSingleObject(hProcess, 0);
        CloseHandle(hProcess);
        return ret == WAIT_TIMEOUT; // WAIT_TIMEOUT 表示仍在运行
    }
    return false;
}

// 强制结束进程
bool force_kill(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) return false;
    bool ok = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return ok;
}

// 解析参数（带重复检测）
bool parse_args(int argc, char* argv[], DWORD& pid, std::string& path, std::string& updatePath, int& timeout) {
    timeout = 10; // 默认值
    std::unordered_set<std::string> seen;
    for (int i = 1; i < argc; i++) {
        Debug() << "参数 " << i << " " << argv[i];
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-pid" && i + 1 < argc) {
            if (seen.count("-pid")) return false;
            seen.insert("-pid");
            pid = std::stoul(local_to_utf8(argv[++i]));
        }
        else if (arg == "-path" && i + 1 < argc) {
            if (seen.count("-path")) return false;
            seen.insert("-path");
            path = local_to_utf8(argv[++i]);
        }
        else if (arg == "-timeout" && i + 1 < argc) {
            if (seen.count("-timeout")) return false;
            seen.insert("-timeout");
            timeout = std::stoi(local_to_utf8(argv[++i]));
        }
        else if (arg == "-updatePath" && i + 1 < argc) {
            if (seen.count("-updatePath")) return false;
            seen.insert("-updatePath");
            updatePath = local_to_utf8(argv[++i]);
        }
        else {
            return false;
        }
    }
    return pid != 0 && !path.empty();
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8); // 控制台输出 UTF-8
    logger::installDailyFileLogger("./log", 7);
    Info() << "更新程序启动";
    DWORD pid = 0;
    std::string path;
    std::string updatePath;
    int timeout = 10;

    // 参数校验
    if (!parse_args(argc, argv, pid, path, updatePath, timeout)) {
        Warning() << "参数错误";
        show_message("参数错误！\n用法: program.exe -pid <进程号> -path <程序路径> -updatePath <更新文件路径> [-timeout <秒>]");
        return 1;
    }
    
    // 路径检查
    if (!fs::exists(std::filesystem::u8path(path))) {
        Warning() << "path不存在 " << path;
        show_message("指定路径不存在: " + path);
        return 1;
    }

    if (!fs::exists(std::filesystem::u8path(updatePath))) {
        Warning() << "updatePath不存在 " << updatePath;
        show_message("指定路径不存在: " + updatePath);
        return 1;
    }

    // 检测进程状态
    Info() << "检测 PID " << pid << " 是否存在";
    int elapsed = 0;
    bool finished = false;
    while (elapsed < timeout) {
        if (!process_exists(pid)) {
            Info() << "进程已结束，开始更新";
            finished = true;
            break;
        }
        Sleep(1000);
        elapsed++;
    }

    // 超时未退出，尝试强杀
    if (!finished) {
        Warning() << "进程未退出，尝试强制结束";
        if (!force_kill(pid)) {
            Error() << "无法强制结束进程 PID " << std::to_string(pid);
            show_message("无法强制结束进程 PID " + std::to_string(pid));
            return 1;
        }

        // 再次等待确认是否杀死
        elapsed = 0;
        while (elapsed < timeout) {
            if (!process_exists(pid)) {
                Info() << "进程已成功结束";
                finished = true;
                break;
            }
            Sleep(1000);
            elapsed++;
        }
        if (!finished) {
            // 强杀失败
            Error() << "无法强制结束进程 PID " << std::to_string(pid);
            show_message("进程强制结束失败！");
            return 1;
        }
    }

    
    //读取updateConfig
    json updateConfig;
    Info() << "读取更新配置";
    try {
        updateConfig = ReadJsonFile(updatePath + "/updateConfig.json");
    }
    catch (const std::exception& e) {
        Error() << "更新配置读取失败 " << e.what();
        show_message(std::string("读取 updateConfig.json 失败: ") + e.what());
        return 1;
    }

    json current_version_config = updateConfig["currentVersion"];
    json new_version_config = updateConfig["newVersion"];
    //删除当前版本updater相关文件
    std::filesystem::path workPath = std::filesystem::u8path(path);
    Info() << "正在删除文件";

    for (auto& item : current_version_config["files"]) {
        std::filesystem::path itemPath = std::filesystem::u8path(item["path"].get<std::string>());
        if (std::filesystem::exists(itemPath)) {
            if (!isSubPath(workPath, itemPath)) {
                Error() << "检测到路径穿越 工作路径:" << workPath.u8string() << " 目标路径:" << itemPath.u8string();
                show_message("检测到路径穿越");
                return -1;
            }
            std::error_code ec;
            Info() << "删除文件: " << itemPath.u8string();
            std::filesystem::remove_all(itemPath, ec);
            if (ec) {
                Error() << "删除文件失败 " << itemPath.u8string() << " " << ec.message();
                show_message("删除文件失败");
                return -1;
            }
        }
        else {
            Warning() << "文件不存在 " << itemPath;
        }
    }
    //替换更新版本updater相关文件
    std::filesystem::path versionRoot = std::filesystem::u8path(updatePath + "/" + updateConfig["version"].get<std::string>());
    std::filesystem::path contentRoot = std::filesystem::weakly_canonical(versionRoot / new_version_config["path"].get<std::string>());
    Info() << "正在复制文件";

    for (auto& item : new_version_config["files"]) {

        std::string relativePathStr = item["path"].get<std::string>();
        std::filesystem::path relativePath = std::filesystem::u8path(relativePathStr);

        std::filesystem::path sourcePath = std::filesystem::weakly_canonical(contentRoot / relativePath);
        std::filesystem::path targetPath = std::filesystem::weakly_canonical(workPath / relativePath);

        if (!std::filesystem::exists(sourcePath)) {
            Error() << "源文件不存在 " << sourcePath.u8string();
            show_message("源文件不存在");
            return -1;
        }

        if (!makedirs(targetPath.parent_path().u8string())) {
            Error() << "创建目录失败 " << targetPath.parent_path().u8string();
            show_message("创建目录失败");
            return -1;
        }

        std::string type = item["type"];
        std::error_code ec;
        if (type == "file") {
            if (!std::filesystem::is_regular_file(sourcePath)) {
                Error() << "文件类型错误 " << sourcePath.u8string() << "应为file 而非 folder";
                show_message("文件类型错误");
                return -1;
            }
            Info() << "正在复制文件: " << sourcePath.u8string() << " 至 " << targetPath.u8string();
            std::filesystem::copy_file(sourcePath, targetPath, std::filesystem::copy_options::overwrite_existing, ec);
        }
        else if (type == "folder") {
            if (!std::filesystem::is_directory(sourcePath)) {
                Error() << "文件类型错误 " << sourcePath.u8string() << "应为folder 而非 file";
                show_message("文件类型错误");
                return -1;
            }
            Info() << "正在复制文件: " << sourcePath.u8string() << " 至 " << targetPath.u8string();
            std::filesystem::copy(sourcePath, targetPath, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive, ec);
        }
        else {
            Error() << "未知类型";
            continue;
        }

        if (ec) {
            Error() << sourcePath.u8string() << " 至 " << targetPath.u8string() << " 复制失败 " << ec.message();
            show_message("复制文件失败");
            return -1;
        }
    }
    // 构造命令行参数
    std::string cmdLine = "\".\\" + new_version_config["mainFile"].get<std::string>() + "\"";
    Debug() << "命令行参数 " << cmdLine;

    // 启动新进程（非阻塞）
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL success = CreateProcessA(
        nullptr,
        &cmdLine[0],
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (success) {
        Info() << "程序已启动，PID: " << pi.dwProcessId;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        //删除配置文件
        std::error_code ec;
        std::filesystem::path removePath = std::filesystem::u8path(updatePath + "/updateConfig.json");
        std::filesystem::remove(removePath, ec);
        if (ec) {
            Warning() << "删除配置文件失败 " << removePath.u8string() << " 原因 " << ec.message();
            show_message("删除配置文件失败");
            return -1;
        }
    }
    else {
        DWORD errCode = GetLastError();
        LPVOID msgBuf;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, errCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&msgBuf, 0, nullptr
        );
        Error() << "启动程序失败" << local_to_utf8(std::string((char*)msgBuf));
        show_message("启动程序失败");
        LocalFree(msgBuf);
        return -1;
    }
    return 0;
}
