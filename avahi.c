/* avahi extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <net/if.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "php.h"
#include "ext/standard/info.h"
#include "phpc/phpc.h"
#include "php_avahi.h"

#include "avahi-data.h"

static AvahiThreadedPoll *threaded_poll = NULL;
static AvahiClient *client = NULL;
static AvahiServiceTypeBrowser *stb = NULL;
static AvahiServiceBrowserList *sbl = NULL;

/* {{{ void avahi_get_services()
 */
PHP_FUNCTION(avahi_get_services)
{
	avahi_threaded_poll_lock(threaded_poll);

	PHPC_ARRAY_INIT(return_value);

	for (AvahiServiceBrowserList *i = sbl; i; i = i->next) {
		for (AvahiServiceList *j = i->sl; j; j = j->next) {
			phpc_val service;
			PHPC_VAL_MAKE(service);
			PHPC_ARRAY_INIT(PHPC_VAL_CAST_TO_PZVAL(service));

			char ifname[IF_NAMESIZE];
			PHPC_ARRAY_ADD_ASSOC_CSTR(
				PHPC_VAL_CAST_TO_PZVAL(service),
				"interface",
				j->interface != AVAHI_IF_UNSPEC ?
					if_indextoname(j->interface, ifname) : "n/a"
			);
			PHPC_ARRAY_ADD_ASSOC_CSTR(
				PHPC_VAL_CAST_TO_PZVAL(service),
				"protocol",
				j->protocol != AVAHI_PROTO_UNSPEC ?
					avahi_proto_to_string(j->protocol) : "n/a"
			);
			PHPC_ARRAY_ADD_ASSOC_CSTR(PHPC_VAL_CAST_TO_PZVAL(service), "name", j->name);
			PHPC_ARRAY_ADD_ASSOC_CSTR(PHPC_VAL_CAST_TO_PZVAL(service), "type", j->type);
			PHPC_ARRAY_ADD_ASSOC_CSTR(PHPC_VAL_CAST_TO_PZVAL(service), "domain", j->domain);
			if (j->resolved == 0)
				PHPC_ARRAY_ADD_ASSOC_NULL(PHPC_VAL_CAST_TO_PZVAL(service), "resolved");
			else if (j->resolved == -1)
				PHPC_ARRAY_ADD_ASSOC_BOOL(PHPC_VAL_CAST_TO_PZVAL(service), "resolved", false);
			else /* if (j->resolved == 1) */ {
				PHPC_ARRAY_ADD_ASSOC_BOOL(PHPC_VAL_CAST_TO_PZVAL(service), "resolved", true);
				PHPC_ARRAY_ADD_ASSOC_CSTR(PHPC_VAL_CAST_TO_PZVAL(service), "host_name", j->host_name);
				PHPC_ARRAY_ADD_ASSOC_CSTR(PHPC_VAL_CAST_TO_PZVAL(service), "address", j->address);
				PHPC_ARRAY_ADD_ASSOC_LONG(PHPC_VAL_CAST_TO_PZVAL(service), "port", j->port);
				PHPC_ARRAY_ADD_ASSOC_BOOL(PHPC_VAL_CAST_TO_PZVAL(service), "is_local", j->is_local);
				PHPC_ARRAY_ADD_ASSOC_BOOL(PHPC_VAL_CAST_TO_PZVAL(service), "our_own", j->our_own);
				PHPC_ARRAY_ADD_ASSOC_BOOL(PHPC_VAL_CAST_TO_PZVAL(service), "wide_area", j->wide_area);
				PHPC_ARRAY_ADD_ASSOC_BOOL(PHPC_VAL_CAST_TO_PZVAL(service), "multicast", j->multicast);
				PHPC_ARRAY_ADD_ASSOC_BOOL(PHPC_VAL_CAST_TO_PZVAL(service), "cached", j->cached);

				phpc_val txt;
				PHPC_VAL_MAKE(txt);
				PHPC_ARRAY_INIT(PHPC_VAL_CAST_TO_PZVAL(txt));
				for (AvahiStringList *k = j->txt; k; k = k->next)
					PHPC_ARRAY_ADD_NEXT_INDEX_CSTR(PHPC_VAL_CAST_TO_PZVAL(txt), k->text);
				PHPC_ARRAY_ADD_ASSOC_ZVAL(
					PHPC_VAL_CAST_TO_PZVAL(service),
					"txt",
					PHPC_VAL_CAST_TO_PZVAL(txt));
			}

			PHPC_ARRAY_ADD_NEXT_INDEX_ZVAL(return_value, PHPC_VAL_CAST_TO_PZVAL(service));
		}
	}

	avahi_threaded_poll_unlock(threaded_poll);
}
/* }}} */

/* {{{ proto array avahi_get_service_types()
 */
PHP_FUNCTION(avahi_get_service_types)
{
	avahi_threaded_poll_lock(threaded_poll);

	PHPC_ARRAY_INIT(return_value);

	for (AvahiServiceBrowserList *i = sbl; i; i = i->next)
		PHPC_ARRAY_ADD_NEXT_INDEX_CSTR(return_value, i->stype);

	avahi_threaded_poll_unlock(threaded_poll);
}
/* }}} */

static void resolve_callback(
	AvahiServiceResolver *r,
	AVAHI_GCC_UNUSED AvahiIfIndex interface,
	AVAHI_GCC_UNUSED AvahiProtocol protocol,
	AvahiResolverEvent event,
	const char *name,
	const char *type,
	const char *domain,
	const char *host_name,
	const AvahiAddress *address,
	uint16_t port,
	AvahiStringList *txt,
	AvahiLookupResultFlags flags,
	void* userdata) {
	assert(r);
	AvahiServiceList *sl = userdata;
	/* Called whenever a service has been resolved successfully or timed out */
	switch (event) {
		case AVAHI_RESOLVER_FAILURE:
			fprintf(stderr, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
			avahi_service_list_resolve_fail(
				sl,
				interface,
				protocol,
				name,
				type,
				domain);
			break;
		case AVAHI_RESOLVER_FOUND: {
			fprintf(stderr, "Service '%s' of type '%s' in domain '%s':\n", name, type, domain);
			avahi_service_list_resolve(
				sl,
				interface,
				protocol,
				name,
				type,
				domain,
				host_name,
				address,
				port,
				txt,
				!!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
				!!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
				!!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
				!!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
				!!(flags & AVAHI_LOOKUP_RESULT_CACHED));
		}
	}
	avahi_service_resolver_free(r);
}

static void service_browse_callback(
	AvahiServiceBrowser *sb,
	AvahiIfIndex interface,
	AvahiProtocol protocol,
	AvahiBrowserEvent event,
	const char *name,
	const char *type,
	const char *domain,
	AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
	void* userdata) {
	assert(sb);
	AvahiServiceBrowserList *sbl = userdata;
	/* Called whenever a new services becomes available on the LAN or is removed from the LAN */
	switch (event) {
		case AVAHI_BROWSER_FAILURE:
			fprintf(stderr, "(Browser) %s\n", avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(sb))));
			avahi_threaded_poll_quit(threaded_poll);
			return;
		case AVAHI_BROWSER_NEW:
			for (AvahiServiceList *i = sbl->sl; i; i = i->next)
				if (strcmp(name, i->name) == 0)
					return;
			fprintf(stderr, "(Browser) NEW: service '%s' of type '%s' in domain '%s'\n", name, type, domain);
			/* We ignore the returned resolver object. In the callback
				function we free it. If the server is terminated before
				the callback function is called the server will free
				the resolver for us. */
			sbl->sl = avahi_service_list_add(sbl->sl, interface, protocol, name, type, domain);
			if (!(avahi_service_resolver_new(client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, resolve_callback, sbl->sl)))
				fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(client)));
			break;
		case AVAHI_BROWSER_REMOVE:
			fprintf(stderr, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n", name, type, domain);
			// TODO
			break;
		case AVAHI_BROWSER_ALL_FOR_NOW:
			fprintf(stderr, "(Browser) ALL_FOR_NOW\n");
			break;
		case AVAHI_BROWSER_CACHE_EXHAUSTED:
			fprintf(stderr, "(Browser) CACHE_EXHAUSTED\n");
			// TODO: clean cache ?
			break;
	}
}

static void service_type_browser_callback(
	AvahiServiceTypeBrowser *b,
	AVAHI_GCC_UNUSED AvahiIfIndex interface,
	AVAHI_GCC_UNUSED AvahiProtocol protocol,
	AvahiBrowserEvent event,
	const char *stype,
	const char *domain,
	AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
	void *userdata) {
	AvahiServiceBrowser *sb = NULL;
	assert(b);
	/* Called whenever a new services type becomes available on the LAN or is removed from the LAN */
	switch (event) {
		case AVAHI_BROWSER_FAILURE:
			fprintf(stderr, "(Browser) %s\n", avahi_strerror(avahi_client_errno(avahi_service_type_browser_get_client(b))));
			avahi_threaded_poll_quit(threaded_poll);
			return;
		case AVAHI_BROWSER_NEW:
			for (AvahiServiceBrowserList *i = sbl; i; i = i->next)
				if (strcmp(stype, i->stype) == 0)
					return;
			fprintf(stderr, "(Browser) NEW: service type '%s' in domain '%s'\n", stype, domain);
			/* We ignore the returned resolver object. In the callback
				function we free it. If the server is terminated before
				the callback function is called the server will free
				the resolver for us. */

			/* Create the service browser */
			sbl = avahi_service_browser_list_add(sbl, NULL, stype);
			if (!(sbl->sb = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, stype, domain, 0, service_browse_callback, sbl)))
				fprintf(stderr, "Failed to create service browser: %s\n", avahi_strerror(avahi_client_errno(client)));
			break;
		case AVAHI_BROWSER_REMOVE:
			fprintf(stderr, "(Browser) REMOVE: service type '%s' in domain '%s'\n", stype, domain);
			// TODO
			break;
		case AVAHI_BROWSER_ALL_FOR_NOW:
			fprintf(stderr, "(Browser) ALL_FOR_NOW\n");
			break;
		case AVAHI_BROWSER_CACHE_EXHAUSTED:
			fprintf(stderr, "(Browser) CACHE_EXHAUSTED\n");
			// TODO: clean cache ?
			break;
	}

}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
	assert(c);
	/* Called whenever the client or server state changes */
	if (state == AVAHI_CLIENT_FAILURE) {
		fprintf(stderr, "Server connection failure: %s\n", avahi_strerror(avahi_client_errno(c)));
		avahi_threaded_poll_quit(threaded_poll);
	}
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(avahi)
{
#if defined(ZTS) && defined(COMPILE_DL_AVAHI)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	int error;
	/* Allocate main loop object */
	if (!(threaded_poll = avahi_threaded_poll_new())) {
		fprintf(stderr, "Failed to create threaded poll object.\n");
		goto fail;
	}
	/* Allocate a new client */
	client = avahi_client_new(avahi_threaded_poll_get(threaded_poll), 0, client_callback, NULL, &error);
	/* Check wether creating the client object succeeded */
	if (!client) {
		fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
		goto fail;
	}
	/* Create the service type browser */
	if (! (stb = avahi_service_type_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, 0, service_type_browser_callback, NULL))) {
		fprintf(stderr, "Failed to create service type browser: %s\n", avahi_strerror(avahi_client_errno(client)));
		goto fail;
	}
	if (avahi_threaded_poll_start(threaded_poll)) {
		fprintf(stderr, "Failed to start threaded poll loop.\n");
		goto fail;
	}
	return SUCCESS;
fail:
	/* Cleanup things */
	if (sbl)
		avahi_service_browser_list_free(sbl);
	if (stb)
		avahi_service_type_browser_free(stb);
	if (client)
		avahi_client_free(client);
	if (threaded_poll)
		avahi_threaded_poll_free(threaded_poll);
	return FAILURE;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(avahi)
{
	/* Stop threaded poll object */
	avahi_threaded_poll_stop(threaded_poll);

	/* Cleanup things */
	if (sbl)
		avahi_service_browser_list_free(sbl);
	if (stb)
		avahi_service_type_browser_free(stb);
	if (client)
		avahi_client_free(client);
	if (threaded_poll)
		avahi_threaded_poll_free(threaded_poll);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(avahi)
{
	size_t types = 0;
	size_t services = 0;
	for (AvahiServiceBrowserList *i = sbl; i; i = i->next) {
		++types;
		for (AvahiServiceList *j = i->sl; j; j = j->next) {
			++services;
		}
	}

	char buf[32];

	php_info_print_table_start();
	php_info_print_table_header(2, "avahi support", "enabled");
	snprintf(buf, sizeof(buf), ZEND_LONG_FMT, types);
	php_info_print_table_row(2, "Service types", buf);
	snprintf(buf, sizeof(buf), ZEND_LONG_FMT, services);
	php_info_print_table_row(2, "Services", buf);
	php_info_print_table_end();
}
/* }}} */


/* {{{ avahi_functions[]
 */
static const zend_function_entry avahi_functions[] = {
	PHP_FE(avahi_get_services,  NULL)
	PHP_FE(avahi_get_service_types,  NULL)
	PHP_FE_END
};
/* }}} */

/* {{{ avahi_module_entry
 */
zend_module_entry avahi_module_entry = {
	STANDARD_MODULE_HEADER,
	"avahi",						/* Extension name */
	avahi_functions,			/* zend_function_entry */
	PHP_MINIT(avahi),			/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(avahi),	/* PHP_MSHUTDOWN - Module shutdown */
	NULL,							/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(avahi), 		/* PHP_MINFO - Module info */
	PHP_AVAHI_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_AVAHI
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(avahi)
#endif

