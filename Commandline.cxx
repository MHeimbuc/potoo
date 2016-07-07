//
// Created by markus on 7/07/16.
//

#include "Commandline.hxx"

#include <boost/program_options.hpp>

#include <iostream>

boost::optional<Command> parse_options(int argc, const char **argv) {
    using namespace boost::program_options;

    // The command line variables
    std::string config, first_page, output;
    bool human;

    // All available options
    options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config,c", value<std::string>(&config))
        ("human,h", bool_switch(&human)->default_value(false))
        ("first_page,f", value<std::string>(&first_page))
        ("output,o", value<std::string>(&output));

    static const std::string description = "Allowed options:\n"
        "  --help\t\t\tproduce this help message\n"
        "\n"
        "  -c [--config] path\t\tpath to config json file (Required)\n"
        "\n"
        "And ONE of the following:\n"
        "  -f [--first_page] path\trender the first page of the pdf as PNG into <path>\n"
        "  -h [--human] \t\t\tprocess everything and print to stdout, mainly for debugging\n"
        "  -o [--output] path\t\tprocess everything and save it as json at <path>";

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    // Only print help text if --help or -h is specified
    if (vm.count("help") || argc == 1) {
        std::cout << description << std::endl;
        return boost::none;
    }

    // config is set, but output and human are not
    if (!config.empty() && (output.empty() || !human || first_page.empty())) {
        throw std::runtime_error(
            "please either specify an output file, the human flag or the first_page parameter\n\n" + description);
    }

    // We need a valid config, so we exit if there is none
    if (config.empty()) {
        std::cerr << "ERROR: no configuration file set" << std::endl;
        std::cerr << desc << std::endl;
        return boost::none;
    }

    if (!first_page.empty()) { // first_page parameter was supplied, so we just save the image
        FirstPageCommand fp;
        fp._config = config;
        fp._path = vm["first_page"].as<std::string>();
        return boost::make_optional(Command(fp));
    } else { // first_page parameter was not supplied, run the main routine
        if (human) { // We want to print for humans, so let's config that
            HumanCommand hc;
            hc._config = config;
            return boost::make_optional(Command(hc));
        } else { // Return the path for saving
            OutputCommand oc;
            oc._config = config;
            oc._path = vm["output"].as<std::string>();
            return boost::make_optional(Command(oc));
        }
    }
}