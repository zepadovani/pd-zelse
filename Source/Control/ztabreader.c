// porres

#include <m_pd.h>
#include <zbuffer.h>

typedef struct _ztabreader{
    t_object  x_obj;
    t_zbuffer *x_zbuffer;
    int       x_i_mode;
    int       x_ch;
    int       x_idx;
    int       x_loop;
    t_float   x_bias;
    t_float   x_tension;
    t_float   x_value;
    // new var to store namemode: 0 = <ch>-<arrayname>, 1 = <arrayname>-<ch>
    int       x_namemode;         // 0 is <ch>-<arrayname>, 1 is <arrayname>-<ch>
    t_outlet *x_outlet;
}t_ztabreader;

static t_class *ztabreader_class;

static void ztabreader_set_nointerp(t_ztabreader *x){
    x->x_i_mode = 0;
}

static void ztabreader_set_linear(t_ztabreader *x){
    x->x_i_mode = 1;
}

static void ztabreader_set_cos(t_ztabreader *x){
    x->x_i_mode = 2;
}

static void ztabreader_set_lagrange(t_ztabreader *x){
    x->x_i_mode = 3;
}

static void ztabreader_set_cubic(t_ztabreader *x){
    x->x_i_mode = 4;
}

static void ztabreader_set_spline(t_ztabreader *x){
    x->x_i_mode = 5;
}

static void ztabreader_set_hermite(t_ztabreader *x, t_floatarg tension, t_floatarg bias){
    x->x_tension = 0.5 * (1. - tension);
    x->x_bias = bias;
    x->x_i_mode = 6;
}

static void ztabreader_set(t_ztabreader *x, t_symbol *s){
    zbuffer_setarray(x->x_zbuffer, s);
}

static void ztabreader_float(t_ztabreader *x, t_float f){
    t_zbuffer *buf = x->x_zbuffer;
    zbuffer_validate(buf, 1); // 2nd arg for error posting
    t_word *vp = buf->c_vectors[0];
    int npts = x->x_loop ? buf->c_npts : buf->c_npts - 1;
    if(vp){
        double index = (double)(f);
        double xpos = x->x_idx ? index : index*npts;
        if(xpos < 0)
            xpos = 0;
        if(xpos >= npts)
            xpos = x->x_loop ? 0 : npts;
        int ndx = (int)xpos;
        double frac = xpos - ndx;
        if(ndx == npts && x->x_loop)
            ndx = 0;
        int ndx1 = ndx + 1;
        if(ndx1 == npts)
            ndx1 = x->x_loop ? 0 : npts;
        int ndxm1 = 0, ndx2 = 0;
        if(x->x_i_mode){
            ndxm1 = ndx - 1;
            if(ndxm1 < 0)
                ndxm1 = x->x_loop ? npts - 1 : 0;
            ndx2 = ndx1 + 1;
            if(ndx2 >= npts)
                ndx2 = x->x_loop ? ndx2 - npts : npts;
        }
        double a = 0, b = 0, c = 0, d = 0;
        b = (double)vp[ndx].w_float;
        if(x->x_i_mode){
            c = (double)vp[ndx1].w_float;
            if(x->x_i_mode > 2){
                a = (double)vp[ndxm1].w_float;
                d = (double)vp[ndx2].w_float;
            }
        }
        float out = b; // no interpolation
        switch(x->x_i_mode){
            case 1: // linear
                out = interp_lin(frac, b, c);
                break;
            case 2: // cos
                out = interp_cos(frac, b, c);
                break;
            case 3: // lagrange
                out = interp_lagrange(frac, a, b, c, d);
                break;
            case 4: // cubic
                out = interp_cubic(frac, a, b, c, d);
                break;
            case 5: // spline
                out = interp_spline(frac, a, b, c, d);
                break;
            case 6: // hermite
                out = interp_hermite(frac, a, b, c, d, x->x_bias, x->x_tension);
                break;
            default:
                break;
        }
        outlet_float(x->x_outlet, out);
    }
}

static void ztabreader_channel(t_ztabreader *x, t_floatarg f){
    x->x_ch = f < 1 ? 1 : (f > 64 ? 64 : (int) f);
    zbuffer_getchannel(x->x_zbuffer, x->x_ch, 1);
}

static void ztabreader_index(t_ztabreader *x, t_floatarg f){
    x->x_idx = (int)(f != 0);
}

static void ztabreader_loop(t_ztabreader *x, t_floatarg f){
    x->x_loop = (int)(f != 0);
}

static void ztabreader_free(t_ztabreader *x){
    outlet_free(x->x_outlet);
    zbuffer_free(x->x_zbuffer);
}

static void *ztabreader_new(t_symbol *s, int ac, t_atom * av){
    t_ztabreader *x = (t_ztabreader *)pd_new(ztabreader_class);
    t_symbol *name = s = NULL;
    int nameset = 0;
    x->x_idx = x->x_loop = 0;
    x->x_bias = x->x_tension = 0;
    x->x_i_mode = 5; // spline
    int ch = 1;
    while(ac){
        if(av->a_type == A_SYMBOL){ // symbol
            t_symbol *curarg = atom_getsymbol(av);
            // added flag to set namemode: 0 = <ch>-<arrayname>, 1 = <arrayname>-<ch>
            if(curarg == gensym("-chafter")){
                if(nameset)
                    goto errstate;
            x->x_namemode = 1; ac--, av++;
            }             
            else if(curarg == gensym("-none")){
                if(nameset)
                    goto errstate;
                ztabreader_set_nointerp(x), ac--, av++;
            }
            else if(curarg == gensym("-lin")){
                if(nameset)
                    goto errstate;
                ztabreader_set_linear(x), ac--, av++;
            }
            else if(curarg == gensym("-cos")){
                if(nameset)
                    goto errstate;
                ztabreader_set_cos(x), ac--, av++;
            }
            else if(curarg == gensym("-cubic")){
                if(nameset)
                    goto errstate;
                ztabreader_set_cubic(x), ac--, av++;
            }
            else if(curarg == gensym("-lagrange")){
                if(nameset)
                    goto errstate;
                ztabreader_set_lagrange(x), ac--, av++;
            }
            else if(curarg == gensym("-hermite")){
                if(nameset)
                    goto errstate;
                if(ac >= 3){
                    x->x_i_mode = 5, ac--, av++;
                    float bias = atom_getfloat(av);
                    ac--, av++;
                    float tension = atom_getfloat(av);
                    ac--, av++;
                    ztabreader_set_hermite(x, bias, tension);
                }
                else
                    goto errstate;
            }
            else if(curarg == gensym("-ch")){
                if(nameset)
                    goto errstate;
                if(ac >= 2){
                    ac--, av++;
                    ch = atom_getfloat(av);
                    ac--, av++;
                }
                else
                    goto errstate;
            }
            else if(curarg == gensym("-index")){
                if(nameset)
                    goto errstate;
                x->x_idx = 1, ac--, av++;
            }
            else if(curarg == gensym("-loop")){
                if(nameset)
                    goto errstate;
                x->x_loop = 1, ac--, av++;
            }
            else{
                if(nameset)
                    goto errstate;
                name = atom_getsymbol(av);
                nameset = 1, ac--, av++;
            }
        }
        else{
            if(!nameset)
                goto errstate;
            ch = (int)atom_getfloat(av), ac--, av++;
        }
    };
    x->x_ch = (ch < 0 ? 1 : ch > 64 ? 64 : ch);
    // init zbuffer according to namemode: 0 = <ch>-<arrayname>, 1 = <arrayname>-<ch>
    if(x->x_namemode == 0) {
        x->x_zbuffer = zbuffer_init((t_class *)x, name, 1, x->x_ch, 0); /// new arg added 0: <ch>-<arrayname> mode / 1: <arrayname>-<ch> mode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }
    else if(x->x_namemode == 1){
        x->x_zbuffer = zbuffer_init((t_class *)x, name, 1, x->x_ch, 1); 
    }      
    zbuffer_getchannel(x->x_zbuffer, x->x_ch, 1);
    zbuffer_setminsize(x->x_zbuffer, 2);
    zbuffer_playcheck(x->x_zbuffer);
    x->x_outlet = outlet_new(&x->x_obj, &s_float);
    return(x);
    errstate:
        post("ztabreader: improper args");
        return(NULL);
}

void ztabreader_setup(void){
    ztabreader_class = class_new(gensym("ztabreader"), (t_newmethod)ztabreader_new,
        (t_method)ztabreader_free, sizeof(t_ztabreader), 0, A_GIMME, 0);
    class_addfloat(ztabreader_class, ztabreader_float);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set, gensym("set"), A_SYMBOL, 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_channel, gensym("channel"), A_FLOAT, 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_index, gensym("index"), A_FLOAT, 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_loop, gensym("loop"), A_FLOAT, 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set_nointerp, gensym("none"), 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set_linear, gensym("lin"), 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set_lagrange, gensym("lagrange"), 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set_cubic, gensym("cubic"), 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set_spline, gensym("spline"), 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set_cos, gensym("cos"), 0);
    class_addmethod(ztabreader_class, (t_method)ztabreader_set_hermite, gensym("hermite"),
        A_FLOAT, A_FLOAT, 0);
}
