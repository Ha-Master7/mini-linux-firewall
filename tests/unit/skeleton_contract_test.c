#include <stdio.h>

#include "cli_parser.h"
#include "printer.h"
#include "mfw_packet.h"
#include "mfw_engine.h"
#include "mfw_netfilter.h"
#include "mfw_debugfs.h"

int main(void)
{
    /*
     * These modules are still skeletons in the current tree. This test keeps a
     * compile-time contract for their headers so each file has a unit-test
     * anchor before real logic is added.
     */
    printf("[PASS] skeleton module headers compile as unit-test contracts\n");
    return 0;
}
