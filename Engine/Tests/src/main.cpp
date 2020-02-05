//#ifndef PLATFORM_PS
#define CATCH_CONFIG_NO_POSIX_SIGNALS
#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"

int main(int argc, char* argv[]) {
	// global setup...

	Catch::Session session{};
	session.applyCommandLine(argc, argv);

	std::cout << "Initial command line rng seed: " << session.config().rngSeed() << "\n";


	if(session.config().rngSeed() == 0) {
		Catch::ConfigData configDataWithSeed = session.configData();
		configDataWithSeed.rngSeed = 123321;
		session.useConfigData(configDataWithSeed);
		std::cout << "No random seed was specified in initial command line arguments. New hardcoded seed: " << session.config().rngSeed() << "\n";
	}

	std::cout.flush();

	int result = session.run();

	// global clean-up...

	return result;
}
//#endif
