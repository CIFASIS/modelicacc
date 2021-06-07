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
  for i in 1:1:1 loop
    G_p_eff[i] = S_n_eff[i];
  end for;
  for i in 1:1:1 loop
    C_n_eff[i] = S_n_eff[i];
  end for;
  for i in 1000:1:1000 loop
    C_n_eff[i] = S_n_eff[i-999];
  end for;
  for i in 2:1:999 loop
    C_n_eff[i] = S_n_eff[1];
  end for;
  for i in 1:1:1 loop
    S_n_fl[i]+sum(C_n_fl[2:1:999])+C_n_fl[i+999]+C_n_fl[i]+G_p_fl[i] = 0;
  end for;
  for i in 1000:1:1000 loop
    C_p_eff[i] = R_n_eff[i];
  end for;
  for i in 1000:1:1000 loop
    C_p_fl[i]+R_n_fl[i] = 0;
  end for;
  for i in 1:1:999 loop
    C_p_eff[i] = R_p_eff[i+1];
  end for;
  for i in 1:1:999 loop
    R_n_eff[i] = R_p_eff[i+1];
  end for;
  for i in 2:1:1000 loop
    R_n_fl[i-1]+R_p_fl[i]+C_p_fl[i-1] = 0;
  end for;
  for i in 1:1:1 loop
    R_p_eff[i] = S_p_eff[i];
  end for;
  for i in 1:1:1 loop
    S_p_fl[i]+R_p_fl[i] = 0;
  end for;
end SBGraph2;
