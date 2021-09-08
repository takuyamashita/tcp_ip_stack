#include "graph.h"
#include "utils.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned int hash_code(void *ptr, unsigned int size){
    
    unsigned int value = 0, i = 0;
    char *str = (char *)ptr;

    while(i < size){
	/* 1byteずつ計算する */
	value += *str;
	value *= 97;
	str++;
	i++;
    }

    return value;
}

/* interfaceに一意なMACアドレスを割り振ります */
void interface_assign_mac_address(interface_t *interface){
    
    node_t *node = interface->att_node;

    if(!node)
	return;

    unsigned int hash_code_val = 0;

    /* ノードの名前と、インターフェースの名前を組み合わせて一意にする */
    hash_code_val = hash_code(node->node_name, NODE_NAME_SIZE);
    hash_code_val *= hash_code(interface->if_name, IF_NAME_SIZE);

    /* アドレスをセット */
    memset(IF_MAC(interface), 0, sizeof(IF_MAC(interface)));
    memcpy(IF_MAC(interface), (char *)&hash_code_val, sizeof(unsigned int));
}

/* ノードにループバックアドレスをセットします */
bool_t node_set_loopback_address(node_t *node, char *ip_addr){
    
    assert(ip_addr);

    /* nodeのループバックアドレスにセット */
    strncpy(NODE_LO_ADDR(node), ip_addr, 16);
    NODE_LO_ADDR(node)[15] = '\0';
    /* 設定されたのでtrue */
    node->node_nw_prop.is_lb_addr_config = TRUE;

    return TRUE;
}

/* ノードの特定のインターフェースにipアドレスをセットします */
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask){
    
    interface_t *interface = get_node_if_by_name(node, local_if);

    if(!interface)
	assert(0);

    /* アドレスをセット */
    strncpy(IF_IP(interface), ip_addr, 16);
    IF_IP(interface)[15] = '\0';
    
    /* マスクをセット */
    interface->intf_nw_props.mask = mask;

    /* ipアドレスをセットしたのでtrueにする */
    interface->intf_nw_props.is_ipadd_config = TRUE;

    return TRUE;
}

bool_t node_unset_intf_ip_address(node_t *node, char *local_if){
    
    return TRUE;
}

/* ノードのネットワークプロパティーをダンプします */
void dump_node_nw_props(node_t *node){
    
    printf("\nNode Name = %s\n", node->node_name);

    if(node->node_nw_prop.is_lb_addr_config){
	
	printf("\t  lo addr : %s/32", NODE_LO_ADDR(node));
    }

    printf("\n");
}

/* インターフェースのネットワークプロパティをダンプします */
void dump_intf_props(interface_t *interface){
    
    dump_interface(interface);

    if(interface->intf_nw_props.is_ipadd_config){
	
	printf("\t IP Addr = %s/%u", IF_IP(interface), interface->intf_nw_props.mask);
	printf(
	    "\t MAC : %u:%u:%u:%u:%u:%u\n",
	    IF_MAC(interface)[0],
	    IF_MAC(interface)[1],
	    IF_MAC(interface)[2],
	    IF_MAC(interface)[3],
	    IF_MAC(interface)[4],
	    IF_MAC(interface)[5]    
	);
    } else {
	
	
    }
}


void dump_nw_graph(graph_t *graph){
    
    node_t *node;
    glthread_t *curr;
    interface_t *interface;
    unsigned int i;

    printf("Topology Name = %s\n", graph->topology_name);

    ITERATE_GLTHREAD_BEGIN(&graph->node_list, curr){

	node = graph_glue_to_node(curr);
	
	/* nodeのpropをダンプ */
	dump_node_nw_props(node);

	for(i = 0; i < MAX_INTF_PER_NODE; i++){
	    
	    interface = node->intf[i];

	    if(!interface)
		break;
	    
	    /* interfaceのpropsをダンプ */
	    dump_intf_props(interface);
	}

    }ITERATE_GLTHREAD_END(&graph->node_list, curr);
}

/*
 * パケットを最大まで右(後ろに)に詰めます
 * 詰めた後のpktのポインターを返します
 *
 * |                  MAX_PACKET_SIZE                    |
 * | | | | | | | | | | | | | | | | | | | | | | | | | | | | 
 * |IF_NAME_SIZE |   pkt_size    |         empty         |
 * |             |            total_buffer_size          |
 */
char *pkt_buffer_shift_right(
    char *pkt,
    unsigned int pkt_size,
    unsigned int total_buffer_size
){
    
    char *temp = NULL;
    bool_t need_temp_memory = FALSE;

    if(pkt_size * 2 > total_buffer_size){
	
	need_temp_memory = TRUE;
    }

    /* pkt_sizeが2つ分入らない */
    if(need_temp_memory){
	
	temp = calloc(1, pkt_size);
	memcpy(temp, pkt, pkt_size);
	memset(pkt, 0, total_pkt_size);

	/* 右に詰めてセットする */
	memcpy(pkt + (total_buffer_size - pkt_size), temp, pkt_size)

	/* 解放 */
	free(temp);

	/* pointerは変わっていないので、ずらして返す */
	return pkt + (total_buffer_size - pkt_size);
    }

    /* pke_sizeが2分入る */
    memcpy(pkt + (total_buffer_size - pkt_size), pkt, pkt_size);
    memset(pkt, 0, pkt_size);

    return pkt + (total_buffer_size - pkt_size);
}

