#include "graph.h"
#include "CommandParser/libcli.h"
#include "comm.h"

extern graph_t *build_first_topo();
extern graph_t *build_linear_topo();

graph_t *topo = NULL;

int main(int argc, char **argv){

    nw_init_cli();

    //topo = build_first_topo();
    topo = build_linear_topo();

    sleep(2);

    /* 送る方のノード */
    //node_t *snode = get_node_by_name(topo, "R0_re");
    //interface_t *oif = get_node_if_by_name(snode, "eth0/0");

    //char msg[] = "Hello, how are you\0";
    //send_pkt_out(msg, strlen(msg), oif);

    start_shell();

    return 0;
}
