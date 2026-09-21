#ifndef YUMCONFIG_H
#define YUMCONFIG_H

#include <string>
#include <map>
#include <vector>

namespace yumConfig {
    typedef bool (*Predicate)(void);

    struct Options {
        const char *preComment = NULL;
        const char *postComment = NULL;
        // loadPredicate is called before the setting's value has been parsed!
        Predicate loadPredicate = NULL;
        Predicate savePredicate = NULL;
        // YummyLife: keep it out of the settings page (bookkeeping, dead options)
        bool hidden = false;
    };

    void registerSetting(const char *name, int &value, yumConfig::Options options = {});
    void registerSetting(const char *name, bool &value, yumConfig::Options options = {});
    void registerSetting(const char *name, std::string &value, yumConfig::Options options = {});
    void registerSetting(const char *name, std::vector<std::string> &value, yumConfig::Options options = {});
    void registerSetting(const char *name, unsigned char &value, yumConfig::Options options = {});
    void registerMappedSetting(const char *name, int &value, const std::map<std::string, int> &mapping, yumConfig::Options options = {});
    // minValue/maxValue are in the setting's own units, not scaled units
    void registerScaledSetting(const char *name, float &value, int scale, float minValue, float maxValue, yumConfig::Options options = {});

    bool checkSettingsFileExists(const char *filename);
    void loadSettings(const char *filename);
    void saveSettings(const char *filename);


    // ---------------------------------------------------------------------
    // YummyLife: introspection, so the settings page can be generated from
    // the same registration calls that write the config file.  Nothing here
    // knows about any particular setting.
    // ---------------------------------------------------------------------

    enum SettingKind {
        SETTING_INT,
        SETTING_BOOL,
        SETTING_STRING,
        SETTING_LIST,
        SETTING_KEY,
        SETTING_CHOICE,
        SETTING_SCALED
    };

    struct SettingInfo {
        std::string name;
        SettingKind kind = SETTING_STRING;
        std::string value;    // as it would be written to the file
        std::string hint;     // one line of help, recovered from the comments
        std::string section;  // from the ==== banner comments
        std::vector<std::string> choices; // SETTING_CHOICE only
        // scaled settings only, already in scaled (file) units
        int scaledMin = 0;
        int scaledMax = 0;
    };

    // every visible setting, in registration order
    std::vector<SettingInfo> listSettings();

    // is inValue acceptable for this setting?  fills outError when it is not
    bool validateSetting(const std::string &name, const std::string &value,
                         std::string &outError);

    // validate, then write through to the live variable
    bool applySetting(const std::string &name, const std::string &value,
                      std::string &outError);

    // current value, formatted the way the file would hold it
    std::string getSettingValue(const std::string &name);
}

#endif // YUMCONFIG_H
