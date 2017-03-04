/*
 * Copyright (c) 2017, Stanislav Vorobiov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vdp_py_usb_error.h"
#include "vdp_py_usb_urb.h"
#include "vdp_py_usb_context.h"
#include "vdp_py_usb_device.h"

static PyMethodDef vdp_py_usb_methods[] =
{
    { NULL }
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initusb(void)
{
    PyObject* module = Py_InitModule3("usb", vdp_py_usb_methods, "vdpusb module");

    PyModule_AddIntConstant(module, "SUCCESS", vdp_usb_success);
    PyModule_AddIntConstant(module, "NOMEM", vdp_usb_nomem);
    PyModule_AddIntConstant(module, "MISUSE", vdp_usb_misuse);
    PyModule_AddIntConstant(module, "UNKNOWN", vdp_usb_unknown);
    PyModule_AddIntConstant(module, "NOT_FOUND", vdp_usb_not_found);
    PyModule_AddIntConstant(module, "BUSY", vdp_usb_busy);
    PyModule_AddIntConstant(module, "PROTOCOL_ERROR", vdp_usb_protocol_error);

    PyModule_AddIntConstant(module, "SPEED_LOW", vdp_usb_speed_low);
    PyModule_AddIntConstant(module, "SPEED_FULL", vdp_usb_speed_full);
    PyModule_AddIntConstant(module, "SPEED_HIGH", vdp_usb_speed_high);

    vdp_py_usb_error_init(module);
    vdp_py_usb_context_init(module);
    vdp_py_usb_urb_init(module);
    vdp_py_usb_device_init(module);
}