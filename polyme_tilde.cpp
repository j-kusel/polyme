#include "c74_max.h"
#include "c74_msp.h"
#include "string.h"

#define SEP "/"

using namespace c74::max;

static t_class* this_class = nullptr;

struct _row {
    short col[16];
    short blink;
    short rate;
};

struct t_polyme {
	t_pxobject	obj;
    void *outlet1;
    void *m_qelem;
    
    t_atom command[16];
    short command_len;
    
    struct _row rows[16];
    
    int triad[3];
};

/*void dispatcher(t_polyme *self, char *remote) {
    char *comm;
    char *input, *tofree;
    tofree = input = strdup(remote);
    comm = strsep(&remote, "/");
    if (! strcmp(comm, "monome")) {
        object_post((t_object *) self, "monome was detected!");
    }
}*/

void key_update(t_polyme *self, t_symbol *msg, long argc, t_atom *argv) {
    self->rows[atom_getlong(argv+1)].col[atom_getlong(argv)] = atom_getlong(argv + 2);
    object_post((t_object *) self, "success!!!");
}

void send_command(t_polyme *self) {
    outlet_list(self->outlet1, NULL, self->command_len, self->command);
}

void polyme_sendclear(t_polyme *self) {
    atom_setsym(self->command, gensym("/monome/grid/led/all"));
    atom_setlong(self->command + 1, 0);
    self->command_len = 2;
    qelem_set((t_qelem *) self->m_qelem);
    //outlet_list(self->outlet1, NULL, 2, list_out);
}

void polyme_onListMsg(t_polyme* self, t_symbol *msg, long argc, t_atom *argv) {
    /*t_symbol *command;
    char *comm;
    char *input, *tofree;
    command = atom_getsym(argv);
    tofree = input = strdup(command->s_name);
    // argv is preserved, input is consumed
    comm = strsep(&input, SEP); // strip initial slash
    comm = strsep(&input, SEP); // strip first command
    if (! strcmp(comm, "monome")) {
        object_post((t_object *) self, "monome was detected! %s", input);
        comm = strsep(&input, SEP); // strip 2nd command
        if (! strcmp(comm, "grid")) {
            object_post((t_object *) self, "grid was detected! %s", input);
            comm = strsep(&input, SEP); // strip 3rd command
            if (! strcmp(comm, "key")) {
                key_update(self, atom_getlong(argv + 1), atom_getlong(argv + 2), atom_getlong(argv + 3));
            }
        }
    }*/
    
    short r = atom_getlong(argv + 1);
    short c = atom_getlong(argv);
    short onoff = atom_getlong(argv + 2);
    self->rows[r].col[c] = onoff;
    for (auto i=0; i < 3; i++) {
        self->triad[i] = atom_getlong(argv + i);
        atom_setlong(self->command + 1 + i, atom_getlong(argv + i));
    }
    atom_setsym(self->command, gensym("/monome/grid/led/set"));
    self->command_len = 4;
    qelem_set((t_qelem *) self->m_qelem);
}

void polyme_sysport(t_polyme* self, t_symbol *msg, long argc, t_atom *argv) {
    ;
}

void polyme_sysprefix(t_polyme* self, t_symbol *msg, long argc, t_atom *argv) {
    ;
}

void polyme_serialoscdevice(t_polyme* self, t_symbol *msg, long argc, t_atom *argv) {
    ;
}

void* polyme_new(void) {
	t_polyme* self = (t_polyme*)object_alloc(this_class);
    
    self->triad[0] = 0;
    self->triad[1] = 0;
    self->triad[2] = 0;
	
	dsp_setup((t_pxobject*)self, 1);
    
    self->outlet1 = outlet_new(self, "list");
    self->m_qelem = qelem_new((t_object *) self, (method)send_command);
    
    outlet_new(self, "signal");
    outlet_new(self, "signal");
    outlet_new(self, "signal");
    
	return self;
}


void polyme_free(t_polyme* self) {
    qelem_free((t_qelem *) self->m_qelem);
	dsp_free((t_pxobject*)self);
}


void polyme_perform64(t_polyme* self, t_object* dsp64, double** ins, long numins, double** outs, long numouts, long sampleframes, long flags, void* userparam) {
    for (auto i=0; i<sampleframes; ++i) {
		outs[0][i] = self->triad[0];
        outs[1][i] = self->triad[1];
        outs[2][i] = self->triad[2];
    }
}


void polyme_dsp64(t_polyme* self, t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
	object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
						 dsp64, gensym("dsp_add64"), (t_object*)self, (t_perfroutine64)polyme_perform64, 0, NULL);
}


void polyme_assist(t_polyme* self, void* unused, t_assist_function io, long index, char* string_dest) {
	if (io == ASSIST_INLET) {
		switch (index) {
			case 0: 
				strncpy(string_dest,"INLET 1", ASSIST_STRING_MAXSIZE);
				break;
		}
	}
	else if (io == ASSIST_OUTLET) {
		switch (index) {
			case 0: 
				strncpy(string_dest,"OUTLET 1", ASSIST_STRING_MAXSIZE);
				break;
		}
	}
}


void ext_main(void* r) {
	this_class = class_new("polyme~", (method)polyme_new, (method)polyme_free, sizeof(t_polyme), 0, A_GIMME, 0);

	class_addmethod(this_class, (method)polyme_assist,	"assist",           A_CANT,		0);
	class_addmethod(this_class, (method)polyme_dsp64,	"dsp64",            A_CANT,		0);
    
    class_addmethod(this_class, (method)polyme_onListMsg, "list",           A_GIMME,    0);
    class_addmethod(this_class, (method)key_update,     "/monome/grid/key", A_GIMME,    0);
    class_addmethod(this_class, (method)polyme_sysport, "/sys/port",        A_GIMME,    0);
	class_addmethod(this_class, (method)polyme_sysport, "/sys/prefix",        A_GIMME,  0);
    class_addmethod(this_class, (method)polyme_serialoscdevice, "/serialosc/device", A_GIMME, 0);
    class_addmethod(this_class, (method)polyme_sendclear,   "clear",                    0);
    
	class_dspinit(this_class);
	class_register(CLASS_BOX, this_class);
}
