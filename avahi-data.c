#include "avahi-data.h"

#include <stddef.h>
#include <string.h>
#include <avahi-common/malloc.h>

AvahiServiceList *avahi_service_list_add(
	AvahiServiceList *sl,
	AvahiIfIndex interface,
	AvahiProtocol protocol,
	const char *name,
	const char *type,
	const char *domain)
{
	AvahiServiceList *n;

	if (!(n = avahi_malloc(sizeof(AvahiServiceList))))
		return NULL;

	n->next = sl;
	n->interface = interface;
	n->protocol = protocol;
	n->name = strdup(name);
	n->type = strdup(type);
	n->domain = strdup(domain);

	n->resolved = 0;
	n->host_name = NULL;
	n->address = NULL;
	n->port = 0;
	n->txt = NULL;
	n->is_local = false;
	n->our_own = false;
	n->wide_area = false;
	n->multicast = false;
	n->cached = false;

	return n;
}

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
	bool cached)
{
	assert(sl->interface == interface);
	assert(sl->protocol == protocol);
	assert(strcmp(sl->name, name) == 0);
	assert(strcmp(sl->type, type) == 0);
	assert(strcmp(sl->domain, domain) == 0);

	sl->resolved = 1;
	sl->host_name = strdup(host_name);
	char addr[AVAHI_ADDRESS_STR_MAX];
	avahi_address_snprint(addr, sizeof(addr), address);
	sl->address = strdup(addr);
	sl->port = port;
	sl->txt = txt; // TODO
	sl->is_local = is_local;
	sl->our_own = our_own;
	sl->wide_area = wide_area;
	sl->multicast = multicast;
	sl->cached = cached;
}

void avahi_service_list_resolve_fail(
	AvahiServiceList *sl,
	AvahiIfIndex interface,
	AvahiProtocol protocol,
	const char *name,
	const char *type,
	const char *domain)
{
	assert(sl->interface == interface);
	assert(sl->protocol == protocol);
	assert(strcmp(sl->name, name) == 0);
	assert(strcmp(sl->type, type) == 0);
	assert(strcmp(sl->domain, domain) == 0);

	sl->resolved = -1;
}

AvahiServiceList *avahi_service_list_remove(
	AvahiServiceList *sl,
	const char* name)
{
	assert(false);
	return sl;
}

void avahi_service_list_free(AvahiServiceList *sl)
{
	AvahiServiceList *n;

	while (sl) {
		n = sl->next;
		avahi_free((void*)sl->name);
		avahi_free((void*)sl->type);
		avahi_free((void*)sl->domain);
		avahi_free((void*)sl->host_name);
		avahi_free((void*)sl->address);
		avahi_free(sl);
		sl = n;
	}
}

AvahiServiceBrowserList *avahi_service_browser_list_add(
	AvahiServiceBrowserList *sbl,
	AvahiServiceBrowser* sb,
	const char* stype)
{
	AvahiServiceBrowserList *n;

	if (!(n = avahi_malloc(sizeof(AvahiServiceBrowserList))))
		return NULL;

	n->next = sbl;
	n->sb = sb;
	n->sl = NULL;
	n->stype = strdup(stype);

	return n;

}

AvahiServiceBrowserList *avahi_service_browser_list_remove(
	AvahiServiceBrowserList *sbl,
	const char* stype)
{
	assert(false);
	return sbl;
}

void avahi_service_browser_list_free(AvahiServiceBrowserList *sl)
{
	AvahiServiceBrowserList *n;

	while (sl) {
		n = sl->next;
		avahi_free((void*)sl->stype);
		avahi_free(sl);
		sl = n;
	}
}
