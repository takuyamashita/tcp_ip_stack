#ifndef __NET__
#define __NET__

#include "utils.h"
#include "graph.h"
#include <memory.h>

typedef struct graph_ graph_t;

typedef struct interface_ interface_t;

typedef struct node_ node_t;

/* 
 * #pragmaから次の#pragmaまでの構造体の
 * アライメントを指定する
 * packで1(bytes)を指定すればint型であっても
 * つめる用にメモリをとる
 * |c|     |  int  |
 * | | | | | | | | |
 * 以下の様になる
 * |c|  int  |
 * | | | | | | | | |
 */
//#pragma pack(push,1)

/* 
 * 192.168.0.1
 * 192.168.000.001
 */
typedef struct ip_add_ {
   unsigned char ip_addr[16]; 
} ip_add_t;

/*
 * C8-1F-66-24-3A-C8
 * 0xC81F66243AC8
 */
typedef struct mac_add_ {
    unsigned char mac[6];
} mac_add_t;

//#pragma pack(pop)


typedef struct node_nw_prop_ {   
    /* loop back かどうか */
    bool_t is_lb_addr_config;
    ip_add_t lb_addr;
} node_nw_prop_t;

/* nodeのネットワークプロパティの初期化 */
static inline void init_node_nw_prop(node_nw_prop_t *node_nw_prop){
    
    node_nw_prop->is_lb_addr_config = FALSE;

    memset(node_nw_prop->lb_addr.ip_addr, 0, 16);
}

/* ネットワークインタフェースのプロパティ */
typedef struct intf_nw_props {
    mac_add_t mac_add;
    /* ip addressが設定済かどうか */
    bool_t is_ipadd_config;
    ip_add_t ip_add;
    char mask;
} intf_nw_props_t;

/* インターフェースのプロパティの初期化 */
static inline void init_intf_nw_prop(intf_nw_props_t *intf_nw_props){
    
    memset(intf_nw_props->mac_add.mac, 0, 8);
    intf_nw_props->is_ipadd_config = FALSE;
    memset(intf_nw_props->ip_add.ip_addr, 0, 16);
    intf_nw_props->mask = 0;
}

#define IF_MAC(intf_ptr) ((intf_ptr)->intf_nw_props.mac_add.mac)
#define IF_IP(intf_ptr) ((intf_ptr)->intf_nw_props.ip_add.ip_addr)

#define NODE_LO_ADDR(node_ptr) (node_ptr->node_nw_prop.lb_addr.ip_addr)

bool_t node_set_loopback_address(node_t *node, char *ip_addr);
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask);
bool_t node_unset_intf_ip_address(node_t *node, char *local_if);

void dump_nw_graph(graph_t *graph);
void dump_node_nw_props(node_t *node);
void dump_intf_props(interface_t *interface);

#endif

