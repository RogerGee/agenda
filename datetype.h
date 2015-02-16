/* datetype.h */
#ifndef AGENDA_DATETYPE_H
#define AGENDA_DATETYPE_H
#include <exception>
#include "binstream.h"

namespace agenda
{
    class date_exception : public std::exception
    {
    public:
        date_exception(const char* message) : _msg(message) {}
    private:
        const char* _msg;
        virtual const char* what() const throw()
        { return _msg; }
    };

    enum dow
    {
        sunday,
        monday,
        tuesday,
        wednesday,
        thursday,
        friday,
        saturday,
        invalid_dow
    };

    // represents either a specific date or the diff between two dates (time span)
    class date_t
    {
        friend binwriter& operator<<(binwriter&,const date_t&);
        friend binreader& operator>>(binreader&,date_t&);
    public:
        date_t(); // initialize to now
        date_t(uint16_t month,uint16_t day); // initialize to month/day of now
        date_t(uint16_t month,uint16_t day,uint16_t year); // initialize to month/day/year of now
        date_t(std::string tokens[],int count); // initialize to now with modifications based on input tokens

        void set_year(uint16_t year)
        { _year = year-1990; }
        void set_month(uint16_t month)
        { _month = month; }
        void set_day(uint16_t day)
        { _day = day; }
        void set_time(uint16_t hour,uint16_t minute = 0,uint16_t second = 0);

        dow get_dow() const;
        uint16_t get_year() const
        { return _year; }
        uint16_t get_month() const
        { return _month; }
        uint16_t get_day() const
        { return _day; }
        uint16_t get_hour() const;   // [0-23]
        uint16_t get_minute() const; // [0-59]
        uint16_t get_second() const; // [0-59]

        // non-zero years, non-zero months, non-zero days, ... else "time has elapsed"
        std::string to_span_string() const;
        std::string to_span_string_withtime() const;

        // DOW MON   DAY YEAR [full==false]
        // DOW MONTH DAY YEAR [full==true]
        // [withtime]
        // DOW MON   DAY YEAR HOUR[:MINUTE>0[:SECOND>0]] PM|AM [full==false]
        // DOW MONTH DAY YEAR HOUR[:MINUTE>0[:SECOND>0]] PM|AM [full==true]
        std::string to_date_string(bool full = false) const;
        std::string to_date_string_withtime(bool full = false) const;
    private:
        uint16_t _year;
        uint16_t _month;
        uint16_t _day;
        uint32_t _seconds; // if 0xffffffff then time is not to be used
    };

    binwriter& operator <<(binwriter&,const date_t&);
    binreader& operator >>(binreader&,date_t&);

    bool operator >(const date_t&,const date_t&);
    bool operator >=(const date_t&,const date_t&);
    bool operator <(const date_t&,const date_t&);
    bool operator <=(const date_t&,const date_t&);
    bool operator ==(const date_t&,const date_t&);
    bool operator !=(const date_t&,const date_t&);
    date_t operator +(const date_t&,const date_t&); // add two timespans (returns timespan)
    date_t operator -(const date_t&,const date_t&); // subtract two dates (returns timespan)
}

#endif
