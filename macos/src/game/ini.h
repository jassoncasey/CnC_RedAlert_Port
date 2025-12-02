/**
 * Red Alert macOS Port - INI File Parser
 *
 * Parses Windows-style INI files used throughout the game.
 * Based on original INI.H/INI.CPP but using modern C++ containers.
 */

#ifndef GAME_INI_H
#define GAME_INI_H

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

/**
 * INI file parser and writer.
 *
 * Format:
 *   ; Comment line
 *   [SectionName]
 *   EntryName=Value
 *   AnotherEntry=Another Value
 */
class INIClass {
public:
    INIClass() = default;
    ~INIClass() = default;

    // Non-copyable but movable
    INIClass(const INIClass&) = delete;
    INIClass& operator=(const INIClass&) = delete;
    INIClass(INIClass&&) = default;
    INIClass& operator=(INIClass&&) = default;

    //=========================================================================
    // Loading and Saving
    //=========================================================================

    /**
     * Load INI data from a file path
     */
    bool Load(const char* filename);

    /**
     * Load INI data from a memory buffer
     */
    bool LoadFromBuffer(const char* data, size_t size);

    /**
     * Save INI data to a file path
     */
    bool Save(const char* filename) const;

    /**
     * Clear all sections and entries
     */
    void Clear();

    /**
     * Clear a specific section, or a specific entry within a section
     */
    bool Clear(const char* section, const char* entry = nullptr);

    //=========================================================================
    // Section Queries
    //=========================================================================

    /**
     * Check if any data is loaded
     */
    bool IsLoaded() const { return !sections_.empty(); }

    /**
     * Get the number of sections
     */
    int SectionCount() const { return static_cast<int>(sectionOrder_.size()); }

    /**
     * Check if a section exists
     */
    bool SectionPresent(const char* section) const;

    /**
     * Get section name by index
     */
    const char* GetSectionName(int index) const;

    /**
     * Get all section names
     */
    std::vector<std::string> GetSectionNames() const { return sectionOrder_; }

    //=========================================================================
    // Entry Queries
    //=========================================================================

    /**
     * Get the number of entries in a section
     */
    int EntryCount(const char* section) const;

    /**
     * Get entry name by index within a section
     */
    const char* GetEntry(const char* section, int index) const;

    /**
     * Check if an entry exists
     */
    bool IsPresent(const char* section, const char* entry) const;

    //=========================================================================
    // Get Values
    //=========================================================================

    /**
     * Get a string value (returns chars written, not including null)
     */
    int GetString(const char* section, const char* entry,
                  const char* defvalue, char* buffer, int bufsize) const;

    /**
     * Get a string value as std::string
     */
    std::string GetString(const char* section, const char* entry,
                          const std::string& defvalue = "") const;

    /**
     * Get an integer value
     */
    int GetInt(const char* section, const char* entry, int defvalue = 0) const;

    /**
     * Get a hexadecimal integer value
     */
    int GetHex(const char* section, const char* entry, int defvalue = 0) const;

    /**
     * Get a boolean value (yes/no, true/false, 1/0)
     */
    bool GetBool(const char* section, const char* entry,
                 bool defvalue = false) const;

    /**
     * Get a fixed-point value (stored as floating point string)
     */
    float GetFixed(const char* section, const char* entry,
                   float defvalue = 0.0f) const;

    //=========================================================================
    // Put Values
    //=========================================================================

    /**
     * Set a string value
     */
    bool PutString(const char* section, const char* entry, const char* value);

    /**
     * Set an integer value
     */
    bool PutInt(const char* section, const char* entry, int value);

    /**
     * Set a hexadecimal integer value
     */
    bool PutHex(const char* section, const char* entry, int value);

    /**
     * Set a boolean value
     */
    bool PutBool(const char* section, const char* entry, bool value);

    /**
     * Set a fixed-point value
     */
    bool PutFixed(const char* section, const char* entry, float value);

private:
    /**
     * Internal section structure
     */
    struct Section {
        std::unordered_map<std::string, std::string> entries;
        std::vector<std::string> entryOrder;  // Preserve order
    };

    /**
     * Find or create a section
     */
    Section* FindOrCreateSection(const std::string& name);

    /**
     * Find a section (const version)
     */
    const Section* FindSection(const std::string& name) const;

    /**
     * Case-insensitive string comparison
     */
    static bool StrCaseEqual(const std::string& a, const std::string& b);

    /**
     * Normalize section/entry name to lowercase
     */
    static std::string NormalizeName(const char* name);

    /**
     * Trim whitespace from string
     */
    static std::string Trim(const std::string& str);

    /**
     * Strip comments from a line
     */
    static std::string StripComments(const std::string& line);

    // Section storage (lowercase name -> section data)
    std::unordered_map<std::string, Section> sections_;

    // Preserve section order for iteration
    std::vector<std::string> sectionOrder_;
};

#endif // GAME_INI_H
