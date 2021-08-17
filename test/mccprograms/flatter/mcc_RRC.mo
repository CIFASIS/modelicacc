model RRC
  constant Integer N=100;
  Real G_p_v;
  Real G_p_i;
  parameter Real R_R[N](each start=1);
  parameter Real R_T_ref[N]=fill(300.15,N);
  parameter Real R_alpha[N]=fill(0,N);
  Real R_v[N];
  Real R_i[N];
  Real R_p_v[N];
  Real R_p_i[N];
  Real R_n_v[N];
  Real R_n_i[N];
  Real R_LossPower[N];
  Real R_T_heatPort[N](each start=288.15);
  Real R_R_actual[N];
  Real C_v[N](each start=0);
  Real C_i[N];
  Real C_p_v[N];
  Real C_p_i[N];
  Real C_n_v[N];
  Real C_n_i[N];
  parameter Real C_C[N](each start=1);
  parameter Real S_V(start=1);
  Real S_v;
  Real S_i;
  Real S_p_v;
  Real S_p_i;
  Real S_n_v;
  Real S_n_i;
equation
  G_p_v = 0;
  for _Index_0 in 1:N loop
    R_R_actual[_Index_0] = R_R[_Index_0]*(1+R_alpha[_Index_0]*(R_T_heatPort[_Index_0]-R_T_ref[_Index_0]));
    R_v[_Index_0] = R_R_actual[_Index_0]*R_i[_Index_0];
    R_LossPower[_Index_0] = R_v[_Index_0]*R_i[_Index_0];
    R_T_heatPort[_Index_0] = R_T_ref[_Index_0];
    R_v[_Index_0] = R_p_v[_Index_0]-R_n_v[_Index_0];
    0 = R_p_i[_Index_0]+R_n_i[_Index_0];
    R_i[_Index_0] = R_p_i[_Index_0];
  end for;
  for _Index_1 in 1:N loop
    C_i[_Index_1] = C_C[_Index_1]*der(C_v[_Index_1]);
    C_v[_Index_1] = C_p_v[_Index_1]-C_n_v[_Index_1];
    0 = C_p_i[_Index_1]+C_n_i[_Index_1];
    C_i[_Index_1] = C_p_i[_Index_1];
  end for;
  S_v = S_V;
  S_v = S_p_v-S_n_v;
  0 = S_p_i+S_n_i;
  S_i = S_p_i;
  C_n_v[1] = G_p_v;
  C_n_v[1] = C_n_v[2];
  C_n_v[1] = S_n_v;
  C_n_v[1] = C_n_v[100];
  for i in 1:1:97 loop
    C_n_v[1] = C_n_v[i+2];
  end for;
  C_n_i[1]+G_p_i+sum(C_n_i[3:1:99])+C_n_i[2]+S_n_i+C_n_i[100] = 0;
  for i in 1:1:99 loop
    C_p_i[i]+R_n_i[i]+R_p_i[i+1] = 0;
  end for;
  C_p_i[100]+R_n_i[100] = 0;
  R_p_v[1] = S_p_v;
  R_p_i[1]+S_p_i = 0;
  C_p_v[100] = R_n_v[100];
  for i in 1:1:99 loop
    C_p_v[i] = R_n_v[i];
    C_p_v[i] = R_p_v[i+1];
  end for;
end RRC;
