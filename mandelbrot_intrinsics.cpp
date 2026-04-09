#include "TXLib.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ammintrin.h>

const int N_MAX             = 255;
const float R_SQUARE_MAX   = 10;

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;
const int TEXT_SIZE     = 12;

const float DEFAULT_SCALE = 0.45;
float current_scale = 0.45;

float x_offset    = 0;
float y_offset    = 0;
float x_step    = 0.005;
float y_step    = 0.005;

const float default_x_step             =  0.005;
const float default_y_step             =  0.005;
const float default_additional_offset  = 0.005;
const float default_scale_addition     = 0.001;

__m256 _01234567_vector = _mm256_set_ps(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
__m256 R_SQUARE_MAX_ARRAY = _mm256_set1_ps(R_SQUARE_MAX);

int DrawMandelbrot(RGBQUAD* window_buffer);
HWND CreateMyWindow();

int main() {

    Win32::_fpreset();

    HWND Window = CreateMyWindow();
    RGBQUAD* window_buffer = txVideoMemory();


    while (!GetAsyncKeyState(VK_ESCAPE)) {
        txBegin();

        if (GetAsyncKeyState(VK_RIGHT))
            x_offset += default_additional_offset;
        if (GetAsyncKeyState(VK_LEFT))
            x_offset -= default_additional_offset;
        if (GetAsyncKeyState(VK_UP))
            y_offset += default_additional_offset;  // домножить на scale offset
        if (GetAsyncKeyState(VK_DOWN))
            y_offset -= default_additional_offset;

        if (GetAsyncKeyState('E') && current_scale > 0.001)
            current_scale -= default_scale_addition;
        if (GetAsyncKeyState('Q'))
            current_scale += default_scale_addition;


        x_step = default_x_step * current_scale;
        y_step = default_y_step * current_scale;

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

inline void mul_256_arrays(float dest[8], float a[8], float b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] * b[i];
}

inline void add_256_arrays(float dest[8], float a[8], float b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] + b[i];
}

inline void sub_256_arrays(float dest[8], float a[8], float b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] - b[i];
}

inline void set_256_array(float a[8], float b[8]) {
    for (int i = 0; i < 8; i++)
        a[i] = b[i];
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
        float y0 = y_offset + (y_cord - WINDOW_HEIGHT / 2) * x_step;
        float x0 = x_offset + (       - WINDOW_WIDTH  / 2) * y_step;

        __m256 y0_array = _mm256_set1_ps(y0);

        for (int x_cord = 0; x_cord < WINDOW_WIDTH; x_cord += 8, x0 += x_step * 8) {
            __m256 x0_array = _mm256_set1_ps(x0);
            __m256 x_step_array = _mm256_set1_ps(x_step);

            x0_array = _mm256_add_ps(x0_array, _mm256_mul_ps(x_step_array, _01234567_vector));

            __m256 X_array = x0_array;
            __m256 Y_array = y0_array;

            __m256i N_array = _mm256_setzero_si256();

            for (int n = 0; n < N_MAX; n++) {
                __m256 x_square_array = _mm256_mul_ps(X_array, X_array);
                __m256 y_square_array = _mm256_mul_ps(Y_array, Y_array);

                __m256 xy_mul_array = _mm256_mul_ps(X_array, Y_array);

                __m256 radius_square_array = _mm256_add_ps(x_square_array, y_square_array);
                __m256 cmp =  _mm256_cmp_ps(radius_square_array, R_SQUARE_MAX_ARRAY, _CMP_LT_OQ);

                int mask = _mm256_movemask_ps(cmp);
                if (!mask)
                    break;

                N_array = _mm256_sub_epi32(N_array, _mm256_castps_si256(cmp)); 

                X_array = _mm256_sub_ps(x_square_array, y_square_array);
                X_array = _mm256_add_ps(X_array, x0_array);

                Y_array = _mm256_add_ps(xy_mul_array, xy_mul_array);
                Y_array = _mm256_add_ps(Y_array, y0_array);
            }

            int N_normal_array[8] = {};
            _mm256_storeu_si256((__m256i*)N_normal_array, N_array);
            
            for (int i = 0; i < 8; i++) {
                int curr_N = N_normal_array[i];
                RGBQUAD new_color = {(BYTE) (curr_N) % 255 * 123, (BYTE) (curr_N) % 255 * 123, (BYTE) curr_N*curr_N};
                window_buffer[x_cord + i + y_cord * WINDOW_WIDTH] = new_color;
            }

        }
    }

    printf ("FPS %.0lf\t\t\r", txGetFPS());
    return 0;
}