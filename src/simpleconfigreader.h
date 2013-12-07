/** \file simpleconfigreader.h
 * Contains the declarations for the SimpleConfigReader class.
 * This class reads a given config file as soon as one is passed and returns the values for a given key. See doxygen documentation for further advices.
 * \author  Friedrich Horschig
 * \date    November 11, 2013
 * \version 1.0
 */

#pragma once

#define DISALLOW_COPY_AND_ASSIGN(typename) \
            typename(const typename&); \
            void operator=(const typename&);

#include <unordered_map>
#include <string>

/** This class reads a given config file as soon as one is passed and returns the values for a given key. 
 * The config is read if you pass the constructor a file name or call readConfig() using a filename. Calling this function makes only sense if the filename was not given initially or if you want to refresh the read entries.
 * Calling writeConfig() is only necessary if you disabled the auto_update using setAutoUpdae(false) which is only useful if you make a lot of new entries in a row or if you want to reduce your file I/O. If auto_update is enabled all entries are refreshed everytime one entry changes. (Maybe this will change in a newer Version)
 * To Create a new value, use setValue() with your value and the key wou want to access the data with.
 * The given config file is not held open.
 */
class SimpleConfigReader
{
public:
    SimpleConfigReader();

    /** This constructor calls readConfig(). */
    SimpleConfigReader(const std::string& filename);
    ~SimpleConfigReader();

    /** If any error occurs or no Filename is given this function returns false. Otherwise it reads every entry of the config to the given file and retuns true. */
    bool readConfig();

    /** If any error occurs this function returns false. Otherwise it reads every entry of the config to the given file and retuns true. */
    bool readConfig(const std::string& filename);

    /** If any error occurs or no Filename is given this function returns false. Otherwise it writes every entry of the config to the given file and retuns true. */
    bool writeConfig();

    /** If any error occurs this function returns false. Otherwise it writes every entry of the config to the given file and retuns true. */
    bool writeConfig(const std::string& filename);

    /** If the parameter is set to false, the config is only written when writeConfig() is called. This is only useful if you make a lot of new entries in a row or if you want to reduce your file I/O. If auto_update is enabled all entries are refreshed everytime one entry changes. (Maybe this will change in a newer Version)*/
    void setAutoUpdate(bool enable_auto_update);

    /** Allows to specify a new key-value-pair. Old ones are simply overwritten. If key or value are empty or if the key contains '=', this function hsa no effect. */
    void setValue(const std::string& key, const std::string& value);

    /** If the key hasn't been defined, this function returns an empty string. */
    std::string  value(std::string key) const;
    bool         auto_update() const;

protected:
    std::unordered_map<std::string, std::string>    m_values;
    std::string                                     m_filename;
    bool                                            m_auto_update;

    /** Creates a new Key-Value-Pair. The Key is the substring before the FIRST '='. The Value is the remaining string. Whitespaces and further '='s are included as well. */
    void    parseLine(const std::string& line);

private:
    DISALLOW_COPY_AND_ASSIGN(SimpleConfigReader)
};