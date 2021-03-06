
#include "Commandline.hxx"

#include <boost/program_options.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>

Command parse_options(int argc, const char **argv) {
    using namespace boost::program_options;

    // The command line variables
    std::string config, single_page, output, info;
    bool human;
    int start = -1, end = -1, page = -1;

    // All available options
    options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config,c", value<std::string>(&config))
        ("human,h", bool_switch(&human)->default_value(false))
        ("single_page,S", value<std::string>(&single_page))
        ("output,o", value<std::string>(&output))
        ("info,i", value<std::string>(&info))
        ("start,s", value<int>(&start))
        ("end,e", value<int>(&end))
        ("page,p", value<int>(&page));

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    // Only print help text if --help or -h is specified
    if (vm.count("help") || argc == 1) {
        return HelpCommand{};
    }

    // We need a valid config, so we exit if there is none
    if (config.empty()) {
        throw std::runtime_error(
            "no configuration file set\n\n" + potoo_description);
    }

    // config is set, but output and human are not
    if (!config.empty() && !(!output.empty() || human || !single_page.empty() || !info.empty())) {
        throw std::runtime_error(
            "please either specify an output file, the human flag, the info or the single_page parameter"
        );
    }

    if (
        (vm.count("start") && start <= -1)
        || (vm.count("end") && end <= -1)
        || (vm.count("page") && page <= -1)
        ) {
        throw std::runtime_error("invalid start or end range or page");
    }

    boost::algorithm::trim(config);
    boost::algorithm::trim(single_page);
    boost::algorithm::trim(output);

    const static auto integer_to_optional = [](int param) -> boost::optional<int> {
        if (param == -1) return boost::none;
        else return boost::make_optional(param);
    };

    if (!single_page.empty()) { // single_page parameter was supplied, so we just save the image

        if (vm.count("start") || vm.count("end")) {
            throw std::runtime_error("start and end are invalid parameters for single_page");
        }

        PageCommand fp;
        fp._config = config;
        fp._path = single_page;
        fp._page = integer_to_optional(page);
        return fp;
    } else if (!info.empty()) {
        InfoCommand ic;
        ic._config = config;
        ic._path = info;
        return ic;
    }
    else { // single_page parameter was not supplied, run the main routine

        if ((vm.count("start") || vm.count("end")) && vm.count("page")) {
            throw std::runtime_error("start and/or end and page cannot be used at the same time");
        }

        if (human) { // We want to print for humans, so let's config that
            HumanCommand hc;
            hc._config = config;
            hc._start = integer_to_optional(start);
            hc._end = integer_to_optional(end);
            hc._page = integer_to_optional(page);
            return hc;
        } else { // Return the path for saving
            OutputCommand oc;
            oc._config = config;
            oc._path = output;
            oc._start = integer_to_optional(start);
            oc._end = integer_to_optional(end);
            oc._page = integer_to_optional(page);
            return oc;
        }
    }
}
