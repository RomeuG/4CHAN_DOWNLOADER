#include <Client.hpp>
#include <argp.h>
#include <cstdio>
#include <cstring>
#include <getopt.h>

struct ArgOpts {
    int OptJ;
    int OptC;
    int OptB;
    int OptT;
    char* ArgC;
    char* ArgB;
    char* ArgT;
};

ArgOpts pargs = {};

// argp stuff
const char* argp_program_version = "4chandownloader.0.0.1";
const char* argp_program_bug_address = "<romeu.bizz@gmail.com>";
static char doc[] = "Software to download 4chan threads.";
static char args_doc[] = "[FILENAME]...";
static struct argp_option options[] = {
    { "json", 'j', 0, 0, "Print json as output." },
    { "catalog", 'c', "value", 0, "Get catalog." },
    { "board", 'b', "value", 0, "Board option." },
    { "thread", 't', "value", 0, "Get thread." },
    { 0 }
};

auto argopts_debug() -> void
{
    std::printf("OptJ = %d\n"
                "OptC = %d | ArgC = %s\n"
                "OptB = %d | ArgB = %s\n"
                "OptT = %d | ArgT = %s\n",
                pargs.OptJ,
                pargs.OptC, pargs.ArgC,
                pargs.OptB, pargs.ArgB,
                pargs.OptT, pargs.ArgT);
}

static auto argp_parseopts(int key, char* arg, struct argp_state* state) -> error_t
{
    switch (key) {
        case 'j':
            pargs.OptJ = 1;
            break;
        case 'c':
            pargs.OptC = 1;
            pargs.ArgC = strdup(arg);
            break;
        case 'b':
            pargs.OptB = 1;
            pargs.ArgB = strdup(arg);
            break;
        case 't':
            pargs.OptT = 1;
            pargs.ArgT = strdup(arg);
            break;
        case ARGP_KEY_ARG:
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

auto get_catalog_json() -> std::string
{
    std::string result = "";

    channer::get_catalog_json(
        "g",
        [&result](std::string json) {
            result = json;
        },
        [](std::string const& e) {
            std::printf("Exception: %s\n", e.c_str());
        });

    return result;
}

auto get_thread_json() -> std::string
{
    std::string result = "";

    channer::get_thread_json(
        "g", "",
        [&result](std::string json) {
            result = json;
        },
        [](std::string const& e) {
            std::printf("Exception %s\n", e.c_str());
        });

    return result;
}

auto main(int argc, char** argv) -> int
{
    static struct argp argp = { options, argp_parseopts, args_doc, doc, 0, 0, 0 };
    argp_parse(&argp, argc, argv, 0, 0, &pargs);

    argopts_debug();

    return 0;
}
