
#ifndef __zbuffer_H__
#define __zbuffer_H__

#ifdef INT_MAX
#define SHARED_INT_MAX  INT_MAX
#else
#define SHARED_INT_MAX  0x7FFFFFFF
#endif

#define zbuffer_MAXCHANS 64 //max number of channels

#define _USE_MATH_DEFINES
#include <math.h>

#define TWO_PI (M_PI * 2)
#define HALF_PI (M_PI * 0.5)

#define ONE_SIXTH 0.16666666666666666666667f

#define ELSE_SIN_TABSIZE  16384
#define ELSE_FADE_TABSIZE 4096

typedef struct _zbuffer{
    void       *c_owner;     // owner of zbuffer, note i don't know if this actually works
    int         c_npts;      // used also as a validation flag, number of samples in an array */
    int         c_numchans;
    t_word    **c_vectors;
    t_symbol  **c_channames;
    t_symbol   *c_bufname;
    int         c_playable;
    int         c_minsize;
    int         c_disabled;
    int         c_single;    // flag: 0-regular mode, 1-load this particular channel (1-idx)
                             // should be used with c_numchans == 1
    // Now zbuffer will have a bufname mode: 0 - <ch>-<bufname> [default/legacy], 1 - <bufname>-<ch>
    int         c_bufnamemode; 
}t_zbuffer;

double interp_lin(double frac, double b, double c);
double interp_cos(double frac, double b, double c);
double interp_pow(double frac, double b, double c, double p);
double interp_lagrange(double frac, double a, double b, double c, double d);
double interp_cubic(double frac, double a, double b, double c, double d);
double interp_spline(double frac, double a, double b, double c, double d);
double interp_hermite(double frac, double a, double b, double c, double d,
    double bias, double tension);

double read_sintab(double phase);
double read_partab(double phase);
double read_fadetab(double phase, int tab);

void init_sine_table(void);
void init_parabolic_table(void);
void init_fade_tables(void);

void zbuffer_bug(char *fmt, ...);
void zbuffer_clear(t_zbuffer *c);
void zbuffer_redraw(t_zbuffer *c);
void zbuffer_playcheck(t_zbuffer *c);

// use this function during zbuffer_init
// passing 0 to complain suppresses warnings
void zbuffer_initarray(t_zbuffer *c, t_symbol *name, int complain);
void zbuffer_validate(t_zbuffer *c, int complain);
// called by zbuffer_validate
t_word *zbuffer_get(t_zbuffer *c, t_symbol * name, int *bufsize, int indsp, int complain);

// wrap around initarray, but allow warnings (pass 1 to complain)
void zbuffer_setarray(t_zbuffer *c, t_symbol *name);

void zbuffer_setminsize(t_zbuffer *c, int i);
void zbuffer_enable(t_zbuffer *c, t_floatarg f);

// single channel mode used for poke~/peek~
// Now zbuffer will have a bufname mode: 0 - <ch>-<bufname> [default/legacy], 1 - <bufname>-<ch>
void *zbuffer_init(t_class *owner, t_symbol *bufname, int numchans, int singlemode, int bufnamemode); 
void zbuffer_free(t_zbuffer *c);
void zbuffer_checkdsp(t_zbuffer *c);
void zbuffer_getchannel(t_zbuffer *c, int chan_num, int complain);

#endif