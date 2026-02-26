
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>

#include "plc.h"

#define MATPLC_OPEN_CMD 1
#define MATPLC_CLOSE_CMD 2
#define MATPLC_GETPT_CMD 3
#define MATPLC_SETPT_CMD 4
#define MATPLC_RDCONF_CMD 5
#define MATPLC_UPDATE_CMD 6
 
static int lplc_cmd(
		ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
	int cmd, i, j, index = -1;
	char *ptname;
	char *module = "TCL";
	char *conffile = NULL, *array = NULL, *name = NULL;	
	char idxtok[128];
	char result[128];

	u32 value;	
	plc_pt_t handle;

	if (!strcmp(argv[1], "init")) {
		cmd = MATPLC_OPEN_CMD;
	} else if (!strcmp(argv[1], "done")) {
		cmd = MATPLC_CLOSE_CMD;
	} else if (!strcmp(argv[1], "update")) {
		cmd = MATPLC_UPDATE_CMD;
	} else if (!strcmp(argv[1], "getpt")) {
		cmd = MATPLC_GETPT_CMD;
	} else if (!strcmp(argv[1], "setpt")) {
		cmd = MATPLC_SETPT_CMD;
	} else if (!strcmp(argv[1], "readconf")) {
		cmd = MATPLC_RDCONF_CMD;
	} else {
		Tcl_AppendResult (interp,
				"Error, command must be one of: init, done, getpt, setpt\n",
				(char *) NULL);
		return(TCL_ERROR);
	}

	for (i = 2; i < argc; i++) {
		if (!strcmp(argv[i], "-module")) {
			module = argv[++i];
		} else if (!strcmp(argv[i], "-config")) {
			conffile = argv[++i];
		} else if (!strcmp(argv[i], "-name")) {
			name = argv[++i];
		} else if (!strcmp(argv[i], "-array")) {
			array = argv[++i];
		} else if (!strcmp(argv[i], "-index")) {
			index = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-value")) {
			value = atoi(argv[++i]);
/*
unknown options may be useful to plc_init()
		} else {
			Tcl_AppendResult (interp, "Unknown option ", argv[i], " \n",
					(char *) NULL);
			return(TCL_ERROR);
*/
		}
	}

	switch (cmd) {
	case MATPLC_OPEN_CMD:
		if (plc_init(module, argc, argv) < 0) {
			Tcl_AppendResult (interp, "Error initializing PLC\n", (char *) NULL);
			return(TCL_ERROR);
		}
		if (array) {
			sprintf(result, "%d", j = plc_pt_count());
			Tcl_SetVar2(interp, array, "count", result, 0);
			for (i = 0; i < j; i++) {
				handle = plc_pt_by_index(i, &ptname, NULL);
				sprintf(idxtok, "%d,name", i);
				if (handle.valid) {
					Tcl_SetVar2(interp, array, idxtok, ptname, 0);
				} else {
					Tcl_SetVar2(interp, array, idxtok, "INVALID", 0);
				}
			}
		}
		break;
	case MATPLC_UPDATE_CMD:
		plc_update();
		if (array) {
			j = plc_pt_count();
			for (i = 0; i < j; i++) {
				sprintf(idxtok, "%d,value", i);
				handle = plc_pt_by_index(i, &ptname, NULL);
				if (handle.valid) {
					sprintf(result, "%d", plc_get(handle));
				} else {
					strcpy(result, "INVALID");
				}
				Tcl_SetVar2(interp, array, idxtok, result, 0);
			}
		}
		break;
	case MATPLC_CLOSE_CMD:
		plc_done();
		break;
	case MATPLC_GETPT_CMD:
		if (!name && (index == -1)) {
			Tcl_AppendResult (interp,
					"Error, specify a point name or an index\n",
					(char *) NULL);
			return(TCL_ERROR);
		}
		if (name && (index != -1)) {
			Tcl_AppendResult (interp,
					"Error, specify either a point name or an index, not both\n",
					(char *) NULL);
			return(TCL_ERROR);
		}
		if (name) {
			handle = plc_pt_by_name(name);
  			if (handle.valid == 0) {
				Tcl_AppendResult (interp,
					"Error, could not get handle for point ", name, "\n",
					(char *) NULL);
				return(TCL_ERROR);
			}
			plc_update();
			value = plc_get(handle);
			sprintf(result, "%d", value);
			Tcl_SetResult(interp, result, TCL_VOLATILE);
		} else {
			Tcl_AppendResult (interp,
					"Sorry, get by index is not yet implemented\n", (char *)NULL);
			return(TCL_ERROR);
		}
		break;
	case MATPLC_SETPT_CMD:
		if (!name) {
			Tcl_AppendResult(interp, "Error, specify a point name\n",
					(char *)NULL);
			return(TCL_ERROR);
		}
		handle = plc_pt_by_name(name);
  		if (!handle.valid) {
			Tcl_AppendResult (interp,
				"Error, could not get handle for point ", name, "\n", (char *)NULL);
			return(TCL_ERROR);
		}
		plc_set(handle, value);
		plc_update();
		break;
	default:
		Tcl_AppendResult (interp,
				"Sorry, that command is not yet implemented\n", (char *)NULL);
		return(TCL_ERROR);
	}

	return (TCL_OK);
}

/*
 *----------------------------------------------------------------------
 *
 * Lplc_Init --
 *
 *        Initialize the new package.  The string "lplc" in the
 *        function name must match the PACKAGE declaration at the top of
 *        configure.in.  note that the init routine has an initial capital
 *        'L'.  it works this way, and doesnt work without it.
 *
 * Results:
 *        A standard Tcl result
 *
 * Side effects:
 *        The lplc package is created.
 *        One new command "lplc" is added to the Tcl interpreter.
 *
 *----------------------------------------------------------------------
 */

int Lplc_Init(Tcl_Interp *interp)
{
	if (Tcl_InitStubs(interp, "8.0", 0) == NULL) {
		return TCL_ERROR;
	}
	if (Tcl_PkgRequire(interp, "Tcl", TCL_VERSION, 0) == NULL) {
		if (TCL_VERSION[0] == '7') {
			if (Tcl_PkgRequire(interp, "Tcl", "8.0", 0) == NULL) {
				return TCL_ERROR;
			}
		}
	}
	if (Tcl_PkgProvide(interp, "lplc", VERSION) != TCL_OK) {
		return TCL_ERROR;
	}
	Tcl_CreateCommand(interp, "lplc", lplc_cmd, (ClientData)NULL,
			(Tcl_CmdDeleteProc *)NULL);
	return TCL_OK;
}
