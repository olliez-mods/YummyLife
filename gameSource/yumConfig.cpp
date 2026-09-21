#include "yumConfig.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

class yumConfigVar {
    public:
        virtual ~yumConfigVar() {}
        virtual void parse(const std::string &value) = 0;
        virtual void format(std::string &value) = 0;

        // YummyLife: what the settings page needs in order to render and
        // check this setting without knowing what it is
        virtual yumConfig::SettingKind kind() = 0;
        virtual void getChoices(std::vector<std::string> &) {}
        virtual void getScaledRange(int &outMin, int &outMax) { outMin = 0; outMax = 0; }
        // parse() deliberately ignores anything it can't read, which is right
        // for a hand-edited file and useless for a text box - so the checking
        // lives here, next to the parsing it guards
        virtual bool validate(const std::string &, std::string &) { return true; }

        yumConfig::Options options;
};

// YummyLife: config values are matched without regard to case, so a
// hand-edited file saying TRUE or None works as well as the typed-out form
static std::string yumLower(const std::string &in) {
    std::string out = in;
    for (size_t i = 0; i < out.length(); i++) {
        out[i] = tolower(out[i]);
    }
    return out;
}

static std::unordered_map<std::string, yumConfigVar*> configVars;
static std::vector<std::string> configVarNames;

static void registerSetting(const char *name, yumConfigVar *var, yumConfig::Options options) {
    configVars[name] = var;
    configVars[name]->options = options;
    configVarNames.push_back(name);
}

class yumConfigIntVar : public yumConfigVar {
    public:
        yumConfigIntVar(int &value) : mValue(value) {
            // mValue is not set to a default
        }

        virtual void parse(const std::string &value) {
            try {
                mValue = atoi(value.c_str());
            } catch (...) {
                // leave mValue as is
            }
        }

        virtual void format(std::string &value) {
            std::stringstream ss;
            ss << mValue;
            value = ss.str();
        }

        virtual yumConfig::SettingKind kind() { return yumConfig::SETTING_INT; }

        virtual bool validate(const std::string &value, std::string &outError) {
            if (value.empty()) {
                outError = "needs a whole number";
                return false;
            }
            size_t i = (value[0] == '-' || value[0] == '+') ? 1 : 0;
            if (i >= value.length()) {
                outError = "needs a whole number";
                return false;
            }
            for (; i < value.length(); i++) {
                if (value[i] < '0' || value[i] > '9') {
                    outError = "whole numbers only, no decimal point";
                    return false;
                }
            }
            return true;
        }

    private:
        int &mValue;
};

void yumConfig::registerSetting(const char *name, int &value, yumConfig::Options options) {
    registerSetting(name, new yumConfigIntVar(value), options);
}

class yumConfigBoolVar : public yumConfigVar {
    public:
        yumConfigBoolVar(bool &value) : mValue(value) {
            // mValue is not set to a default
        }

        virtual void parse(const std::string &rawValue) {
            std::string value = yumLower(rawValue);
            if (value == "1" || value == "true" || value == "yes" || value == "on") {
                mValue = true;
            } else if (value == "0" || value == "false" || value == "no" || value == "off") {
                mValue = false;
            }
        }

        virtual void format(std::string &value) {
            value = mValue ? "yes" : "no";
        }

        virtual yumConfig::SettingKind kind() { return yumConfig::SETTING_BOOL; }

        virtual bool validate(const std::string &rawValue, std::string &outError) {
            std::string value = yumLower(rawValue);
            if (value == "1" || value == "true" || value == "yes" || value == "on" ||
                value == "0" || value == "false" || value == "no" || value == "off") {
                return true;
            }
            outError = "yes / no (or true / false, on / off, 1 / 0)";
            return false;
        }

    private:
        bool &mValue;
};

void yumConfig::registerSetting(const char *name, bool &value, yumConfig::Options options) {
    registerSetting(name, new yumConfigBoolVar(value), options);
}

class yumConfigStringVar : public yumConfigVar {
    public:
        yumConfigStringVar(std::string &value) : mValue(value) {
            // mValue is not set to a default
        }

        virtual void parse(const std::string &value) {
            mValue = value;
        }

        virtual void format(std::string &value) {
            value = mValue;
        }

        virtual yumConfig::SettingKind kind() { return yumConfig::SETTING_STRING; }

    private:
        std::string &mValue;
};

void yumConfig::registerSetting(const char *name, std::string &value, yumConfig::Options options) {
    registerSetting(name, new yumConfigStringVar(value), options);
}

class yumConfigVectorVar : public yumConfigVar {
    public:
        yumConfigVectorVar(std::vector<std::string> &value) : mValue(value) {
            // mValue is not set to a default
        }

        virtual void parse(const std::string &value) {
            mValue.clear();
            std::string item;
            std::stringstream ss(value);
            // this works because we currently strip all interior whitespace
            while (std::getline(ss, item, ',')) {
                mValue.push_back(item);
            }
        }

        virtual void format(std::string &value) {
            value.clear();
            for (size_t i = 0; i < mValue.size(); i++) {
                if (i > 0) {
                    value += ", ";
                }
                value += mValue[i];
            }
        }

        virtual yumConfig::SettingKind kind() { return yumConfig::SETTING_LIST; }

    private:
        std::vector<std::string> &mValue;
};

void yumConfig::registerSetting(const char *name, std::vector<std::string> &value, yumConfig::Options options) {
    registerSetting(name, new yumConfigVectorVar(value), options);
}

class yumConfigKeyVar : public yumConfigVar {
    public:
        yumConfigKeyVar(unsigned char &value) : mValue(value) {
            // mValue is not set to a default
        }

        virtual void parse(const std::string &value) {
            if (value == "<space>") {
                mValue = ' ';
            } else if (!value.empty()) {
                mValue = value[0];
            } else {
                // hetuw defaulted to 254 for unbound keys; keep that behavior for now
                // in case there's a bug somewhere that causes a key event of 0
                mValue = 254;
            }
        }

        virtual void format(std::string &value) {
            if (mValue == 0 || mValue == 254) {
                value = "";
            } else if (mValue == ' ') {
                value = "<space>";
            } else {
                value = mValue;
            }
        }

        virtual yumConfig::SettingKind kind() { return yumConfig::SETTING_KEY; }

        virtual bool validate(const std::string &value, std::string &outError) {
            // empty means unbound, which is a legitimate choice
            if (value.empty() || value == "<space>") return true;
            if (value.length() > 1) {
                outError = "one key only, or <space>, or empty to unbind";
                return false;
            }
            return true;
        }

    private:
        unsigned char &mValue;
};

void yumConfig::registerSetting(const char *name, unsigned char &value, yumConfig::Options options) {
    registerSetting(name, new yumConfigKeyVar(value), options);
}

class yumConfigMappedVar : public yumConfigVar {
public:
    yumConfigMappedVar(int& value, const std::map<std::string, int>& map)
        : mValue(value), mMap(map) {
        // mValue is not set to a default
    }

    virtual void parse(const std::string& rawValue) {
        std::string value = yumLower(rawValue);
        auto it = mMap.find(value);
        if (it != mMap.end()) {
            mValue = it->second;
            parsedValue = value;
            return;
        }

        // try parsing as an integer so we can upgrade
        // the old integer values to some new string values
        try {
            int candidate = std::stoi(value);
            for (const auto& pair : mMap) {
                if (pair.second == candidate) {
                    mValue = candidate;
                    parsedValue = pair.first;
                    return;
                }
            }
        } catch (...) { }

        // if we didn't find a match, leave mValue as is
    }

    virtual void format(std::string& value) {
        // prefer the value the user originally specified,
        // in case there's a duplicate value in the map
        if (!parsedValue.empty()) {
            value = parsedValue;
            return;
        }

        // otherwise, find the first matching value
        for (const auto& pair : mMap) {
            if (pair.second == mValue) {
                value = pair.first;
                return;
            }
        }

        // this is awkward, but should only happen if the referenced
        // mValue was never initialized to something in the map
        value = "";
    }

    virtual yumConfig::SettingKind kind() { return yumConfig::SETTING_CHOICE; }

    virtual void getChoices(std::vector<std::string> &outChoices) {
        for (const auto& pair : mMap) {
            outChoices.push_back(pair.first);
        }
    }

    virtual bool validate(const std::string& rawValue, std::string& outError) {
        if (mMap.find(yumLower(rawValue)) != mMap.end()) return true;

        outError = "one of: ";
        bool first = true;
        for (const auto& pair : mMap) {
            if (!first) outError += ", ";
            outError += pair.first;
            first = false;
        }
        return false;
    }

private:
    int& mValue;
    std::string parsedValue;
    std::map<std::string, int> mMap;
};

void yumConfig::registerMappedSetting(const char* name, int& value, const std::map<std::string, int>& mapping, yumConfig::Options options) {
    registerSetting(name, new yumConfigMappedVar(value, mapping), options);
}

class yumConfigScaledVar : public yumConfigVar {
public:
    yumConfigScaledVar(float& value, int scale, float minValue, float maxValue)
        : mValue(value), mScale(scale), mMin(minValue), mMax(maxValue) {
        // mValue is not set to a default
    }

    virtual void parse(const std::string& value) {
        try {
            int ival = std::stoi(value);
            mValue = float(ival) / mScale;
            mValue = std::max(mMin, std::min(mMax, mValue));
        } catch (...) {
            // leave mValue as is
        }
    }

    virtual void format(std::string& value) {
        std::stringstream ss;
        ss << int(round(mValue * mScale));
        value = ss.str();
    }

    virtual yumConfig::SettingKind kind() { return yumConfig::SETTING_SCALED; }

    virtual void getScaledRange(int &outMin, int &outMax) {
        outMin = int(round(mMin * mScale));
        outMax = int(round(mMax * mScale));
    }

    virtual bool validate(const std::string& value, std::string& outError) {
        int ival;
        try {
            size_t used = 0;
            ival = std::stoi(value, &used);
            if (used != value.length()) throw std::invalid_argument("trailing");
        } catch (...) {
            outError = "needs a whole number";
            return false;
        }

        int lo, hi;
        getScaledRange(lo, hi);
        if (ival < lo || ival > hi) {
            std::stringstream ss;
            ss << "must be between " << lo << " and " << hi;
            outError = ss.str();
            return false;
        }
        return true;
    }

private:
    float& mValue;
    int mScale;
    float mMin;
    float mMax;
};

void yumConfig::registerScaledSetting(const char* name, float& value, int scale, float minValue, float maxValue, yumConfig::Options options) {
    registerSetting(name, new yumConfigScaledVar(value, scale, minValue, maxValue), options);
}

static void stripWhitespaceAndComments(std::string& str, bool stripComments) {
	// Strip leading whitespace
	size_t firstNonWhitespace = str.find_first_not_of(" \t\r");
	if (firstNonWhitespace != std::string::npos) {
		str.erase(0, firstNonWhitespace);
	}

	// Strip trailing whitespace
	size_t lastNonWhitespace = str.find_last_not_of(" \t\r");
	if (lastNonWhitespace != std::string::npos && lastNonWhitespace + 1 < str.length()) {
		str.erase(lastNonWhitespace + 1);
	}

	// Strip trailing // comments
	if (stripComments) {
		size_t commentPos = str.find("//");
		if (commentPos != std::string::npos) {
			str.erase(commentPos);
		}
	}
}

static bool getSettingsFileLine(std::string& name, std::string& value, const std::string& line) {
	bool readName = true;

    if (line.length() >= 2 && line[0] == '/' && line[1] == '/') {
        return false;
    }

	for (size_t i = 0; i < line.length(); i++) {
        // we should remove this later so that we can have nice strings with spaces in them,
        // but for now we'll stay compatible with current parsing
		if (line[i] == ' ') continue;

		if (readName) {
			if (line[i] == '=') {
				readName = false;
				continue;
			}
			name.push_back(line[i]);
		} else {
            value.push_back(line[i]);
		}
	}

    // we didn't find an '='
    if (readName) {
        return false;
    }

	stripWhitespaceAndComments(name, false);
	stripWhitespaceAndComments(value, true);

    return true;
}

// Checks ionly if it exists or not
bool yumConfig::checkSettingsFileExists(const char *filename) {
    std::ifstream file(filename);
    return file.is_open();
}

void yumConfig::loadSettings(const char *filename) {
    std::ifstream file(filename);
    if (!file.good()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string name, value;
        if (!getSettingsFileLine(name, value, line)) {
            continue;
        }

        auto it = configVars.find(name);
        if (it != configVars.end()) {
            if (it->second->options.loadPredicate && !it->second->options.loadPredicate()) {
                continue;
            }
            it->second->parse(value);
        }
    }
}

void yumConfig::saveSettings(const char *filename) {
    std::ofstream file(filename);
    if (!file.good()) {
        return;
    }

    for (const auto& name : configVarNames) {
        auto it = configVars.find(name);
        if (it != configVars.end()) {
            if (it->second->options.savePredicate != NULL && !it->second->options.savePredicate()) {
                continue;
            }

            std::string value;
            it->second->format(value);

            if (it->second->options.preComment) {
                file << it->second->options.preComment;
            }

            file << name << " = " << value;

            if (it->second->options.postComment) {
                file << it->second->options.postComment;
            }

            file << std::endl;
        }
    }
}

// ---------------------------------------------------------------------------
// YummyLife: introspection for the settings page.
//
// Everything the page needs comes back out of the same registerSetting calls
// that write the config file, so a setting added there shows up in the UI with
// no further work, and the two can never drift apart.
// ---------------------------------------------------------------------------

// turns a comment blob from the config file into one line of prose:
// drops the banner rules, the // markers and the blank lines
static std::string cleanComment(const char *comment) {
    if (comment == NULL) return "";

    std::string out;
    std::stringstream ss(comment);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.find("====") != std::string::npos) continue;
        if (line.find("^^^^") != std::string::npos) continue;

        // strip the comment marker and the space around it
        size_t start = line.find_first_not_of(" \t\r/");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r");
        line = line.substr(start, end - start + 1);
        if (line.empty()) continue;

        if (!out.empty()) out += " ";
        out += line;
    }

    return out;
}

// pulls "Phex Config" out of "// ======== Phex Config ========"
static bool bannerSection(const char *comment, std::string &outName) {
    if (comment == NULL) return false;

    std::string text(comment);
    size_t open = text.find("========");
    if (open == std::string::npos) return false;

    open += 8;
    size_t close = text.find("========", open);
    if (close == std::string::npos) return false;

    size_t start = text.find_first_not_of(" \t", open);
    if (start == std::string::npos || start >= close) return false;
    size_t end = text.find_last_not_of(" \t", close - 1);

    outName = text.substr(start, end - start + 1);
    return true;
}

static bool isHidden(yumConfigVar *var) {
    if (var->options.hidden) return true;
    // a setting that isn't being written to the file has been retired or is a
    // debug flag someone left off - either way it isn't worth a row
    if (var->options.savePredicate != NULL && !var->options.savePredicate()) {
        return true;
    }
    return false;
}

std::vector<yumConfig::SettingInfo> yumConfig::listSettings() {
    std::vector<SettingInfo> out;
    std::string section = "General";

    for (const auto &name : configVarNames) {
        auto it = configVars.find(name);
        if (it == configVars.end()) continue;

        yumConfigVar *var = it->second;

        std::string banner;
        if (bannerSection(var->options.preComment, banner)) {
            section = banner;
        }

        if (!isHidden(var)) {
            SettingInfo info;
            info.name = name;
            info.kind = var->kind();
            info.section = section;
            var->format(info.value);
            var->getChoices(info.choices);
            var->getScaledRange(info.scaledMin, info.scaledMax);

            // the trailing comment is the one written as help; fall back to the
            // block above the setting when there isn't one
            info.hint = cleanComment(var->options.postComment);
            if (info.hint.empty()) {
                info.hint = cleanComment(var->options.preComment);
            }

            out.push_back(info);
        }

        // a closing banner ends the group, whichever comment it sits in
        const char *post = var->options.postComment;
        if ((post != NULL && std::string(post).find("^^^^") != std::string::npos) ||
            (var->options.preComment != NULL &&
             std::string(var->options.preComment).find("^^^^") != std::string::npos)) {
            section = "More Options";
        }
    }

    return out;
}

bool yumConfig::validateSetting(const std::string &name, const std::string &value,
                                std::string &outError) {
    auto it = configVars.find(name);
    if (it == configVars.end()) {
        outError = "no such setting";
        return false;
    }
    return it->second->validate(value, outError);
}

bool yumConfig::applySetting(const std::string &name, const std::string &value,
                             std::string &outError) {
    auto it = configVars.find(name);
    if (it == configVars.end()) {
        outError = "no such setting";
        return false;
    }
    if (!it->second->validate(value, outError)) {
        return false;
    }
    it->second->parse(value);
    return true;
}

std::string yumConfig::getSettingValue(const std::string &name) {
    auto it = configVars.find(name);
    if (it == configVars.end()) return "";

    std::string value;
    it->second->format(value);
    return value;
}
