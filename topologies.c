#include "graph.h"
#include "net.h"
#include "comm.h"

extern void network_start_pkt_receiver_thread(graph_t *topo);

/* トポロジーグラフを作成します */
graph_t *build_first_topo(){

    graph_t *topo = create_new_graph("Hello World Generic Graph");

    /* nodeを作成 */
    node_t *R0_re = create_graph_node(topo, "R0_re");
    node_t *R1_re = create_graph_node(topo, "R1_re");
    node_t *R2_re = create_graph_node(topo, "R2_re");

    /* node同士を繋げる */
    insert_link_between_two_nodes(R0_re, R1_re, "eth0/0", "eth0/1", 1);
    insert_link_between_two_nodes(R1_re, R2_re, "eth0/2", "eth0/3", 1);
    insert_link_between_two_nodes(R0_re, R2_re, "eth0/4", "eth0/5", 1);

    /* 
     * MAC/ip addressをノードやインターフェースにセット 
     * MACアドレスはランダムな値を自動でセット
     */
    node_set_loopback_address(R0_re, "122.1.1.0");
    node_set_intf_ip_address(R0_re, "eth0/4", "40.1.1.1", 24);
    node_set_intf_ip_address(R0_re, "eth0/0", "20.1.1.1", 24);

    node_set_loopback_address(R1_re, "122.1.1.1");
    node_set_intf_ip_address(R1_re, "eth0/1", "20.1.1.2", 24);
    node_set_intf_ip_address(R1_re, "eth0/2", "30.1.1.1", 24);

    node_set_loopback_address(R2_re, "122.1.1.2");
    node_set_intf_ip_address(R2_re, "eth0/3", "30.1.1.2", 24);
    node_set_intf_ip_address(R2_re, "eth0/5", "40.1.1.2", 24);

    network_start_pkt_receiver_thread(topo);

    return topo;
}
