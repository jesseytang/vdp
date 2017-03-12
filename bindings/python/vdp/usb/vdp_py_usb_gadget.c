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

#include "vdp_py_usb_gadget.h"
#include "vdp/usb_gadget.h"

static int getint(PyObject* obj, const char* name, int* res)
{
    PyObject* name_obj;
    PyObject* value_obj;
    int value;

    if (!*res) {
        return 0;
    }

    name_obj = PyString_FromString(name);
    value_obj = PyObject_GetItem(obj, name_obj);
    Py_DECREF(name_obj);
    if (!value_obj) {
        PyErr_Format(PyExc_AttributeError, "attribute '%s' not found", name);
        *res = 0;
        return 0;
    }

    if (!PyInt_Check(value_obj)) {
        Py_DECREF(value_obj);
        PyErr_Format(PyExc_TypeError, "value of '%s' is not numeric", name);
        *res = 0;
        return 0;
    }

    value = PyInt_AsLong(value_obj);
    Py_DECREF(value_obj);

    return value;
}

static int getenum(PyObject* obj, const char* name, int (*validate)(int), int* res)
{
    PyObject* name_obj;
    PyObject* value_obj;
    int value;

    if (!*res) {
        return 0;
    }

    name_obj = PyString_FromString(name);
    value_obj = PyObject_GetItem(obj, name_obj);
    Py_DECREF(name_obj);
    if (!value_obj) {
        PyErr_Format(PyExc_AttributeError, "attribute '%s' not found", name);
        *res = 0;
        return 0;
    }

    if (!PyInt_Check(value_obj)) {
        Py_DECREF(value_obj);
        PyErr_Format(PyExc_TypeError, "value of '%s' is not numeric", name);
        *res = 0;
        return 0;
    }

    value = PyInt_AsLong(value_obj);
    Py_DECREF(value_obj);

    if (!validate(value)) {
        PyErr_Format(PyExc_ValueError, "invalid '%s' value", name);
        *res = 0;
        return 0;
    }

    return value;
}

static void free_descriptors(struct vdp_usb_descriptor_header** descriptors)
{
    int i;

    for (i = 0; descriptors && descriptors[i]; ++i) {
        free(descriptors[i]);
    }
    free(descriptors);
}

static struct vdp_usb_descriptor_header** get_descriptors(PyObject* obj, const char* name, int* res)
{
    struct vdp_usb_descriptor_header** descriptors = NULL;
    Py_ssize_t cnt, i;
    PyObject* name_obj;
    PyObject* value_obj;

    if (!*res) {
        return NULL;
    }

    name_obj = PyString_FromString(name);
    value_obj = PyObject_GetItem(obj, name_obj);
    Py_DECREF(name_obj);
    if (!value_obj || (value_obj == Py_None)) {
        PyErr_Clear();
        return NULL;
    }

    if (!PyList_Check(value_obj)) {
        PyErr_Format(PyExc_TypeError, "value of '%s' is not a list", name);
        goto error;
    }

    cnt = PyList_Size(value_obj);

    if (cnt < 1) {
        Py_DECREF(value_obj);
        return NULL;
    }

    descriptors = malloc(sizeof(descriptors[0]) * (cnt + 1));

    for (i = 0; i < cnt; ++i) {
        PyObject* desc;
        int desc_type = 0;
        const char* desc_data = NULL;
        int desc_data_size = 0;

        descriptors[i] = NULL;

        desc = PyList_GetItem(value_obj, i);

        if (!PyTuple_Check(desc)) {
            PyErr_Format(PyExc_TypeError, "value of '%s' elements are not tuples", name);
            goto error;
        }

        if (!PyArg_ParseTuple(desc, "it#", &desc_type, &desc_data, &desc_data_size)) {
            goto error;
        }

        descriptors[i] = malloc(sizeof(*descriptors[0]) + desc_data_size);

        descriptors[i]->bDescriptorType = desc_type;
        descriptors[i]->bLength = sizeof(*descriptors[0]) + desc_data_size;
        memcpy(descriptors[i] + 1, desc_data, desc_data_size);
    }

    descriptors[i] = NULL;

    Py_DECREF(value_obj);

    return descriptors;

error:
    free_descriptors(descriptors);
    Py_DECREF(value_obj);
    *res = 0;

    return NULL;
}

struct vdp_py_usb_gadget_ep
{
    PyObject_HEAD

    struct vdp_usb_gadget_ep_caps caps;

    PyObject* fn_enable;
    PyObject* fn_enqueue;
    PyObject* fn_dequeue;
    PyObject* fn_clear_stall;
    PyObject* fn_destroy;

    struct vdp_usb_gadget_ep* ep;
};

static int vdp_py_usb_gadget_ep_init_obj(struct vdp_py_usb_gadget_ep* self, PyObject* args, PyObject* kwargs)
{
    PyObject* caps;
    int res = 1;

    if (!PyArg_ParseTuple(args, "O", &caps)) {
        return -1;
    }

    self->caps.address = getint(caps, "address", &res);
    self->caps.dir = getenum(caps, "dir", &vdp_usb_gadget_ep_dir_validate, &res);
    self->caps.type = getenum(caps, "type", &vdp_usb_gadget_ep_type_validate, &res);
    if (self->caps.type == vdp_usb_gadget_ep_iso) {
        self->caps.sync = getenum(caps, "sync", &vdp_usb_gadget_ep_sync_validate, &res);
        self->caps.usage = getenum(caps, "usage", &vdp_usb_gadget_ep_usage_validate, &res);
    }
    self->caps.max_packet_size = getint(caps, "max_packet_size", &res);
    self->caps.interval = getint(caps, "interval", &res);
    self->caps.descriptors = get_descriptors(caps, "descriptors", &res);

    if (!res) {
        return -1;
    }

    self->fn_enable = PyObject_GetAttrString((PyObject*)self, "enable");
    self->fn_enqueue = PyObject_GetAttrString((PyObject*)self, "enqueue");
    self->fn_dequeue = PyObject_GetAttrString((PyObject*)self, "dequeue");
    self->fn_clear_stall = PyObject_GetAttrString((PyObject*)self, "clear_stall");
    self->fn_destroy = PyObject_GetAttrString((PyObject*)self, "destroy");

    PyErr_Clear();

    return 0;
}

static void vdp_py_usb_gadget_ep_dealloc(struct vdp_py_usb_gadget_ep* self)
{
    Py_XDECREF(self->fn_enable);
    Py_XDECREF(self->fn_enqueue);
    Py_XDECREF(self->fn_dequeue);
    Py_XDECREF(self->fn_clear_stall);
    Py_XDECREF(self->fn_destroy);

    free_descriptors(self->caps.descriptors);

    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyTypeObject vdp_py_usb_gadget_eptype =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    "usb.gadget.Endpoint", /* tp_name */
    sizeof(struct vdp_py_usb_gadget_ep), /* tp_basicsize */
    0, /* tp_itemsize */
    (destructor)vdp_py_usb_gadget_ep_dealloc, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_reserved */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_hash  */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "vdpusb gadget Endpoint", /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_clear */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    0, /* tp_methods */
    0, /* tp_members */
    0, /* tp_getset */
};

static PyMethodDef vdp_py_usb_gadget_methods[] =
{
    { NULL }
};

void vdp_py_usb_gadget_init(PyObject* module)
{
    PyObject* module2 = Py_InitModule3("usb.gadget", vdp_py_usb_gadget_methods, "vdpusb gadget module");

    PyModule_AddIntConstant(module2, "EP_IN", vdp_usb_gadget_ep_in);
    PyModule_AddIntConstant(module2, "EP_OUT", vdp_usb_gadget_ep_out);
    PyModule_AddIntConstant(module2, "EP_INOUT", vdp_usb_gadget_ep_inout);

    PyModule_AddIntConstant(module2, "EP_CONTROL", vdp_usb_gadget_ep_control);
    PyModule_AddIntConstant(module2, "EP_ISO", vdp_usb_gadget_ep_iso);
    PyModule_AddIntConstant(module2, "EP_BULK", vdp_usb_gadget_ep_bulk);
    PyModule_AddIntConstant(module2, "EP_INT", vdp_usb_gadget_ep_int);

    PyModule_AddIntConstant(module2, "EP_SYNC_NONE", vdp_usb_gadget_ep_sync_none);
    PyModule_AddIntConstant(module2, "EP_SYNC_ASYNC", vdp_usb_gadget_ep_sync_async);
    PyModule_AddIntConstant(module2, "EP_SYNC_ADAPTIVE", vdp_usb_gadget_ep_sync_adaptive);
    PyModule_AddIntConstant(module2, "EP_SYNC_SYNC", vdp_usb_gadget_ep_sync_sync);

    PyModule_AddIntConstant(module2, "EP_USAGE_DATA", vdp_usb_gadget_ep_usage_data);
    PyModule_AddIntConstant(module2, "EP_USAGE_FEEDBACK", vdp_usb_gadget_ep_usage_feedback);
    PyModule_AddIntConstant(module2, "EP_USAGE_IMPLICIT_FB", vdp_usb_gadget_ep_usage_implicit_fb);

    vdp_py_usb_gadget_eptype.tp_new = PyType_GenericNew;
    vdp_py_usb_gadget_eptype.tp_init = (initproc)vdp_py_usb_gadget_ep_init_obj;
    if (PyType_Ready(&vdp_py_usb_gadget_eptype) < 0) {
        return;
    }

    Py_INCREF(&vdp_py_usb_gadget_eptype);
    PyModule_AddObject(module2, "Endpoint", (PyObject*)&vdp_py_usb_gadget_eptype);

    Py_INCREF(module2);
    PyModule_AddObject(module, "gadget", module2);
}