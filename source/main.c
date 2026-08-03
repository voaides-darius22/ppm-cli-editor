#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header_files/cli_engine.h"

#define RUNNING 1

int main()
{

    CliEngine *sys = create_cli_engine();
    if (!sys) {
        printf("Memory allocation for system failed!\n");
        return ERROR_SYSTEM_SIGNAL;
    }

    while (RUNNING) {
        uint8_t signal = cli_parser(sys);
        if (signal == KILL_SYSTEM_SIGNAL) {
            break;
        }
    }

    return 0;
}
