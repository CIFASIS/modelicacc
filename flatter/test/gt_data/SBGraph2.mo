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
  for i in 1:1:999 loop
    C_p_fl[i]+R_n_fl[i]+R_p_fl[i+1] = 0;
  end for;
  C_p_fl[1000]+R_n_fl[1000] = 0;
  C_n_eff[1] = S_n_eff;
  C_n_eff[1] = G_p_eff;
  C_n_eff[1] = C_n_eff[1000];
  for i in 1:1:998 loop
    C_n_eff[1] = C_n_eff[i+1];
  end for;
  C_n_fl[1]+S_n_fl+G_p_fl+C_n_fl[1000]+sum(C_n_fl[2:1:999]) = 0;
  R_p_eff[1] = S_p_eff;
  R_p_fl[1]+S_p_fl = 0;
  G_n_fl = 0;
  C_p_eff[1000] = R_n_eff[1000];
  for i in 1:1:999 loop
    C_p_eff[i] = R_n_eff[i];
    C_p_eff[i] = R_p_eff[i+1];
  end for;
end SBGraph2;
