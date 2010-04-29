#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* CSP includes */
#include <csp/csp.h>
#include <csp/csp_thread.h>
#include <csp/csp_queue.h>
#include <csp/csp_semaphore.h>
#include <csp/csp_malloc.h>
#include <csp/csp_time.h>

/* Static allocation of interfaces */
csp_iface_t iface[17];

/** Connection Fallback
 * This queue is called each time a false connection is created
 * this is used by the CSP router to receive any connection */
static csp_socket_t * fallback_socket = NULL;

/** csp_route_table_init
 * Initialises the storage for the routing table
 */
void csp_route_table_init(void) {

	memset(iface, 0, sizeof(csp_iface_t) * 17);

}

/** Router Task
 * This task received any non-local connection and collects the data
 * on the connection. All data is forwarded out of the router
 * using the zsend call from csp */
csp_thread_return_t vTaskCSPRouter(void * pvParameters) {

	/* Register Connection Fallback */
	//connection_fallback = xQueueCreate(20, sizeof(conn_t *));

	csp_conn_t *conn;
	csp_packet_t * packet;

    fallback_socket = csp_socket();

	while (1) {

		conn = csp_accept(fallback_socket, CSP_MAX_DELAY);

		csp_debug("ROUTER: Recevied connection from %u to %u\r\n", conn->idin.src, conn->idin.dst);

		while ((packet = csp_read(conn, 10)) != NULL) {

			if (!csp_send_direct(conn->idin, packet, 0))
				csp_buffer_free(packet);

		}

		csp_close(conn);

		csp_debug("Connection Closed...\r\n");

	}

}

/** Set route
 * This function maintains the routing table,
 * To set default route use nodeid 16
 * To set a value pass a callback function
 * To clear a value pass a NULL value
 */
void csp_route_set(const char * name, uint8_t node, nexthop_t nexthop) {

	if (node <= 16) {
		iface[node].nexthop = nexthop;
		iface[node].name = name;
	} else {
		printf("ERROR: Failed to set route: invalid nodeid %u\r\n", node);
	}

}

/** Routing table lookup
 * This is the actual lookup in the routing table
 * The table consists of one entry per possible node
 * If there is no explicit nexthop route for the destination
 * the default route (node 16) is used.
 */
csp_iface_t * csp_route_if(uint8_t id) {

	if (iface[id].nexthop != NULL) {
		iface[id].count++;
		return &iface[id];
	}
	if (iface[16].nexthop != NULL) {
		iface[16].count++;
		return &iface[16];
	}
	return NULL;

}

/** CSP Router (WARNING: ISR)
 * This function uses the connection pool to search for already
 * established connetions with the given identifyer, if no connection
 * was found, a new one is created which is routed to the local task
 * which is listening on the port. If no listening task was found
 * the connection is sent to the fallback handler, otherwise
 * the attempt is aborted
 */
csp_conn_t * csp_route(csp_id_t id, nexthop_t avoid_nexthop, signed CSP_BASE_TYPE * pxTaskWoken) {

	csp_conn_t * conn;
	csp_queue_handle_t * queue = NULL;
	csp_iface_t * dst;

	/* Search for an existing connection */
	conn = csp_conn_find(id.ext, 0x03FFFF00);

	/* If a conneciton was found return that one. */
	if (conn != NULL)
		return conn;

	/* See if the packet belongs to this node
	 * Try local fist: */
	if (id.dst == my_address) {

		/* Try to deliver to incoming port number */
		if (ports[id.dport].state == PORT_OPEN) {
			queue = &(ports[id.dport].socket->conn_queue);

		/* Otherwise, try local "catch all" port number */
		} else if (ports[CSP_ANY].state == PORT_OPEN) {
			queue = &(ports[CSP_ANY].socket->conn_queue);

		/* Or reject */
		} else {
			return NULL;
		}

	/* If local node rejected the packet, try to route the frame */
	} else if (fallback_socket != NULL && fallback_socket->conn_queue != NULL) {

		/* If both sender and receiver resides on same segment
		 * don't route the frame. */
		dst = csp_route_if(id.dst);

		if (dst == NULL)
			return NULL;

		if (dst->nexthop == avoid_nexthop)
			return NULL;

		queue = &(fallback_socket->conn_queue);

	/* If the packet was not routed, reject it */
	} else {
		return NULL;

	}

	/* New incoming connection accepted */
	csp_id_t idout;
	idout.pri = id.pri;
	idout.dst = id.src;
	idout.src = id.dst;
	idout.dport = id.sport;
	idout.sport = id.dport;
	conn = csp_conn_new(id, idout);

	if (conn == NULL)
		return NULL;

	/* Try to queue up the new connection pointer */
	if ((queue != NULL) && (*queue != NULL)) {
		if (csp_queue_enqueue_isr(*queue, &conn, pxTaskWoken) == CSP_QUEUE_FULL) {
			printf("Warning Routing Queue Full\r\n");
			conn->state = SOCKET_CLOSED;	// Don't call csp_conn_close, since this is ISR context.
			return NULL;
		}
	}

	return conn;

}

/**
 * Inputs a new packet into the system
 * This function is mainly called from interface drivers
 * to route and accept packets.
 *
 * This function is fire and forget, it returns void, meaning
 * that a packet will always be either accepted or dropped
 * so the memory will always be freed.
 *
 * @param packet A pointer to the incoming packet
 * @param interface A pointer to the incoming interface TX function.
 *
 * @todo Implement Task-safe version of this function! (Currently runs only from criticial section or ISR)
 */
void csp_new_packet(csp_packet_t * packet, nexthop_t interface, signed CSP_BASE_TYPE * pxTaskWoken) {

	csp_conn_t * conn;

	csp_debug(
			"\r\nPacket P 0x%02X, S 0x%02X, D 0x%02X, Dp 0x%02X, Sp 0x%02X, T 0x%02X\r\n",
			packet->id.pri, packet->id.src, packet->id.dst, packet->id.dport,
			packet->id.sport, packet->id.type);

	/* Route the frame */
	conn = csp_route(packet->id, interface, pxTaskWoken);

	/* If routing failed, discard frame */
	if (conn == NULL) {
		csp_buffer_free(packet);
		return;
	}

	/* Save buffer pointer */
	if (csp_queue_enqueue_isr(conn->rx_queue, &packet, (signed CSP_BASE_TYPE *) pxTaskWoken) != CSP_QUEUE_OK) {
		printf("ERROR: Connection buffer queue full!\r\n");
		csp_buffer_free(packet);
		return;
	}

	/* If a local callback is used, call it */
	if ((packet->id.dst == my_address) && (packet->id.dport < 16)
			&& (ports[packet->id.dport].callback != NULL))
		ports[packet->id.dport].callback(conn);

}
