#include "TXLib.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

const int N_MAX     = 255;
const double R_SQUARE_MAX  = 10;

const int TEXT_SIZE = 12;

const double default_x_step             =  4.0 / 800.0;
const double default_y_step             =  3.0 / 600.0;
const double default_additional_offset  = 5;
const double default_scale_addition     = 0.01;

double x_step = 4.0 / 800.0;
double y_step = 3.0 / 600.0;

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;

int x_offset = -400;
int y_offset = 0;

double scale = 0.45;

int DrawMandelbrot(HWND Window);
HWND CreateMyWindow();

int main() {
    HWND Window = CreateMyWindow();

    while (!GetAsyncKeyState(VK_ESCAPE)) {
        double additional_offset = default_additional_offset * scale;

        if (GetAsyncKeyState(VK_RIGHT))
            x_offset += additional_offset;
        if (GetAsyncKeyState(VK_LEFT))
            x_offset -= additional_offset;
        if (GetAsyncKeyState(VK_UP))
            y_offset += additional_offset;
        if (GetAsyncKeyState(VK_DOWN))
            y_offset -= additional_offset;

        if (GetAsyncKeyState('E') && scale > 0.1)
            scale -= default_scale_addition;
        if (GetAsyncKeyState('Q'))
            scale += default_scale_addition;

        x_step = default_x_step * scale;
        y_step = default_y_step * scale;
        DrawMandelbrot(Window);
    }

    return 0;
}

HWND CreateMyWindow() {

    HWND NewWindow = txCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, true);

    txSetColor(TX_WHITE);
    txSetFillColor(TX_BLACK);
    txSelectFont("Comic Sans MS", TEXT_SIZE);

    return NewWindow;
}

int DrawMandelbrot(HWND Window) {
    RGBQUAD* window_buffer = txVideoMemory();
    txBegin();

    for (int y_cord = 0; y_cord < WINDOW_HEIGHT; y_cord++) {
        double y0 = (y_cord - WINDOW_HEIGHT / 2 + y_offset) * y_step;
        double x0 = (       - WINDOW_WIDTH  / 2 + x_offset) * x_step;

        for (int x_cord = 0; x_cord < WINDOW_WIDTH; x_cord++, x0 += x_step) {
            double X = x0, Y = y0;

            int N = 0;
            for (; N < N_MAX; N++) {
                double x_square = X * X, y_square = Y * Y, x_y_mul = X * Y;
                double radius_square = x_square + y_square;

                if (radius_square >= R_SQUARE_MAX)
                    break;

                X = x_square - y_square + x0;
                Y = x_y_mul + x_y_mul + y0;
            }

            RGBQUAD new_color = {(BYTE) (N) % 255 * 123, (BYTE) (N) % 255 * 123, (BYTE) N*N};
            window_buffer[x_cord + y_cord * WINDOW_WIDTH] = new_color;
        }
    }

    txEnd();
    printf ("FPS %.0lf\t\t\r", txGetFPS());
    return 0;
}