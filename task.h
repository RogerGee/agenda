/* task.h */
#ifndef AGENDA_TASK_H
#define AGENDA_TASK_H
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include "datetype.h"

namespace agenda
{
    enum task_kind
    {
        task_homework,
        task_paper,
        task_project,
        task_exam,
        task_appmt,
        task_event,
        task_task,
        task_misc
    };

    class task_exception : public std::exception
    {
    public:
        task_exception(const char* message) : _msg(message) {}
    private:
        const char* _msg;
        virtual const char* what() const throw()
        { return _msg; }
    };

    class task
    {
        friend class task_file;
    public:
        static task_kind task_kind_from_string(std::string& s);
        static const char* task_kind_to_string(task_kind kind);
        void print(std::ostream&) const;

        uint32_t get_id() const
        { return _id; }
        const std::string& get_desc() const
        { return _desc; }
        task_kind get_kind() const
        { return _kind; }
        const date_t& get_date() const
        { return _date; }

        void set_desc(const std::string& desc)
        { _desc = desc; }
        void set_kind(task_kind kind)
        { _kind = kind; }
        void set_date(const date_t& date)
        { _date = date; }
    private:
        task(task_kind kind);
        task(binreader&);

        void save(binwriter&) const;

        uint32_t _id;
        std::string _desc;
        task_kind _kind;
        date_t _date;
    };

    class task_file
    {
    public:
        task_file(const char* name);
        ~task_file();

        task* new_task(task_kind kind,const std::string& desc,const date_t& date); // create new task
        void add_task(task* item);
        task* remove(uint32_t id); // return removed task or NULL if not found
        void remove_outdated(task_file& dest,std::vector<const task*>& removed); // remove all outdated tasks and place in destination task_file

        void print(std::ostream&) const;

        task* get_task(uint32_t id);
        const task* get_task(uint32_t id) const;
        
        static uint32_t get_next_id();
    private:
        static uint32_t _idTop;
        const char* _name;
        std::unordered_map<uint32_t,task*> _tasks;
        bool _modified;
    };
}

#endif
