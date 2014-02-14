#include "menupage.h"

MenuPage::MenuPage(const std::string& page_name):
m_pageName(page_name)
{
    m_entries.reserve(6);
}

MenuPage::~MenuPage()
{
    m_entries.clear();
}

unsigned int MenuPage::activeEntry()const
{
    return m_activeEntry;
}

void MenuPage::highlightPreviousEntry()
{
    m_activeEntry = m_activeEntry > 0 ?
        m_activeEntry - 1
        : static_cast<unsigned int>(m_entries.size()) - 1;
}

void MenuPage::highlightNextEntry()
{
    m_activeEntry = (m_activeEntry + 1) % m_entries.size();
}


void MenuPage::addEntry(std::string caption)
{
    m_entries.push_back(caption);
}


std::string MenuPage::entryCaption(unsigned int index)const
{
    return m_entries[index];
}

size_t MenuPage::entryCount()const
{
    return m_entries.size();
}