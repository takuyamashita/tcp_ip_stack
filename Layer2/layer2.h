
#ifndef __LAYER2__
#define __LAYER2__

#include "../net.h"
#include "../gluethread/glthread.h"
#include "../tcpconst.h"

#pragma pack (push,1)

typedef struct arp_hdr_ {
    /*
     * 2 bytes
     * ハードウェアタイプ
     * Ethernetは0001
     */
    short hw_type;

    /*
     * 2bytes
     * Ethernetヘッダで使用できるものと同じ
     * 0x0800 -> ipv4
     */
    short proto_type;

    /*
     * 1byte
     * ハードウェアのアドレスの長さ(bytes)
     * MACアドレスなら6
     */
    char hw_addr_len;

    /*
     * 1byte
     * プロトコルアドレスの長さ
     * プロトコルアドレスとはここでいうIPアドレス
     */
    char proto_addr_len;

    /*
     * 2 bytes
     * requestかreplyかを示すフラグ 
     */
    short op_code;

    /*
     * 6bytes
     * 送信元のハードウェアアドレス
     * 本来は可変長
     */
    mac_add_t src_mac;

    /*
     * 4bytes
     * 送信元プロトコルアドレス
     * 本来は可変長
     */
    unsigned int src_ip;

    /*
     * 6bytes
     * 宛先ハードウェアアドレス
     * 本来は可変長
     */
    mac_add_t dst_mac;

    /*
     * 4bytes
     * 宛先プロトコルアドレス
     * 本来は可変長
     */
    unsigned int dst_ip;


} arp_hdr_t;

/* 
 * プリアンブルとsfdは入れない
 * 10101010....10101011がプリアンブル(とsfd)
 * ヘッダがどこから始まるのかがわかる(物理的に同期が取れる)
 */
typedef struct ethernet_hdr_ {
    /* 
     * 6 bytes
     * 宛先MACアドレス
     */
    mac_add_t dst_mac;

    /*
     * 6 bytes
     * 送信元MACアドレス
     */
    mac_add_t src_mac;

    /*
     * 2 bytes
     * 上位層のプロトコルを示す
     * 0x0800 -> ipv4
     * 0x0806 -> arp 
     * etc...
     */
    short type;

    /* frameのデータ */
    char payload[248];

    /* 
     * 各フィールドの値を使って計算されたハッシュ値のようなもの
     * 受信側で再計算して値が異なるならデータが破損していることがわかる
     */
    unsigned int FCS;
} ethernet_hdr_t;

#pragma pack(pop)

/* arp table */
typedef struct arp_table_ {
    glthread_t arp_entries;
} arp_table_t;

/* arp tableの1行 */
typedef struct arp_entry_ {
    ip_add_t ip_addr;
    mac_add_t mac_addr;
    char oif_name[IF_NAME_SIZE];
    glthread_t arp_glue;
} arp_entry_t;

GLTHREAD_TO_STRUCT(arp_glue_to_arp_entry, arp_entry_t, arp_glue);

void init_arp_table(arp_table_t **arp_table);
arp_entry_t *arp_table_lookup(arp_table_t *arp_table, char *ip_addr);
void clear_arp_table(arp_table_t *arp_table);
void delete_arp_table(arp_table_t *arp_table);
bool_t arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry);
void arp_table_update_from_arp_reply(
    arp_table_t *arp_table,
    arp_hdr_t *arp_hdr,
    interface_t *iif
);

void dump_arp_table(arp_table_t *arp_table);


/* l2の受け入れ可否を返します */
static inline bool_t l2_frame_recv_qualify_on_interface(
    interface_t *interface,
    ethernet_hdr_t *ethernet
){
    
    /* interfaceがL3モードならfalse */
    if(!IS_INTF_L3_MODE(interface)){

	return FALSE;
    }

    /* 宛先が正しいならtrue */
    if(memcmp(IF_MAC(interface), ethernet_hdr->dst_mac.mac, sizeof(mac_add_t)) == 0){
	
	return TRUE;
    }

    /* ブロードキャストだった場合もtrue */
    if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
	
	return TRUE;
    }

    return FALSE;
}

/* header => dst_mac dist_src type  */
#define ETH_HDR_SIZE_EXCL_PAYLOAD \
    (sizeof(ethernet_hdr_t) - sizeof(((ethernet_hdr_t *)0)->payload))

#endif
