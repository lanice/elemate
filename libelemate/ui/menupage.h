#pragma once

#include <vector>
#include <string>

class MenuPage{
public:
    MenuPage(const std::string& page_name);
    ~MenuPage();

    unsigned int activeEntry()const;
    void highlightPreviousEntry();
    void highlightNextEntry();

    float topOffset()const;
    void setTopOffset(float topOffset);
    
    void addEntry(std::string caption);

    std::string entryCaption(unsigned int index)const;
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