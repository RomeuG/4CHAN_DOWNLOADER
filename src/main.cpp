#include <Client.hpp>
#include <getopt.h>

auto get_catalog_json() -> std::string
{
    std::string result = "";

    channer::get_catalog_json(
        "g",
        [&result](std::string json) {
            result = json;
        },
        [](std::string const& e) {
            std::printf("Exception: %s\n");
        });

    return result;
}

int main(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "j")) != -1) {
        switch (opt) {
            case 'j':
                std::printf("%s\n", get_catalog_json().c_str());
                break;
            default:
                break;
        }
    }

    return 0;
}
