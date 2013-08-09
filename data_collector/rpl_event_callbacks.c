#include "rpl_event_callbacks.h"
#include "../utlist.h"
#include "../data_info/rpl_data.h"
#include <stdlib.h>

static rpl_event_callbacks_t event_callbacks = {0};

typedef enum rpl_object_type {
	ROT_Node,
	ROT_Dodag,
	ROT_Link,
	ROT_RplInstance,
	ROT_Packet
} rpl_object_type_e;

typedef struct rpl_event_el {
	rpl_event_type_e type;
	rpl_object_type_e object_type;
	union {
		di_node_ref_t node_ref;
		di_dodag_ref_t dodag_ref;
		di_rpl_instance_ref_t rpl_instance_ref;
		di_link_ref_t link_ref;
		int packet_id;
	};
	struct rpl_event_el *next;
	struct rpl_event_el *prev;
} *rpl_event_list_t, rpl_event_el_t;

rpl_event_list_t head;

void rpl_event_set_callbacks(rpl_event_callbacks_t *callbacks) {
	event_callbacks = *callbacks;
}

void rpl_event_packet(int packet_id) {
	rpl_event_el_t *element = (rpl_event_el_t*) calloc(1, sizeof(rpl_event_el_t));

	element->type = RET_Created;
	element->object_type = ROT_Packet;
	element->packet_id = packet_id;

	DL_APPEND(head, element);
}

void rpl_event_node(di_node_t *node, rpl_event_type_e type) {
	rpl_event_el_t *element = (rpl_event_el_t*) calloc(1, sizeof(rpl_event_el_t));

	element->type = type;
	element->object_type = ROT_Node;
	element->node_ref = node_get_key(node)->ref;

	DL_APPEND(head, element);
}

void rpl_event_dodag(di_dodag_t *dodag, rpl_event_type_e type) {
	rpl_event_el_t *element = (rpl_event_el_t*) calloc(1, sizeof(rpl_event_el_t));

	element->type = type;
	element->object_type = ROT_Dodag;
	element->dodag_ref = dodag_get_key(dodag)->ref;

	DL_APPEND(head, element);
}

void rpl_event_link(di_link_t *link, rpl_event_type_e type) {
	rpl_event_el_t *element = (rpl_event_el_t*) calloc(1, sizeof(rpl_event_el_t));

	element->type = type;
	element->object_type = ROT_Link;
	element->link_ref = link_get_key(link)->ref;

	DL_APPEND(head, element);
}

void rpl_event_rpl_instance(di_rpl_instance_t *rpl_instance, rpl_event_type_e type) {
	rpl_event_el_t *element = (rpl_event_el_t*) calloc(1, sizeof(rpl_event_el_t));

	element->type = type;
	element->object_type = ROT_RplInstance;
	element->rpl_instance_ref = rpl_instance_get_key(rpl_instance)->ref;

	DL_APPEND(head, element);
}

void rpl_event_process_events(int wsn_version) {
	rpl_event_el_t *event, *tmp;
	int version;
	hash_container_ptr container;

	DL_FOREACH_SAFE(head, event, tmp) {
		if(event->type == RET_Deleted)
			version = wsn_version - 1;
		else version = wsn_version;

		switch(event->object_type) {
			case ROT_Dodag: {
				di_dodag_t *dodag;
				container = rpldata_get_dodags(version);
				dodag = *(di_dodag_t**)hash_value(container, hash_key_make(event->dodag_ref), HVM_FailIfNonExistant, NULL);
				if(event_callbacks.onDodagEvent) event_callbacks.onDodagEvent(dodag, event->type);
				break;
			}

			case ROT_Link: {
				di_link_t *link;
				container = rpldata_get_links(version);
				link = *(di_link_t**)hash_value(container, hash_key_make(event->link_ref), HVM_FailIfNonExistant, NULL);
				if(event_callbacks.onLinkEvent) event_callbacks.onLinkEvent(link, event->type);
				break;
			}

			case ROT_Node: {
				di_node_t *node;
				container = rpldata_get_nodes(version);
				node = *(di_node_t**)hash_value(container, hash_key_make(event->node_ref), HVM_FailIfNonExistant, NULL);
				if(event_callbacks.onNodeEvent) event_callbacks.onNodeEvent(node, event->type);
				break;
			}

			case ROT_RplInstance: {
				di_rpl_instance_t *rpl_instance;
				container = rpldata_get_rpl_instances(version);
				rpl_instance = *(di_rpl_instance_t**)hash_value(container, hash_key_make(event->rpl_instance_ref), HVM_FailIfNonExistant, NULL);
				if(event_callbacks.onRplInstanceEvent) event_callbacks.onRplInstanceEvent(rpl_instance, event->type);
				break;
			}

			case ROT_Packet:
				if(event_callbacks.onPacketEvent) event_callbacks.onPacketEvent(event->packet_id);
		}
		DL_DELETE(head, event);
	}
}
