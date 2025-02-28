#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_xfrm_setup() {
	local cfg="$1"
	local mode="xfrm"

	local tunlink ifid mtu zone multicast
	json_get_vars tunlink ifid mtu zone multicast

<<<<<<< HEAD
=======
	[ -z "$tunlink" ] && {
		proto_notify_error "$cfg" NO_TUNLINK
		proto_block_restart "$cfg"
		exit
	}

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	[ -z "$ifid" ] && {
		proto_notify_error "$cfg" NO_IFID
		proto_block_restart "$cfg"
		exit
	}

<<<<<<< HEAD
=======
	( proto_add_host_dependency "$cfg" '' "$tunlink" )

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
	proto_init_update "$cfg" 1

	proto_add_tunnel
	json_add_string mode "$mode"
	json_add_int mtu "${mtu:-1280}"

<<<<<<< HEAD
	[ -n "$tunlink" ] && {
		( proto_add_host_dependency "$cfg" '' "$tunlink" )
		json_add_string link "$tunlink"
	}
=======
	json_add_string link "$tunlink"
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

	json_add_boolean multicast "${multicast:-1}"

	json_add_object 'data'
	[ -n "$ifid" ] && json_add_int ifid "$ifid"
	json_close_object

	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

proto_xfrm_teardown() {
	local cfg="$1"
}

proto_xfrm_init_config() {
	no_device=1
	available=1

	proto_config_add_int "mtu"
	proto_config_add_string "tunlink"
	proto_config_add_string "zone"
	proto_config_add_int "ifid"
	proto_config_add_boolean "multicast"
}


[ -n "$INCLUDE_ONLY" ] || {
<<<<<<< HEAD
	[ -d /sys/module/xfrm_interface ] && add_protocol xfrm
=======
	[ -f /lib/modules/$(uname -r)/xfrm_interface.ko -o -d /sys/module/xfrm_interface ] && add_protocol xfrm
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
}
