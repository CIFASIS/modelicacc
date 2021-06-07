model SBGraph3
  Real N=1000;
  Real M=1000;
  Real S_n_eff[1, 1];
  Real S_n_fl[1, 1];
  Real S_p_eff[1, 1];
  Real S_p_fl[1, 1];
  Real G_n_eff[1, 1];
  Real G_n_fl[1, 1];
  Real G_p_eff[1, 1];
  Real G_p_fl[1, 1];
  Real C_u_eff[N, M];
  Real C_u_fl[N, M];
  Real C_d_eff[N, M];
  Real C_d_fl[N, M];
  Real C_l_eff[N, M];
  Real C_l_fl[N, M];
  Real C_r_eff[N, M];
  Real C_r_fl[N, M];
equation
  for i in 1:1:1,j in 1:1:1 loop
    G_p_eff = S_n_eff;
  end for;
  for i in 1000:1:1000,j in 1:1:1000 loop
    C_d_eff[i,j] = S_n_eff;
  end for;
  for i in 1:1:1,j in 1:1:1 loop
    S_n_fl+sum(C_d_fl[i+999, 1:1:1000])+G_p_fl = 0;
  end for;
  for i in 1:1:1000,j in 1:1:999 loop
    C_r_eff[i,j] = C_l_eff[i,j+1];
  end for;
  for i in 1:1:1000,j in 2:1:1000 loop
    C_r_fl[i,j-1]+C_l_fl[i,j] = 0;
  end for;
  for i in 1:1:1,j in 1:1:1000 loop
    C_u_eff[i,j] = S_p_eff;
  end for;
  for i in 1:1:1,j in 1:1:1 loop
    sum(C_u_fl[i, 1:1:1000])+S_p_fl = 0;
  end for;
  for i in 1:1:999,j in 1:1:1000 loop
    C_d_eff[i,j] = C_u_eff[i+1,j];
  end for;
  for i in 2:1:1000,j in 1:1:1000 loop
    C_d_fl[i-1,j]+C_u_fl[i,j] = 0;
  end for;
  for i in 1:1:1000,j in 1000:1:1000 loop
    C_r_eff[i,j] = C_l_eff[i,j-999];
  end for;
  for i in 1:1:1000,j in 1:1:1 loop
    C_r_fl[i,j+999]+C_l_fl[i,j] = 0;
  end for;
end SBGraph3;
