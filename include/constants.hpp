#ifndef _CONSTANTS_HPP
#define _CONSTANTS_HPP

#include <string>

#define WRITE_DB 1
#define LOG_GA_STEPS 1
//note: must LOG_GA_STEPS in order to LOG_REG_STEPS
#define LOG_REG_STEPS 1

using namespace std;

const string RUN_FILE = "runs.toml";
const string FIG_PATH = "fig";
const bool FIX_RNG = true;
const uint FIXED_RNG_SEED = 12345678;
const string LOG_DIR = "data";

#endif
