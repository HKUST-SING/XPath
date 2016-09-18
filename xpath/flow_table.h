#ifndef __FLOW_TABLE_H__
#define __FLOW_TABLE_H__

#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/list.h>
#include <linux/ip.h>
#include <net/tcp.h>

struct xpath_flow_info
{
        u16 path_id;    //current path ID (not path IP address)
        u32 seq;        //update for every TX packet
        u32 ack_seq;    //update for every RX packet
        ktime_t last_tx_time;   //last time when we observe a sent packet

        /* following variables are only reset when we change the path */
        u32 bytes_sent; //bytes sent in current path
        u32 bytes_rtx;  //bytes retransmiited in current path
        u16 timeouts;   //# of timeouts in current path

        u32 bytes_acked_total;  //bytes acknowledged by the remote side
        u32 bytes_acked_ecn;    //bytes get ECN marked
        u32 ecn_fraction;       //fraction of ECN marked packets
        ktime_t last_ecn_update_time;   //last time when we update ECN fraction
};

/* A TCP flow <local_ip, remote_ip, local_port, remote_port> */
struct xpath_flow_entry
{
        u32 local_ip;   //Local IP address
        u32 remote_ip;  //Remote IP address
        u16 local_port; //Local port
        u16 remote_port;        //Remote port
        struct xpath_flow_info info;    //Information for this flow
        struct list_head list;  //linked list
};

/* Link List of Flows */
struct xpath_flow_list
{
        struct list_head head_node;    //head node of the flow list
        unsigned int len;   //total number of flows in the list
        spinlock_t lock;    //lock for this flow list
};

/* Hash Table of Flows */
struct xpath_flow_table
{
        struct xpath_flow_list *flow_lists;  //array of linked lists
        atomic_t size;
};

/* Print functions */
void xpath_print_flow_entry(struct xpath_flow_entry *f, char *operation);
void xpath_print_flow_list(struct xpath_flow_list *fl);
void xpath_print_flow_table(struct xpath_flow_table *ft);

inline unsigned int xpath_hash_flow(struct xpath_flow_entry *f);
inline bool xpath_equal_flow(struct xpath_flow_entry *f1,
                             struct xpath_flow_entry *f2);

/* Initialization functions */
bool xpath_init_flow_info(struct xpath_flow_info *info);
bool xpath_init_flow_entry(struct xpath_flow_entry *f);
bool xpath_init_flow_list(struct xpath_flow_list *fl);
bool xpath_init_flow_table(struct xpath_flow_table *ft);
void xpath_set_flow_4tuple(struct xpath_flow_entry *f,
                           u32 local_ip,
                           u32 remote_ip,
                           u16 local_port,
                           u16 remote_port);

/* Search functions: search a flow entry from flow table/list */
struct xpath_flow_entry *xpath_search_flow_list(struct xpath_flow_list *fl,
                                                struct xpath_flow_entry *f);
struct xpath_flow_entry *xpath_search_flow_table(struct xpath_flow_table *ft,
                                                 struct xpath_flow_entry *f);

/* Insert functions: insert a new flow entry to flow table/list */
bool xpath_insert_flow_list(struct xpath_flow_list *fl,
                            struct xpath_flow_entry *f,
                            int flags);
bool xpath_insert_flow_table(struct xpath_flow_table *ft,
                             struct xpath_flow_entry *f,
                             int flags);

/* Delete functions: delete a flow entry from flow table/list */
bool xpath_delete_flow_list(struct xpath_flow_list *fl,
                            struct xpath_flow_entry *f);
bool xpath_delete_flow_table(struct xpath_flow_table *ft,
                             struct xpath_flow_entry *f);

/* Clear functions: clear flow entries from flow table/list */
bool xpath_clear_flow_list(struct xpath_flow_list *fl);
bool xpath_clear_flow_table(struct xpath_flow_table *ft);

/* Exit functions: delete whole flow table */
bool xpath_exit_flow_table(struct xpath_flow_table *ft);

#endif
