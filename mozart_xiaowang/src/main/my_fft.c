#include"math.h"
#include"my_fft.h"
#include "my_fft.h"
#include"my_cossin.h"

extern char out[256];

complex x[DECORDE_SIZE]; 


void clear_out(void)
{
	int i=0;
	for(i=0;i<sizeof(out);i++)
			out[i]=0;
}
int changefrequencytonum(int ft){
	char f;
	if(ft<2900)f=20;
	if(ft>6700)f=20;
	if(ft>2900)if(ft<3100)f=0;
	if(ft>3100)if(ft<3300)f=1;
	if(ft>3300)if(ft<3500)f=2;
	if(ft>3500)if(ft<3700)f=3;
	if(ft>3700)if(ft<3900)f=4;
	if(ft>3900)if(ft<4100)f=5;
	if(ft>4100)if(ft<4300)f=6;
	if(ft>4300)if(ft<4500)f=7;
	if(ft>4500)if(ft<4700)f=8;
	if(ft>4700)if(ft<4900)f=9;
	if(ft>4900)if(ft<5100)f=10;
	if(ft>5100)if(ft<5300)f=11;
	if(ft>5300)if(ft<5500)f=12;
	if(ft>5500)if(ft<5700)f=13;
	if(ft>5700)if(ft<5900)f=14;
	if(ft>5900)if(ft<6100)f=15;
	if(ft>6100)if(ft<6300)f=16;
	if(ft>6300)if(ft<6500)f=17;
	if(ft>6500)if(ft<6700)f=18;
	return f;
}

int decode_num( char *data,int len,char *out)
{
	int i;
	int status=0;
	int start=0;
	int end=0;
	char seiral[256];
	int k=0,km=0;
	int change=0;
	for(i=0;i<len;i++){
		switch (status)
		{
			case 0:
				if(data[i]==16){
					status=1;
					start++;
				}
				break;
			case 1:
				if(data[i]==16){
					start++;
				}else{
					if(start<3){
						start=0;
						status=0;
						break;
					}else{
						change=1;
						seiral[k++]=data[i];
						status=2;
						break;
					}
				}
				break;
			case 2:
				if((data[i]<16)&&(data[i]>=0)){
					if(change==0){
						change=1;
						seiral[k++]=data[i];
					}else if(change==1){
						if(seiral[k-1]!=data[i]){
							change=2;
							seiral[k++]=data[i];
						}
					}
				}else if(data[i]==18){
					if(change==1){
						seiral[k]=seiral[k-1];
						k++;
					}
					change=0;
				}else if(data[i]==17){
					end++;
					status=3;
				}
				break;
			case 3:
				if(data[i]==17){
					end++;
				}
				break;
		}	
	}
	km=0;
	for(i=0;i<k;i+=2){
		out[km++]=(char)((seiral[i]*16+seiral[i+1])&0xFF);
	}
	return km;
}

void fft(void)
{
	int i=0,j=0,k=0,l=0;
	complex up,down,product;
	change();
	for(i=0;i<10;i++)
	{   
		l=1<<i;
		for(j=0;j<DECORDE_SIZE;j+= 2*l )
		{            
			for(k=0;k<l;k++)
			{       
				mul(x[j+k+l],W[512*k/l],&product);
				add(x[j+k],product,&up);
				sub(x[j+k],product,&down);
				x[j+k]=up;
				x[j+k+l]=down;
			}
		}
	}
}

void change(void)      
{
  complex temp;
  unsigned short i=0,j=0,k=0;
  double t;
  for(i=0;i<DECORDE_SIZE;i++)
  {
    k=i;j=0;
    t=10;
  while( (t--)>0 )
  {
    j=j<<1;
	j|=(k & 1);
	k=k>>1;
  }
  if(j>i)
  {
	temp=x[i];
	x[i]=x[j];
	x[j]=temp;
  }
  }
}
int output(void)
{
	int i;
	float Max=0;
	int MaxNum=0;
	float t=0,t2=0;	
	for(i=DECORDE_FREQUENCY_START;i<DECORDE_FREQUENCY_END;i++)
	{
		t=x[i].real;
		t/=100;
		t*=t;
		t2=x[i].img/100;
		t2*=t2;
		t+=t2;
		if(t>Max){
			Max=t;
			MaxNum=i;
		}
	}
	Max=FREQUENCY_OFFSET;
	Max*=MaxNum;
	return (int)Max;
}

void add(complex a,complex b,complex *c)
{
	c->real=a.real+b.real;
	c->img=a.img+b.img;
}


void mul(complex a,complex b,complex *c)
{
	c->real=a.real*b.real - a.img*b.img;
	c->img=a.real*b.img + a.img*b.real;
}


void sub(complex a,complex b,complex *c)
{
	c->real=a.real-b.real;
	c->img=a.img-b.img;
}

