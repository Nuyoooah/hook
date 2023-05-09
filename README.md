# hook
PHP core function zend_compile_file  hook

Compile hook.so as follows:

phpize && ./configure --enable-hook
make
make install
php -d extension=hook.so
