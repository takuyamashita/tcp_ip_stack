#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>

#include "graph.h"

extern init_udp_socket(node_t *node);

/* 
 * nodeとnodeをlinkで繋げます
 * インターフェースもセットします
 */
void insert_link_between_two_nodes(
    node_t *node1,
    node_t *node2,
    char *from_if_name,
    char *to_if_name,
    unsigned int cost
){
    link_t *link;
    int empty_intf_slot;
    
    /* link1つ分を確保 */
    if((link = calloc(1, sizeof(link_t))) == NULL){
	perror("calloc");
	exit(1);
    }

    /* 名前をセット */
    strncpy(link->intf1.if_name, from_if_name, IF_NAME_SIZE);
    strncpy(link->intf2.if_name, to_if_name, IF_NAME_SIZE);
    
    link->intf1.if_name[IF_NAME_SIZE - 1] = '\0';
    link->intf2.if_name[IF_NAME_SIZE - 1] = '\0';

    /* インターフェースに接続しているリンクをセット */
    link->intf1.link = link;
    link->intf2.link = link;

    /* インターフェースに、アタッチされているnodeをセット */
    link->intf1.att_node = node1;
    link->intf2.att_node = node2;

    /* コストをセット */
    link->cost = cost;

    /* nodeのインターフェースリストにインターフェースを加える */
    empty_intf_slot = get_node_intf_available_slot(node1);
    node1->intf[empty_intf_slot] = &link->intf1;

    empty_intf_slot = get_node_intf_available_slot(node2);
    node2->intf[empty_intf_slot] = &link->intf2;

    /* リンクのインターフェースのプロパティを初期化 */
    init_intf_nw_prop(&link->intf1.intf_nw_props);
    init_intf_nw_prop(&link->intf2.intf_nw_props);

    /* ランダムなMACアドレスをインターフェースに付与 */
    interface_assign_mac_address(&link->intf1);
    interface_assign_mac_address(&link->intf2);
}

/* 新しくgraphを作成して返します */
graph_t *create_new_graph(char *topology_name){
    
    graph_t *graph;

    /* graph_t1つ分確保 */
    if((graph = calloc(1, sizeof(graph_t))) == NULL){
	perror("calloc");
	exit(1);
    }

    /* 32文字コピーしてnull文字をセット */
    strncpy(graph->topology_name, topology_name, 32);
    graph->topology_name[31] = '\n';

    /* glthreadを初期化 */
    init_glthread(&graph->node_list);

    return graph;
}

/* graphに新しくnodeをセットします 
 * 作成したnodeを返します
 * */
node_t *create_graph_node(graph_t *graph, char *node_name){
    
    node_t *node;

    /* node_t1つ分確保 */
    if((node = calloc(1, sizeof(node_t))) == NULL){
	perror("calloc");
	exit(1);
    }

    /* udp socketをセット */
    init_udp_socket(node);

    /* nodeの名前をセット */
    strncpy(node->node_name, node_name, NODE_NAME_SIZE);
    node->node_name[NODE_NAME_SIZE - 1] = '\0';

    /* graph_blueを初期化 */
    init_glthread(&node->graph_glue);

    /* 追加されるnode_listの2番目にセット */
    glthread_add_next(&graph->node_list, &node->graph_glue);

    return node;
}

/* graphをstdoutにdumpします */
void dump_graph(graph_t *graph){
    
    node_t *node;
    glthread_t *curr;

    printf("Topology Name = %s\n", graph->topology_name);

    ITERATE_GLTHREAD_BEGIN(&graph->node_list, curr){
	
	node = graph_glue_to_node(curr);
	dump_node(node);
    }ITERATE_GLTHREAD_END(&graph->node_list, curr);
}

/* nodeのをstdoutにdumpします */
void dump_node(node_t *node){
    
    unsigned int i = 0;
    interface_t *intf;

    printf("Node Name = %s : \n", node->node_name);

    for(; i < MAX_INTF_PER_NODE; i++){
	
	intf = node->intf[i];
	
	if(!intf)
	    break;
	
	dump_interface(intf);
    }
}

/* interfaceをstdoutにdumpします */
void dump_interface(interface_t *interface){
    
    link_t *link = interface->link;
    /* 接続先のノード */
    node_t *nbr_node = get_nbr_node(interface);

    printf(
	"Interface Name = %s\tNbr Node %s, Local Node : %s, cost %u\n",
	interface->if_name,
	nbr_node->node_name,
	interface->att_node->node_name,
	link->cost
    ); 
}

