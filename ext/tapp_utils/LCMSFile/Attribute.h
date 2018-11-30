#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <assert.h>
#include <string.h>
#include <cstdlib>  //includes std::atof, recommended. or #include <stdlib.h> less recomended
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define ATTRIBDELIM "<==>"

// Attribute had generic type
class Attribute {
   public:
    enum AttributeType {
        UNKNOWN = 0,
        INT = 1,
        FLOAT = 2,
        DOUBLE = 3,
        STRING = 4,
        CHAR = 5,
        BOOL = 6
    };

   private:
    char mChar;
    int mInt;
    float mFloat;
    double mDouble;
    bool mBool;
    std::string mString;

    AttributeType mType;

   public:
    friend std::ostream &operator<<(std::ostream &s, const Attribute &a) {
        switch (a.mType) {
            case Attribute::CHAR:
                return s << a.mChar;
            case Attribute::INT:
                return s << a.mInt;
            case Attribute::FLOAT:
                return s << a.mFloat;
            case Attribute::DOUBLE:
                return s << a.mDouble;
            case Attribute::STRING:
                return s << a.mString;
        }
        return s << "TypeUnknown";
    }

    Attribute() { mType = UNKNOWN; }

    ~Attribute() {
        mString.clear();
        mType = UNKNOWN;
    }

    Attribute(char c) {
        mChar = c;
        mType = CHAR;
        char buff[100];
        sprintf(buff, "%c", c);
        mString = buff;
    }

    Attribute(int i) {
        mInt = i;
        mFloat = (float)i;
        mDouble = (double)i;
        mBool = (bool)i;
        mType = INT;
        char buff[100];
        sprintf(buff, "%d", i);
        mString = buff;
    }

    Attribute(float f) {
        mFloat = f;
        mInt = (int)f;
        mDouble = (double)f;
        mBool = (bool)f;
        mType = FLOAT;
        char buff[100];
        sprintf(buff, "%f", f);
        mString = buff;
    }

    Attribute(double d) {
        mDouble = d;
        mInt = (int)d;
        mFloat = (float)d;
        mBool = (bool)d;
        mType = DOUBLE;
        char buff[100];
        sprintf(buff, "%lf", d);
        mString = buff;
    }

    Attribute(char *v) {
        mString = v;
        mType = STRING;
        mDouble = atof(v);
        mInt = atoi(v);
        mFloat = mDouble;
        mBool = mInt;
    }

    Attribute(std::string &s) {
        mString = s;
        mType = STRING;
        mDouble = atof(s.c_str());
        mInt = atoi(s.c_str());
        mFloat = mDouble;
        mBool = mInt;
    }

    void setType(AttributeType at) { mType = at; }

    int getIntValue() { return mInt; }

    char getCharValue() { return mChar; }

    float getFloatValue() { return mFloat; }

    double getDoubleValue() { return mDouble; }

    std::string getStringValue() { return mString; }

    bool getBoolValue() { return mBool; }

    int get(char &c) {
        assert(mType == CHAR);
        c = mChar;
        return 1;
    }

    int get(int &i) {
        assert(mType == INT);
        i = getIntValue();
        return 1;
    }

    int get(float &f) {
        assert(mType == FLOAT);
        f = getFloatValue();
        return 1;
    }

    int get(double &d) {
        assert(mType == DOUBLE);
        d = getDoubleValue();
        return 1;
    }

    int get(std::string &s) {
        assert(mType == STRING);
        s = getStringValue();
        return 1;
    }

    int get(bool &b) {
        assert(mType == BOOL);
        b = getBoolValue();
        return 1;
    }
};

// AttributeMap is a wrapper for <string, att> map
class AttributeMap {
    std::map<std::string, Attribute> mMap;

   public:
    friend std::ostream &operator<<(std::ostream &s, const AttributeMap &a) {
        for (std::map<std::string, Attribute>::const_iterator p =
                 a.mMap.begin();
             p != a.mMap.end(); p++) {
            s << p->first << " " << p->second << std::endl;
        }
        return s;
    }

    int get(const std::string &s, Attribute &a) {
        std::map<std::string, Attribute>::iterator q = mMap.find(s);
        if (q == mMap.end()) return 0;
        a = q->second;
        return 1;
    }

    int get(const std::string &s, double &v) {
        Attribute a;
        v = 0;
        if (!get(s, a)) return 0;
        v = a.getDoubleValue();
        return 1;
    }

    int get(const std::string &s, float &v) {
        Attribute a;
        v = 0;
        if (!get(s, a)) return 0;
        v = a.getFloatValue();
        return 1;
    }

    int get(const std::string &s, int &v) {
        Attribute a;
        v = 0;
        if (!get(s, a)) return 0;
        v = a.getIntValue();
        return 1;
    }

    int get(const std::string &s, std::string &v) {
        Attribute a;
        int rc = get(s, a);
        v = a.getStringValue();
        return rc;
    }

    // this could overwrite an existing attribute, but that's ok
    void add(const std::string &s, const Attribute &a) { mMap[s] = a; }

    // add number to name if it already is there
    void addUnique(const std::string &s, const Attribute &a) {
        if (mMap.find(s) == mMap.end()) {
            mMap[s] = a;
            return;
        }
        int done = 0;
        for (int i = 0; !done; i++) {
            std::ostringstream t;
            t << s << i;
            if (mMap.find(t.str()) == mMap.end()) {
                done = 1;
                mMap[t.str()] = a;
            }
        }
    }

    // add string pair separated by a space
    void addStringPair(char *s) {
        char *lf = &s[strlen(s) - 1];
        while (lf >= s && !isprint(*lf)) *lf-- = '\0';
        char *p = strchr(s, ' ');
        if (!p) {
            Attribute a("");
            mMap[s] = a;
        } else {
            *p = '\0';
            p++;
            Attribute a(p);
            mMap[s] = a;
        }
    }

    void addStoredPair(char *s) {
        char *lf = &s[strlen(s) - 1];
        while (lf >= s && !isprint(*lf)) *lf-- = '\0';
        char *p = strstr(s, ATTRIBDELIM);
        if (!p) {
            Attribute a("");
            mMap[s] = a;
        } else {
            *p = '\0';
            p += strlen(ATTRIBDELIM);
            Attribute a(p);
            mMap[s] = a;
        }
    }

    void addStringPairChar(char *s, char delim = '=') {
        char *lf = &s[strlen(s) - 1];
        while (lf >= s && !isprint(*lf)) *lf-- = '\0';
        if (!strlen(s)) return;
        char *p = strchr(s, delim);
        if (!p) {
            Attribute a("");
            mMap[s] = a;
        } else {
            *p = '\0';
            p++;
            Attribute a(p);
            mMap[s] = a;
        }
    }

    void addStringPairChar2(char *s, char delim = '=', char comm = '#') {
        while (*s == comm) s++;
        char *lf = &s[strlen(s) - 1];
        while (lf >= s && !isprint(*lf)) *lf-- = '\0';
        if (!strlen(s)) return;
        char *p = strchr(s, delim);
        if (!p) {
            Attribute a("");
            mMap[s] = a;
        } else {
            *p = '\0';
            p++;
            Attribute a(p);
            mMap[s] = a;
        }
    }

    void setType(const std::string &s, Attribute::AttributeType at) {
        std::map<std::string, Attribute>::iterator q = mMap.find(s);
        if (q == mMap.end()) return;
        q->second.setType(at);
    }

    Attribute &operator[](const std::string &s) { return mMap[s]; }

    bool MatchesString(const std::string &s, const std::string &a) {
        std::string b;
        if (!get(s, b)) return false;
        return (b == a);
    }

    void dumpFile(FILE *f) {
        for (std::map<std::string, Attribute>::iterator q = mMap.begin();
             q != mMap.end(); q++) {
            fprintf(f, "%s%s%s\n", q->first.c_str(), ATTRIBDELIM,
                    q->second.getStringValue().c_str());
        }
    }

#if 0
    ostream& operator<<(ostream& s) {
        for (map<string, Attribute>::iterator p = mMap.begin(); p != mMap.end(); p++) {
            s << p->second << endl;
        }
    }
#endif
};

class CommandArg {
   public:
    Attribute mAttribute;
    bool mUserRequired;
    bool mUserSet;
    int mNValuesExpected;
    double *mDoubles;
    int *mInts;
};

#if 0

addInt("mesh", false, 14)
addDouble("dim", true, 15.6);
addString("FileName", true, "");
addIntArray("dims", false, 4);
addDoubleArray("doubs", true, 6);

Args.parse(argv, argc));

#endif

#endif
