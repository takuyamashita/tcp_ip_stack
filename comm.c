#include "comm.h"
#include "graph.h"
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

static unsigned int udp_port_number = 40000;

/* ユニークになるように現在のポート+1を返します */
static unsigned int get_next_udp_port_number(){
    
    return udp_port_number++;
}

/* udp socketを開いてnodeにセットします */
void init_udp_socket(node_t *node){

    node->udp_port_number = get_next_udp_port_number();

    /* socketを作成 */
    int udp_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in node_addr;
    /* アドレスファミリ */
    node_addr.sin_family = AF_INET;
    node_addr.sin_port = node->udp_port_number;
    /* アドレス 全てのアドレスを監視 */
    node_addr.sin_addr.s_addr = INADDR_ANY;

    /* fdにアドレスをバインドする(割り振る) */
    if(bind(
	udp_sock_fd,
	(struct sockaddr *)&node_addr,
	sizeof(struct sockaddr)
    ) == -1){
	perror("bind");
	exit(1);
    }

    node->udp_sock_fd = udp_sock_fd;
}

static char recv_buffer[MAX_PACKET_BUFFER_SIZE];

static char send_buffer[MAX_PACKET_BUFFER_SIZE];

int pkt_receive(node_t *node, interface_t *interface, char *pkt, unsigned int pkt_size){
    
    printf(
	"msg recvd = %s, on node = %s, IIF = %s\n",
	pkt,
	node->node_name,
	interface->if_name
    );

    return 0;
}

static void _pkt_receive(
    node_t *receving_node,
    char *pkt_with_aux_data, 
    unsigned int pkt_size
){
    
    char *recv_intf_name = pkt_with_aux_data;
    
    interface_t *recv_intf = get_node_if_by_name(receving_node, recv_intf_name);
    
    if(!recv_intf){
	
	printf(
	    "Error : Packet received on unknown interface %s on node %s\n",
	    recv_intf->if_name,
	    receving_node->node_name
	);

	return;
    }

    pkt_receive(
	receving_node,
	recv_intf,
	pkt_with_aux_data + IF_NAME_SIZE,
	pkt_size - IF_NAME_SIZE
    );
}

/* topologyの全てのノードに来たデータを読みます */
static void *_network_start_pkt_receiver_thread(void *arg){
    
    node_t *node;
    glthread_t *curr;

    fd_set active_sock_fd_set, backup_sock_fd_set;

    int sock_max_fd = 0;
    int bytes_recvd = 0;

    graph_t *topo = (void *)arg;

    int addr_len = sizeof(struct sockaddr);

    /* 初期化 */
    FD_ZERO(&active_sock_fd_set);
    FD_ZERO(&backup_sock_fd_set);

    struct sockaddr_in sender_addr;

    ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr){
	
	node = graph_glue_to_node(curr);

	if(!node->udp_sock_fd)
	    continue;
	
	/* selectでfd_set中のfdの最大値が必要なため */
	if(node->udp_sock_fd > sock_max_fd)
	    sock_max_fd = node->udp_sock_fd;

	/* 今あるノードのソケットファイルディスクリプタをバックアップにセット */
	FD_SET(node->udp_sock_fd, &backup_sock_fd_set);

    }ITERATE_GLTHREAD_END(&topo->node_list, curr);

    while(1){
	
	/* backup */
	memcpy(&active_sock_fd_set, &backup_sock_fd_set, sizeof(fd_set));

	/* ブロッキング無しでreadできる状態になるまで待つ(ずっと) */
	select(sock_max_fd + 1, &active_sock_fd_set, NULL, NULL, NULL); 

	ITERATE_GLTHREAD_BEGIN(&topo->node_list, curr){
	    
	    node = graph_glue_to_node(curr);
	    
	    /* 
	     * 対象のノードのfdが読み込み可能かを調べる
	     * selectがactive_sock_fd_setの中身をいじって
	     * フラグを立てている
	     */
	    if(FD_ISSET(node->udp_sock_fd, &active_sock_fd_set)){
		
		memset(recv_buffer, 0, MAX_PACKET_BUFFER_SIZE);

		/* 読み込んでrecv_bufferに詰める */
		bytes_recvd = recvfrom(
		    node->udp_sock_fd,
		    (char *)recv_buffer,
		    MAX_PACKET_BUFFER_SIZE,
		    0,
		    (struct sockaddr *)&sender_addr,
		    &addr_len
		);

		_pkt_receive(node, recv_buffer, bytes_recvd);
	    }

	}ITERATE_GLTHREAD_END(&topo->node_list, curr);
    }

}



void network_start_pkt_receiver_thread(graph_t *topo){
    
    pthread_attr_t attr;
    pthread_t recv_pkt_thread;
    
    /* attrの初期化 */
    pthread_attr_init(&attr);

    /*
     * attrのdetach stateにPTHREAD_CREATE_DETACHEDをセット
     * スレッドが作成される時に、メインスレッドから
     * 切り離された状態で作成される
     */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    /* スレッドを作成 */
    pthread_create(
	&recv_pkt_thread,
	&attr,
	_network_start_pkt_receiver_thread,
	(void *)topo
    );
}

static int _send_pkt_out(
    int sock_fd,
    char *pkt_data,
    unsigned int pkt_size,
    unsigned int dst_udp_port_no
){
    
    int rc;
    struct sockaddr_in dest_addr;

    
    struct hostent *host = (struct hostent *)gethostbyname("127.0.0.1");//:XXX

    /* 通信の設定 */
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = dst_udp_port_no;
    dest_addr.sin_addr = *((struct in_addr *)host->h_addr);

    /* dataを送信 */
    rc = sendto(sock_fd, pkt_data, pkt_size, 0,
	(struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

    return rc;
}


int send_pkt_out(char *pkt, unsigned int pkt_size, interface_t *interface){
    
    int rc = 0;

    node_t *sending_node = interface->att_node;
    node_t *nbr_node = get_nbr_node(interface);

    if(!nbr_node)
	return -1;

    unsigned int dst_udp_port_no = nbr_node->udp_port_number;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sock < 0){
	
	perror("socket");
	exit(1);
    }

    /* 使用しているリンクのもう一方のインターフェースを見つける */
    interface_t *other_interface = &interface->link->intf1 == interface ? \
	&interface->link->intf2 : &interface->link->intf1;

    memset(send_buffer, 0, MAX_PACKET_BUFFER_SIZE);

    char *pkt_with_aux_data = send_buffer;

    
    strncpy(pkt_with_aux_data, other_interface->if_name, IF_NAME_SIZE);

    pkt_with_aux_data[IF_NAME_SIZE] = '\0';

    /*
     *     if_name |    data       |
     *   | | | | | | | | | | | | | | |
     */
    memcpy(pkt_with_aux_data + IF_NAME_SIZE, pkt, pkt_size);

    rc = _send_pkt_out(
	sock,
	pkt_with_aux_data,
	pkt_size + IF_NAME_SIZE, 
	dst_udp_port_no
    );

    close(sock);

    return rc;
}

/* ノードの全てのインターフェースからパケットを送信します */
int send_pkt_flood(node_t *node, char *pkt, unsigned int pkt_size){
    
    unsigned int i = 0;
    interface_t *intf;

    for(; i < MAX_INTF_PER_NODE; i++){
	
	intf = node->intf[i];
	
	if(!intf)
	    return;
	
	send_pkt_out(pkt, pkt_size, intf);
    }
}


