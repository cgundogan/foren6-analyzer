/*
 * File:   interfaces_register.h
 * Author: am
 *
 * Created on June 18, 2013, 2:00 PM
 */

#ifndef INTERFACES_REGISTER_H
#define	INTERFACES_REGISTER_H

#include <stdbool.h>
#include <sys/time.h>
#include "../data_info/hash_container.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct ifinstance {
	void *interface_data;
	const char *target; //for information only, not used

	//For timestamp sync
	struct timeval delta_to_parent;
	struct ifinstance *parent;
	hash_container_ptr last_packets;
	bool first_packet;

	//for lists
	struct ifinstance* next;
} ifinstance_t, *ifreader_t;

typedef struct interface {
	const char *interface_name;
	void (*init)();
	ifreader_t (*open)(const char *target, int channel);
	bool (*start)(ifreader_t handle);
	void (*stop)(ifreader_t handle);
	void (*close)(ifreader_t handle);

} interface_t;

typedef void (*interface_enum_function_t)(const char* interface_name);
typedef int (*interface_version_function_t)();
typedef interface_t (*interface_register_function_t)();

interface_t *interfacemgr_get(const char *name);

ifinstance_t* interfacemgr_create_handle(const char* target);
void interfacemgr_destroy_handle(ifinstance_t *handle);

struct timeval interfacemgr_get_absolute_timestamp(const ifinstance_t* iface, struct timeval packet_timestamp);
void interfacemgr_set_origin(ifinstance_t* iface, struct timeval packet_timestamp);
void inferfacemgr_set_master_time(ifinstance_t* iface, struct timeval packet_timestamp);
ifinstance_t *interfacemgr_get_root(ifinstance_t* iface);
void interfacemgr_rebase_parent(ifinstance_t *iface);
void interfacemgr_sync_time(ifinstance_t *iface1, struct timeval packet_timestamp1, ifinstance_t *iface2, struct timeval packet_timestamp2);
void interfacemgr_process_packet(ifinstance_t *iface, const unsigned char* data, int len, struct timeval timestamp);
bool interfacemgr_check_multisniffer_packet(ifinstance_t *iface, ifinstance_t *other, const unsigned char* data, int len, struct timeval timestamp);
bool interfacemgr_check_duplicate_packet(ifinstance_t *iface, const unsigned char* data, int len, struct timeval timestamp);
struct packet_data *interfacemgr_get_old_packet(ifinstance_t *iface, const unsigned char *data, int len);

#ifdef	__cplusplus
}
#endif

#endif	/* INTERFACES_REGISTER_H */

