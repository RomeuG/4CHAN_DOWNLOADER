#include <Client.hpp>
#include <argp.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

struct ArgOpts {
    char* argc;
    char* argb;
    char* argt;
    char* argf;
    char* argi;
    int optj;
    int optc;
    int optb;
    int optt;
    int optf;
    int opti;
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
    { "file", 'f', "value", 0, "Get file." },
    { "images", 'i', "value", 0, "Get images." },
    { 0 }
};

auto argopts_debug() -> void
{
    fprintf(stdout,
            "OptJ = %d\n"
            "OptC = %d | ArgC = %s\n"
            "OptB = %d | ArgB = %s\n"
            "OptT = %d | ArgT = %s\n"
            "OptI = %d | ArgI = %s\n",
            pargs.optj,
            pargs.optc, pargs.argc,
            pargs.optb, pargs.argb,
            pargs.optt, pargs.argt,
            pargs.opti, pargs.argi);
}

static auto argp_parseopts(int key, char* arg, struct argp_state* state) -> error_t
{
    switch (key) {
        case 'j':
            pargs.optj = 1;
            break;
        case 'c':
            pargs.optc = 1;
            pargs.argc = strdup(arg);
            break;
        case 'b':
            pargs.optb = 1;
            pargs.argb = strdup(arg);
            break;
        case 't':
            pargs.optt = 1;
            pargs.argt = strdup(arg);
            break;
        case 'i':
            pargs.opti = 1;
            pargs.argi = strdup(arg);
            break;
        case ARGP_KEY_ARG:
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

auto thread_download_files(Thread const& thread) -> void
{
    for (auto& post : thread.posts) {
        if (post.file.has_value()) {
            auto file_url = post.file->url;
            auto file_path = std::string(pargs.argi) + "/" + post.file->name + post.file->ext;
            channer::download_media(
                file_url, file_path,
                [](bool success) {},
                [](std::string const& e) {
                    fprintf(stderr, "Exception: %s\n", e.c_str());
                });
        }
    }
}

auto catalog_download_files(Catalog const& catalog) -> void
{
    for (auto& entry : catalog.entries) {
        if (entry.file.has_value()) {
            auto file_url = entry.file->url;
            auto file_path = std::string(pargs.argi) + "/" + entry.file->name + entry.file->ext;
            channer::download_media(
                file_url, file_path,
                [](bool success) {},
                [](std::string const& e) {
                    fprintf(stderr, "Exception: %s\n", e.c_str());
                });
        }
    }
}

auto catalog_to_str(Catalog const& catalog) -> std::string
{
    return "catalog";
}

auto thread_to_str(Thread const& catalog) -> std::string
{
    return "thread";
}

auto get_catalog_json(std::string const& board) -> std::string
{
    std::string result = "";

    channer::get_catalog_json(
        board,
        [&result](std::string json) {
            result = json;
        },
        [](std::string const& e) {
            fprintf(stderr, "Exception: %s\n", e.c_str());
        });

    return result;
}

auto get_thread_json(std::string const& board, std::string const& thread) -> std::string
{
    std::string result = "";

    channer::get_thread_json(
        board, thread,
        [&result](std::string json) {
            result = json;
        },
        [](std::string const& e) {
            fprintf(stderr, "Exception: %s\n", e.c_str());
        });

    return result;
}

auto get_catalog_obj(std::string const& board) -> std::optional<Catalog>
{
    std::optional<Catalog> result;

    channer::get_catalog(
        board,
        [&result](std::optional<Catalog> catalog) {
            result = catalog;
        },
        [&result](std::string const& e) {
            fprintf(stderr, "Exception: %s\n", e.c_str());
            result = std::nullopt;
        });

    return result;
}

auto get_thread_obj(std::string const& board, std::string const& thread) -> std::optional<Thread>
{
    std::optional<Thread> result;
    channer::get_thread(
        board, thread,
        [&result](std::optional<Thread> thread) {
            result = thread;
        },
        [&result](std::string const& e) {
            fprintf(stderr, "Exception: %s\n", e.c_str());
            result = std::nullopt;
        });

    return result;
}

auto get_catalog() -> void
{
    if (pargs.optj) {
        std::string result = get_catalog_json(pargs.argc);
        fprintf(stdout, "%s\n", result.c_str());
    } else {
        auto result = get_catalog_obj(pargs.argc);
        if (result.has_value()) {
            if (pargs.opti) {
                catalog_download_files(result.value());
            } else {
                char const* converted = catalog_to_str(result.value()).c_str();
                fprintf(stdout, "%s\n", converted);
            }
        }
    }
}

auto get_thread() -> void
{
    if (pargs.optj) {
        std::string result = get_thread_json(pargs.argb, pargs.argt);
        fprintf(stdout, "%s\n", result.c_str());
    } else {
        auto result = get_thread_obj(pargs.argb, pargs.argt);
        if (result.has_value()) {
            if (pargs.opti) {
                thread_download_files(result.value());
            } else {
                char const* converted = thread_to_str(result.value()).c_str();
                fprintf(stdout, "%s\n", converted);
            }
        }
    }
}

auto main(int argc, char** argv) -> int
{
    static struct argp argp = { options, argp_parseopts, args_doc, doc, 0, 0, 0 };
    argp_parse(&argp, argc, argv, 0, 0, &pargs);

    argopts_debug();

    if (pargs.optc) {
        get_catalog();
    } else if (pargs.optt) {
        get_thread();
    }

    return 0;
}
