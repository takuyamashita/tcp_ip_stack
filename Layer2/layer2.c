#include "layer2.h"

void layer2_frame_recv(
    node_t *node,
    interface_t *interface,
    char *pkt,
    unsigned int pkt_size
){

}

/* arp tableを初期化します */
void init_arp_table(arp_table_t **arp_table){
    
    *arp_table = calloc(1, sizeof(arp_table_t));

    init_glthread(&((*arp_table)->arp_entries));
}

/* 
 * arp tableの各エントリから指定されたIPアドレスのものを見つけて返します 
 * 見つからなければNULLを返します
 */
arp_entry_t *arp_table_lookup(arp_table_t *arp_table, char *ip_addr){
    
    glthread_t *curr;
    arp_entry_t *arp_entry;

    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){
    
	arp_entry = arp_glue_to_arp_entry(curr);

	if(strncmp(arp_entry->ip_addr.ip_addr, ip_addr, 16) == 0){
	    
	    return arp_entry;
	}

    } ITERATE_GLTHREAD_END(&arp_table->arp_entries, curr);

    return NULL;
}

void clear_arp_table(arp_table_t *arp_table){

    
}

void delete_arp_table(arp_table_t *arp_table){
    
}

bool_t arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry){

    arp_entry_t *arp_entry_old = arp_table_lookup(
	arp_table,
	arp_entry->ip_addr.ip_addr
    );

    /* 
     * 既に同じものがテーブルの中にあればfalse 
     * MACアドレスやIFも含めて同じもの
     */
    if(arp_entry_old && memcmp(arp_entry_old, arp_entry, sizeof(arp_entry_t)) == 0)
	return FALSE;
    
    /* 同じIPアドレスの場合は古いものを削除して新しく追加 */
    if(arp_entry_old){
	
	delete_arp_table_entry(arp_table, arp_entry->ip_addr.ip_addr);
    }

    init_glthread(&arp_entry->arp_glue);
    glthread_add_next(&arp_table->arp_entries, arp_entry->ip_addr.ip_addr);

    return TRUE;
}

/*
 * arpパケットを元にarpテーブルを更新します
 * 既に存在している場合は更新しません
 */
void arp_table_update_from_arp_reply(
    arp_table_t *arp_table,
    arp_hdr_t *arp_hdr,
    interface_t *iif
){
    
    unsigned int src_ip = 0;

    /* 
     * リプライなのかを確認 
     * リプライの送信元を知りたいため
     */
    assert(arp_hdr->op_code == ARP_REPLY);

    arp_entry_t *arp_entry = calloc(1, sizeof(arp_entry_t));

    /* ネットワークバイトオーダーに変換*/
    src_ip = htonl(arp_hdr->src_ip);

    /* 
     * ipアドレスをaaa.bbb.ccc.dddの形に変換 
     * arp_entryにセット
     */
    inet_ntop(AF_INET, &src_ip, &arp_entry->ip_addr.ip_addr, 16);
    arp_entry->ip_addr.ip_addr[15] = '\0';

    /* arp_entryにMACアドレスをセット */
    memcpy(arp_entry->mac_addr.mac, arp_hdr->src_mac.mac, sizeof(mac_add_t));

    /* arp_entry インタフェースをセット */
    strncpy(arp_entry->oif_name, iif->name, IF_NAME_SIZE);

    /* arp tableに作成したarp entryを追加 */
    bool_t rc = arp_table_entry_add(arp_table, arp_entry);

    /* 失敗したら解放 */
    if(rc == FALSE){
	free(arp_entry);
    }

}

/* arp tableをdumpします */
void dump_arp_table(arp_table_t *arp_table){
    
    glthread_t *curr;
    arp_entry_t *arp_entry;

    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){

	arp_entry = arp_glue_to_arp_entry(curr);

	printf(
	    "IP : %s, MAC : %u:%u:%u:%u:%u:%u:, OIF = %s\n",
	    arp_entry->ip_addr.ip_addr,
	    arp_entry->mac_addr.mac[0],
	    arp_entry->mac_addr.mac[1],
	    arp_entry->mac_addr.mac[2],
	    arp_entry->mac_addr.mac[3],
	    arp_entry->mac_addr.mac[4],
	    arp_entry->mac_addr.mac[5],
	    arp_entry->oif_name
	);

    } ITERATE_GLTHREAD_END(&arp_table->arp_entries, curr);
}

