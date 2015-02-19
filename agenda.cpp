#include "task.h"
#include <iostream>
#include <cctype>
using namespace std;
using namespace agenda;

// errors
enum home_directory_not_set {};

// functions
static int agenda_stat(int argc,string argv[]);
static int agenda_add(int argc,string argv[]);
static int agenda_remove(int argc,string argv[]);
static int agenda_edit(int argc,string argv[]);
static int agenda_old(int argc,string argv[]);
static const char* get_current_task_file();
static const char* get_old_task_file();
template<typename T> T convert(const string& arg);

// globals
static const char* programName;
static const char* const currentTaskFile = get_current_task_file();
static const char* const oldTaskFile = get_old_task_file();
static task_file currentTasks(currentTaskFile), oldTasks(oldTaskFile);

int main(int argc,const char* argv[])
{
    int exitCode = 0;
    vector<string> args;
    for (int i = 1;i < argc;++i)
        args.push_back(argv[i]);
    programName = argv[0];
    { // remove old tasks and display that they went out of date
        vector<const task*> outdated;
        currentTasks.remove_outdated(oldTasks,outdated);
        if (outdated.size() > 0) {
            cout << "The following tasks have become outdated:\n";
            for (auto iter = outdated.begin();iter != outdated.end();++iter) {
                cout << " - ";
                (*iter)->print(cout);
                cout.put('\n');
            }
            cout.put('\n');
        }
    }
    if (argc <= 1)
        exitCode = agenda_stat(0,NULL);
    else if (args[0] == "stat")
        exitCode = agenda_stat(argc-2,&args[1]);
    else if (args[0] == "add")
        exitCode = agenda_add(argc-2,&args[1]);
    else if (args[0] == "remove" || args[0] == "rm")
        exitCode = agenda_remove(argc-2,&args[1]);
    else if (args[0] == "edit" || args[0] == "ed")
        exitCode = agenda_edit(argc-2,&args[1]);
    else if (args[0] == "old")
        exitCode = agenda_old(argc-2,&args[1]);
    else {
        cerr << argv[0] << ": unrecognized command '" << argv[1] << "'\n";
        exitCode = 1;
    }
    return exitCode;
}

int agenda_stat(int,string[])
{
    /* syntax: stat */
    currentTasks.print(cout);
    return 0;
}

int agenda_add(int argc,string argv[])
{
    /* syntax: add <kind> <desc> [date/time] */
    if (argc < 2) {
        cerr << programName << ": add <kind> <desc> [date/time]\n";
        return 1;
    }
    try {
        date_t date(argv+2,argc-2);
        if (date < date_t())
            throw date_exception("unable to add task; date has already passed");
        currentTasks.new_task(task::task_kind_from_string(argv[0]),argv[1],date);
    } catch (exception& ex) {
        cerr << programName << ": add: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

int agenda_remove(int argc,string argv[])
{
    /* syntax: remove|rm <id> [--keep] */
    if (argc < 1) {
        cerr << programName << ": remove <id>\n";
        return 1;
    }
    try {
        task* t;
        uint32_t id;
        id = convert<uint32_t>(argv[0]);
        t = currentTasks.remove(id);
        if (t == NULL)
            cout << programName << ": remove: item with id='" << id << "' was not found\n";
        else {
            // go through optional arguments
            for (int i = 1;i < argc;++i) {
                if (argv[i] == "--keep") {
                    oldTasks.add_task(t);
                    return 0;
                }
            }
            delete t;
        }
    } catch (exception& ex) {
        cerr << programName << ": remove: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

int agenda_edit(int argc,string argv[])
{
    /* syntax: edit|ed <id> [--date <date-string>] [--desc <desc-string>] [--kind <kind>] */
    if (argc < 3) {
        cerr << programName << ": edit <id> [--date <date-string>] [--desc <desc-string>] [--kind <kind>]\n";
        return 1;
    }
    try {
        int i;
        uint32_t id;
        task* item;
        id = convert<uint32_t>(argv[0]);
        item = currentTasks.get_task(id);
        i = 1;
        while (i < argc) {
            if (argv[i] == "--date") {
                // compute the number of date arguments by
                // finding the next --option token or end of stream
                int j = i+1;
                while (j<argc && argv[j][0]!='-')
                    ++j;
                j -= i+1;
                // parse date
                date_t date(argv+i+1,j);
                item->set_date(date);
                i += j+1;
            }
            else if (argv[i] == "--desc") {
                // the description should be a SINGLE argument
                item->set_desc(argv[i+1]);
                i += 2;
            }
            else if (argv[i] == "--kind") {
                // the kind should be a SINGLE argument
                item->set_kind( task::task_kind_from_string(argv[i+1]) );
                i += 2;
            }
            else {
                cerr << programName << ": edit: unrecognized option '" << argv[i] << "'\n";
                return 1;
            }
        }
    } catch (exception& ex) {
        cerr << programName << ": edit: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

int agenda_old(int,string[])
{
    /* syntax: old */
    oldTasks.print(cout);
    return 0;
}

const char* get_current_task_file()
{
#ifdef AGENDA_DEBUG
    return "current-tasks-debug";
#else
    static string file;
    if (file.length() == 0) {
        const char* HOME = getenv("HOME");
        if (HOME == NULL)
            throw home_directory_not_set(0);
        file = HOME;
        file += "/.agenda-current";
    }
    return file.c_str();
#endif
}
const char* get_old_task_file()
{
#ifdef AGENDA_DEBUG
    return "old-tasks-debug";
#else
    static string file;
    if (file.length() == 0) {
        const char* HOME = getenv("HOME");
        if (HOME == NULL)
            throw home_directory_not_set(0);
        file = HOME;
        file += "/.agenda-old";
    }
    return file.c_str();
#endif
}

template<typename T>
T convert(const string& arg)
{
    T value = 0;
    for (size_t i = 0;i < arg.size();++i) {
        
        if (!isdigit(arg[i]))
            throw runtime_error("argument must be an integer");
        value *= 10;
        value += arg[i] - '0';
    }
    return value;
}
