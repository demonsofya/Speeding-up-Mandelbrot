#include "TXLib.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ammintrin.h>
#include <intrin.h>

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

volatile int N_array[8] = {};
volatile int N_normal_array[8] = {};
volatile int N = 0;

__m256 _01234567_vector = _mm256_set_ps(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
__m256 R_SQUARE_MAX_ARRAY = _mm256_set1_ps(R_SQUARE_MAX);

float _01234567_array[8] = {0, 1, 2, 3, 4, 5, 6, 7};

const int MEASURES_COUNT = 100;

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

inline void set_256_only_array(float a[8], float b) {
    for (int i = 0; i < 8; i++)
        a[i] = b;
}

inline void add_int_256_arrays(volatile int dest[8], volatile int a[8], volatile int b[8]) {
    for (int i = 0; i < 8; i++)
        dest[i] = a[i] + b[i];
}
inline void set_256_only_int_array(volatile int a[8], int b) {
    for (int i = 0; i < 8; i++)
        a[i] = b;
}

int DrawMandelbrot_intrinsics(RGBQUAD* window_buffer);
int DrawMandelbrot_funcs(RGBQUAD* window_buffer);
int DrawMandelbrot_no_optimisation(RGBQUAD* window_buffer);

HWND CreateMyWindow();

int main() {

    Win32::_fpreset();

    HWND Window = CreateMyWindow();
    RGBQUAD* window_buffer = txVideoMemory();

    txBegin();
    unsigned long long ticks_count_before_call_no_optimisation = __rdtsc();
    for (int i = 0; i < MEASURES_COUNT; i++) {
        DrawMandelbrot_no_optimisation(window_buffer);
    }
    unsigned long long ticks_no_optimisation = (__rdtsc() - ticks_count_before_call_no_optimisation) / MEASURES_COUNT;
    printf("without optimisation - %llu\n", ticks_no_optimisation);
    txEnd();

    txBegin();
    unsigned long long ticks_count_before_call_on_funcs = __rdtsc();
    for (int i = 0; i < MEASURES_COUNT; i++) {
        DrawMandelbrot_funcs(window_buffer);
    }
    unsigned long long ticks_on_funcs = (__rdtsc() - ticks_count_before_call_on_funcs) / MEASURES_COUNT;
    printf("on funcs - with 8-length arrays - %llu\n", ticks_on_funcs);
    txEnd();

    txBegin();
    unsigned long long ticks_count_before_call_intrinsics = __rdtsc();
    for (int i = 0; i < MEASURES_COUNT; i++) {
        DrawMandelbrot_intrinsics(window_buffer);
    }
    unsigned long long ticks_intrinsics = (__rdtsc() - ticks_count_before_call_intrinsics) / MEASURES_COUNT;
    printf("with intrinsics - %llu\n", ticks_intrinsics);
    txEnd();

    return 0;   
}

HWND CreateMyWindow() {

    HWND NewWindow = txCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, true);

    txSetColor(TX_WHITE);
    txSetFillColor(TX_BLACK);
    txSelectFont("Comic Sans MS", TEXT_SIZE);

    return NewWindow;
}

int DrawMandelbrot_intrinsics(RGBQUAD* window_buffer) {
    
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
            for (int i = 0; i < 8; i++) N_normal_array[i] = 0;

            for (int n = 0; n < N_MAX; n++) {
                __m256 x_square_array = _mm256_mul_ps(X_array, X_array);
                __m256 y_square_array = _mm256_mul_ps(Y_array, Y_array);

                __m256 xy_mul_array = _mm256_mul_ps(X_array, Y_array);

                __m256 radius_square_array = _mm256_add_ps(x_square_array, y_square_array);
                __m256 cmp =  _mm256_cmp_ps(radius_square_array, R_SQUARE_MAX_ARRAY, _CMP_LT_OQ);

                int mask = _mm256_movemask_ps(cmp);
                if (!mask)
                    break;

       //TODO: зависимость по данным             
                X_array = _mm256_sub_ps(x_square_array, y_square_array);
                Y_array = _mm256_add_ps(Y_array, y0_array);
                    
                N_array = _mm256_sub_epi32(N_array, _mm256_castps_si256(cmp)); 
                
                X_array = _mm256_add_ps(X_array, x0_array);
                Y_array = _mm256_add_ps(xy_mul_array, xy_mul_array);
            }

            _mm256_storeu_si256((__m256i*)N_normal_array, N_array);
            
            for (int i = 0; i < 8; i++) {
                int curr_N = N_normal_array[i];
                RGBQUAD new_color = {(BYTE) curr_N, (BYTE) curr_N, (BYTE) curr_N};
                window_buffer[x_cord + i + y_cord * WINDOW_WIDTH] = new_color;
            }

        }
    }

    return 0;
}
// TODO: N_array - глобальная переменная
int DrawMandelbrot_funcs(RGBQUAD* window_buffer) {
    
    assert(window_buffer);

    for (int y_cord = 0; y_cord < WINDOW_HEIGHT; y_cord++) {
        float y0 = y_offset + (y_cord - WINDOW_HEIGHT / 2) * x_step;
        float x0 = x_offset + (       - WINDOW_WIDTH  / 2) * y_step;

        float y0_array[8] = {y0, y0, y0, y0, y0, y0, y0, y0};

        for (int x_cord = 0; x_cord < WINDOW_WIDTH; x_cord += 8, x0 += x_step * 8) {
            float x0_array[8] = {};
            set_256_only_array(x0_array, x0);

            float x_step_array[8] = {};
            set_256_only_array(x_step_array, x_step);
            mul_256_arrays(x_step_array, x_step_array, _01234567_array);

            add_256_arrays(x0_array, x0_array, x_step_array);

            float X_array[8] = {};
            set_256_array(X_array, x0_array);

            float Y_array[8] = {};
            set_256_array(Y_array, y0_array);

            for (int i= 0; i < 8; i++)  N_array[i] = 0;
            //set_256_only_int_array(N_array, 0);

            for (int n = 0; n < N_MAX; n++) {
                float x_square_array[8] = {};
                mul_256_arrays(x_square_array, X_array, X_array);

                float y_square_array[8] = {};
                mul_256_arrays(y_square_array, Y_array, Y_array);

                float xy_mul_array[8] = {};
                mul_256_arrays(xy_mul_array, X_array, Y_array);

                float radius_square_array[8] = {};
                add_256_arrays(radius_square_array, x_square_array, y_square_array);

                volatile int cmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                for (int i = 0; i < 8; i++) {
                    if (radius_square_array[i] < R_SQUARE_MAX)
                        cmp[i] = 1;
                }

                int mask = 0;
                for (int i = 0; i < 8; i++)
                    mask = mask | (cmp[i] << i);
                if (!mask)
                    break;

                add_int_256_arrays(N_array, N_array, cmp); 

                sub_256_arrays(X_array, x_square_array, y_square_array);
                add_256_arrays(X_array, X_array, x0_array);
                
                add_256_arrays(Y_array, xy_mul_array, xy_mul_array);
                add_256_arrays(Y_array, Y_array, y0_array);
            }

            for (int i = 0; i < 8; i++) {
                RGBQUAD new_color = {(BYTE) N_array[i], (BYTE) N_array[i], (BYTE) N_array[i]};
                window_buffer[x_cord + i + y_cord * WINDOW_WIDTH] = new_color;
            }
        }
    }

    return 0;
}

int DrawMandelbrot_no_optimisation(RGBQUAD* window_buffer) {

    assert(window_buffer);

    for (int y_cord = 0; y_cord < WINDOW_HEIGHT; y_cord++) {
        float y0 = y_offset + (y_cord - WINDOW_HEIGHT / 2) * x_step;
        float x0 = x_offset + (       - WINDOW_WIDTH  / 2) * y_step;

        for (int x_cord = 0; x_cord < WINDOW_WIDTH; x_cord++, x0 += x_step) {
            float X = x0, Y = y0;

            N = 0;
            for (; N < N_MAX; (N)++) {
                float x_square = X * X, y_square = Y * Y, x_y_mul = X * Y;
                float radius_square = x_square + y_square;

                if (radius_square >= R_SQUARE_MAX)
                    break;

                X = x_square - y_square + x0;
                Y = x_y_mul + x_y_mul + y0;
            }

            RGBQUAD new_color = {(BYTE) (N), (BYTE) (N), (BYTE) (N)};
            window_buffer[x_cord + y_cord * WINDOW_WIDTH] = new_color;
        }
    }

    return 0;
}