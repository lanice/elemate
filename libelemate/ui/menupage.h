#pragma once

#include <vector>
#include <string>

/** @brief Represents a page in the main menu. */
class MenuPage
{
public:
    MenuPage(const std::string& page_name);
    ~MenuPage();

    unsigned int activeEntry() const;
    void highlightPreviousEntry();
    void highlightNextEntry();

    /** Returns top offset between two page entries. */
    float topOffset() const;
    /** Sets top offset between two page entries. */
    void setTopOffset(float topOffset);
    
    void addEntry(std::string caption);

    /** Returns entry with index index. */
    std::string entryCaption(unsigned int index) const;
    /** Returns number of entries. */
    size_t      entryCount() const;
protected:
    std::vector<std::string> m_entries;
    unsigned int             m_activeEntry;
    std::string              m_pageName;
    float                    m_topOffset;

private:
    MenuPage(const MenuPage&) = delete;
    void operator=(const MenuPage&) = delete;
};