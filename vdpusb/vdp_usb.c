#include "vdp/usb.h"
#include "vdp_usb_urbi.h"
#include <stdlib.h>
#include <assert.h>

const char* vdp_usb_result_to_str(vdp_usb_result res)
{
    switch (res) {
    case vdp_usb_success: return "success";
    case vdp_usb_nomem: return "not enough memory";
    case vdp_usb_misuse: return "misuse";
    case vdp_usb_unknown: return "unknown error";
    case vdp_usb_not_found: return "entity not found";
    case vdp_usb_busy: return "device is busy";
    case vdp_usb_protocol_error: return "kernel-user protocol error";
    default: return "undefined error";
    };
}

const char* vdp_usb_signal_type_to_str(vdp_usb_signal_type signal_type)
{
    switch (signal_type) {
    case vdp_usb_signal_reset_start: return "reset_start";
    case vdp_usb_signal_reset_end: return "reset_end";
    case vdp_usb_signal_power_on: return "power_on";
    case vdp_usb_signal_power_off: return "power_off";
    default: return "undefined";
    };
}

const char* vdp_usb_urb_type_to_str(vdp_usb_urb_type urb_type)
{
    switch (urb_type) {
    case vdp_usb_urb_control: return "control";
    case vdp_usb_urb_bulk: return "bulk";
    case vdp_usb_urb_int: return "int";
    case vdp_usb_urb_iso: return "iso";
    default: return "undefined";
    };
}

const char* vdp_usb_request_type_direction_to_str(vdp_u8 bRequestType)
{
    if (VDP_USB_REQUESTTYPE_IN(bRequestType)) {
        return "in";
    } else {
        return "out";
    }
}

const char* vdp_usb_request_type_type_to_str(vdp_u8 bRequestType)
{
    switch (VDP_USB_REQUESTTYPE_TYPE(bRequestType)) {
    case VDP_USB_REQUESTTYPE_TYPE_STANDARD: return "standard";
    case VDP_USB_REQUESTTYPE_TYPE_CLASS: return "class";
    case VDP_USB_REQUESTTYPE_TYPE_VENDOR: return "vendor";
    case VDP_USB_REQUESTTYPE_TYPE_RESERVED: return "reserved";
    default: return "unknown";
    }
}

const char* vdp_usb_request_type_recipient_to_str(vdp_u8 bRequestType)
{
    switch (VDP_USB_REQUESTTYPE_RECIPIENT(bRequestType)) {
    case VDP_USB_REQUESTTYPE_RECIPIENT_DEVICE: return "device";
    case VDP_USB_REQUESTTYPE_RECIPIENT_INTERFACE: return "interface";
    case VDP_USB_REQUESTTYPE_RECIPIENT_ENDPOINT: return "endpoint";
    case VDP_USB_REQUESTTYPE_RECIPIENT_OTHER: return "other";
    case VDP_USB_REQUESTTYPE_RECIPIENT_PORT: return "port";
    case VDP_USB_REQUESTTYPE_RECIPIENT_RPIPE: return "rpipe";
    default: return "unknown";
    }
}

const char* vdp_usb_request_to_str(vdp_u8 bRequest)
{
    switch (bRequest) {
    case VDP_USB_REQUEST_GET_STATUS: return "VDP_USB_REQUEST_GET_STATUS";
    case VDP_USB_REQUEST_CLEAR_FEATURE: return "VDP_USB_REQUEST_CLEAR_FEATURE";
    case VDP_USB_REQUEST_SET_FEATURE: return "VDP_USB_REQUEST_SET_FEATURE";
    case VDP_USB_REQUEST_SET_ADDRESS: return "VDP_USB_REQUEST_SET_ADDRESS";
    case VDP_USB_REQUEST_GET_DESCRIPTOR: return "VDP_USB_REQUEST_GET_DESCRIPTOR";
    case VDP_USB_REQUEST_SET_DESCRIPTOR: return "VDP_USB_REQUEST_SET_DESCRIPTOR";
    case VDP_USB_REQUEST_GET_CONFIGURATION: return "VDP_USB_REQUEST_GET_CONFIGURATION";
    case VDP_USB_REQUEST_SET_CONFIGURATION: return "VDP_USB_REQUEST_SET_CONFIGURATION";
    case VDP_USB_REQUEST_GET_INTERFACE: return "VDP_USB_REQUEST_GET_INTERFACE";
    case VDP_USB_REQUEST_SET_INTERFACE: return "VDP_USB_REQUEST_SET_INTERFACE";
    case VDP_USB_REQUEST_SYNCH_FRAME: return "VDP_USB_REQUEST_SYNCH_FRAME";
    case VDP_USB_REQUEST_SET_ENCRYPTION: return "VDP_USB_REQUEST_SET_ENCRYPTION";
    case VDP_USB_REQUEST_GET_ENCRYPTION: return "VDP_USB_REQUEST_GET_ENCRYPTION";
    case VDP_USB_REQUEST_SET_HANDSHAKE: return "VDP_USB_REQUEST_SET_HANDSHAKE";
    case VDP_USB_REQUEST_GET_HANDSHAKE: return "VDP_USB_REQUEST_GET_HANDSHAKE";
    case VDP_USB_REQUEST_SET_CONNECTION: return "VDP_USB_REQUEST_SET_CONNECTION";
    case VDP_USB_REQUEST_SET_SECURITY_DATA: return "VDP_USB_REQUEST_SET_SECURITY_DATA";
    case VDP_USB_REQUEST_GET_SECURITY_DATA: return "VDP_USB_REQUEST_GET_SECURITY_DATA";
    case VDP_USB_REQUEST_SET_WUSB_DATA: return "VDP_USB_REQUEST_SET_WUSB_DATA";
    case VDP_USB_REQUEST_LOOPBACK_DATA_WRITE: return "VDP_USB_REQUEST_LOOPBACK_DATA_WRITE";
    case VDP_USB_REQUEST_LOOPBACK_DATA_READ: return "VDP_USB_REQUEST_LOOPBACK_DATA_READ";
    case VDP_USB_REQUEST_SET_INTERFACE_DS: return "VDP_USB_REQUEST_SET_INTERFACE_DS";
    default : return "VDP_USB_REQUEST_XXX";
    }
}

void vdp_usb_urb_to_str(const struct vdp_usb_urb* urb, char* buff, size_t buff_size)
{
    int ret;

    if (urb->type == vdp_usb_urb_int) {
        ret = snprintf(
            buff,
            buff_size,
            "id = %u, type = %s, flags = 0x%X, ep = 0x%.2X, interval = %u, buff = (%u)",
            urb->id,
            vdp_usb_urb_type_to_str(urb->type),
            urb->flags,
            urb->endpoint_address,
            urb->interval,
            urb->transfer_length);
    } else if (urb->type == vdp_usb_urb_iso) {
        ret = snprintf(
            buff,
            buff_size,
            "id = %u, type = %s, flags = 0x%X, ep = 0x%.2X, num_packets = %u, interval = %u, buff = (%u)",
            urb->id,
            vdp_usb_urb_type_to_str(urb->type),
            urb->flags,
            urb->endpoint_address,
            urb->number_of_packets,
            urb->interval,
            urb->transfer_length);
    } else if (urb->type == vdp_usb_urb_control) {
        ret = snprintf(
            buff,
            buff_size,
            "id = %u, type = %s, flags = 0x%X, ep = 0x%.2X, bRequestType = %s:%s:%s, bRequest = %s, wValue = %u, wIndex = %u, buff = (%u)",
            urb->id,
            vdp_usb_urb_type_to_str(urb->type),
            urb->flags,
            urb->endpoint_address,
            vdp_usb_request_type_direction_to_str(urb->setup_packet->bRequestType),
            vdp_usb_request_type_type_to_str(urb->setup_packet->bRequestType),
            vdp_usb_request_type_recipient_to_str(urb->setup_packet->bRequestType),
            vdp_usb_request_to_str(urb->setup_packet->bRequest),
            urb->setup_packet->wValue,
            urb->setup_packet->wIndex,
            urb->setup_packet->wLength);
    } else {
        ret = snprintf(
            buff,
            buff_size,
            "id = %u, type = %s, flags = 0x%X, ep = 0x%.2X, buff = (%u)",
            urb->id,
            vdp_usb_urb_type_to_str(urb->type),
            urb->flags,
            urb->endpoint_address,
            urb->transfer_length);
    }

    if (ret <= 0) {
        buff[0] = '\0';
    } else {
        buff[buff_size - 1] = '\0';
    }
}

void vdp_usb_free_urb(struct vdp_usb_urb* urb)
{
    struct vdp_usb_urbi* urbi;

    assert(urb);
    if (!urb) {
        return;
    }

    urbi = vdp_containerof(urb, struct vdp_usb_urbi, urb);

    vdp_usb_urbi_destroy(urbi);
}
