model SBGraph3
  Real N=1000;
  Real M=1000;
  Real S_n_eff;
  Real S_n_fl;
  Real S_p_eff;
  Real S_p_fl;
  Real G_n_eff;
  Real G_n_fl;
  Real G_p_eff;
  Real G_p_fl;
  Real C_u_eff[N, M];
  Real C_u_fl[N, M];
  Real C_d_eff[N, M];
  Real C_d_fl[N, M];
  Real C_l_eff[N, M];
  Real C_l_fl[N, M];
  Real C_r_eff[N, M];
  Real C_r_fl[N, M];
equation
  for i in 1:1:999,j in 1:1:1000 loop
    C_d_eff[i,j] = C_u_eff[i+1,j];
  end for;
  for i in 1:1:999,j in 1:1:1000 loop
    C_d_fl[i,j]+C_u_fl[i+1,j] = 0;
  end for;
  for i in 1:1:1000,j in 1:1:999 loop
    C_l_eff[i,j+1] = C_r_eff[i,j];
  end for;
  for i in 1:1:1000,j in 1:1:999 loop
    C_l_fl[i,j+1]+C_r_fl[i,j] = 0;
  end for;
  G_n_fl = 0;
  C_u_eff[1,1] = S_p_eff;
  for i in 1:1:1,j in 1:1:999 loop
    C_u_eff[1,1] = C_u_eff[1,j+1];
  end for;
  C_u_fl[1,1]+sum(C_u_fl[1:1:1, 2:1:1000])+S_p_fl = 0;
  C_d_eff[1000,1] = G_p_eff;
  C_d_eff[1000,1] = S_n_eff;
  for i in 1:1:1,j in 1:1:999 loop
    C_d_eff[1000,1] = C_d_eff[1000,j+1];
  end for;
  C_d_fl[1000,1]+sum(C_d_fl[1000:1:1000, 2:1:1000])+G_p_fl+S_n_fl = 0;
  for i in 1:1:1000,j in 1:1:1 loop
    C_l_eff[i,1] = C_r_eff[i,1000];
  end for;
  for i in 1:1:1000,j in 1:1:1 loop
    C_l_fl[i,1]+C_r_fl[i,1000] = 0;
  end for;
end SBGraph3;
