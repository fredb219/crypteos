#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "DBManager.h"

#ifdef HAVE_UNISTD_H
#include "unistd.h"
#elif HAVE_CONIO_H
#include "conio.h"
#endif

namespace po = boost::program_options;

int main(int argc, char **argv) {
	// Declare a group of options that will be allowed only on command line
	po::options_description optionsGeneric("Generic options");
	optionsGeneric.add_options()("version,v", "print version string")("help,h", "produce help message");

	po::options_description optionsConfigure("Configuration options");
	optionsConfigure.add_options()("file,f", po::value<std::string>()->default_value("./keys.cryped"), "Specify a file containing the encrypted password.");

	po::options_description optionsGet("Get Keys");
	optionsGet.add_options()("list,l", "List all the services on which password has been saved.")("get,g", po::value<std::string>(),
			"Get a password for an existing service.");

	po::options_description optionsAdd("Add keys");
	optionsAdd.add_options()("add,a", po::value<std::string>(), "Add a service. (use the --key switch to input the key).")("key,k", po::value<std::string>(),
			"Add a key on a specified service (use the --add switch to specify the service).");

	po::options_description cmdline_options;
	cmdline_options.add(optionsGeneric).add(optionsConfigure).add(optionsGet).add(optionsAdd);
	po::options_description visible("Allowed options");
	visible.add(optionsGeneric).add(optionsConfigure).add(optionsGet).add(optionsAdd);

	po::positional_options_description pd;
	pd.add("file", 1);

	po::variables_map options;
	try {
		// Parse command line
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(visible).positional(pd).run();
		po::store(parsed, options);
		po::notify(options);
	} catch (std::logic_error & error) {
		std::cerr << error.what() << std::endl << visible << std::endl;
		return -1;
	}

	if (options.count("help")) {
		std::cout << visible << std::endl;
		return 0;
	}
	if (options.count("version")) {
		std::cout << "Version 0.9" << std::endl;
		return 0;
	}

	std::string filename = options["file"].as<std::string> ();

	// Get keys
	std::string password;
	std::cout << "Need the encryption passphrase: ";
	std::cin >> password;
	DBManager manager(password, filename);

	if (options.count("list")) {
		std::vector<std::string> services = manager.getServiceNames();
		if (services.size() == 0) {
			std::cout << "No service ..." << std::endl;
		}
		for (unsigned int i = 0; i < services.size(); ++i) {
			std::cout << services[i] << std::endl;
		}
		return 0;
	}
	if (options.count("get")) {
		std::cout << manager.getKey(options["get"].as<std::string>()) << std::endl;
		return 0;
	}
	if (options.count("add") && options.count("key")) {
		manager.addKey(options["add"].as<std::string>(), options["key"].as<std::string>());
		manager.applyChanges();
		return 0;
	}

	std::cerr << visible << std::endl;
	return 1;

	return 0;
}
