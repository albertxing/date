//
//  date.h
//  C++ pretty date formatter and parser
//
//  Created by Albert Xing on 21/12/16.
//  Copyright © 2016 Albert Xing. All rights reserved.
//  

#ifndef date_h
#define date_h

#include <algorithm>
#include <iostream>
#include <vector>
#include <ctime>

#define SSTART 0
#define SINNUM 1
#define SINUNIT 2
#define SAT 3
#define SNEXT 4

#define TDAY (24 * 60 * 60)
#define THOUR (60 * 60)

class Date {
public:
    Date(std::string s) {
        time_t ct = time(nullptr);
        tm time = *localtime(&ct);
        
        time.tm_sec = 0;
        time.tm_min = 0;
        time.tm_hour++;
        
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        std::vector<std::string> words = split(s);
        
        int state = 0; int num = 0;
        bool mtime = false, mday = false, mmonth = false;
        for (std::string w : words) {
            switch (state) {
                case 0:
                    if (is_time(w)) {
                        time_t t = get_time(w);
                        time.tm_hour = 0;
                        time.tm_min = 0;
                        inc_time(&time, t);
                        mtime = true;
                        if (mktime(&time) < ct && !mday && !mmonth)
                            inc_time(&time, TDAY);
                    } else if (is_date(w)) {
                        time_t t = get_date(w);
                        inc_time(&time, t);
                        mday = true; mmonth = true;
                    } else if (is_day(w)) {
                        int d = get_day(w);
                        time.tm_mday = 0;
                        inc_time(&time, d * TDAY);
                        mday = true;
                        if (mktime(&time) < ct && !mmonth) {
                            time.tm_mon++;
                        }
                    } else if (is_month(w)) {
                        int m = get_month(w);
                        time.tm_mon = m;
                        mmonth = true;
                        if (mktime(&time) < ct) {
                            time.tm_year++;
                        }
                    } else if (w == "at") {
                        state = SAT;
                    } else if (w == "in") {
                        state = SINNUM;
                    } else if (w == "next") {
                        num = 1;
                        state = SINUNIT;
                    }
                    break;
                case SAT:
                {
                    time_t t = get_time(w);
                    time.tm_hour = 0;
                    time.tm_min = 0;
                    inc_time(&time, t);
                    if (mktime(&time) < ct && !mday && !mmonth)
                        inc_time(&time, TDAY);
                    
                    state = 0;
                    break;
                }
                case SINNUM:
                {
                    num = 0;
                    int ns = sscanf(w.c_str(), "%d", &num);
                    if (ns != 1) {
                        if (w == "one" || w == "a") num = 1;
                        if (w == "two") num = 2;
                        if (w == "three") num = 3;
                        if (w == "four") num = 4;
                        if (w == "five") num = 5;
                        if (w == "six") num = 6;
                        if (w == "seven") num = 7;
                        if (w == "eight") num = 8;
                        if (w == "nine") num = 9;
                        if (w == "ten") num = 10;
                    }
                    if (num != 0) state = SINUNIT;
                    else state = 0;
                    break;
                }
                case SINUNIT:
                {
                    std::string w2 = w.substr(0,2);
                    if (w2 == "ho") {
                        time.tm_hour += num;
                    } else if (w2 == "da") {
                        time.tm_mday += num;
                    } else if (w2 == "we") {
                        time.tm_mday += 7 * num;
                    } else if (w2 == "mo") {
                        time.tm_mon += num;
                    } else if (w2 == "ye") {
                        time.tm_year += num;
                    } else if (num == 1 && is_day(w)) {
                        int d = get_day(w);
                        time.tm_mday = 0;
                        inc_time(&time, d * TDAY && !mmonth);
                        if (mktime(&time) < ct) {
                            time.tm_mon++;
                        }
                    }
                }
            }
        }
        
        d = mktime(&time);
    }
    std::string print() {
        char str[256];
        time_t ct = time(nullptr);
        tm lt = *localtime(&ct);
        tm dt = *localtime(&d);
        
        int diff = (int)d - (int)ct;
        int wdiff = (dt.tm_wday - lt.tm_wday + 7) % 7;
        
        if (dt.tm_year == lt.tm_year && dt.tm_yday == lt.tm_yday) {
            strftime(str, 256, "Today %R", &dt);
        } else if (diff < 0 && (-diff) < 2 * TDAY && dt.tm_wday == (lt.tm_wday + 6)%7) {
            strftime(str, 256, "Yesterday %R", &dt);
        } else if (diff > 0 && diff < 2 * TDAY && dt.tm_wday == (lt.tm_wday + 1)%7) {
            strftime(str, 256, "Tomorrow %R", &dt);
        } else if (diff > 0 && (diff < 7 * TDAY || (diff < 8 * TDAY && wdiff > 5))) {
            strftime(str, 256, "%A %R", &dt);
        } else {
            strftime(str, 256, "%a %b %d %R", &dt);
        }
        
        return std::string(str);
    }
private:
    const static time_t DTIME = 8 * 60 * 60;
    const static int MORNING = 8;
    const static int NOON = 12;
    const static int AFTERNOON = 14;
    const static int EVENING = 16;
    const static int NIGHT = 18;
    
    time_t d;
    
    std::vector<std::string> split(std::string s) {
        std::vector<std::string> r;
        std::string t = "";
        for (char c : s) {
            if (c != ' ') t += c;
            else {
                r.push_back(t);
                t = "";
            }
        }
        r.push_back(t);
        return r;
    }
    
    void inc_time(tm* m, time_t t) {
        time_t nt = mktime(m) + t;
        *m = *localtime(&nt);
    }
    
    bool is_time(std::string w) {
        int h, m = 0; char c;
        const char* wc = w.c_str();
        if (sscanf(wc, "%d:%d", &h, &m) == 2 ||
            sscanf(wc, "%da%c", &h, &c) == 2 ||
            sscanf(wc, "%dp%c", &h, &c) == 2 ||
            (sscanf(wc, "%d", &h) == 1 && h > 99) ||
            w == "morning" ||
            w == "noon" ||
            w == "afternoon" ||
            w == "evening" ||
            w == "night")
            return true;
        return false;
    }
    
    time_t get_time(std::string w) {
        int h, min; char c;
        const char* wc = w.c_str();
        if (sscanf(wc, "%d:%d", &h, &min) == 2)
            return (h * 60 + min) * 60;
        if (sscanf(wc, "%da%c", &h, &c) == 2)
            return h * 60 * 60;
        if (sscanf(wc, "%dp%c", &h, &c) == 2)
            return (h + 12) * 60 * 60;
        if (sscanf(wc, "%d", &h) == 1 && h > 99)
            return ((h / 100) * 60 + (h % 100)) * 60;
        if (sscanf(wc, "%d", &h) == 1 && h < 30)
            return h * 60 * 60;
        if (w == "morning")
            return MORNING * 60 * 60;
        if (w == "noon")
            return NOON * 60 * 60;
        if (w == "afternoon")
            return AFTERNOON * 60 * 60;
        if (w == "evening")
            return EVENING * 60 * 60;
        if (w == "night")
            return NIGHT * 60 * 60;
        return 0;
    }
    
    bool is_day(std::string w) {
        int d; char c;
        const char* wc = w.c_str();
        if (sscanf(wc, "%ds%c", &d, &c) == 2 ||
            sscanf(wc, "%dn%c", &d, &c) == 2 ||
            sscanf(wc, "%dr%c", &d, &c) == 2 ||
            sscanf(wc, "%dt%c", &d, &c) == 2 ||
            (sscanf(wc, "%d", &d) == 1 && d < 30))
            return true;
        
        std::string w3 = w.substr(0,3);
        if (w == "today" ||
            w == "tomorrow" ||
            w3 == "mon" ||
            w3 == "tue" ||
            w3 == "wed" ||
            w3 == "thu" ||
            w3 == "fri" ||
            w3 == "sat" ||
            w3 == "sun")
            return true;
        return false;
    }
    
    int get_day(std::string w) {
        int d;
        const char* wc = w.c_str();
        if (sscanf(wc, "%dst", &d) == 1 ||
            sscanf(wc, "%dnd", &d) == 1 ||
            sscanf(wc, "%drd", &d) == 1 ||
            sscanf(wc, "%dth", &d) == 1 ||
            sscanf(wc, "%d", &d) == 1)
            return d;
        
        std::string w2 = w.substr(0,2);
        time_t ct = time(nullptr);
        tm* lt = localtime(&ct);
        int cd = lt->tm_mday;
        if (w == "today")
            return cd;
        if (w == "tomorrow")
            return cd + 1;
        
        int td;
        if (w2 == "mo") td = 1;
        else if (w2 == "tu") td = 2;
        else if (w2 == "we") td = 3;
        else if (w2 == "th") td = 4;
        else if (w2 == "fr") td = 5;
        else if (w2 == "sa") td = 6;
        else if (w2 == "su") td = 0;
        
        int cwd = lt->tm_wday;
        int pdiff = (td - cwd + 7) % 7;
        
        return cd + pdiff;
    }
    
    bool is_month(std::string w) {
        std::string w3 = w.substr(0,3);
        if (w3 == "jan" ||
            w3 == "feb" ||
            w3 == "mar" ||
            w3 == "apr" ||
            w3 == "may" ||
            w3 == "jun" ||
            w3 == "jul" ||
            w3 == "aug" ||
            w3 == "sep" ||
            w3 == "oct" ||
            w3 == "nov" ||
            w3 == "dec")
            return true;
        return false;
    }
    
    int get_month(std::string w) {
        std::string w3 = w.substr(0,3);
        if (w3 == "jan") return 0;
        if (w3 == "feb") return 1;
        if (w3 == "mar") return 2;
        if (w3 == "apr") return 3;
        if (w3 == "may") return 4;
        if (w3 == "jun") return 5;
        if (w3 == "jul") return 6;
        if (w3 == "aug") return 7;
        if (w3 == "sep") return 8;
        if (w3 == "oct") return 9;
        if (w3 == "nov") return 10;
        if (w3 == "dec") return 11;
        return 0;
    }
    
    bool is_date(std::string w) {
        int d, m, y;
        const char* wc = w.c_str();
        if (sscanf(wc, "%d-%d-%d", &d, &m, &y) == 3 ||
            sscanf(wc, "%d-%d", &d, &m) == 2 ||
            sscanf(wc, "%d/%d/%d", &d, &m, &y) == 3 ||
            sscanf(wc, "%d/%d", &d, &m) == 2)
            return true;
        return false;
    }
    
    time_t get_date(std::string w) {
        int d, m, y = 0;
        const char* wc = w.c_str();
        if (sscanf(wc, "%d-%d-%d", &m, &d, &y) == 3 ||
            sscanf(wc, "%d-%d", &m, &d) == 2 ||
            sscanf(wc, "%d/%d/%d", &m, &d, &y) == 3 ||
            sscanf(wc, "%d/%d", &m, &d) == 2) {
            
            time_t ct = time(nullptr);
            tm* lt = localtime(&ct);
            lt->tm_hour = 0;
            lt->tm_min = 0;
            lt->tm_sec = 0;
            time_t ot = mktime(lt);
            
            m--;
            if (y == 0) y = lt->tm_year;
            
            lt->tm_mday = d;
            lt->tm_mon = m;
            lt->tm_year = y;
            
            time_t nt = mktime(lt);
            return nt - ot;
        }
        return 0;
    }

};

#endif
