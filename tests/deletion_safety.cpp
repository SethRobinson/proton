#include "../shared/util/SafeDelete.h"
#include <fstream>
#include <iostream>
#include <cstdlib>

static void Require(bool condition, const char* message)
{
    if (!condition) { std::cerr << message << std::endl; std::exit(1); }
}

static void MakeDir(const std::string& path)
{
#ifdef _WIN32
    Require(CreateDirectoryA(path.c_str(), NULL) != 0, "CreateDirectory failed");
#else
    Require(mkdir(path.c_str(), 0700) == 0, "mkdir failed");
#endif
}

int main(int argc, char** argv)
{
    // The runner creates a unique, empty tests/output/deletion-safety/... fixture.
    Require(argc == 2, "Pass an absolute fixture directory");
    const std::string root(argv[1]);
    Require(root.find("deletion-safety") != std::string::npos &&
        ProtonSafeDelete::IsSafeDirectory(root), "Invalid fixture root");
    const char* invalid[] = {"", " ", ".", "..", "/", "//", "relative/cache", "C:",
        "C:cache", "C:\\", "\\cache", "\\\\server\\share", "/tmp/../cache", "/tmp/./cache",
        "/tmp//cache", "/tmp/*", "/tmp/cache.", "/tmp/cache "};
    for (size_t i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i)
        Require(!ProtonSafeDelete::IsAbsoluteChildPath(invalid[i]), "Accepted unsafe path syntax");
    MakeDir(root + "/outside");
    std::ofstream((root + "/outside/keep.txt").c_str()) << "keep";
#ifndef _WIN32
    Require(!ProtonSafeDelete::RemoveTree(root + "/outside\\"), "Trimmed a POSIX backslash and deleted a different target");
    Require(std::ifstream((root + "/outside/keep.txt").c_str()).good(), "Backslash cleanup touched outside sentinel");
#endif
    MakeDir(root + "/cache");
    MakeDir(root + "/cache/nested");
    MakeDir(root + "/cache/empty");
    std::ofstream((root + "/cache/first.txt").c_str()) << "first entry";
    std::ofstream((root + "/cache/nested/file with spaces.txt").c_str()) << "nested";
    Require(!ProtonSafeDelete::RemoveTree(root + "/cache/../outside"), "Traversal was not refused");
#ifdef _WIN32
    std::ofstream((root + "/cache/packed texture.rttex").c_str()) << "packed";
    Require(ProtonSafeDelete::RemoveMatchingFiles(root + "/cache", "../outside/*") == 0, "Accepted parent wildcard");
    Require(ProtonSafeDelete::RemoveMatchingFiles(root + "/cache", "*.rttex") == 1, "Wildcard cleanup failed");
    Require(std::ifstream((root + "/cache/first.txt").c_str()).good(), "Wildcard deleted nonmatch");
#endif
#ifndef _WIN32
    MakeDir(root + "/cache/art: old");
    MakeDir(root + "/cache/draft.");
    MakeDir(root + "/cache/name with trailing space ");
    MakeDir(root + "/cache/literal*question?");
    std::ofstream((root + "/cache/art: old/file.txt").c_str()) << "legacy DMOD asset";
    Require(symlink((root + "/outside").c_str(), (root + "/redirect").c_str()) == 0, "symlink failed");
    Require(!ProtonSafeDelete::RemoveTree(root + "/redirect"), "Followed a root symlink");
    Require(symlink((root + "/outside").c_str(), (root + "/cache/link").c_str()) == 0, "symlink failed");
#endif
    Require(ProtonSafeDelete::RemoveTree(root + "/cache/"), "Failed valid nested cleanup");
    Require(!ProtonSafeDelete::IsSafeDirectory(root + "/cache"), "Cleanup left directory behind");
    Require(std::ifstream((root + "/outside/keep.txt").c_str()).good(), "Touched outside sentinel");
    Require(!ProtonSafeDelete::RemoveTree(root + "/missing"), "Missing directory reported success");
#ifdef _WIN32
    MakeDir(root + "/locked");
    const std::string lockedFile = root + "/locked/in-use.txt";
    HANDLE locked = CreateFileA(lockedFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    Require(locked != INVALID_HANDLE_VALUE, "Create locked fixture failed");
    Require(!ProtonSafeDelete::RemoveTree(root + "/locked"), "Locked file reported cleanup success");
    CloseHandle(locked);
    Require(SetFileAttributesA(lockedFile.c_str(), FILE_ATTRIBUTE_READONLY) != 0, "Set readonly failed");
    Require(ProtonSafeDelete::RemoveTree(root + "/locked"), "Readonly file cleanup failed");
#endif
    std::cout << "Native deletion safety tests passed" << std::endl;
}
