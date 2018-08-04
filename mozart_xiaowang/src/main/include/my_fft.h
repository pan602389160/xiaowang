
#ifndef MY_FFT_H_
#define MY_FFT_H_
#define 	DECORDE_SIZE				1024
#define 	DECORDE_FREQUENCY_START 	10
#define 	DECORDE_FREQUENCY_END		600

#define 	FREQUENCY_OFFSET			15.5335

typedef struct{
double real;
double img;
}complex;
void fft(void); 
void change(void);
void add(complex ,complex ,complex *); 
void mul(complex ,complex ,complex *); 
void sub(complex ,complex ,complex *);
int output(void);
#endif
