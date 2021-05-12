#include <string.h>
#include <systemd/sd-bus.h>
#define _cleanup_(f) __attribute__((cleanup(f)))

#include "logging.h"
#include "rtsp_connection.h"
#include "rtsp_server.h"

/*----------------------------------------------------------------------------*/
RtspServer::RtspServer()
{
    LOG_DEBUG("RtspServer");

    register_connection_factory(RtspServer::connection_factory, this);
    unsigned port = 554;

    // Start with port 554
    try {
        init(port, IpAddress::AnyAddress());
    } catch (SocketException e) {
	port = 8554;
	init(port, IpAddress::AnyAddress());
    } catch (NetworkException e) {
	port = 8554;
	init(port, IpAddress::AnyAddress());
    }

    register_service(port);
}

/*----------------------------------------------------------------------------*/
void RtspServer::register_service(uint16_t port)
{
    _cleanup_(sd_bus_flush_close_unrefp) sd_bus * bus = NULL;
    _cleanup_(sd_bus_message_unrefp) sd_bus_message * request = NULL;

    int status = sd_bus_open_system(&bus);
    if(status < 0) {
	LOG_ERROR("Failed to open system bus: %s", strerror(-status));
	return;
    }
    uint16_t zero = 0;
    status = sd_bus_message_new_method_call(bus, &request,
		    "org.freedesktop.resolve1",
		    "/org/freedesktop/resolve1", 
		    "org.freedesktop.resolve1.Manager",
		    "RegisterService");
    if(status < 0) {
	LOG_ERROR("Failed create Dbus message: %s", strerror(-status));
	return;
    }
    status = sd_bus_message_append(request, "sssqqq",
		    "sound_relay", "sound_relay", "_rtsp._tcp", port,
		    zero, zero);
    if(status < 0) {
	LOG_ERROR("Failed create Dbus message: %s", strerror(-status));
	return;
    }

    status = sd_bus_message_append(request, "aa{say}", 1, 1, "name", 1, 'a');
    if(status < 0) {
	LOG_ERROR("Failed create Dbus message: %s", strerror(-status));
	return;
    }

    sd_bus_message_seal(request, 12345, -1);

    sd_bus_message_dump(request, stdout, SD_BUS_MESSAGE_DUMP_WITH_HEADER);

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message * reply = NULL;


    status = sd_bus_call(bus, request, -1, &error, &reply);
//    status = sd_bus_send(bus, request, NULL);
    if(status < 0) {
	LOG_ERROR("Failed to Register service: %s", strerror(-status));
	LOG_ERROR("Failed to Register service: %s", error.message);
	sd_bus_error_free(&error);
	sd_bus_message_unref(reply);
	return;
    }


    sd_bus_message_unref(reply);
//  sd_bus_error_free(&error);
}


/*----------------------------------------------------------------------------*/
RtspServer::~RtspServer() 
{
    LOG_DEBUG("~RtspServer");
}


/*----------------------------------------------------------------------------*/
Connection * RtspServer::connection_factory(void *)
{
    return new RtspConnection();
}




