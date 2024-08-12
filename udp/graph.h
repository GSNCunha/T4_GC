#ifndef GRAPH_H
#define GRAPH_H

#include <SDL/SDL.h>

typedef Uint32 PixelType;

typedef struct canvas {
    SDL_Surface *canvas;
    int Height;
    int Width;
    int Xoffset;
    int Yoffset;
    int Xext;
    int Yext;
    double Xmax;
    double Ymax;
    double Xstep;
    PixelType *zpixel;
} Tcanvas;

inline void c_pixeldraw(Tcanvas *canvas, int x, int y, PixelType color) {
    *( ((PixelType*)canvas->canvas->pixels) + ((-y+canvas->Yoffset) * canvas->canvas->w + x+ canvas->Xoffset)) = color;
}

inline void c_hlinedraw(Tcanvas *canvas, int xstep, int y, PixelType color) {
    int offset =  (-y+canvas->Yoffset) * canvas->canvas->w;
    int x;

    for (x = 0; x< canvas->Width+canvas->Xoffset ; x+=xstep) {
        *( ((PixelType*)canvas->canvas->pixels) + (offset + x)) = color;
    }
}

inline void c_vlinedraw(Tcanvas *canvas, int x, int ystep, PixelType color) {
    int offset = x+canvas->Xoffset;
    int y;
    int Ystep = ystep*canvas->canvas->w;

    for (y = 0; y< canvas->Height+canvas->Yext ; y+=ystep) {
        *( ((PixelType*)canvas->canvas->pixels) + (offset + y*canvas->canvas->w)) = color;
    }
}

inline void c_linedraw(Tcanvas *canvas, double x0, double y0, double x1, double y1, PixelType color) {
    double x;

    for (x=x0; x<=x1; x+=canvas->Xstep) {
        c_pixeldraw(canvas, (int)(x*canvas->Width/canvas->Xmax+0.5), (int)((double)canvas->Height/canvas->Ymax*(y1*(x1-x)+y1*(x-x0))/(x1-x0)+0.5),color);
    }
}

#endif
