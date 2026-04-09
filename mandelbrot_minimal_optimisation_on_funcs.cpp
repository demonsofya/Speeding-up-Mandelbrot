#include "TXLib.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

const int N_MAX             = 255;
const double R_SQUARE_MAX   = 10;

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;
const int TEXT_SIZE     = 12;

double scale = 0.45;

int x_offset    = -400;
int y_offset    = 0;
double x_step   = 4.0 / 800.0;
double y_step   = 3.0 / 600.0;

const double default_x_step             =  4.0 / 800.0;
const double default_y_step             =  3.0 / 600.0;
const double default_additional_offset  = 0.005;
const double default_scale_addition     = 0.01;

double _01234567_vector[8] = {0, 1, 2, 3, 4, 5, 6, 7};

int DrawMandelbrot(RGBQUAD* window_buffer);
HWND CreateMyWindow();

int main() {

    Win32::_fpreset();

    HWND Window = CreateMyWindow();
    RGBQUAD* window_buffer = txVideoMemory();

    while (!GetAsyncKeyState(VK_ESCAPE)) {
        txBegin();
        int additional_offset = (int) (default_additional_offset * scale);

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

        DrawMandelbrot(window_buffer);
        txEnd();
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

inline void mul_256_arrays(double dest[8], double a[8], double b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] * b[i];
}

inline void add_256_arrays(double dest[8], double a[8], double b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] + b[i];
}

inline void sub_256_arrays(double dest[8], double a[8], double b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] - b[i];
}

inline void set_256_array(double a[8], double b[8]) {
    for (int i = 0; i < 8; i++)
        a[i] = b[i];
}

inline void set_256_only_array(double a[8], double b) {
    for (int i = 0; i < 8; i++)
        a[i] = b;
}

inline void add_int_256_arrays(int dest[8], int a[8], int b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] + b[i];
}
inline void set_256_only_int_array(int a[8], int b) {
    for (int i = 0; i < 8; i++)
        a[i] = b;
}

int DrawMandelbrot(RGBQUAD* window_buffer) {
    
    assert(window_buffer);

    for (int y_cord = 0; y_cord < WINDOW_HEIGHT; y_cord++) {
        double y0 = (y_cord - WINDOW_HEIGHT / 2 + y_offset) * y_step;
        double x0 = (       - WINDOW_WIDTH  / 2 + x_offset) * x_step;
        //set_256_only_array(x0_array, x0);

        //double x_step_array[8] = {};
        //set_256_only_array(x_step_array, x_step);
        //mul_256_arrays(x_step_array, x_step_array, _01234567_vector);

        //add_256_arrays(x0_array, x0_array, x_step_array);

        double y0_array[8] = {y0, y0, y0, y0, y0, y0, y0, y0};
        //set_256_only_array(y0_array, y0);

        for (int x_cord = 0; x_cord < WINDOW_WIDTH; x_cord += 8, x0 += x_step * 8) {
            double x0_array[8] = {};
            set_256_only_array(x0_array, x0);

            double x_step_array[8] = {};
            set_256_only_array(x_step_array, x_step);
            mul_256_arrays(x_step_array, x_step_array, _01234567_vector);

            add_256_arrays(x0_array, x0_array, x_step_array);

            double X_array[8] = {};
            //for (int i = 0; i < 8; i++) X_array[i] = x0_array[i];
            set_256_array(X_array, x0_array);

            double Y_array[8] = {};
            //for (int i = 0; i < 8; i++) Y_array[i] = y0_array[i];
            set_256_array(Y_array, y0_array);

            //int cmp[8] = {};
            //set_256_only_int_array(cmp, 1);

            int N_array[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            set_256_only_int_array(N_array, 0);

            for (int n = 0; n < N_MAX; n++) {
                double x_square_array[8] = {};
                //for (int i = 0; i < 8; i++) x_square_array[i] = X_array[i] * X_array[i];
                mul_256_arrays(x_square_array, X_array, X_array);

                double y_square_array[8] = {};
                //for (int i = 0; i < 8; i++) y_square_array[i] = Y_array[i] * Y_array[i];
                mul_256_arrays(y_square_array, Y_array, Y_array);

                double xy_mul_array[8] = {};
                //for (int i = 0; i < 8; i++) xy_mul_array[i] = X_array[i] * Y_array[i];
                mul_256_arrays(xy_mul_array, X_array, Y_array);

                double radius_square_array[8] = {};
                //for (int i = 0; i < 8; i++) radius_square_array[i] = x_square_array[i] + y_square_array[i];
                add_256_arrays(radius_square_array, x_square_array, y_square_array);

                int cmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                for (int i = 0; i < 8; i++) {
                    if (radius_square_array[i] < R_SQUARE_MAX)
                        cmp[i] = 1;
                }

                int mask = 0;
                for (int i = 0; i < 8; i++)
                    mask = mask | (cmp[i] << i);
                if (!mask)
                    break;

                //for (int i = 0; i < 8; i++) N_array[i] = N_array[i] + cmp[i];
                add_int_256_arrays(N_array, N_array, cmp); 

                //for (int i = 0; i < 8; i++) X_array[i] = x_square_array[i] - y_square_array[i] + x0_array[i];
                sub_256_arrays(X_array, x_square_array, y_square_array);
                add_256_arrays(X_array, X_array, x0_array);
                //for (int i = 0; i < 8; i++) Y_array[i] = xy_mul_array[i] + xy_mul_array[i] + y0_array[i];
                add_256_arrays(Y_array, xy_mul_array, xy_mul_array);
                add_256_arrays(Y_array, Y_array, y0_array);
            }

            for (int i = 0; i < 8; i++) {
                RGBQUAD new_color = {(BYTE) (N_array[i]) % 255 * 123, (BYTE) (N_array[i]) % 255 * 123, (BYTE) N_array[i]*N_array[i]};
                window_buffer[x_cord + i + y_cord * WINDOW_WIDTH] = new_color;
            }
        }
    }

    printf ("FPS %.0lf\t\t\r", txGetFPS());
    return 0;
}