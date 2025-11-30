/**
 * Red Alert macOS Port - INI File Parser Implementation
 *
 * Parses Windows-style INI files used throughout the game.
 */

#include "ini.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>

//===========================================================================
// Static Helper Functions
//===========================================================================

std::string INIClass::NormalizeName(const char* name) {
    if (name == nullptr) return "";
    std::string result(name);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string INIClass::Trim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    if (start == str.size()) return "";

    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
        end--;
    }
    return str.substr(start, end - start + 1);
}

std::string INIClass::StripComments(const std::string& line) {
    // Find comment marker (;) that's not inside quotes
    size_t pos = line.find(';');
    if (pos != std::string::npos) {
        return Trim(line.substr(0, pos));
    }
    return Trim(line);
}

bool INIClass::StrCaseEqual(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

//===========================================================================
// Loading and Saving
//===========================================================================

bool INIClass::Load(const char* filename) {
    if (filename == nullptr) return false;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Read entire file into string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    return LoadFromBuffer(content.c_str(), content.size());
}

bool INIClass::LoadFromBuffer(const char* data, size_t size) {
    if (data == nullptr || size == 0) return false;

    Clear();

    std::string currentSection;
    Section* currentSec = nullptr;

    // Parse line by line
    size_t lineStart = 0;
    while (lineStart < size) {
        // Find end of line
        size_t lineEnd = lineStart;
        while (lineEnd < size && data[lineEnd] != '\n' && data[lineEnd] != '\r') {
            lineEnd++;
        }

        // Extract line
        std::string line(data + lineStart, lineEnd - lineStart);
        line = StripComments(line);

        // Skip to next line (handle \r\n, \n, or \r)
        lineStart = lineEnd;
        if (lineStart < size && data[lineStart] == '\r') lineStart++;
        if (lineStart < size && data[lineStart] == '\n') lineStart++;

        // Skip empty lines
        if (line.empty()) continue;

        // Check for section header [SectionName]
        if (line[0] == '[') {
            size_t endBracket = line.find(']');
            if (endBracket != std::string::npos) {
                currentSection = Trim(line.substr(1, endBracket - 1));
                std::string normalizedSection = NormalizeName(currentSection.c_str());

                // Check if section already exists
                auto it = sections_.find(normalizedSection);
                if (it != sections_.end()) {
                    currentSec = &it->second;
                } else {
                    // Create new section
                    sections_[normalizedSection] = Section{};
                    sectionOrder_.push_back(currentSection);  // Keep original case
                    currentSec = &sections_[normalizedSection];
                }
            }
            continue;
        }

        // Skip entries before first section
        if (currentSec == nullptr) continue;

        // Parse entry: Key=Value
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;

        std::string key = Trim(line.substr(0, equalPos));
        std::string value = Trim(line.substr(equalPos + 1));

        // Skip entries with empty key or value
        if (key.empty()) continue;

        // Store entry
        std::string normalizedKey = NormalizeName(key.c_str());

        // Check if entry already exists
        if (currentSec->entries.find(normalizedKey) == currentSec->entries.end()) {
            currentSec->entryOrder.push_back(key);  // Keep original case
        }
        currentSec->entries[normalizedKey] = value;
    }

    return true;
}

bool INIClass::Save(const char* filename) const {
    if (filename == nullptr) return false;

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    for (const auto& sectionName : sectionOrder_) {
        std::string normalizedSection = NormalizeName(sectionName.c_str());
        auto secIt = sections_.find(normalizedSection);
        if (secIt == sections_.end()) continue;

        const Section& sec = secIt->second;

        // Write section header
        file << "[" << sectionName << "]\r\n";

        // Write entries
        for (const auto& entryName : sec.entryOrder) {
            std::string normalizedEntry = NormalizeName(entryName.c_str());
            auto entryIt = sec.entries.find(normalizedEntry);
            if (entryIt != sec.entries.end()) {
                file << entryName << "=" << entryIt->second << "\r\n";
            }
        }

        file << "\r\n";  // Blank line after section
    }

    return true;
}

void INIClass::Clear() {
    sections_.clear();
    sectionOrder_.clear();
}

bool INIClass::Clear(const char* section, const char* entry) {
    if (section == nullptr) {
        Clear();
        return true;
    }

    std::string normalizedSection = NormalizeName(section);
    auto secIt = sections_.find(normalizedSection);
    if (secIt == sections_.end()) return false;

    if (entry == nullptr) {
        // Remove entire section
        sections_.erase(secIt);
        sectionOrder_.erase(
            std::remove_if(sectionOrder_.begin(), sectionOrder_.end(),
                          [&](const std::string& s) { return StrCaseEqual(s, section); }),
            sectionOrder_.end());
        return true;
    }

    // Remove specific entry
    std::string normalizedEntry = NormalizeName(entry);
    Section& sec = secIt->second;
    auto entryIt = sec.entries.find(normalizedEntry);
    if (entryIt == sec.entries.end()) return false;

    sec.entries.erase(entryIt);
    sec.entryOrder.erase(
        std::remove_if(sec.entryOrder.begin(), sec.entryOrder.end(),
                      [&](const std::string& e) { return StrCaseEqual(e, entry); }),
        sec.entryOrder.end());
    return true;
}

//===========================================================================
// Section Queries
//===========================================================================

bool INIClass::SectionPresent(const char* section) const {
    return FindSection(NormalizeName(section)) != nullptr;
}

const char* INIClass::GetSectionName(int index) const {
    if (index < 0 || index >= static_cast<int>(sectionOrder_.size())) {
        return nullptr;
    }
    return sectionOrder_[index].c_str();
}

//===========================================================================
// Entry Queries
//===========================================================================

int INIClass::EntryCount(const char* section) const {
    const Section* sec = FindSection(NormalizeName(section));
    if (sec == nullptr) return 0;
    return static_cast<int>(sec->entryOrder.size());
}

const char* INIClass::GetEntry(const char* section, int index) const {
    const Section* sec = FindSection(NormalizeName(section));
    if (sec == nullptr) return nullptr;
    if (index < 0 || index >= static_cast<int>(sec->entryOrder.size())) {
        return nullptr;
    }
    return sec->entryOrder[index].c_str();
}

bool INIClass::IsPresent(const char* section, const char* entry) const {
    const Section* sec = FindSection(NormalizeName(section));
    if (sec == nullptr) return false;
    return sec->entries.find(NormalizeName(entry)) != sec->entries.end();
}

//===========================================================================
// Get Values
//===========================================================================

int INIClass::GetString(const char* section, const char* entry,
                        const char* defvalue, char* buffer, int bufsize) const {
    if (buffer == nullptr || bufsize <= 0) return 0;

    std::string value = GetString(section, entry, defvalue ? defvalue : "");

    // Copy to buffer
    int len = std::min(static_cast<int>(value.size()), bufsize - 1);
    std::memcpy(buffer, value.c_str(), len);
    buffer[len] = '\0';
    return len;
}

std::string INIClass::GetString(const char* section, const char* entry,
                                const std::string& defvalue) const {
    const Section* sec = FindSection(NormalizeName(section));
    if (sec == nullptr) return defvalue;

    auto it = sec->entries.find(NormalizeName(entry));
    if (it == sec->entries.end()) return defvalue;

    return it->second;
}

int INIClass::GetInt(const char* section, const char* entry, int defvalue) const {
    std::string value = GetString(section, entry, "");
    if (value.empty()) return defvalue;

    try {
        // Handle negative numbers and various formats
        return std::stoi(value, nullptr, 10);
    } catch (...) {
        return defvalue;
    }
}

int INIClass::GetHex(const char* section, const char* entry, int defvalue) const {
    std::string value = GetString(section, entry, "");
    if (value.empty()) return defvalue;

    try {
        // Strip 0x or $ prefix if present
        size_t start = 0;
        if (value.size() > 2 && (value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))) {
            start = 2;
        } else if (value.size() > 1 && value[0] == '$') {
            start = 1;
        }
        return std::stoi(value.substr(start), nullptr, 16);
    } catch (...) {
        return defvalue;
    }
}

bool INIClass::GetBool(const char* section, const char* entry, bool defvalue) const {
    std::string value = GetString(section, entry, "");
    if (value.empty()) return defvalue;

    // Normalize to lowercase
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Check for true values
    if (value == "yes" || value == "true" || value == "1" || value == "on") {
        return true;
    }
    // Check for false values
    if (value == "no" || value == "false" || value == "0" || value == "off") {
        return false;
    }

    return defvalue;
}

float INIClass::GetFixed(const char* section, const char* entry, float defvalue) const {
    std::string value = GetString(section, entry, "");
    if (value.empty()) return defvalue;

    try {
        // Handle percentage format (e.g., "75%")
        if (!value.empty() && value.back() == '%') {
            return std::stof(value.substr(0, value.size() - 1)) / 100.0f;
        }
        return std::stof(value);
    } catch (...) {
        return defvalue;
    }
}

//===========================================================================
// Put Values
//===========================================================================

INIClass::Section* INIClass::FindOrCreateSection(const std::string& name) {
    std::string normalized = NormalizeName(name.c_str());
    auto it = sections_.find(normalized);
    if (it != sections_.end()) {
        return &it->second;
    }

    // Create new section
    sections_[normalized] = Section{};
    sectionOrder_.push_back(name);  // Keep original case
    return &sections_[normalized];
}

const INIClass::Section* INIClass::FindSection(const std::string& name) const {
    auto it = sections_.find(name);
    if (it != sections_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool INIClass::PutString(const char* section, const char* entry, const char* value) {
    if (section == nullptr || entry == nullptr) return false;

    Section* sec = FindOrCreateSection(section);
    if (sec == nullptr) return false;

    std::string normalizedEntry = NormalizeName(entry);

    // Add to order list if new entry
    if (sec->entries.find(normalizedEntry) == sec->entries.end()) {
        sec->entryOrder.push_back(entry);  // Keep original case
    }

    sec->entries[normalizedEntry] = value ? value : "";
    return true;
}

bool INIClass::PutInt(const char* section, const char* entry, int value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%d", value);
    return PutString(section, entry, buffer);
}

bool INIClass::PutHex(const char* section, const char* entry, int value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "0x%X", static_cast<unsigned int>(value));
    return PutString(section, entry, buffer);
}

bool INIClass::PutBool(const char* section, const char* entry, bool value) {
    return PutString(section, entry, value ? "yes" : "no");
}

bool INIClass::PutFixed(const char* section, const char* entry, float value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.4f", value);
    return PutString(section, entry, buffer);
}
