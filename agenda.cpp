#include "task.h"
#include <iostream>
using namespace std;
using namespace agenda;

// errors
enum home_directory_not_set {};

// functions
static int agenda_stat(int argc,string argv[]);
static int agenda_add(int argc,string argv[]);
static int agenda_remove(int argc,string argv[]);
static const char* get_current_task_file();
static const char* get_old_task_file();

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
    else {
        cerr << argv[0] << ": unrecognized command '" << argv[1] << "'\n";
        exitCode = 1;
    }
    return exitCode;
}

int agenda_stat(int,string[])
{
    currentTasks.print(cout);
    return 0;
}

int agenda_add(int argc,string argv[])
{
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

int agenda_remove(int,string[])
{
    return 0;
}

const char* get_current_task_file()
{
    static string file;
    if (file.length() == 0) {
        const char* HOME = getenv("HOME");
        if (HOME == NULL)
            throw home_directory_not_set(0);
        file = HOME;
        file += "/.agenda-current";
    }
    return file.c_str();
}
const char* get_old_task_file()
{
    static string file;
    if (file.length() == 0) {
        const char* HOME = getenv("HOME");
        if (HOME == NULL)
            throw home_directory_not_set(0);
        file = HOME;
        file += "/.agenda-old";
    }
    return file.c_str();
}