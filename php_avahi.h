/* avahi extension for PHP */

#ifndef PHP_AVAHI_H
# define PHP_AVAHI_H

extern zend_module_entry avahi_module_entry;
# define phpext_avahi_ptr &avahi_module_entry

# define PHP_AVAHI_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_AVAHI)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_AVAHI_H */

