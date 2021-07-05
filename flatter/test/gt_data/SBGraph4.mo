model SBGraph4
  Real N=100000;
  Real R_n_eff[2*N];
  Real R_n_fl[2*N];
  Real R_p_eff[2*N];
  Real R_p_fl[2*N];
  Real C1_n_eff[N];
  Real C1_n_fl[N];
  Real C1_p_eff[N];
  Real C1_p_fl[N];
  Real C2_n_eff[N];
  Real C2_n_fl[N];
  Real C2_p_eff[N];
  Real C2_p_fl[N];
equation
  for i in 1:1:100000 loop
    C1_n_fl[i]+R_n_fl[2*i] = 0;
  end for;
  for i in 1:1:100000 loop
    C1_p_fl[i]+R_p_fl[2*i] = 0;
  end for;
  for i in 1:1:100000 loop
    C2_n_fl[i]+R_n_fl[2*i-1] = 0;
  end for;
  for i in 1:1:100000 loop
    C2_p_fl[i]+R_p_fl[2*i-1] = 0;
  end for;
  for i in 1:1:100000 loop
    C2_p_eff[i] = R_p_eff[2*i-1];
  end for;
  for i in 1:1:100000 loop
    C2_n_eff[i] = R_n_eff[2*i-1];
  end for;
  for i in 1:1:100000 loop
    C1_p_eff[i] = R_p_eff[2*i];
  end for;
  for i in 1:1:100000 loop
    C1_n_eff[i] = R_n_eff[2*i];
  end for;
end SBGraph4;
