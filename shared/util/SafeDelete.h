#ifndef PROTON_SAFE_DELETE_H
#define PROTON_SAFE_DELETE_H

#include <string>
#include <cerrno>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

// Shared by the platform backends. Generic APIs cannot hardcode an app's cache
// name: callers must construct an absolute path from a checked base plus a
// literal child. Never turn a relative/empty input into an absolute one here.
namespace ProtonSafeDelete
{
inline bool IsAbsoluteChildPath(const std::string& path)
{
    if (path.empty() || path.find_first_of("*?\n\r") != std::string::npos ||
        path.find('\0') != std::string::npos) return false;
#if defined(_WIN32)
    if (path.size() < 4 || path[1] != ':' ||
        !((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) ||
        (path[2] != '/' && path[2] != '\\')) return false;
    const size_t first = 3; // UNC/device paths deliberately fail closed.
#else
    if (path[0] != '/' || path.size() < 2) return false;
    const size_t first = 1;
#endif
    size_t start = first;
    while (start < path.size())
    {
        const size_t end = path.find_first_of("/\\", start);
        const std::string part = path.substr(start, end == std::string::npos ? end : end - start);
        if (part.empty() || part == "." || part == ".." || part.find(':') != std::string::npos ||
            part[part.size()-1] == '.' || part[part.size()-1] == ' ') return false;
        if (end == std::string::npos) break;
#if !defined(_WIN32)
        if (path[end] == '\\') return false;
#endif
        start = end + 1;
    }
    return true;
}

inline bool IsSafeDirectory(const std::string& path)
{
    if (!IsAbsoluteChildPath(path)) return false;
#if defined(_WIN32)
    const size_t first = 3;
#else
    const size_t first = 1;
#endif
    // A link anywhere in the supplied directory path could redirect cleanup.
    size_t end = first;
    do
    {
        end = path.find_first_of("/\\", end);
        const std::string ancestor = path.substr(0, end);
#if defined(_WIN32)
        const DWORD attributes = GetFileAttributesA(ancestor.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) return false;
#else
        struct stat info;
        if (lstat(ancestor.c_str(), &info) != 0 || !S_ISDIR(info.st_mode) || S_ISLNK(info.st_mode)) return false;
#endif
        if (end == std::string::npos) break;
        ++end;
    } while (end < path.size());
    return true;
}

namespace Detail
{
// Called only after validating the supplied root. Children come from directory
// enumeration, not caller input: POSIX names such as "art: old" or "draft."
// are literal names and must not be rejected as Windows path syntax.
inline bool RemoveTreeContents(const std::string& path)
{
#if defined(_WIN32)
    WIN32_FIND_DATAA entry;
    HANDLE search = FindFirstFileA((path + "/*").c_str(), &entry);
    if (search == INVALID_HANDLE_VALUE) return false;
    bool ok = true;
    do
    {
        const std::string name(entry.cFileName);
        if (name == "." || name == "..") continue;
        const std::string child = path + "/" + name;
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) { ok = false; break; }
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ok = RemoveTreeContents(child);
        else
        {
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                ok = SetFileAttributesA(child.c_str(), entry.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY) != 0;
            if (ok) ok = DeleteFileA(child.c_str()) != 0;
        }
        if (!ok) break;
    } while (FindNextFileA(search, &entry));
    if (ok && GetLastError() != ERROR_NO_MORE_FILES) ok = false;
    FindClose(search);
    return ok && RemoveDirectoryA(path.c_str()) != 0;
#else
    DIR* directory = opendir(path.c_str());
    if (!directory) return false;
    bool ok = true;
    for (;;)
    {
        errno = 0;
        dirent* entry = readdir(directory);
        if (!entry) { if (errno) ok = false; break; }
        const std::string name(entry->d_name);
        if (name == "." || name == "..") continue;
        const std::string child = path + "/" + name;
        struct stat info;
        if (lstat(child.c_str(), &info) != 0) { ok = false; break; }
        // Unlink a symlink itself; never descend through it. Do not rely on
        // dirent.d_type, which can be DT_UNKNOWN on otherwise valid filesystems.
        ok = S_ISDIR(info.st_mode) ? RemoveTreeContents(child) : unlink(child.c_str()) == 0;
        if (!ok) break;
    }
    closedir(directory);
    return ok && rmdir(path.c_str()) == 0;
#endif
}
}

inline bool RemoveTree(std::string path)
{
    // A backslash is a separator only on Windows. Trimming one on POSIX could
    // turn a refused literal name into a different, valid deletion target.
#if defined(_WIN32)
    const char* separators = "/\\";
#else
    const char* separators = "/";
#endif
    while (path.size() > 1 && path.find_last_of(separators) == path.size()-1)
        path.erase(path.size()-1);
    if (!IsSafeDirectory(path)) return false;
    return Detail::RemoveTreeContents(path);
}

#if defined(_WIN32)
inline int RemoveMatchingFiles(const std::string& directory, const std::string& pattern)
{
    if (!IsSafeDirectory(directory) || pattern.empty() || pattern == "." || pattern == ".." ||
        pattern.find_first_of("/\\:") != std::string::npos) return 0;
    WIN32_FIND_DATAA entry;
    HANDLE search = FindFirstFileA((directory + "/" + pattern).c_str(), &entry);
    if (search == INVALID_HANDLE_VALUE) return 0;
    int count = 0;
    do
    {
        if (!(entry.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)))
        {
            const std::string file = directory + "/" + entry.cFileName;
            if (DeleteFileA(file.c_str())) ++count;
        }
    } while (FindNextFileA(search, &entry));
    FindClose(search);
    return count;
}
#endif
}
#endif
