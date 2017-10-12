void bar(int *a){
    *a+=1;
}

void foo(float **in, float **out, float gain, int nsamps)
{
	int i,j;

	for (i = 0; i < nsamps; i++) {
        for(j = 0; j < nsamps; j++){
		    out[i][j] = in[i][j] * gain;
		    out[i][j] = in[i][j] * gain;
        }
	}

    bar(&i);
}

int main(){
    float **in, **out;
    float gain;
    int nsamps;
    foo(in, out, gain, nsamps);

    return 0;
}
