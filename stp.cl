
kernel void basicSTP(int N, int k, global float *stp) {
  int i = get_global_id(0);  
  float ik = stp[i+k*N];
  for(int j=0;j<N;j++) {
    float ij = stp[i+j*N];
    float kj = stp[k+j*N];
    if(ik + kj < ij) stp[i+j*N]=ik+kj;
  }
}
