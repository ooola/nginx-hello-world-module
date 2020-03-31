PWD =`pwd`
NGINX ?= "/home/ec2-user/nginx-1.16.1" # path to nginx source
MODULE_DIR ?= "/home/ec2-user/module"
NGINX_INSTALL_DIR ?= "/usr/local/nginx/sbin"
NGINX_MODULE_DIR ?= "/usr/local/nginx/modules"

ifeq (,$(wildcard optimizely-sdk.so))
$(error optimizely sdk is missing please copy it here)
endif

ifeq (,$(wildcard nginx))
$(error nginx source code is missing please copy it here)
endif

.PHONY: all
all: build install restart

.PHONY: config 
config:
	(cd ${NGINX} && ./configure --add-dynamic-module=${MODULE_DIR}) # --with-debug for nginx dbg logging

.PHONY: build
build:
	(cd ${NGINX} && make) 

.PHONY: install
install:
	(cd ${NGINX} && sudo make install)
	sudo cp optimizely-sdk.so ${NGINX_INSTALL_DIR}

.PHONY: restart 
restart: 
	-sudo killall nginx
	sudo /usr/local/nginx/sbin/nginx

.PHONY: sync
sync:
	rsync -e ssh -r ${PWD}/* passrole:module/

.PHONY: runwithoptimizelysdk
runwithoptimizelysdk:
	sudo cp hello_world_with_optimizely_sdk.so ${NGINX_MODULE_DIR}/ngx_http_hello_world_module.so
	${MAKE} restart

.PHONY: runhelloworld
runhelloworld:
	sudo cp ngx_http_hello_world_module.so ${NGINX_MODULE_DIR}/ngx_http_hello_world_module.so
	${MAKE} restart
