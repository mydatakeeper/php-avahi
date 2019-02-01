#ifndef AVAHI_DATA_H
#define AVAHI_DATA_H

#include <stdbool.h>
#include <avahi-common/address.h>
#include <avahi-client/lookup.h>

typedef struct AvahiServiceList {
	struct AvahiServiceList *next;

	AvahiIfIndex interface;
	AvahiProtocol protocol;
	const char *name;
	const char *type;
	const char *domain;

	AvahiResolverEvent resolved;
	const char *host_name;
	const char *address;
	uint16_t port;
	AvahiStringList *txt;
	bool is_local;
	bool our_own;
	bool wide_area;
	bool multicast;
	bool cached;
} AvahiServiceList;

AvahiServiceList *avahi_service_list_add(
	AvahiServiceList *sl,
	AvahiIfIndex interface,
	AvahiProtocol protocol,
	const char *name,
	const char *type,
	const char *domain);
void avahi_service_list_resolve(
	AvahiServiceList *sl,
	AvahiIfIndex interface,
	AvahiProtocol protocol,
	const char *name,
	const char *type,
	const char *domain,
	const char *host_name,
	const AvahiAddress *address,
	uint16_t port,
	AvahiStringList *txt,
	bool is_local,
	bool our_own,
	bool wide_area,
	bool multicast,
	bool cached);
void avahi_service_list_resolve_fail(
	AvahiServiceList *sl,
	AvahiIfIndex interface,
	AvahiProtocol protocol,
	const char *name,
	const char *type,
	const char *domain);
AvahiServiceList *avahi_service_list_remove(
	AvahiServiceList *sl,
	const char* name);
void avahi_service_list_free(AvahiServiceList *sl);

typedef struct AvahiServiceBrowserList {
	struct AvahiServiceBrowserList *next;
	AvahiServiceBrowser* sb;
	AvahiServiceList* sl;
	const char* stype;
} AvahiServiceBrowserList;

AvahiServiceBrowserList *avahi_service_browser_list_add(
	AvahiServiceBrowserList *sbl,
	AvahiServiceBrowser* sb,
	const char* stype);
AvahiServiceBrowserList *avahi_service_browser_list_remove(
	AvahiServiceBrowserList *sbl,
	const char* stype);
void avahi_service_browser_list_free(AvahiServiceBrowserList *sl);


#endif /* AVAHI_DATA_H */
