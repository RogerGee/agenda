/* task.cpp */
#include "task.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
using namespace std;
using namespace agenda;

extern void string_to_lower(string&);

// task
task::task(task_kind kind)
    : _kind(kind)
{
    _id = task_file::get_next_id();
}
task::task(binreader& input)
{
    uint16_t t;
    input >> _id >> _desc;
    input >> t;
    _kind = task_kind(t);
    input >> _date;
}
/*static*/ task_kind task::task_kind_from_string(std::string& s)
{
    string_to_lower(s);
    if (s == "hw" || s == "homework")
        return task_homework;
    if (s == "paper")
        return task_paper;
    if (s == "proj" || s == "project")
        return task_project;
    if (s == "exam" || s == "test")
        return task_exam;
    if (s == "app" || s == "appmt" || s == "appointment")
        return task_appmt;
    if (s == "evnt" || s == "event")
        return task_event;
    if (s == "task" || s == "job")
        return task_task;
    if (s == "other" || s == "misc")
        return task_misc;
    throw task_exception("bad task kind string");
}
/*static*/ const char* task::task_kind_to_string(task_kind kind)
{
    switch (kind) {
    case task_homework:
        return "homework";
    case task_paper:
        return "paper";
    case task_project:
        return "project";
    case task_exam:
        return "exam";
    case task_appmt:
        return "appmt";
    case task_event:
        return "event";
    case task_task:
        return "task";
    case task_misc:
        return "misc";
    default:
        return "unknown task";
    }
    return "unknown task";
}
void task::print(ostream& stream) const
{
    stream << setw(3) << _id << " [" << task_kind_to_string(_kind) <<  "]\t" << setw(30) 
           <<  _desc << "\t[" << _date.to_date_string_withtime() << ']';
}
void task::save(binwriter& stream) const
{
    stream << _id << _desc << uint16_t(_kind) << _date;
}

// task_file
/*static*/ uint32_t task_file::_idTop = 0;
task_file::task_file(const char* name)
{
    try {
        binreader reader(name);
        while (!reader.eof()) {
            task* t = new task(reader);
            if (t->get_id() > _idTop)
                _idTop = t->get_id();
            _tasks[t->get_id()] = t;
        }
    } catch (file_does_not_exist) {}
    _name = name;
    _modified = false;
}
task_file::~task_file()
{
    if (_modified) {
        vector<const task*> tasks;
        binwriter writer(_name);
        for (auto iter = _tasks.begin();iter != _tasks.end();++iter)
            tasks.push_back(iter->second);
        std::sort(tasks.begin(),tasks.end(),[](const task* left,const task* right){return left->get_id()<right->get_id();});
        for (size_t i = 0;i < tasks.size();++i) {
            tasks[i]->save(writer);
            delete tasks[i];
        }
    }
}
task* task_file::new_task(task_kind kind,const string& desc,const date_t& date)
{
    task* t = new task(kind);
    t->_desc = desc;
    t->_date = date;
    _tasks[t->get_id()] = t;
    _modified = true;
    return t;
}
void task_file::add_task(task* item)
{
    _tasks[item->get_id()] = item;
    _modified = true;
}
task* task_file::remove(uint32_t id)
{
    auto iter = _tasks.find(id);
    if (iter != _tasks.end()) {
        task* t = iter->second;
        _tasks.erase(iter); // just erases the reference
        _modified = true;
        return t;
    }
    return NULL;
}
void task_file::remove_outdated(task_file& dest,vector<const task*>& removed)
{
    date_t now;
    for (auto iter = _tasks.begin();iter != _tasks.end();++iter) {
        if (iter->second->_date < now) {
            dest._tasks[iter->second->get_id()] = iter->second;
            dest._modified = true;
            removed.push_back(iter->second);
            _modified = true;
        }
    }
    // remove items from the collection (this can't be done while iterating)
    for (size_t i = 0;i < removed.size();++i)
        _tasks.erase(removed[i]->get_id());
}
void task_file::print(ostream& stream) const
{
    vector<const task*> tasks;
    for (auto iter = _tasks.begin();iter != _tasks.end();++iter)
        tasks.push_back(iter->second);
    std::sort(tasks.begin(),tasks.end(),[](const task* left,const task* right){return left->get_date()<right->get_date();});
    for (size_t i = 0;i < tasks.size();++i) {
        tasks[i]->print(stream);
        stream.put('\n');
    }
}
task* task_file::get_task(uint32_t id)
{
    task* t;
    try {
        t = _tasks.at(id);
    } catch (out_of_range ex) {
        return NULL;
    }
    // assume a modification will be made
    _modified = true;
    return t;
}
const task* task_file::get_task(uint32_t id) const
{
    const task* t;
    try {
        t = _tasks.at(id);
    } catch (out_of_range ex) {
        return NULL;
    }
    return t;
}
/*static*/ uint32_t task_file::get_next_id()
{
    return ++_idTop;
}
