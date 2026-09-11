#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../header_files/engine/cli_engine.h"

#define RUNNING 1

int main()
{

    CliEngine *sys = cli_engine_ctor();
    if (!sys) {
        printf("Memory allocation for system failed!\n");
        return ERROR_SYSTEM_SIGNAL;
    }

    while (RUNNING) {
        int8_t signal = cli_parser(sys);
        if (signal == KILL_SYSTEM_SIGNAL) {
            break;
        }
    }
    return 0;
}
