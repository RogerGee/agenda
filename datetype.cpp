/* datetype.cpp */
#include "datetype.h"
#include <cstring>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <time.h>
using namespace std;
using namespace agenda;

void string_to_lower(string& s) // this is referenced externally
{
    for (size_t j = 0;j < s.size();++j)
        if (isupper(s[j]))
            s[j] = s[j]-'A' + 'a';
}
static bool verify_convert(const string& token,uint16_t& result,uint16_t min,uint16_t max)
{
    char digits[32];
    uint16_t value = 0;
    for (size_t i = 0;i < token.length();++i) {
        if (!isdigit(token[i]))
            return false;
        value *= 10;
        value += token[i] - '0';
    }
    if (value < min || value > max)
        return false;
    result = value;
    return true;
}
static const char* dow_to_string(dow d,bool shortForm = false)
{
    switch (d) {
    case sunday:
        return shortForm ? "Sun" : "Sunday";
    case monday:
        return shortForm ? "Mon" : "Monday";
    case tuesday:
        return shortForm ? "Tue" : "Tuesday";
    case wednesday:
        return shortForm ? "Wed" : "Wednesday";
    case thursday:
        return shortForm ? "Thu" : "Thursday";
    case friday:
        return shortForm ? "Fri" : "Friday";
    case saturday:
        return shortForm ? "Sat" : "Saturday";
    default:
        return "noday";
    }
    return "noday";
}
static const char* month_to_string(uint16_t month,bool shortForm = false)
{
    switch (month) {
    case 1:
        return shortForm ? "Jan" : "January";
    case 2:
        return shortForm ? "Feb" : "February";
    case 3:
        return shortForm ? "Mar" : "March";
    case 4:
        return shortForm ? "Apr" : "April";
    case 5:
        return shortForm ? "May" : "May";
    case 6:
        return shortForm ? "Jun" : "June";
    case 7:
        return shortForm ? "Jul" : "July";
    case 8:
        return shortForm ? "Aug" : "August";
    case 9:
        return shortForm ? "Sep" : "September";
    case 10:
        return shortForm ? "Oct" : "October";
    case 11:
        return shortForm ? "Nov" : "November";
    case 12:
        return shortForm ? "Dec" : "December";
    }
    return "nomonth";
}
static dow match_dow_string(const string& tok)
{
    string s;
    for (int j = sunday;j <= saturday;++j) {
        s = dow_to_string(dow(j));
        string_to_lower(s);
        if (tok == s)
            return dow(j);
        else {
            s = dow_to_string(dow(j),true);
            string_to_lower(s);
            if (tok == s)
                return dow(j);
        }
    }
    return invalid_dow;
}
static uint16_t match_month_string(const string& tok)
{
    string s;
    for (uint16_t j = 1;j <= 12;++j) {
        s = month_to_string(j);
        string_to_lower(s);
        if (tok == s)
            return j;
        else {
            s = month_to_string(j,true);
            string_to_lower(s);
            if (tok == s)
                return j;
        }
    }
    return 0;
}

date_t::date_t()
{
    tm timedata;
    timespec tspec;
    if (clock_gettime(CLOCK_REALTIME,&tspec) != 0)
        throw runtime_error("fail clock_getttime()");
    if (localtime_r(&tspec.tv_sec,&timedata) == NULL)
        throw runtime_error("fail localtime()");
    _year = timedata.tm_year+1900;
    _month = timedata.tm_mon+1;
    _day = timedata.tm_mday;
    _seconds = timedata.tm_hour*3600 + timedata.tm_min*60 + timedata.tm_sec;
}
date_t::date_t(uint16_t month,uint16_t day)
{
    tm timedata;
    timespec tspec;
    if (clock_gettime(CLOCK_REALTIME,&tspec) != 0)
        throw runtime_error("fail clock_getttime()");
    if (localtime_r(&tspec.tv_sec,&timedata) == NULL)
        throw runtime_error("fail localtime()");
    timedata.tm_mon = month-1;
    timedata.tm_mday = day;
    if (mktime(&timedata) == (time_t)-1)
        throw runtime_error("fail mktime()");
    _year = timedata.tm_year+1900;
    _month = timedata.tm_mon+1;
    _day = timedata.tm_mday;
    _seconds = timedata.tm_hour*3600 + timedata.tm_min*60 + timedata.tm_sec;
}
date_t::date_t(uint16_t month,uint16_t day,uint16_t year)
{
    tm timedata;
    timespec tspec;
    if (clock_gettime(CLOCK_REALTIME,&tspec) != 0)
        throw runtime_error("fail clock_getttime()");
    if (localtime_r(&tspec.tv_sec,&timedata) == NULL)
        throw runtime_error("fail localtime()");
    timedata.tm_year = year-1900;
    timedata.tm_mon = month-1;
    timedata.tm_mday = day;
    if (mktime(&timedata) == (time_t)-1)
        throw runtime_error("fail mktime()");
    _year = timedata.tm_year+1900;
    _month = timedata.tm_mon+1;
    _day = timedata.tm_mday;
    _seconds = timedata.tm_hour*3600 + timedata.tm_min*60 + timedata.tm_sec;
}
date_t::date_t(string tokens[],int count)
{
    /* time input sequence grammar:
        (DOW|(DAY|MONTH|YEAR)+ (HOUR(:MINUTE(:SECOND)?)? AM|PM)?)? */
    int i, s;
    bool useTime, foundDay;
    tm timedata;
    timespec tspec;
    uint16_t originalMonth;
    if (clock_gettime(CLOCK_REALTIME,&tspec) != 0)
        throw runtime_error("fail clock_gettime()");
    if (localtime_r(&tspec.tv_sec,&timedata) == NULL)
        throw runtime_error("fail localtime()");
    i = s = 0;
    originalMonth = timedata.tm_mon;
    timedata.tm_hour = 0;
    timedata.tm_min = 0;
    timedata.tm_sec = 0;
    useTime = false;
    foundDay = false;
    while (i < count) {
        string& tok = tokens[i];
        string_to_lower(tok);
        if (s <= 4) {
            // try reading dow string; must be in state0
            dow d = s==0 ? match_dow_string(tok) : invalid_dow;
            if (d != invalid_dow) {
                do {
                    ++timedata.tm_mday;
                    ++timedata.tm_wday;
                    if (timedata.tm_wday >= invalid_dow)
                        timedata.tm_wday = sunday;
                } while (d != timedata.tm_wday);
                s = 5;
            }
            // try reading a month-day value (the range for days and times overlaps, so 
            // we must be sure we haven't found a day value before now)
            else if (!foundDay && verify_convert(tok,_day,1,31)) {
                if (timedata.tm_mon==originalMonth && _day<timedata.tm_mday)
                    // choose day of next month since the next occurange of the
                    // day value is in the next month
                    ++timedata.tm_mon;
                timedata.tm_mday = _day;
                ++s;
                foundDay = true;
            }
            // try reading a year value
            else if (verify_convert(tok,_year,1000,9999)) {
                timedata.tm_year = _year - 1900;
                ++s;
            }
            // try reading a month literal
            else if ((_month = match_month_string(tok)) > 0) {
                timedata.tm_mon = _month-1;
                ++s;
            }
            else {
                if (s == 0) // user did not supply one of non-optional part
                    throw date_exception("expected one of Day, Month or Year or DayOfWeek in time input");
                s = 5;
                continue;
            }
        }
        else {
            uint16_t t;
            if (s == 5) {
                string str;
                size_t pos, pos2;
                // see if tok is a time string (HOUR:MIN:SEC)
                useTime = true;
                pos = tok.find_first_of(':');
                str = tok.substr(0,pos);
                if (verify_convert(str,t,1,12)) {
                    timedata.tm_hour = t;
                    if (pos != string::npos) {
                        pos2 = tok.find_first_of(':',++pos);
                        str = tok.substr(pos,pos2-pos);
                        if (verify_convert(str,t,0,59)) {
                            timedata.tm_min = t;
                            if (pos2 != string::npos) {
                                str = tok.substr(++pos2);
                                if (verify_convert(str,t,0,59))
                                    timedata.tm_sec = t;
                            }
                        }
                    }
                    ++s;
                }
                else
                    throw date_exception("expected HOUR[:MINUTE[:SECOND]] in time input");
            }
            else if (s == 6) {
                if (tok == "am") {
                    if (timedata.tm_hour == 12)
                        timedata.tm_hour = 0;
                }
                else if (tok == "pm") {
                    if (timedata.tm_hour != 12)
                        timedata.tm_hour += 12;
                }
                else
                    throw date_exception("expected AM or PM in time input");
                ++s;
            }
            else
                throw date_exception("unexpected token in time input");
        }
        ++i;
    }
    if (mktime(&timedata) == (time_t)-1)
        throw runtime_error("fail mktime()");
    _year = timedata.tm_year+1900;
    _month = timedata.tm_mon+1;
    _day = timedata.tm_mday;
    if (!useTime)
        _seconds = 0xffffffff;
    else
        _seconds = timedata.tm_hour*3600 + timedata.tm_min*60 + timedata.tm_sec;
}
void date_t::set_time(uint16_t hour,uint16_t minute,uint16_t second)
{
    _seconds = hour*3600 + minute*60 + second;
}
dow date_t::get_dow() const
{
    tm timedata;
    memset(&timedata,0,sizeof(struct tm));
    timedata.tm_year = _year-1900;
    timedata.tm_mon = _month-1;
    timedata.tm_mday = _day;
    if (mktime(&timedata) == (time_t)-1)
        throw runtime_error("fail mktime()");
    return dow(timedata.tm_wday);
}
uint16_t date_t::get_hour() const
{
    return _seconds / 3600;
}
uint16_t date_t::get_minute() const
{
    return _seconds % 3600 / 60;
}
uint16_t date_t::get_second() const
{
    return _seconds % 3600 % 60;
}
string date_t::to_span_string() const
{
    // interpret date as a time span (time difference)
    bool b;
    ostringstream ss;
    if (_year > 0) {
        ss << _year << " year" << (_year>1 ? "s" : "");
        b = true;
    }
    if (_month > 0) {
        ss << (b ? ", " : "") << _month << " month" << (_month>1 ? "s" : "");
        b = true;
    }
    if (_day > 0) {
        ss << (b ? ", " : "") << _day << " day" << (_day>1 ? "s" : "");
        b = true;
    }
    if (!b)
        ss << "time has elapsed";
    return ss.str();
}
string date_t::to_span_string_withtime() const
{
    // interpret date as a time span (time difference)
    bool b;
    uint16_t tm;
    ostringstream ss;
    if (_year > 0) {
        ss << _year << " year" << (_year>1 ? "s" : "");
        b = true;
    }
    if (_month > 0) {
        ss << (b ? ", " : "") << _month << " month" << (_month>1 ? "s" : "");
        b = true;
    }
    if (_day > 0) {
        ss << (b ? ", " : "") << _day << " day" << (_day>1 ? "s" : "");
        b = true;
    }
    if ((tm = get_hour()) > 0) {
        ss << (b ? ", " : "") << tm << " hour" << (tm>1 ? "s" : "");
        b = true;
    }
    if ((tm = get_minute()) > 0) {
        ss << (b ? ", " : "") << tm << " minute" << (tm>1 ? "s" : "");
        b = true;
    }
    if ((tm = get_second()) > 0) {
        ss << (b ? ", " : "") << tm << " second" << (tm>1 ? "s" : "");
        b = true;
    }
    if (!b)
        ss << "time has elapsed";
    return ss.str();

}
string date_t::to_date_string(bool full) const
{
    ostringstream ss;
    ss << dow_to_string(get_dow(),!full) << ' ' << month_to_string(_month,!full) << ' ' << _day << ' ' << _year;
    return ss.str();
}
string date_t::to_date_string_withtime(bool full) const
{
    uint16_t hr, tm;
    const char* tod;
    ostringstream ss;
    ss << dow_to_string(get_dow(),!full) << ' ' << month_to_string(_month,!full) << ' ' << _day << ' ' << _year;
    if (_seconds != 0xffffffff) {
        hr = get_hour();
        tod = hr<=11 ? "AM" : "PM";
        if (hr == 0)
            hr = 12;
        else if (hr >= 13)
            hr -= 12;
        ss.fill('0');
        ss << ' ' << hr;
        if ((tm = get_minute()) > 0)
            ss << ':' << setw(2) << tm;
        if ((tm = get_second()) > 0)
            ss << ':' << setw(2) << tm;
        ss << ' ' << tod;
    }
    return ss.str();
}

// operator overloads
binwriter& agenda::operator <<(binwriter& stream,const date_t& date)
{
    return stream << date._year << date._month << date._day << date._seconds;
}
binreader& agenda::operator >>(binreader& stream,date_t& date)
{
    return stream >> date._year >> date._month >> date._day >> date._seconds;
}
bool agenda::operator >(const date_t& d1,const date_t& d2)
{
    uint16_t y[2], m[2], h[2], n[2], s[2];
    y[0] = d1.get_year(); y[1] = d2.get_year();
    if (y[0] > y[1])
        return true;
    if (y[0] == y[1]) {
        m[0] = d1.get_month(); m[1] = d2.get_month();
        if (m[0] > m[1])
            return true;
        if (m[0] == m[1]) {
            if (d1.get_day() > d2.get_day())
                return true;
            if (d1.get_day() == d2.get_day()) {
                h[0] = d1.get_hour(); h[1] = d2.get_hour();
                if (h[0] > h[1])
                    return true;
                if (h[0] == h[1]) {
                    n[0] = d1.get_minute(); n[1] = d2.get_minute();
                    if (n[0] > n[1])
                        return true;
                    if (n[0] == n[1]) {
                        s[0] = d1.get_second(); s[1] = d2.get_second();
                        if (s[0] > s[1])
                            return true;
                    }
                }
            }
        }
    }
    return false;
}
bool agenda::operator >=(const date_t& d1,const date_t& d2)
{
    return !operator <(d1,d2);
}
bool agenda::operator <(const date_t& d1,const date_t& d2)
{
    uint16_t y[2], m[2], h[2], n[2], s[2];
    y[0] = d1.get_year(); y[1] = d2.get_year();
    if (y[0] < y[1])
        return true;
    if (y[0] == y[1]) {
        m[0] = d1.get_month(); m[1] = d2.get_month();
        if (m[0] < m[1])
            return true;
        if (m[0] == m[1]) {
            if (d1.get_day() < d2.get_day())
                return true;
            if (d1.get_day() == d2.get_day()) {
                h[0] = d1.get_hour(); h[1] = d2.get_hour();
                if (h[0] < h[1])
                    return true;
                if (h[0] == h[1]) {
                    n[0] = d1.get_minute(); n[1] = d2.get_minute();
                    if (n[0] < n[1])
                        return true;
                    if (n[0] == n[1]) {
                        s[0] = d1.get_second(); s[1] = d2.get_second();
                        if (s[0] < s[1])
                            return true;
                    }
                }
            }
        }
    }
    return false;
}
bool agenda::operator <=(const date_t& d1,const date_t& d2)
{
    return !operator >=(d1,d2);
}
bool agenda::operator ==(const date_t& d1,const date_t& d2)
{
    return d1.get_year()==d2.get_year() && d1.get_month()==d2.get_month()
        && d1.get_day()==d2.get_day() && d1.get_hour()==d2.get_hour()
        && d1.get_minute()==d2.get_minute() && d1.get_second()==d2.get_second();
}
bool agenda::operator !=(const date_t& d1,const date_t& d2)
{
    return d1.get_year()!=d2.get_year() || d1.get_month()!=d2.get_month()
        || d1.get_day()!=d2.get_day() || d1.get_hour()!=d2.get_hour()
        || d1.get_minute()!=d2.get_minute() || d1.get_second()!=d2.get_second();
}
/*date_t agenda::operator +(const date_t& d1,const date_t& d2)
{
}
date_t agenda::operator -(const date_t& d1,const date_t& d2)
{
}*/
