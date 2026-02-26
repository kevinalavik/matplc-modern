/*
 * (c) 2002 Jiri Baum
 *          Juan Carlos Orozco
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 *
 * 04/12/2002 Mario de Sousa
 *              - Fixed blocking bug in scan_beg()
 */

/* To use this module follow the next steps:
 * 1. (optional) ./setup.py build
 * 2. as root: ./setup.py install
 *
 * Sample session with mat running:
 * >>> import matplc
 * >>> matplc.init("modulename")
 * >>> matplc.update()
 * >>> matplc.test("POINT") #only in the testing version that has test()
 * 3
 * >>> p = matplc.point("POINT")
 * >>> p.get() //Can also use matplc.get(p)
 * 3
 * >>> p.set(p, 7)   //You need to have permission to write
 * >>> matplc.update()
 * >>> p.get()
 * 7
 * >>>
 */

#include "Python.h"
#include "plc.h"

/* type-definition & utility-macros */
typedef struct {
    PyObject_HEAD
    plc_pt_t pt;
} point;

staticforward PyTypeObject point_type;

/* a typetesting macro (we don't use it here) */
#define is_cons(v) ((v)->ob_type == &point_type)

/* macros to access car & cdr, both as lvalues & rvalues */
#define ptof(v) (((point*)(v))->pt)

static PyObject*
pt_by_name(PyObject* self, PyObject* args)
{
    char *s;
	
    point *p = PyObject_NEW(point, &point_type);
    if(!PyArg_ParseTuple(args, "s", &s)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else {
        if (p) {
            p->pt = plc_pt_by_name(s);
        }
    }
    return Py_BuildValue("Oi", (PyObject*)p, p->pt.valid);
}

static PyObject*
subpt(PyObject* self, PyObject* args)
{
    PyObject *po;
    point *pn = PyObject_NEW(point, &point_type);
    int start, len;

    if(!PyArg_ParseTuple(args, "Oii", &po, &start, &len)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else{
        pn->pt = plc_subpt(((point*)po)->pt, start, len);
    }
    return Py_BuildValue("Oi", (PyObject*)pn, pn->pt.valid);
}


static PyObject*
pt_null(PyObject* self, PyObject* args)
{
    point *p = PyObject_NEW(point, &point_type);
    if (p) {
	p->pt = plc_pt_null();
    }
    return Py_BuildValue("O", (PyObject*)p);
}

static PyObject*
scan_beg(PyObject* self, PyObject* args)
{
    /* release the global python thread lock */
    Py_BEGIN_ALLOW_THREADS
    plc_scan_beg();
    /* re-aquire the global python thread lock */
    Py_END_ALLOW_THREADS

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
scan_end(PyObject* self, PyObject* args)
{
    Py_BEGIN_ALLOW_THREADS
    plc_scan_end();
    Py_END_ALLOW_THREADS

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
update(PyObject* self, PyObject* args)
{
    Py_BEGIN_ALLOW_THREADS
    plc_update();
    Py_END_ALLOW_THREADS
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
get(PyObject* self, PyObject* args)
{
    PyObject *po;
    point *p;
    int val;

    if(!PyArg_ParseTuple(args, "O", &po)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else{
	p = (point*)po;
        val = plc_get(p->pt);
    }
    return Py_BuildValue("i", val);
}

static PyObject*
get_f(PyObject* self, PyObject* args)
{
    PyObject *po;
    point *p;
    float val;

    if(!PyArg_ParseTuple(args, "O", &po)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else{
	p = (point*)po;
        val = plc_get_f32(p->pt);
    }
    return Py_BuildValue("f", val);
}

static PyObject*
pt_len(PyObject* self, PyObject* args)
{
    PyObject *po;

    if(!PyArg_ParseTuple(args, "O", &po)) {
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        return Py_BuildValue("i", plc_pt_len(((point*)po)->pt));
    }
}

static PyObject*
set(PyObject* self, PyObject* args)
{
    PyObject *po;
    point *p;
    int val;

    if(!PyArg_ParseTuple(args, "Oi", &po, &val)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else{
	p = (point*)po;
        plc_set(p->pt, val);
    }
    Py_INCREF(Py_None);
    return Py_None;
}    

static PyObject*
set_f(PyObject* self, PyObject* args)
{
    PyObject *po;
    point *p;
    float val;

    if(!PyArg_ParseTuple(args, "Of", &po, &val)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else{
	p = (point*)po;
        plc_set_f32(p->pt, val);
    }
    Py_INCREF(Py_None);
    return Py_None;
}    

/*
static PyObject*
test(PyObject* self, PyObject* args)
{
    char *s;

    if(!PyArg_ParseTuple(args, "s", &s)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else {
        plc_pt_t p;
        p = plc_pt_by_name(s);
	return Py_BuildValue("i", plc_get(p));
    }
}    
*/

/* Python type-object */
statichere PyTypeObject point_type = {
    PyObject_HEAD_INIT(0)    /* initialize to 0 to ensure Win32 portability */
    0,                 /*ob_size*/
    "point",            /*tp_name*/
    sizeof(point), /*tp_basicsize*/
    0,                 /*tp_itemsize*/
    /* methods */
    /* implied by ISO C: all zeros thereafter */
};

static PyObject*
init(PyObject *self, PyObject *args)
{
    char *mod;

    if(!PyArg_ParseTuple(args, "s", &mod)) {
        if (plc_init("python_module",0,0)==-1) {
	    PyErr_SetString(PyExc_RuntimeError, "couldn't init MatPLC library");
            // Py_INCREF(Py_None);
            // return Py_None;
            return 0;
        }
    }
    else {
        if (plc_init(mod,0,0)==-1) {
	    PyErr_SetString(PyExc_RuntimeError, "couldn't init MatPLC library");
            // Py_INCREF(Py_None);
            // return Py_None;
	    return 0;
        }
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject*
done(PyObject* self, PyObject* args)
{
    plc_done();
    
    Py_INCREF(Py_None);
    return Py_None;
}

/* module-functions */
static PyMethodDef MatplcMethods[] = {
    {"init",		init,		METH_VARARGS},
    {"pt_by_name",	pt_by_name,	METH_VARARGS},
    {"pt_null",		pt_null,	METH_VARARGS},
    {"subpt",		subpt,		METH_VARARGS},
    {"plc_scan_beg",	scan_beg,	METH_VARARGS},
    {"plc_scan_end",	scan_end,	METH_VARARGS},
    {"plc_update",	update,		METH_VARARGS},
    {"plc_get",         get,		METH_VARARGS},
    {"plc_set",         set,		METH_VARARGS},
    {"plc_get_f",       get_f,		METH_VARARGS},
    {"plc_set_f",       set_f,		METH_VARARGS},
 // {"plc_test",        test,		METH_VARARGS},
    {"pt_len",          pt_len,		METH_VARARGS},
    {"plc_done",        done,           METH_VARARGS},
    {0, 0}
};

/* module entry-point (module-initialization) function */
DL_EXPORT(void)
initmatplc_internal(void)
{
    /* Create the module and add the functions */
    (void)Py_InitModule("matplc_internal", MatplcMethods);
    /* Finish initializing the type-objects */
    point_type.ob_type = &PyType_Type;
}

/* end of file: matplcmodule.c */
