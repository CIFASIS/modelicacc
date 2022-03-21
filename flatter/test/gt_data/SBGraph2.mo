model SBGraph2
  Real N=1000;
  Real S_n_eff;
  Real S_n_fl;
  Real S_p_eff;
  Real S_p_fl;
  Real R_n_eff[N];
  Real R_n_fl[N];
  Real R_p_eff[N];
  Real R_p_fl[N];
  Real G_n_eff;
  Real G_n_fl;
  Real G_p_eff;
  Real G_p_fl;
  Real C_n_eff[N];
  Real C_n_fl[N];
  Real C_p_eff[N];
  Real C_p_fl[N];
equation
  G_n_fl = 0;
  C_n_eff[1] = S_n_eff;
  C_n_eff[1] = G_p_eff;
  for i in 1:1:999 loop
    C_n_eff[1] = C_n_eff[i+1];
  end for;
  C_n_fl[1]+S_n_fl+G_p_fl+sum(C_n_fl[2:1:1000]) = 0;
  R_p_eff[1] = S_p_eff;
  R_p_fl[1]+S_p_fl = 0;
  C_p_eff[1000] = R_n_eff[1000];
  C_p_fl[1000]+R_n_fl[1000] = 0;
  for i in 1:1:999 loop
    C_p_eff[i] = R_p_eff[i+1];
    C_p_eff[i] = R_n_eff[i];
  end for;
  for i in 1:1:999 loop
    C_p_fl[i]+R_p_fl[i+1]+R_n_fl[i] = 0;
  end for;
end SBGraph2;
